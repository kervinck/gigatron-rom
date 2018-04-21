'use strict';

/* exported Audio */

/** Audio output */
class Audio {
	/**
	 * Create an Audio.
	 * @param {Gigatron} cpu - The CPU
	 */
	constructor(cpu) {
		this.cpu = cpu;
		this.cycle = 0;
		let context = getAudioContext();
		// eslint-disable-line no-unused-vars
		this.buffer = context.createBuffer(1, 521, 31260);
		this.source = context.createBufferSource();
		this.source.buffer = this.buffer;
		this.source.loop = true;
		this.source.loopStart = 0;
		this.source.loopEnd = this.buffer.duration;
		this.arrayBuffer = this.buffer.getChannelData(0);
		this.source.connect(context.destination);
		this.source.start();
		this.count = 0;
		this.sample = 0;
		this.popcount = [0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4];
	}

	/** advance simulation by one tick */
	tick() {
		this.count += this.popcount[this.cpu.outx >> 4];
		if (++this.cycle == 50) {
			this.cycle = 0;
			this.arrayBuffer[this.sample++] = ((this.count / 200) - 1) / 2;
			this.count = 0;
		}
		if (this.sample == this.buffer.length) {
			this.sample = 0;
		}
	}
}
