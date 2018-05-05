'use strict';

/* exported Gamepad */

/** Gamepad device */
class Gamepad {
    /** Create a Gamepad
     * @param {Gigatron} cpu - The cpu to control
     * @param {Object.<number,number>} codes - Key codes for controller buttons
     */
    constructor(cpu, codes) {
        this.cpu = cpu;
        this.bits = {
            [codes.right]: 0x01,
            [codes.left]: 0x02,
            [codes.down]: 0x04,
            [codes.up]: 0x08,
            [codes.start]: 0x10,
            [codes.select]: 0x20,
            [codes.b]: 0x40,
            [codes.a]: 0x80,
        };

        document.addEventListener('keydown', (event) => {
            let bit = this.bits[event.key];
            if (bit) {
                this.cpu.inReg &= ~bit;
                event.preventDefault();
            }
        });

        document.addEventListener('keyup', (event) => {
            let bit = this.bits[event.key];
            if (bit) {
                this.cpu.inReg |= bit;
                event.preventDefault();
            }
        });
    }
}
