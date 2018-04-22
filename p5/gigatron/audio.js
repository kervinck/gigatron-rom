'use strict';

/* exported Audio */

const HZ = 6250000;
const SAMPLES_PER_SECOND = 44100;

/** Audio output */
class Audio {
	/**
	 * Create an Audio.
	 * @param {Gigatron} cpu - The CPU
	 * @param {Element} schedText
	 */
	constructor(cpu, schedText) {
		this.cpu = cpu;
		this.schedText = schedText;
		let context = this.context = getAudioContext();
		this.mute = false;
		this.cycle = 0;
		this.scheduled = 0;
		this.tailTime = 0; // time at which tail buffer will start
		this.headTime = 0; // time at which head buffer will end

		this.buffers = [];
		for (let i = 0; i < 4; i++) {
			let buffer = context.createBuffer(1, int(SAMPLES_PER_SECOND/100), SAMPLES_PER_SECOND);
			this.buffers.push(buffer);
		}

		this.headBufferIndex = 0;
		this.tailBufferIndex = 0;
		this.channelData = this.buffers[this.tailBufferIndex].getChannelData(0);
		this.sampleIndex = 0;
	}

	/** drain completed head buffers */
	drain() {
		let currentTime = this.context.currentTime;
		let headTime = this.headTime;
		let headBufferIndex = this.headBufferIndex;
		let scheduled = this.scheduled;

		while (scheduled > 0 && headTime < currentTime) {
			if (++headBufferIndex == this.buffers.length) {
				headBufferIndex = 0;
			}
			headTime += this.buffers[headBufferIndex].duration;
			scheduled--;
		}

		this.headTime = headTime;
		this.scheduled = scheduled;
	}

	/** flush current tail buffer */
	flushChannelData() {
		let context = this.context;
		let currentTime = context.currentTime;
		let tailBufferIndex = this.tailBufferIndex;
		let buffer = this.buffers[tailBufferIndex];

		/* if the tail can't keep up with realtime, jump it to now */
		if (this.tailTime < currentTime) {
			console.log('audio skip');
			this.tailTime = currentTime;
			this.headTime + buffer.duration;
			this.headBufferIndex = tailBufferIndex;
			this.scheduled = 0;
		}

		if (!this.mute) {
			let source = context.createBufferSource();
			source.buffer = buffer;
			source.connect(context.destination);
			source.start(this.tailTime);
		}

		this.scheduled++;
		this.tailTime += buffer.duration;

		if (++tailBufferIndex == this.buffers.length) {
			tailBufferIndex = 0;
		}

		this.channelData = this.buffers[tailBufferIndex].getChannelData(0);
		this.sampleIndex = 0;
		this.tailBufferIndex = tailBufferIndex;
	}

	/** advance simulation by one tick */
	tick() {
		this.cycle += SAMPLES_PER_SECOND;
		if (this.cycle >= HZ) {
			this.cycle -= HZ;
			let sample = ((this.cpu.outx >> 4) - 8) / 8;
			this.channelData[this.sampleIndex++] = sample;

			if (this.sampleIndex == this.channelData.length) {
				this.flushChannelData();
			}
		}
	}
}
