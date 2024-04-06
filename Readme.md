# @bratbit/onoff
GPIO control and interrupt detection for Raspberry Pi

## Synopsis
@bratbit/onoff is a GPIO control and interrupt detection package for nodejs intended for use with Raspberry Pi.

The package strives to offer an API compatible to that of [fivdi/onoff](https://www.npmjs.com/package/onoff), while using native bindings.

## Instalation

### Dependencies
Rapberry Pi OS.
```
apt install build-essential libgpiod2 libgpiod-dev
```
### Package installation
Add the package to your project.
```
npm add @bratbit/onoff
```
Compile the bindings and get your project ready to use.
```
npm install
```

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