/**
 * @typedef {function} Slot
 */
// ot {
//     /**
//      * @param {Antenna} antenna 
//      * @param {function} callback 
//      */
//     constructor(antenna, callback) {
//         this._antenna = antenna;
//         this._callback = callback;
//     }

//     fire(...args) {
        
//     }
// }


class Signal {
    constructor() {
        /** @type {Slot[]} */
        this._slots = [];
    }

    /** @param {Slot} slot */
    connect(slot) {
        this._slots.push(slot);
    }

    /** @param {Slot} slot */
    disconnect(slot) {
        const i = this._slots.indexOf(slot);
        
        if (i >= this._slots.length)
            throw "Slot disconnected from signal it was not connected to";

        this._slots.splice(i, 1);
    }

    /** @param {...*} args */
    fire(...args) {
        for (let slot of this._slots) {
            slot(...args);
        }
    }
}

/** @type {Map<string, Signal>} */
let signals = new Map();

/** @type {Map<string, Signal[]>} */
let pending_slots = new Map();

/** @param {string} name */
function registerSignal(name) {
    if (signals.has(name))
        throw `Global signal ${name} already registered`;

    let signal = new Signal();
    signals.set(name, signal);

    if (pending_slots.has(name)) {
        pending_slots.get(name).forEach(slot => signal.connect(slot));
        pending_slots.delete(name);
    }
}

/** 
 * @param {string} signal_name
 * @param {...*} args
*/
function fireSignal(signal_name, ...args) {
    getSignal(signal_name).fire(...args);
}

/** @param {string} name */
function getSignal(name) {
    let signal = signals.get(name);

    if (signal === undefined)
        throw `Attempt to get nonexistent signal ${name}`;

    return signal;
}

/** @param {string} name */
/** @param {} */

class Antenna extends HTMLElement {
    constructor() {
        super();
    }

    connectedCallback() {
        const signal_name = this.getAttribute("data-signal");

        if (signal_name == null || typeof signal_name != "string" || signal_name.length == 0)
            throw "SignalReceiver element requires a string of non-zero length for data-signal";

        const callback_name = this.getAttribute("data-callback");

        if (callback_name == null || typeof callback_name != "string" || callback_name.length == 0)
                throw "SignalReceiver element requires a string of non-zero length for data-callback";
        
        let callback = window[callback_name];

        if (callback === undefined || typeof callback != "function")
            throw `SignalReceiver data-callback ${callback_name} must exist as a function in window`;

        // Since slots may be added before the signals are globally registered, then 
        // save slots until the signal is registered.
        if (signals.has(signal_name)) {
            signals.get(signal_name).connect(callback.bind(this));
        } else {
            if (pending_slots.has(signal_name)) {
                pending_slots.get(signal_name).push(callback.bind(this));
            } else {
                pending_slots.set(signal_name, [callback.bind(this)]);
            }
        }
    }
}