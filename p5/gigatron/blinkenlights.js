'use strict';

/* exported BlinkenLights */

/** Blinkenlights on outx register */
class BlinkenLights {
	/** Create a new BlinkenLights
	 * @param {Gigatron} cpu
	*/
	constructor(cpu) {
		this.cpu = cpu;
		this.outx = cpu.outx;
	}

	/** advance simulation by one tick */
	tick() {
		let outx = this.cpu.outx;

		if (this.outx != outx) {
			this.outx = outx;

			for (let i = 0; i < 4; i++) {
				fill((outx & (1 << i)) ? 255 : 100, 0, 0);
				noStroke();
				ellipse(320 + 32*(i-2), 490 + 17, 30, 30);
			}
		}
	}
}
