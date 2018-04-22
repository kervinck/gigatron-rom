'use strict';

/* exported Loader */

/** Loader */
class Loader {
    /** Create a new Loader
     * @param {Gigatron} cpu
    */
    constructor(cpu) {
        this.cpu = cpu;
        this.checksum = 0;
    }

    /** load and start a program
     * @param {object} options
     */
    load(options) {
        let payload = new Uint8Array(60);
        let sync = true;

        for (let block of options.blocks) {
            let address = block.address;
            let bytes = block.bytes;
            let n = bytes.length;
            let offset = 0;

            while (n > 0) {
                let l = Math.min(n, 60);

                for (let i = 0; i < l; i++) {
                    payload[i] = bytes[offset++];
                }

                if (sync) {
                    // Send one frame with false checksum to force
                    // a checksum resync at the receiver
                    this.checksum = 0;
                    this.sendFrame(0xff, l, address, payload);

                    // Setup checksum properly
                    loader.checksum = 'g'.charCodeAt(0);

                    sync = false;
                }

                loader.sendFrame('L'.charCodeAt(0), l, address, payload);
                n -= l;
                address += l;
            }
        }

        // Force execution
        loader.sendFrame('L'.charCodeAt(0), 0, options.startAddress, payload);
    }

    /** load an image asynchronously
     * @param {number} firstByte
     * @param {number} len
     * @param {number} address
     * @param {number} message
     */
    sendFrame(firstByte, len, address, message) {
        this.waitVsyncHigh();
        this.waitVsyncLow();

        // account for 2 cycles delay in 74HCT595 and ?
        for (let i = 0; i < 2; i++) {
            this.waitHsyncLow();
            this.waitHsyncHigh();
        }

        this.sendBits(firstByte, 8);
        this.checksum = (this.checksum + (firstByte << 6)) & 0xff;
        this.sendBits(len, 6);
        this.sendBits(address & 0xff, 8);
        this.sendBits(address >> 8, 8);
        for (let i = 0; i < 60; i++) {
            this.sendBits(message[i], 8);
        }
        let lastByte = (-this.checksum) & 0xff;
        this.sendBits(lastByte, 8);
        this.checksum = lastByte;
    }

    /** shift in one bit
     * @param {number} bit
     */
    shiftBit(bit) {
        this.cpu.inReg = ((this.cpu.inReg << 1) & 0xff) | (bit ? 1 : 0);
    }

    /** send bits
     * @param {number} value - byte containing bits to send (msb first)
     * @param {number} n - number of bits to send
    */
    sendBits(value, n) {
        // console.log('sending ' + n + '\'h' + value.toString(16));
        for (let bit = 1 << (n-1); bit; bit >>= 1) {
            this.shiftBit(value & bit);
            this.waitHsyncLow();
            this.waitHsyncHigh();
        }

        this.checksum = (this.checksum + value) & 0xff;
    }

    /** wait for vsync to go high */
    waitVsyncHigh() {
        // let ticks = 0;
        while ((this.cpu.out & 0x80) == 0) {
            this.cpu.tick();
            // ticks++;
        }
        // console.log(ticks + ' ticks to vsync high');
    }

    /** wait for vsync to go low */
    waitVsyncLow() {
        // let ticks = 0;
        while ((this.cpu.out & 0x80) != 0) {
            this.cpu.tick();
            // ticks++;
        }
        // console.log(ticks + ' ticks to vsync low');
    }

    /** wait for hsync to go high */
    waitHsyncHigh() {
        // let ticks = 0;
        while ((this.cpu.out & 0x40) == 0) {
            this.cpu.tick();
            // ticks++;
        }
        // console.log(ticks + ' ticks to hsync high');
    }

    /** wait for hsync to go low */
    waitHsyncLow() {
        // let ticks = 0;
        while ((this.cpu.out & 0x40) != 0) {
            this.cpu.tick();
            // ticks++;
        }
        // console.log(ticks + ' ticks to hsync low');
    }
}
