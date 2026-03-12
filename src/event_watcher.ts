import { parentPort, workerData } from "worker_threads";
import bindings from 'bindings';
import { dn } from './dirname.js';

const gpiod = bindings({ bindings: 'gpiod-wrap', module_root: `${dn}/../..` });

function watchForEvent() {
    const line = workerData;
    function loop() {
        setTimeout(() => {
            const status = gpiod.waitForEvent(line);
            if (status < 0) {
                throw new Error('Interrupt watcher failed');
            }
            if (status === 1) {
                const value = gpiod.getLineValue(line);
                parentPort?.postMessage(value);
            }
            loop();
        }, 0);
    }
    loop();
}

watchForEvent();