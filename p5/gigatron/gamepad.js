'use strict';

/* exported Gamepad */

/** Gamepad device */
class Gamepad {
	/** Create a Gamepad
	 * @param {Gigatron} cpu - The cpu to control
	 * @param {Object} codes - Key codes for controller buttons
	 */
	constructor(cpu, codes) {
		this.cpu = cpu;
		let bits = {};
		bits[codes.right] = 0x01;
		bits[codes.left] = 0x02;
		bits[codes.down] = 0x04;
		bits[codes.up] = 0x08;
		bits[codes.start] = 0x10;
		bits[codes.select] = 0x20;
		bits[codes.b] = 0x40;
		bits[codes.a] = 0x80;
		this.bits = bits;
	}

	/** called when key is pressed
	 * @param {number} code - key code of pressed key
	 * @return {boolean} whether to default process event
	 */
	keyPressed(code) {
		let bit = this.bits[code];
		if (bit) {
			this.cpu.inReg &= ~bit;
			return false;
		}
		return true;
	}

	/** called when key is released
	 * @param {number} code - key code of released key
	 * @return {boolean} whether to default process event
	 */
	keyReleased(code) {
		let bit = this.bits[code];
		if (bit) {
			this.cpu.inReg |= bit;
			return false;
		}
		return true;
	}
}
