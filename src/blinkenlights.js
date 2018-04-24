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

		ctx.save();
		ctx.translate(w, this.canvas.height/2);
		for (let i = 0; i < 4; i++) {
			ctx.fillStyle = (this.outx & (1 << i)) ? '#ff0000' : '#640000';
			ctx.strokeStyle = '#ff0000';
			ctx.lineWidth = 1;
			ctx.beginPath();
			ctx.arc(0, 0, r, 0, 2*Math.PI);
			ctx.closePath();
			ctx.fill();
			ctx.stroke();

			ctx.strokeStyle = '#888';
			ctx.lineWidth = 1;
			ctx.beginPath();
			ctx.arc(0, 0, r-2, -Math.PI/2, 0);
			ctx.stroke();

			ctx.strokeStyle = '#ccc';
			ctx.lineWidth = 1;
			ctx.beginPath();
			ctx.arc(0, 0, r-2, Math.PI/2, 2*Math.PI/2);
			ctx.stroke();

			ctx.translate(2*w, 0);
		}
		ctx.restore();
	}
}
