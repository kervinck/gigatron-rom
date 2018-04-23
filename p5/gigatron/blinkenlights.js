'use strict';

/* exported BlinkenLights */

/** Blinkenlights on outx register */
class BlinkenLights {
	/** Create a new BlinkenLights
	 * @param {Gigatron} cpu
	*/
	constructor(canvas, cpu) {
		this.canvas = canvas;
		this.ctx = canvas.getContext('2d');
		this.cpu = cpu;
		this.outx = cpu.outx;
		this.draw();
	}

	/** advance simulation by one tick */
	tick() {
		let outx = this.cpu.outx;

		if (this.outx != outx) {
			this.outx = outx;
			this.draw();
		}
	}

	draw() {
		const r = 8;
		const w = r+2;

		let ctx = this.ctx;

		ctx.strokeStyle = '#ff0000';
		ctx.lineWidth = 2;

		for (let i = 0; i < 4; i++) {
			ctx.fillStyle = (this.outx & (1 << i)) ? '#ff0000' : '#640000';
			ctx.beginPath();
			ctx.arc(this.canvas.width/2 + w + 2*w*(i-2), this.canvas.height/2, r, 0, 2*Math.PI);
			ctx.closePath();
			ctx.fill();
			ctx.stroke()
		}
	}
}
