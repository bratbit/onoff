# @bratbit/onoff
GPIO control and interrupt detection for Raspberry Pi

## Synopsis
@bratbit/onoff is a GPIO control and interrupt detection package for nodejs intended for use with Raspberry Pi.

The package strives to offer an API compatible to that of [fivdi/onoff](https://www.npmjs.com/package/onoff), while using native bindings.

## Instalation

### Dependencies

#### Raspberry Pi OS
```
apt install build-essential libgpiod2 libgpiod-dev
```

#### Arch Linux
```
pacman -Syq base-build libgpiod
```

#### Alpine Linux
```
apk add build-base python3 libgpiod libgpiod-dev
```

#### Fedora Linux
```
dnf install gcc make pkg-config python3 libgpiod libgpiod-devel
```

### Package installation
Add the package to your project.
```
npm add @bratbit/onoff
```

## API Reference

### Types
```
export type High = 1;
```
```
type Low = 0;
```
```
type Direction = "in" | "out" | "high" | "low";
```
```
type Edge = "none" | "rising" | "falling" | "both";
```
```
type Options = {
    debounceTimeout?: number;
    activeLow?: boolean;
    reconfigureDirection?: boolean;
};
```
```
type ValueCallback = (err: Error | null | undefined, value: BinaryValue) => void;
```
```
type BinaryValue = High | Low;
```
### Class Gpio
- [constructor(gpio: Number, direction: Direction)](#gpiogpio-number-direction-directiongpiogpio-number-direction-direction-edge-edgegpiogpio-number-direction-direction-options-optionsgpiogpio-number-direction-direction-edge-edge-options-options)
- [constructor(gpio: Number, direction: Direction, edge: Edge)](#gpiogpio-number-direction-directiongpiogpio-number-direction-direction-edge-edgegpiogpio-number-direction-direction-options-optionsgpiogpio-number-direction-direction-edge-edge-options-options)
- [constructor(gpio: Number, direction: Direction, options: Options)](#gpiogpio-number-direction-directiongpiogpio-number-direction-direction-edge-edgegpiogpio-number-direction-direction-options-optionsgpiogpio-number-direction-direction-edge-edge-options-options)
- [constructor(gpio: Number, direction: Direction, edge: Edge, options: Options)](#gpiogpio-number-direction-directiongpiogpio-number-direction-direction-edge-edgegpiogpio-number-direction-direction-options-optionsgpiogpio-number-direction-direction-edge-edge-options-options)
- [readSync(): BinaryValue](#readsync-binaryvalue)
- [read(): Promise<BinaryValue>](#read-promisebinaryvalue)
- [read(callback: ValueCallback): void](#readcallback-valuecallback-void)
- [writeSync(value: BinaryValue): number](#writesyncvalue-binaryvalue-number)
- [write(value: BinaryValue): Promise<void>](#writevalue-binaryvalue-promisevoid)
- [write(value: BinaryValue, callback: (err?: Error | null | undefined) => void): void](#writevalue-binaryvalue-callback-err-error--null--undefined--void-void)
- [direction(): Direction](#direction-direction)
- [setDirection(direction: Direction): void](#setdirectiondirection-direction-void)
- [edge(): Edge](#edge-edge)
- [setEdge(edge: Edge): void](#setedgeedge-edge-void)
- [activeLow(): boolean](#activelow-boolean)
- [setActiveLow(invert: boolean): void](#setactivelowinvert-boolean-void)
- [watch(callback: ValueCallback): void](#watchcallback-valuecallback-void)
- [unwatch(callback: ValueCallback): void](#unwatchcallback-valuecallback-void)
- [unwatchAll(): void](#unwatchall-void)

#### Gpio(gpio: Number, direction: Direction)</br>Gpio(gpio: Number, direction: Direction, edge: Edge)</br>Gpio(gpio: Number, direction: Direction, options: Options)</br>Gpio(gpio: Number, direction: Direction, edge: Edge, options: Options)
>Gpio class constructor

@param: gpio  
Number of the Gpio pin to use.

@param: direction  
Direction of the line. This can be a string set to:  
- `in`
Configure the line as input.
- `out`
Configure the line as output.
- `high`
Configure the line as output with the initial value set to logical high.
- `low`
Configure the line as output with the initial value set to logical low.

@param: edge (optional)  
Configure the line for edge detection. This can be a string set to:  
- `none`
Do not listen for edge events. This is the default if not configured.
- `rising`
Listen for rising edge events.
- `falling`
Listen for falling edge events.
- `both`
Listen for both rising and falling edge events.

@param: options (optional)  
Object containing the following keys:
`activeLow`
If set to `true`, configure the line to have its logical state inversed from the physical state. Default is `false`.
`debounceTimeout`
When listening to events, they will be debounced for the ammount of milliseconds specified here. Default is `0`.
`reconfigureDirection`
If set to `true` the line will be opened *as-is* without reconfiguring it. Default is `false`.

#### readSync(): BinaryValue
>Read the line synchronously
>
>@return: BinaryValue indicating the logical state of the line.

#### read(): Promise\<BinaryValue\>
>Read the line and return a Promise resolving to the logical state
>
>@return: A promise, that will resolve to a BinaryValue indicating the logical state of the line.

#### read(callback: ValueCallback): void
>Read the line and call the `callback` function passing an error, and value parameters. Value represents the logical state of the line.
>
>@param: callback  
>callback is a function of type ValueCallback, that will be called when the value of the line is read.

#### writeSync(value: BinaryValue): number
>Set the logical line state to `value` synchronously.
>
>@parmam: value  
>Set the logical line state to `value`.

#### write(value: BinaryValue): Promise\<void\>
>Set the logical line state to `value` and resolve the Promise when done.
>
>@param: value  
>Set the logical line state to `value`.

#### write(value: BinaryValue, callback: (err?: Error | null | undefined) => void): void
>Set the logical line state to `value` and trigger a `callback` when done.
>
>@param: value  
>Set the logical line state to `value`.

#### direction(): Direction
>Get the direction of the line.
>
>@return: Direction  
>Return the direction setting of the line.

#### setDirection(direction: Direction): void
>Set the direciton of the line.
>
>@param: direction  
>Set the direction of the line.

#### edge(): Edge
>Get the edge setting of the Gpio object.
>
>@return: edge

#### setEdge(edge: Edge): void
>Set the edge detection of the line. The line must be set to `in` for this to take any effect.
>
>@param: edge

#### activeLow(): boolean
>Get value of activeLow option of the Gpio object. This will tell you if the line is set to invert logical state.
>
>@return: boolean  
>`true` if the logical state is inverted.

#### setActiveLow(invert: boolean): void
>Set the activeLow option of the Gpio object. This will make the line values invert the physical value.
>
>@param: invert  
>Set this to `true` if you want the line state to be inverted.

#### watch(callback: ValueCallback): void
>Add a `callback` function to the set of callbacks that will be triggered when an edge event is detected. The `edge` option must not be `none` and the `direction` needs to be `in`, in order to listen to edge events.
>
>@param: callback  
>Function to trigger when an edge event is detected.

#### unwatch(callback: ValueCallback): void
>Remove a function from the list of those that will be triggered when an edge event is detected.
>
>@param: callback  
>Function to remove from the list of listeners.

#### unwatchAll(): void
>Remove all listeners and stop listening to edge events.

## Usage examples

### Reading
Read the value of GPIO pin.

#### Typescript
```ts
import { Gpio } from '@bratbit/onoff';

let gpio12: Gpio = new Gpio(12, 'in');

console.log(`Pin value is ${gpio12.readSync()}`);
```

#### ECMAScript
```js
import { Gpio } from '@bratbit/onoff';

let gpio12 = new Gpio(12, 'in');

console.log(`Pin value is ${gpio12.readSync()}`);
```

#### CommonJS
```js
const { Gpio } = require('@bratbit/onoff');

let gpio12 = new Gpio(12, 'in');

console.log(`Pin value is ${gpio12.readSync()}`);
```

### Writing
Write a one second pulse to GPIO pin.

#### Typescript
```ts
import { Gpio } from '@bratbit/onoff';
import { timer } from 'rxjs';

let gpio13: Gpio = new Gpio(13, 'out');

gpio13.writeSync(Gpio.HIGH);
timer(1000).subscribe(() => {
    gpio13.writeSync(Gpio.LOW)
});
```

#### ECMAScript
```js
import { Gpio } from '@bratbit/onoff';
import { timer } from 'rxjs';

let gpio13 = new Gpio(13, 'out');

gpio13.writeSync(Gpio.HIGH);
timer(1000).subscribe(() => {
    gpio13.writeSync(Gpio.LOW)
});
```

#### CommonJS
```js
const { Gpio } = require('@bratbit/onoff');
const { timer } = require('rxjs');

let gpio13 = new Gpio(13, 'out');

gpio13.writeSync(Gpio.HIGH);
timer(1000).subscribe(() => {
    gpio13.writeSync(Gpio.LOW)
});
```

### Watching for interrupts

Listen for interrupts for 30 seconds and print the line value, when an interrupt occurs.

#### Typescript
```ts
import { Gpio, BinaryValue } from '@bratbit/onoff';
import { timer } from 'rxjs';

let gpio12: Gpio = new Gpio(12, 'in', 'both', {debounceTimeout: 300});

function handler (err: Error | null | undefined, value: BinaryValue): void {
    console.log(`Interrupt! Line value is ${value}`);
}

gpio12.watch(handler);

timer(30000).subscribe(() => {
    gpio12.unwatch(handler);
});
```

#### ECMAScript
```js
import { Gpio } from '@bratbit/onoff';
import { timer } from 'rxjs';

let gpio12 = new Gpio(12, 'in', 'both', {debounceTimeout: 300});

function handler (err, value) {
    console.log(`Interrupt! Line value is ${value}`);
}

gpio12.watch(handler);

timer(30000).subscribe(() => {
    gpio12.unwatch(handler);
});
```

#### CommonJS
```js
const { Gpio } = require('@bratbit/onoff');
const { timer } = require('rxjs');

let gpio12 = new Gpio(12, 'in', 'both', {debounceTimeout: 300});

function handler (err, value) {
    console.log(`Interrupt! Line value is ${value}`);
}

gpio12.watch(handler);

timer(30000).subscribe(() => {
    gpio12.unwatch(handler);
});
```