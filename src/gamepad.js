'use strict';

/* exported Gamepad */

/** Gamepad device */
class Gamepad {
    /** Create a Gamepad
     * @param {Gigatron} cpu - The cpu to control
     * @param {Object.<string,string[]>} keys - Keys codes for
     * controller buttons
     */
    constructor(cpu, keys) {
        const keybits = {
            a: 0x80,
            b: 0x40,
            select: 0x20,
            start: 0x10,
            up: 0x08,
            down: 0x04,
            left: 0x02,
            right: 0x01,
        };

        this.cpu = cpu;
        this.bits = {};

        for (let button of Object.keys(keybits)) {
            let bit = keybits[button];
            for (let key of keys[button]) {
                this.bits[key] = bit;
            }
        }
    }

    /** start handling key events */
    start() {
        document.addEventListener('keydown', (event) => {
            let bit = this.bits[event.key];
            if (bit) {
                this.cpu.inReg &= ~bit;
                event.preventDefault();
            }
        });
    }

    /** stop handling key events */
    stop() {
        document.addEventListener('keyup', (event) => {
            let bit = this.bits[event.key];
            if (bit) {
                this.cpu.inReg |= bit;
                event.preventDefault();
            }
        });
    }
}
