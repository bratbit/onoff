import { parentPort, workerData } from "worker_threads";
import bindings from 'bindings';
import { dn } from './dirname.js';
const gpiod = bindings({bindings: 'gpiod-wrap', module_root: `${dn}/../..`});

function watchForEvent() {
    let line = workerData;
    while(true) {
        let status = gpiod.waitForEvent(line);
        if(status < 0) {
            throw('Interrupt watcher failed');
        } else if(status === 1) {
            parentPort?.postMessage(status);
        }
    }
}

watchForEvent();