'use strict';

/* exported Gigatron */

/**
 * @return {Uint8} a random Uint8
 * */
function randomUint8() {
  return Math.floor(Math.random() * 256);
}

/** Gigatron processor */
class Gigatron {
  /** Create a Gigatron
   * @param {Object} options
   */
  constructor(options) {
    this.rom = new Uint16Array(1<<(options.log2rom || 16));
    this.ram = new Uint8Array(1<<(options.log2ram || 15));
    this.currpc = this.pc = 0x0000;
    this.nextpc = this.pc + 1;
    this.ac = 0;
    this.x = 0;
    this.y = 0;
    this.out = 0;
    this.outx = 0;
    this.inReg = 0xff; // active low!
    this.onOut = options.onOut;
    this.onOutx = options.onOutx;

    // randomize ram
    for (let i = 0; i < this.ram.length; i++) {
      this.ram[i] = randomUint8();
    }
  }

  /** advance simulation by one tick */
  tick() {
    let pc = this.currpc = this.pc & 0xffff; // (this.rom.length-1);
    this.pc = this.nextpc;
    this.nextpc = this.pc + 1;

    let instruction = this.rom[pc];
    let op = (instruction >> 13) & 0x0007;
    let mode = (instruction >> 10) & 0x0007;
    let bus = (instruction >> 8) & 0x0003;
    let d = (instruction >> 0) & 0x00ff;

    switch (op) {
      case 0: this.aluOp(mode, bus, d, (b) => b); break;
      case 1: this.aluOp(mode, bus, d, (b) => this.ac & b); break;
      case 2: this.aluOp(mode, bus, d, (b) => this.ac | b); break;
      case 3: this.aluOp(mode, bus, d, (b) => this.ac ^ b); break;
      case 4: this.aluOp(mode, bus, d, (b) => (this.ac + b) & 0xff); break;
      case 5: this.aluOp(mode, bus, d, (b) => (this.ac - b) & 0xff); break;
      case 6: this.storeOp(mode, bus, d); break;
      case 7: this.branchOp(mode, bus, d); break;
    }
  }

  /** perform an alu op
   * @param {number} mode
   * @param {number} bus
   * @param {number} d
   * @param {number} f
   */
  aluOp(mode, bus, d, f) {
    let b;

    switch (bus) {
    case 0: b = d; break;
    case 1:
      let addr = this.addr(mode, d) & (this.ram.length-1);
      b = this.ram[addr];
      break;
    case 2: b = this.ac; break;
    case 3: b = this.inReg; break;
    }

    b = f(b);

    switch (mode) {
      case 0:
      case 1:
      case 2:
      case 3: this.ac = b; break;
      case 4: this.x = b; break;
      case 5: this.y = b; break;
      case 6:
      case 7:
      let rising = ~this.out & b;

      this.out = b;
      this.onOut(this.out);

      // rising edge of out[6] registers outx from ac
      if (rising & 0x40) {
        if (this.outx != this.ac) {
          this.outx = this.ac;
          this.onOutx(this.outx);
        }
      }

      break;
    }
  }

  /** perform a store op
   * @param {number} mode
   * @param {number} bus
   * @param {number} d
   */
  storeOp(mode, bus, d) {
    let b;

    switch (bus) {
      case 0: b = d; break;
      case 1: b = 0; console.error('UNDEFINED BEHAVIOR!'); break;
      case 2: b = this.ac; break;
      case 3: b = this.inReg; break;
    }

    let addr = this.addr(mode, d) & (this.ram.length-1);
    this.ram[addr] = b;

    switch (mode) {
      case 4: this.x = b; break; // not clear whether x++ mode takes priority
      case 5: this.y = b; break;
    }
  }

  /** perform a branch op
   * @param {number} mode
   * @param {number} bus
   * @param {number} d
   */
  branchOp(mode, bus, d) {
    const ZERO = 0x80;
    let c;
    let ac = this.ac ^ ZERO;
    let base = this.pc & 0xff00;

    switch (mode) {
      case 0: c = true; base = this.y << 8; break; // jmp
      case 1: c = ac > ZERO; break; // bgt
      case 2: c = ac < ZERO; break; // blt
      case 3: c = ac != ZERO; break; // bne
      case 4: c = ac == ZERO; break; // beq
      case 5: c = ac >= ZERO; break; // bge
      case 6: c = ac <= ZERO; break; // ble
      case 7: c = true; break; // bra
    }

    if (c) {
      let b = this.offset(bus, d);
      this.nextpc = base | b;
    }
  }

  /** calculate a ram address
   * @param {number} mode
   * @param {number} d
   * @return {number} the address
   */
  addr(mode, d) {
    switch (mode) {
      case 0:
      case 4:
      case 5:
      case 6: return d;
      case 1: return this.x;
      case 2: return (this.y << 8) | d;
      case 3: return (this.y << 8) | this.x;
      case 7:
        let addr = (this.y << 8) | this.x;
        this.x = (this.x + 1) & 0xff;
        return addr;
    }
  }

  /** calculate a branch page offset
   * @param {number} bus
   * @param {number} d
   * @return {number} page offset
   */
  offset(bus, d) {
    switch (bus) {
      case 0: return d;
      case 1: return this.ram[d];
      case 2: return this.ac;
      case 3: return this.inReg;
    }
  }
}
