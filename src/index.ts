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
    static HIGH: High;
    static LOW: Low;
    private _gpio: Number;
    private _chip: any;
    private _line: any;
    private _direction: Direction;

    private static detectChip(): any {
        let chip: any = null;

        readdirSync('/dev').forEach((file) => {
            if (file.match(/gpiochip[0-9]*/)) {
                const currentChip = gpiod.openChip('/dev/' + file);
                const label = gpiod.getChipLabel(currentChip);
                console.log(`Checking chip: ${label}`);
                const n_lines = gpiod.getNumLines(currentChip);
                if(label.match(/pinctrl-bcm.*/)) {
                    chip = currentChip;
                }
            }
        });

        return chip;
    }

    constructor(gpio: Number, direction: Direction, edge: Edge, options: Options) {
        this._gpio = gpio;
        this._chip = Gpio.detectChip();
        this._line = gpiod.chipGetLine(this._chip, this._gpio);
        this._direction = direction;
        this.setDirection(this._direction);
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
        console.log(`High is ${Gpio.HIGH}`);
        console.log(`Low is ${Gpio.LOW}`);
        if(direction === 'out') {
            console.log('Setting line to out');
            let status = gpiod.lineRequestOutput(this._line, Gpio.LOW);
            console.log(status);
        } else if(direction === 'in') {
            console.log('Setting line to in');
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
        console.log(`AL state ${state}`);
        return state !== 1;
    }

    public setActiveLow(): void {
        gpiod.lineSetFlags(this._line, 2);
    }

}