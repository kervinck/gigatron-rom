function randomUint8() { return Math.floor(Math.random() * 256); }

class Gigatron {

  constructor(options) {
    this.rom = new Uint16Array(1<<(options.log2rom || 16));
    this.ram = new Uint8Array(1<<(options.log2ram || 15));
    this.currpc = this.pc = 0x0000;
    this.nextpc = this.pc + 1;
    this.ac = 0;
    this.x = 0;
    this.y = 0;
    this.outReg = 0;
    this.outxReg = 0;
    this.inReg = 0xff;  // active low!
    this.trace = false;
    this.symtab = options.symtab || {};
    this.row = 0;
    this.minRow = options.vertical.backPorch;
    this.maxRow = this.minRow + options.vertical.visible;
    this.col = 0;
    this.minCol = options.horizontal.backPorch;
    this.maxCol = this.minCol + options.horizontal.visible;
    this.poff = 0;
    this.vsync = false;

    for (let i = 0; i < this.ram.length; i++) {
      this.ram[i] = randomUint8();
    }
  }

  step() {
    let pc = this.currpc = this.pc & 0xffff;
    this.pc = this.nextpc;
    this.nextpc = this.pc + 1;
    let instruction = this.rom[pc];
    let op = (instruction >> 13) & 7;
    let mode = (instruction >> 10) & 7;
    let bus = (instruction >> 8) & 3;
    let d = (instruction >> 0) & 0xff;

    switch (op) {
      case 0: this.aluOp(mode, bus, d, b => b); break;
      case 1: this.aluOp(mode, bus, d, b => this.ac & b); break;
      case 2: this.aluOp(mode, bus, d, b => this.ac | b); break;
      case 3: this.aluOp(mode, bus, d, b => this.ac ^ b); break;
      case 4: this.aluOp(mode, bus, d, b => (this.ac + b) & 0xff); break;
      case 5: this.aluOp(mode, bus, d, b => (this.ac - b) & 0xff); break;
      case 6: this.storeOp(mode, bus, d); break;
      case 7: this.branchOp(mode, bus, d); break;
    }

    if ((this.outReg & 0xc0) == 0xc0) {
      if (this.row >= this.minRow &&this.row < this.maxRow && this.col >= this.minCol && this.col < this.maxCol) {
        let r = ((this.outReg >> 0) & 3) << 6;
        let g = ((this.outReg >> 2) & 3) << 6;
        let b = ((this.outReg >> 4) & 3) << 6;

        for (let i = 0; i < 4; i++) {
          pixels[this.poff++] = r;
          pixels[this.poff++] = g;
          pixels[this.poff++] = b;
          pixels[this.poff++] = 255;
        }
      }

      this.col += 4;
    }
  }

  aluOp(mode, bus, d, f) {
    let b;

    switch (bus) {
      case 0: b = d; break;
      case 1: b = this.load(mode, d); break;
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
      let rising = ~this.outReg & b;
      let falling = this.outReg & ~b;

      // vsync starts on falling edge of out[7]
      if (falling & 0x80) {
        this.vsyncStart();
      }
      // vsync ends on rising edge of out[7]
      else if (rising & 0x80) {
        this.vsyncEnd();
      }
      // hsync starts on falling edge of out[6]
      if (falling & 0x40) {
        this.hsyncStart();
      }
      // hsync ends on rising edge of out[6]
      else if (rising & 0x40) {
        this.hsyncEnd();

        // hsync end also latches outx from ac
        this.outxReg = this.ac;
      }

      this.outReg = b;
      break;
    }
  }

  storeOp(mode, bus, d) {
    let b;

    switch (bus) {
      case 0: b = d; break;
      case 1: b = 0; console.error("UNDEFINED BEHAVIOR!"); break;
      case 2: b = this.ac; break;
      case 3: b = this.inReg; break;
    }

    this.store(mode, d, b);

    switch (mode) {
      case 4: this.x = b; break;
      case 5: this.y = b; break;
    }
  }

  branchOp(mode, bus, d) {
    let c;
    let ac = this.ac ^ 0x80;
    let base = this.nextpc & 0xff00;

    switch (mode) {
      case 0: c = true; base = this.y << 8; break; // jmp
      case 1: c = ac >  0x80; break;               // bgt
      case 2: c = ac <  0x80; break;               // blt
      case 3: c = ac != 0x80; break;               // bne
      case 4: c = ac == 0x80; break;               // beq
      case 5: c = ac >= 0x80; break;               // bge
      case 6: c = ac <= 0x80; break;               // ble
      case 7: c = true; break;                     // bra
    }

    if (c) {
      let b = this.offset(bus, d);
      this.nextpc = base | b;
    }
  }

  vsyncStart() {
    this.row = 0;
    this.vsync = true;
    this.poff = 0;
  }

  vsyncEnd() {
  }

  hsyncStart() {
    this.col = 0;
    this.row++;
  }

  hsyncEnd() {
  }

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

  offset(bus, d) {
    switch (bus) {
      case 0: return d;
      case 1: return this.ram[d];
      case 2: return this.ac;
      case 3: return this.inReg;
    }
  }

  load(mode, d) {
    let addr = this.addr(mode, d) & (this.ram.length-1);
    let b = this.ram[addr];
    return b;
  }

  store(mode, d, b) {
    let addr = this.addr(mode, d) & (this.ram.length-1);
    this.ram[addr] = b;
  }
}
