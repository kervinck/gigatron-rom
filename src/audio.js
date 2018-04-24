'use strict';

/* exported Audio */

const SAMPLES_PER_SECOND = 44100;

var AudioContext = window.AudioContext || window.webkitAudioContext;

/** Audio output */
class Audio {
	/**
	 * Create an Audio.
	 * @param {Gigatron} cpu - The CPU
	 */
	constructor(cpu) {
		this.cpu = cpu;
		let context = this.context = new AudioContext();
		let filter = this.filter = context.createBiquadFilter();
		filter.connect(context.destination);
		filter.type = 'lowpass';
		filter.frequency.value = 4000;
		filter.Q.value = 2;

		this.mute = false;
		this.volume = 0.5;
		this.cycle = 0;
		this.bias = 0;
		this.alpha = 0.8;
		this.scheduled = 0;
		this.tailTime = 0; // time at which tail buffer will start
		this.headTime = 0; // time at which head buffer will end

		this.buffers = [];
		for (let i = 0; i < 4; i++) {
			let buffer = context.createBuffer(1, Math.floor(SAMPLES_PER_SECOND/100), SAMPLES_PER_SECOND);
			this.buffers.push(buffer);
		}

		this.headBufferIndex = 0;
		this.tailBufferIndex = 0;
		this.duration = this.buffers[0].duration;
		this.channelData = this.buffers[0].getChannelData(0);
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
			headTime += this.duration;
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

		/* if the tail can't keep ahead of realtime, jump it to now */
		if (this.tailTime < currentTime) {
			console.log('audio skip');
			this.tailTime = currentTime;
			this.headTime = currentTime + this.duration;
			this.headBufferIndex = tailBufferIndex;
			this.scheduled = 0;
		}

		if (!this.mute) {
			let source = context.createBufferSource();
			source.buffer = buffer;
			source.connect(this.filter);
			source.start(this.tailTime);
		}

		this.scheduled++;
		this.tailTime += this.duration;

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
		if (this.cycle >= this.cpu.hz) {
			this.cycle -= this.cpu.hz;

			let sample = (this.cpu.outx >> 4) / 0x10;
			this.bias = (this.alpha * this.bias) + ((1-this.alpha) * sample);
			this.channelData[this.sampleIndex++] = (sample-this.bias) * this.volume;

			if (this.sampleIndex == this.channelData.length) {
				this.flushChannelData();
			}
		}
	}
}
