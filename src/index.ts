import bindings from 'bindings';
import { readdirSync } from 'fs';
import {dn} from './dirname.js';
const gpiod = bindings({bindings: 'gpiod-wrap', module_root: `${dn}/../..`});

export type High = 1;
export type Low = 0;
export type Direction = "in" | "out" | "high" | "low";
export type Edge = "none" | "rising" | "falling" | "both";

export type Options = {
    debounceTimeout?: number,
    activeLow?: boolean,
    reconfigureDirection?: boolean,
}

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

    private static detectChip(): any {
        let chip: any = null;

        readdirSync('/dev').forEach((file) => {
            if (file.match(/gpiochip[0-9]*/)) {
                const currentChip = gpiod.openChip('/dev/' + file);
                const label = gpiod.getChipLabel(currentChip);
                const n_lines = gpiod.getNumLines(currentChip);
                if(label.match(/pinctrl-bcm.*/)) {
                    chip = currentChip;
                }
            }
        });

        return chip;
    }

    constructor(gpio: Number, direction: Direction);
    constructor(gpio: Number, direction: Direction, options: Options);
    constructor(gpio: Number, direction: Direction, edge: Edge, options: Options);
    constructor(gpio: Number, direction: Direction, arg2?: Edge | Options, arg3?: Options | undefined) {
        this._gpio = gpio;
        this._chip = Gpio.detectChip();
        this._line = gpiod.chipGetLine(this._chip, this._gpio);
        this._direction = direction;
        
        this.setDirection(this._direction);

        if(typeof arg2 === 'object') {
            this._options = arg2;
        } else if (typeof arg3 === 'object'){
            this._options = arg3;
        } else {
            this._options = {};
        }
        this.setOptions(this._options);

        if(typeof arg2 === 'string') {
            this._edge = arg2;
        } else {
            this._edge = 'none';
        }

        console.log(this._chip);
        console.log(this._line);
    }

    public readSync(): BinaryValue {
        return gpiod.lineGetValue(this._line);
    }

    public writeSync(value: BinaryValue): Number {
        return gpiod.lineSetValue(this._line, value);
    }

    public direction(): Direction {
        return this._direction;
    }

    public setDirection(direction: Direction): void {
        if(direction === 'out') {
            let status = gpiod.lineRequestOutput(this._line, Gpio.LOW);
            console.log(status);
        } else if(direction === 'in') {
            let status = gpiod.lineRequestInput(this._line);
            console.log(status);           
        } else if(direction === 'high') {
            let status = gpiod.lineRequestOutput(this._line, Gpio.HIGH);
        } else if(direction === 'low') {
            let status = gpiod.lineRequestOutput(this._line, Gpio.LOW);
        }
    }

    public activeLow(): Boolean {
        let state = gpiod.lineActiveState(this._line);
        return state !== 1;
    }

    public setActiveLow(activeLow: boolean): void {
        if(activeLow) {
            gpiod.lineSetFlags(this._line, 2);
        } else {
            gpiod.lineSetFlags(this._line, 0);
        }
    }

    private setOptions(options: Options) {
        if('activeLow' in options) {
            if(options.activeLow === true) {
                this.setActiveLow(true);
            } else {
                this.setActiveLow(false);
            }
        }
    }

}