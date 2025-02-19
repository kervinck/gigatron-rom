/* This version of the loader resets the Gigatron,
   intercepts the SYS_Exec that starts the main menu,
   and patches the memory with the Gt1 file instead.  */

const {
    Observable,
    Subject,
    concat,
    defer,
    range,
    EMPTY,
} = rxjs;

const {
    concatMap,
    concatAll,
} = rxjs.operators;


/** Loader */
export class Loader {
    /** Create a new Loader
     * @param {Gigatron} cpu
     */
    constructor(cpu) {
        this.cpu = cpu;
        this.sysexec = new Subject();
        this.sysexecaddr = -1;
    }

    /** Find alternate exec address (sys_Exec vs SYS_Exec_88)
     */

    findsysexec() {
        this.sysexecaddr = -1;
        let xad = this.cpu.rom[0x00ad];
        let xae = this.cpu.rom[0x00ae];
        if ((xad & 0xff00) == 0x1400 && (xae & 0xff00) == 0xe000) {
            this.sysexecaddr = ((xad & 0xff) << 8) | (xae & 0xff);
        }
    }
    
    /** load a gt1 file
     * @param {File} file
     * @return {Observable}
     */
    load(file) {
        this.findsysexec();
        return this.readFile(file).pipe(
            concatMap((buffer) => {
                let data = new DataView(buffer);
                return concat(
                    defer(() => {
                        this.cpu.reset();
                        return this.atSysExec()  /* exec Reset */
                    }),
                    this.atSysExec(),            /* exec Main  */
                    defer(() => {
                        this.loadGt1(data);
                        return EMPTY;
                    }) );
            }) );
    }

    /** load a rom file
     * @paranm {File} file
     * @return {Observable}
     */

    loadRom(file) {
        return this.readFile(file).pipe(
            concatMap((buffer) => {
                let data = new DataView(buffer);
                for(let i = 0; i < 65536; i++) {
                    this.cpu.rom[i] = data.getUint16(i * 2);
                }
                this.findsysexec();
                return concat(
                    defer(() => {
                        this.cpu.reset();
                        return this.atSysExec()  /* exec Reset */
                    }),
                    this.atSysExec(),            /* exec Main  */
                    EMPTY ) })
        );
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

    /** load gt1 file in memory
     * @param (DataView) data
     */

    loadGt1(data) {
        let offset = 0;
        let execaddr = 0x200;
        while (true) {
            let addr = data.getUint16(offset);
            offset += 2;
            let size = data.getUint8(offset);
            offset += 1;
            size = (size == 0) ? 256 : size;
            for (let i = 0; i < size; i++) {
                this.cpu.ram[addr] = data.getUint8(offset);
                offset += 1;
                addr += 1;
            }
            if (offset >= data.byteLength) {
                break;
            }
            if (data.getUint8(offset) == 0) {
                execaddr = data.getUint16(offset + 1);
                break;
            }
        }
        // Set the vPC and vLR exec address
        this.cpu.ram[0x16] = (execaddr - 2) & 0xff;
        this.cpu.ram[0x17] = (execaddr >> 8) & 0xff;
        this.cpu.ram[0x1a] = (execaddr - 2) & 0xff;
        this.cpu.ram[0x1b] = (execaddr >> 8) & 0xff;
        // shortcut SYS_Exec and jump to NEXTY
        this.cpu.nextpc = 0x300;
        this.cpu.ac = (-18/2) & 0xff;
    }

    /** wait for call to SYS_Exec or sys_Exec */
    atSysExec() {
        return Observable.create((observer) => {
            let subscription = this.sysexec.subscribe((curr) => {
                observer.complete();
            })
        });
    }

    /** advance one tick */
    tick() {
        if (this.cpu.pc == 0xaf || this.cpu.nextpc == this.sysexecaddr) {
            this.sysexec.next(this.cpu.pc);
        }
    }
}
