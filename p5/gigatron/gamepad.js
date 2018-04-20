"use strict";

class Gamepad {
	constructor(cpu, codes) {
		this.cpu = cpu;
		let bits = {};
		bits[codes.right]  = 0x01;
		bits[codes.left]   = 0x02;
		bits[codes.down]   = 0x04;
		bits[codes.up]     = 0x08;
		bits[codes.start]  = 0x10;
		bits[codes.select] = 0x20;
		bits[codes.b]      = 0x40;
		bits[codes.a]      = 0x80;
		this.bits = bits;
	}

	keyPressed(code) {
		let bit = this.bits[code];
		if (bit) {
			this.cpu.inReg &= ~bit;
			return false;
		}
		return true;
	}

	keyReleased(code) {
		let bit = this.bits[code];
		if (bit) {
			this.cpu.inReg |= bit;
			return false;
		}
		return true;
	}
}
