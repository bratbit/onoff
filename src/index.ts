import bindings from 'bindings';
import { Worker } from 'worker_threads';
import { dn } from './dirname.js';
const gpiod = bindings({bindings: 'gpiod-wrap', module_root: `${dn}/../..`});
import { fromEvent, debounceTime } from 'rxjs';

export type High = 1;
export type Low = 0;
export type Direction = "in" | "out" | "high" | "low";
export type Edge = "none" | "rising" | "falling" | "both";

export type Options = {
    debounceTimeout?: number,
    activeLow?: boolean,
    reconfigureDirection?: boolean,
}

export type ValueCallback = (err: Error | null | undefined, value: BinaryValue) => void;

export type BinaryValue = High | Low;

export class Gpio {
    static HIGH: High = 1;
    static LOW: Low = 0;
    private _gpio: Number;
    private _chip: any;
    private _line: any;
    private _direction: Direction;
    private _options: Options;
    private _edge: Edge;
    private _watchers: Array<ValueCallback>;
    private _worker: Worker | undefined;

    public detectChip(re: string = "pinctrl") {
        return gpiod.detectChip("pinctrl.*");
    }

    constructor(gpio: Number, direction: Direction);
    constructor(gpio: Number, direction: Direction, options: Options);
    constructor(gpio: Number, direction: Direction, edge: Edge, options: Options);
    constructor(gpio: Number, direction: Direction, arg2?: Edge | Options | undefined, arg3?: Options | undefined) {
        this._gpio = gpio;
        this._chip = this.detectChip();
        this._direction = direction;
        this._options = {};
        this._edge = 'none';
        this.configureOptions(arg2, arg3);
        this.configureEdge(arg2);
        this._line = gpiod.getLine(this._chip, this._gpio);
        gpiod.configureLine(this._line, this._gpio, this._direction, this._edge, this._options);
        this._watchers = [];
        this._worker = undefined;
        
        process.on('exit', () => {
            if((typeof this._worker) !== undefined) {
                this._worker?.terminate();
            }
        });
    }
    
    private configureOptions(arg2: Edge | Options | undefined = undefined, arg3: Options | undefined = undefined) {
        if(typeof arg2 === 'object') {
            this._options = arg2;
        } else if (typeof arg3 === 'object'){
            this._options = arg3;
        }
        if(!("activeLow" in this._options)) this._options.activeLow = false;
        if(!("debounceTimeout" in this._options)) this._options.debounceTimeout = 0;
        if(!("reconfigureDirection" in this._options)) this._options.reconfigureDirection = true;
    }
    
    private configureEdge(arg2: Edge | Options | undefined = undefined) {
        if(typeof arg2 === 'string') {
            this._edge = arg2;
        } else {
            this._edge = 'none';
        }
    }
    
    public readSync(): BinaryValue {
        return gpiod.getLineValue(this._line);
    }
    
    public read(): Promise<BinaryValue>;
    public read(callback: ValueCallback): void;
    public read(callback?: ValueCallback): Promise<BinaryValue> | void{
        let result = this.readSync();
        if(callback) {
            callback(null, result);
            return;
        } else {
            return new Promise((resolve) => {
                resolve(result);
                return;
            });
        }
    }
    
    public writeSync(value: BinaryValue): number {
        return gpiod.setLineValue(this._line, value);
    }
    
    public write(value: BinaryValue): Promise<void>;
    public write(value: BinaryValue, callback: (err?: Error | null | undefined) => void): void;
    public write(value: BinaryValue, callback?: (err?: Error | null | undefined) => void): Promise<void> | void {
        let result = this.writeSync(value);
        if(result < 0) {
            let err = new Error(`Failed to write to gpio ${this._gpio}`);
        }
        if(callback) {
            callback(null);
            return;
        } else {
            return new Promise((resolve, reject) => {
                if(result < 0) {
                    reject();
                } else {
                    resolve();
                }
                return;
            });
        }
    }
    
    public direction(): Direction {
        return this._direction;
    }
    
    public setDirection(direction: Direction): void {
        this._direction = direction;
        gpiod.configureLine(this._line, this._gpio, this._direction, this._edge, this._options);
    }
    
    public edge(): Edge {
        return this._edge;
    }
    
    public setEdge(edge: Edge): void {
        this._edge = edge;
        gpiod.configureLine(this._line, this._gpio, this._direction, this._edge, this._options);
    }
    
    public activeLow(): boolean {
        return (this._options.activeLow as boolean);
    }
    
    public setActiveLow(invert: boolean): void {
        this._options.activeLow = invert;
        gpiod.configureLine(this._line, this._gpio, this._direction, this._edge, this._options);
    }
    
    private startInterruptHandler(): void {
        if(!this._worker) {
            this._worker = new Worker(`${dn}/event_watcher.js`, {workerData: this._line});
            const interrupts = fromEvent(this._worker, 'message');
            interrupts.pipe(
                debounceTime(this._options.debounceTimeout as number)
            ).subscribe((value: any) => {
                this._watchers.forEach((watcher) => {
                    watcher(null, value);
                })
            });
        }
    }
    
    private stopInterruptHandler(): void {
        if(this._worker) {
            this._worker.terminate();
            this._worker = undefined;
        }
    }

    
    public watch(callback: ValueCallback): void {
        this._watchers.push(callback);
        if((this._edge !== 'none') && (this._direction === 'in')) {
            this.startInterruptHandler();
        }
    }

    public unwatch(callback: ValueCallback): void {
        let index = this._watchers.indexOf(callback);
        if(index >= 0) {
             this._watchers.splice(index, 1);
        }
        if(this._watchers.length == 0) {
            this.stopInterruptHandler();
        }
    }

    public unwatchAll(): void {
        this._watchers = [];
        this.stopInterruptHandler();
    }
    
}