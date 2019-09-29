import {
    HSYNC,
    VSYNC,
} from './vga.js';

import {
    BUTTON_DOWN,
    BUTTON_A,
} from './gamepad.js';

const {
    Observable,
    Subject,
    concat,
    defer,
    range,
} = rxjs;

const {
    concatMap,
    concatAll,
    finalize,
} = rxjs.operators;

const MAX_PAYLOAD_SIZE = 60;
const START_OF_FRAME = 'L'.charCodeAt(0);
const INIT_CHECKSUM = 'g'.charCodeAt(0);

/** Repeat an Observable count times
 * @param {number} count
 * @param {Observable} observable
 * @return {Observable}
 */
function replicate(count, observable) {
    // return concat(...new Array(count).fill(observable));
    let go = (observer) => {
        if (count-- > 0) {
            observable.subscribe({
                next: (value) => observer.next(value),
                error: (err) => observer.error(err),
                complete: () => go(observer),
            });
        } else {
            observer.complete();
        }
    };

    return Observable.create(go);
}

/** Loader */
export class Loader {
    /** Create a new Loader
     * @param {Gigatron} cpu
     */
    constructor(cpu) {
        this.cpu = cpu;
        this.strobes = new Subject();
    }

    /** load a gt1 file
     * @param {File} file
     * @return {Observable}
     */
    load(file) {
        return this.readFile(file).pipe(
            concatMap((buffer) => {
                let data = new DataView(buffer);
                return concat(
                    this.startLoader(),
                    defer(() => {
                        // Send one frame with false checksum to force
                        // a checksum resync at the receiver
                        this.checksum = 0;
                        return this.sendFrame(0xff, 0);
                    }),
                    defer(() => {
                        // Setup checksum properly
                        this.checksum = INIT_CHECKSUM;
                        return this.sendSegments(data);
                    }));
            }),
            finalize(() => {
                // Set the input register back to quiesced state
                this.cpu.inReg = 0xff;
            }));
    }

    /** read a file returning a Promise
     * @param {File} file
     * @return {Observable}
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

    /** start the loader on the gigatron
     * @return {Observable}
     */
    startLoader() {
        return concat(
            defer(() => {
                this.cpu.reset();
                return replicate(100, this.atPosedge(VSYNC));
            }),
            replicate(5, this.pressButton(BUTTON_DOWN, 1, 1)),
            this.pressButton(BUTTON_A, 1, 60)
        );
    }

    /** simulate a button press
     * @param {number} bit
     * @param {number} downTime
     * @param {number} upTime
     * @return {Observable}
     */
    pressButton(bit, downTime, upTime) {
        return concat(
            defer(() => {
                this.cpu.inReg = bit ^ 0xff;
                return replicate(downTime, this.atPosedge(VSYNC));
            }),
            defer(() => {
                this.cpu.inReg = 0xff;
                return replicate(upTime, this.atPosedge(VSYNC));
            })
        );
    }

    /** load sections from data until busy
     * @param {DataView} data
     * @return {Observable<Observable<T>>}
     */
    sendSegments(data) {
        return Observable.create((observer) => {
            let offset = 0;
            while (offset < data.byteLength) {
                if (data.getUint8(offset) == 0 && offset != 0) {
                    // start address segment
                    offset += 1;
                    let startAddr = data.getUint16(offset);
                    offset += 2;
                    if (startAddr != 0) {
                        observer.next(this.sendStartCommand(startAddr));
                    }
                    break;
                } else {
                    // data segment
                    let addr = data.getUint16(offset);
                    offset += 2;
                    let size = data.getUint8(offset);
                    offset += 1;
                    if (size == 0) {
                        size = 256;
                    }
                    let payload = new DataView(
                        data.buffer,
                        data.byteOffset + offset,
                        size);
                    observer.next(this.sendDataSegment(addr, payload));
                    offset += size;
                }
            }

            if (offset > data.byteLength) {
                observer.error(new Error('Last segment exceeds file size'));
            }

            observer.complete();
        }).pipe(concatAll());
    }

    /** send a start command
     * @param {number} addr
     * @return {Observable}
     */
    sendStartCommand(addr) {
        return this.sendFrame(START_OF_FRAME, addr);
    }

    /** send a data block
     * @param {number} addr
     * @param {DataView} data
     * @return {Observable}
     */
    sendDataSegment(addr, data) {
        return Observable.create((observer) => {
            let buffer = data.buffer;
            let size = data.byteLength;
            let offset = data.byteOffset;
            let bytesInPage = 256 - (addr & 255);

            if (size > bytesInPage) {
                observer.error(new Error('Segment crosses page boundary'));
            } else {
                while (size != 0) {
                    let n = Math.min(size, MAX_PAYLOAD_SIZE);
                    let payload = new DataView(buffer, offset, n);
                    observer.next(this.sendFrame(
                        START_OF_FRAME, addr, payload));
                    addr += n;
                    offset += n;
                    size -= n;
                }
                observer.complete();
            }
        }).pipe(concatAll());
    }

    /** send the payload frame
     * @param {number} firstByte
     * @param {number} addr
     * @param {DataView} payload
     * @return {Observable}
     */
    sendFrame(firstByte, addr, payload) {
        return concat(
            this.atNegedge(VSYNC),
            // account for 2 cycles delay in 74HCT595 and ?
            this.atPosedge(HSYNC),
            this.atPosedge(HSYNC),
            this.sendDataBits(firstByte, 8),
            defer(() => {
                this.checksum = (this.checksum + (firstByte << 6)) & 0xff;
                return this.sendDataBits(payload ? payload.byteLength : 0, 6);
            }),
            this.sendDataBits(addr & 0xff, 8),
            this.sendDataBits(addr >> 8, 8),
            this.sendDataBytes(payload),
            defer(() => {
                this.checksum = (-this.checksum) & 0xff;
                return this.sendBits(this.checksum, 8);
            }));
    }

    /** send bytes from payload
     * @param {Uint8Array} payload
     * @return {Observable}
     */
    sendDataBytes(payload) {
        return range(0, MAX_PAYLOAD_SIZE).pipe(
            concatMap((offset) => {
                let byte = (payload && offset < payload.byteLength) ?
                    payload.getUint8(offset) : 0;
                return this.sendDataBits(byte, 8);
            }));
    }

    /** send bits and add to checksum
     * @param {number} value - byte containing bits to send (msb first)
     * @param {number} n - number of bits to send
     * @return {Observable}
     */
    sendDataBits(value, n) {
        return defer(() => {
            this.checksum = (this.checksum + value) & 0xff;
            return this.sendBits(value, n);
        });
    }

    /** shift one bit into inReg
     * @param {number} bit
     */
    shiftBit(bit) {
        this.cpu.inReg = ((this.cpu.inReg << 1) & 0xff) | (bit ? 1 : 0);
    }

    /** send bits
     * @param {number} value - byte containing bits to send (msb first)
     * @param {number} n - number of bits to send
     * @return {Observable}
     */
    sendBits(value, n) {
        return range(0, n).pipe(
            concatMap((i) => {
                this.shiftBit(value & (1 << (n - i - 1)));
                return this.atPosedge(HSYNC);
            }));
    }

    /** wait for negedge of signal
     * @param {number} mask
     * @return {Observable}
     */
    atNegedge(mask) {
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
    atPosedge(mask) {
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
        if ((this.out ^ this.cpu.out) & (HSYNC | VSYNC)) {
            this.out = this.cpu.out;
            this.strobes.next(this.out);
        }
    }
}
