'use strict';

const {
    Observable,
    Subject,
} = rxjs;

const {
    concatMap,
} = rxjs.operators;

/* exported Loader */

/** Loader */
class Loader {
    /** Create a new Loader
     * @param {Gigatron} cpu
     */
    constructor(cpu) {
        this.cpu = cpu;
        this.payload = new Uint8Array(60);
        this.strobes = new Subject();
        this.checksum = 0;
        this.data = null;
    }

    /** load a gt1 file
     * @param {File} file
     * @return {Observable}
     */
    load(file) {
        return rxjs.concat(
            this.readFile(file).pipe(
                rxjs.operators.map((result) => {
                    this.data = new DataView(result);
                    this.offset = 0;

                    // Send one frame with false checksum to force
                    // a checksum resync at the receiver
                    this.checksum = 0;
                })),
            rxjs.defer(() => this.sendFrame(0xff, 0, 0)),
            rxjs.defer(() => {
                // Setup checksum properly
                this.checksum = 'g'.charCodeAt(0);
                return this.sendSections();
            }),
            rxjs.defer(() => {
                this.cpu.inReg = 0xff;
                return rxjs.empty();
            }));
    }

    /** read a file returning a Promise
     * @param {File} file
     * @return {Promise}
     */
    readFile(file) {
        return Observable.create((observer) => {
            let reader = new FileReader();
            reader.onload = (event) => {
                observer.next(reader.result);
                observer.complete();
            };
            reader.onerror = (event) => {
                observer.error(new Error('FileReader error'));
            };
            reader.readAsArrayBuffer(file);
        });
    }

    /** load sections from data until busy
     * @return {Observable}
     */
    sendSections() {
        if (this.offset < this.data.byteLength) {
            return rxjs.concat(
                this.sendSection(),
                rxjs.defer(() => this.sendSections()));
        }
        return rxjs.empty();
    }

    /** load the next section from the data
     * @return {Observable}
     */
    sendSection() {
        if (this.data.getUint8(this.offset) == 0 && this.offset != 0) {
            // start address section
            this.offset += 1;
            let startAddr = this.data.getUint16(this.offset);
            this.offset += 2;
            if (startAddr != 0) {
                return this.sendStartCommand(startAddr);
            }
        } else {
            // data section
            let addr = this.data.getUint16(this.offset);
            this.offset += 2;
            let size = this.data.getUint8(this.offset);
            this.offset += 1;
            if (size != 0) {
                return this.sendData(addr, size);
            }
        }
        return rxjs.empty();
    }

    /** send a start command
     * @param {number} addr
     * @return {Observable}
     */
    sendStartCommand(addr) {
        return this.sendFrame('L'.charCodeAt(0), 0, addr);
    }

    /** send a data block
     * @param {number} addr
     * @param {number} size
     * @return {Observable}
     */
    sendData(addr, size) {
        if (size != 0) {
            let n = Math.min(size, this.payload.length);
            for (let i = 0; i < n; i++) {
                this.payload[i] = this.data.getUint8(this.offset++);
            }
            return rxjs.concat(
                this.sendFrame('L'.charCodeAt(0), n, addr),
                rxjs.defer(() => this.sendData(addr + n, size - n)));
        }
        return rxjs.empty();
    }

    /** send the payload frame
     * @param {number} firstByte
     * @param {number} len
     * @param {number} addr
     * @return {Observable}
     */
    sendFrame(firstByte, len, addr) {
        return rxjs.concat(
            this.negedge(VSYNC),
            // account for 2 cycles delay in 74HCT595 and ?
            this.posedge(HSYNC),
            this.posedge(HSYNC),
            this.sendDataBits(firstByte, 8),
            rxjs.defer(() => {
                this.checksum = (this.checksum + (firstByte << 6)) & 0xff;
                return this.sendDataBits(len, 6);
            }),
            this.sendDataBits(addr & 0xff, 8),
            this.sendDataBits(addr >> 8, 8),
            this.sendDataBytes(this.payload),
            rxjs.defer(() => {
                this.checksum = (-this.checksum) & 0xff;
                return this.sendBits(this.checksum, 8);
            }));
    }

    /** send bytes from payload
     * @param {Uint8Array} payload
     * @return {Observable}
     */
    sendDataBytes(payload) {
        return rxjs.range(0, payload.length).pipe(
            concatMap((offset) => {
                return this.sendDataBits(payload[offset], 8);
            }));
    }

    /** shift in one bit
     * @param {number} bit
     */
    shiftBit(bit) {
        this.cpu.inReg = ((this.cpu.inReg << 1) & 0xff) | (bit ? 1 : 0);
    }

    /** send bits and add to checksum
     * @param {number} value - byte containing bits to send (msb first)
     * @param {number} n - number of bits to send
     * @return {Observable}
     */
    sendDataBits(value, n) {
        return rxjs.defer(() => {
            this.checksum = (this.checksum + value) & 0xff;
            return this.sendBits(value, n);
        });
    }

    /** send bits
     * @param {number} value - byte containing bits to send (msb first)
     * @param {number} n - number of bits to send
     * @return {Observable}
     */
    sendBits(value, n) {
        return rxjs.range(0, n).pipe(
            concatMap((i) => {
                this.shiftBit(value & (1 << (n - i - 1)));
                return this.posedge(HSYNC);
            }));
    }

    /** wait for negedge of signal
     * @param {number} mask
     * @return {Observable}
     */
    negedge(mask) {
        return Observable.create((observer) => {
            let prev = this.cpu.out;
            let subscription = this.strobes.subscribe((curr) => {
                if (prev & ~curr & mask) {
                    observer.complete();
                    subscription.unsubscribe();
                } else {
                    prev = curr;
                }
            });
        });
    }

    /** wait for posedge of signal
     * @param {number} mask
     * @return {Observable}
     */
    posedge(mask) {
        return Observable.create((observer) => {
            let prev = this.cpu.out;
            let subscription = this.strobes.subscribe((curr) => {
                if (~prev & curr & mask) {
                    observer.complete();
                    subscription.unsubscribe();
                } else {
                    prev = curr;
                }
            });
        });
    }

    /** advance one tick */
    tick() {
        if ((this.out ^ this.cpu.out) & 0xc0) {
            this.out = this.cpu.out;
            this.strobes.next(this.out);
        }
    }
}
