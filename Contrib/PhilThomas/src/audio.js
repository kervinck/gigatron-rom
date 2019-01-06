const SAMPLES_PER_SECOND = 44100;

var AudioContext = window.AudioContext || window.webkitAudioContext;

/** Audio output */
export class Audio {
    /**
     * Create an Audio.
     * @param {Gigatron} cpu - The CPU
     */
    constructor(cpu) {
        this.cpu = cpu;
        let context = this.context = new AudioContext();

        this.mute = false;
        this.volume = 0.33;
        this.cycle = 0;
        this.bias = 0;
        this.alpha = 0.99;
        this.scheduled = 0;
        this.full = false;

        let numSamples = Math.floor(SAMPLES_PER_SECOND / 100);
        this.buffers = [];
        for (let i = 0; i < 8; i++) {
            let buffer = context.createBuffer(1,
                numSamples,
                SAMPLES_PER_SECOND);
            this.buffers.push(buffer);
        }

        this.headBufferIndex = 0;
        this.tailBufferIndex = 0;
        this.duration = this.buffers[0].duration;
        this.tailTime = 0; // time at which tail buffer will start
        this.headTime = this.duration; // time at which head buffer will end
        this.channelData = this.buffers[0].getChannelData(0);
        this.sampleIndex = 0;
    }

    /** drain completed head buffers */
    drain() {
        let currentTime = this.context.currentTime;
        let headTime = this.headTime;
        let headBufferIndex = this.headBufferIndex;
        let scheduled = this.scheduled;
        let numBuffers = this.buffers.length;

        while (scheduled > 0 && headTime < currentTime) {
            headBufferIndex = (headBufferIndex == numBuffers - 1) ? 0 :
                (headBufferIndex + 1);
            headTime += this.duration;
            scheduled--;
        }

        this.headTime = headTime;
        this.scheduled = scheduled;
        this.full = scheduled == numBuffers;
    }

    /** flush current tail buffer */
    _flushChannelData() {
        let context = this.context;
        let currentTime = context.currentTime;
        let tailBufferIndex = this.tailBufferIndex;
        let buffer = this.buffers[tailBufferIndex];
        let scheduled = this.scheduled;
        let numBuffers = this.buffers.length;

        /* if the tail can't keep ahead of realtime, jump it to now */
        if (this.tailTime < currentTime) {
            // console.log('audio skip');
            this.tailTime = currentTime;
            this.headTime = currentTime + this.duration;
            this.headBufferIndex = tailBufferIndex;
            scheduled = 0;
        }

        if (!this.mute) {
            let source = context.createBufferSource();
            source.buffer = buffer;
            source.connect(context.destination);
            source.start(this.tailTime);
        }

        scheduled++;
        this.tailTime += this.duration;

        tailBufferIndex = (tailBufferIndex == numBuffers - 1) ? 0 :
            (tailBufferIndex + 1);

        this.channelData = this.buffers[tailBufferIndex].getChannelData(0);
        this.sampleIndex = 0;
        this.tailBufferIndex = tailBufferIndex;
        this.scheduled = scheduled;
        this.full = scheduled == numBuffers;
    }

    /** advance simulation by one tick */
    tick() {
        this.cycle += SAMPLES_PER_SECOND;
        if (this.cycle >= this.cpu.hz) {
            this.cycle -= this.cpu.hz;

            let sample = (this.cpu.outx >> 4) / 8;
            this.bias = (this.alpha * this.bias) + ((1 - this.alpha) * sample);
            sample = (sample - this.bias) * this.volume;
            this.channelData[this.sampleIndex++] = sample;

            if (this.sampleIndex == this.channelData.length) {
                this._flushChannelData();
            }
        }
    }
}
