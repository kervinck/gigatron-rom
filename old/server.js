const fs = require('fs');

function decode_addr(mode, d)
{
    switch (mode) {
        case 0:
        case 4:
        case 5:
        case 6: return '$' + toHex(d, 2);
        case 1: return "x";
        case 2: return "y," + '$' + toHex(d, 2);
        case 3: return "y,x";
        case 7: return "y,x++";
    }
}

function arith(o, d, code) {
    let mode = (o >> 2) & 7;
    let bus = (o >> 0) & 3;

    switch (bus) {
        case 0: code += '$' + toHex(d, 2); break;
        case 1: code += '[' + decode_addr(mode, d) + ']'; break;
        case 2: code += "ac"; break;
        case 3: code += "in"; break;
    }

    switch (mode) {
        case 4: code += ",x"; break;
        case 5: code += ",y"; break;
        case 6: 
        case 7: code += ",out"; break;
    }

    return code;
}

function store(o, d, code)
{
    let mode = (o >> 2) & 7;
    let bus = (o >> 0) & 3;

    switch (bus) {
        case 0: code += '$' + toHex(d, 2) + ','; break;
        case 1: code += "$??,"; break;
        case 3: code += "in,"; break;
    }

    code += '[' + decode_addr(mode, d) + ']';

    switch (mode) {
        case 4: code += ",x"; break;
        case 5: code += ",y"; break;
    }

    return code;
}

function branch(o, d)
{
    let cc = (o >> 2) & 7;
    let bus = (o >> 0) & 3;
    
    switch (cc) {
        case 0: code = "jmp  y,"; break;
        case 1: code = "bgt  "; break;
        case 2: code = "bge  "; break;
        case 3: code = "bne  "; break;
        case 4: code = "beq  "; break;
        case 5: code = "blt  "; break;
        case 6: code = "ble  "; break;
        case 7: code = "bra  "; break;
    }

    switch (bus) {
        case 0: code += '$' + toHex(d, 2); break;
        case 1: code += '[$' + toHex(d, 2) + ']'; break;
        case 2: code += 'ac'; break;
        case 3: code += 'in'; break;
    }

    return code;
}

function toHex(x, n) {
    return padl(x.toString(16), '0', n);
}

function padl(s, c, n) {
    while (s.length < n) {
        s = c + s;
    }
    return s;
}

function padr(s, c, n) {
    while (s.length < n) {
        s += c;
    }
    return s;
}

function disassemble(op) {
    let o = (op >> 8) & 0xff;
    let d = (op >> 0) & 0xff;

    switch ((o >> 5) & 7) {
        case 0: return arith(o, d, "ld   ");
        case 1: return arith(o, d, "anda ");
        case 2: return arith(o, d, "ora  ");
        case 3: return arith(o, d, "xora ");
        case 4: return arith(o, d, "adda ");
        case 5: return arith(o, d, "suba ");
        case 6: return store(o, d, "st   ");
        case 7: return branch(o, d);
    }
}

function toPixel(p) {
    let r = ((p >> 4) & 3)/4;
    let g = ((p >> 2) & 3)/4;
    let b = ((p >> 0) & 3)/4;
    let y = 0.2126*r + 0.7152*g + 0.0722*b;

    return "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ".substr(69-Math.floor(y*70), 1);
}

function randomUInt8() { return Math.floor(Math.random() * 256); }

class Gigatron {

    constructor(options) {
        let log2ram = options.log2ram || 15;
        this.rom = options.rom;
        this.ram = new Buffer(1<<log2ram);
        this.currpc = this.pc = 0x0000;
        this.nextpc = this.pc + 1;
        this.ac = 0x00;
        this.x = 0x00;
        this.y = 0x00;
        this.outReg = 0; //randomUInt8();
        this.outxReg = 0; //randomUInt8();
        this.inReg = 0xff;
        this.trace = false;
        this.symtab = options.symtab || {};
        this.video = true;
        this.cycle = 0;
        this.row = 0;
        this.col = 0;
        this.minCol = options.horizontal.backPorch;
        this.maxCol = this.minCol + options.horizontal.visible;
        this.minRow = options.vertical.backPorch;
        this.maxRow = this.minRow + options.vertical.visible;

        for (let i = 0; i < this.ram.length; i++) {
            this.ram.writeUInt8(randomUInt8(), i);
            //this.ram.writeUInt8(0, i);
        }
    }

    step() {
        let pc = this.currpc = this.pc & 0xffff;
        this.pc = this.nextpc;
        this.nextpc = this.pc + 1;
        let instruction = this.rom.readUInt16BE(pc<<1);
        let op = (instruction >> 13) & 7;
        let mode = (instruction >> 10) & 7;
        let bus = (instruction >> 8) & 3;
        let d = (instruction >> 0) & 0xff;

        if (this.trace) {
            this.traceBuf = "";
            let sym = this.symtab[pc] || "";
            if (sym) {
                sym += ':';
            }
            sym = padr(sym, ' ', 15);
            let code = disassemble(instruction);
            process.stdout.write(sym + toHex(pc, 4) + " " + toHex(instruction, 4) + "  " + padr(code, ' ', 20));
        }

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

        if (this.trace) {
            process.stdout.write(";; ac=$"+toHex(this.ac,2) +
                                 " x=$"+toHex(this.x,2) +
                                 " y=$"+toHex(this.y,2) +
                                 " out=$"+toHex(this.outReg,2) +
                                 " outx=$"+toHex(this.outxReg,2) +
                                 this.traceBuf + "\n");
        }

        if (this.video && (this.outReg & 0xc0) == 0xc0) {
            if (this.row >= this.minRow &&this.row < this.maxRow && this.col >= this.minCol && this.col < this.maxCol) {
                if ((this.row % 4) == 0) {
                    let p = toPixel(this.outReg & 0x3f);
                    process.stdout.write(p.repeat(2));
                }
            }
            this.col += 4;
        }
        else if (this.cycle++ == 1*6250000) {
            this.video = true;
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
                    if (this.outxReg != this.ac) {
                        this.outxReg = this.ac;
                        if (this.trace) {
                            let leds = "";
                            for (let i = 0; i < 4; i++) {
                                leds += (this.outxReg & (1<<i)) ? "*" : ".";
                            }

                            this.traceBuf += " <" + toHex(this.outxReg >> 4, 1) + " " + leds + ">";
                        }
                    }
                }

                this.outReg = b;
                break;
        }
    }

    storeOp(mode, bus, d) {
        let b;

        switch (bus) {
            case 0: b = d; break;
            case 1: b = randomUInt8(); break;
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
        if (this.video) {
            //process.stdout.write("\n-------------------------------------------------------\n");
            process.stdout.write("\x1b[H");
        }
    }

    vsyncEnd() {
    }

    hsyncStart() {
        this.col = 0;
        if (this.video) {
            if ((this.row % 4) == 0) {
                process.stdout.write("\n");
            }
        }
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
            case 1: return this.ram.readUInt8(d);
            case 2: return this.ac;
            case 3: return this.inReg;
        }
    }

    load(mode, d) {
        let addr = this.addr(mode, d) & (this.ram.length-1);
        let b = this.ram.readUInt8(addr);
        if (this.trace) {
            this.traceBuf += " [$"+toHex(addr,4)+"]->$" + toHex(b, 2);
        }
        return b;
    }

    store(mode, d, b) {
        let addr = this.addr(mode, d) & (this.ram.length-1);
        if (this.trace) {
            this.traceBuf += " [$"+toHex(addr,4)+"]<-$" + toHex(b, 2);
        }
        this.ram.writeUInt8(b, addr);
    }
}

let symtab = {1536: 'SYS_LSRW1_48', 1280: 'shiftTable', 4: '.countMem0', 517: 'vBlankStart', 1031: 'def', 1544: '.sysLsrw1a', 1671: 'SYS_LSRW6_48', 685: 'vBlankLast', 1042: 'addi', 19: '.countMem1', 20: '.debounce', 1557: '.sysLsrw1b', 55129: '.sysPi2', 1048: '.addi0', 537: '.rnd0', 538: '.rnd1', 32: '.initEnt0', 1057: 'subi', 546: '.leds0', 36: '.initEnt1', 1063: '.subi0', 41: '.initEnt2', 1066: '.subi1', 59948: 'Racer', 1073: 'lslw', 1586: '.sysLsrw2b', 1590: 'SYS_LSRW3_52', 570: '.leds1', 1119: 'peek', 60: 'warm', 62: 'cold', 575: '.leds3', 65090: 'Reset', 779: 'EXIT', 2048: 'font82up', 585: '.leds5', 98: '.initTri0', 1614: '.sysLsrw3b', 1618: 'SYS_LSRW4_50', 1108: 'poke', 63061: 'Pictures', 2560: 'invTable', 1721: 'SYS_Read3_40', 600: '.snd0', 601: '.snd1', 1626: '.sysLsrw4a', 92: '.loop0', 2816: 'gigatronRaw', 607: '.snd2', 608: '.snd3', 610: 'sound1', 100: '.initTri1', 1125: 'lupReturn', 941: 'PEEK', 106: '.initPul', 1569: '.sysLsrw2a', 1645: 'SYS_LSRW5_50', 116: '.loop1', 1653: '.sysLsrw5a', 1143: 'deek', 633: '.sound1a', 634: '.sound1b', 1130: 'doke', 1667: '.sysLsrw5b', 1156: 'andw', 1641: '.sysLsrw4b', 649: 'vSync0', 653: 'vSync1', 654: 'vSync2', 655: 'vSync3', 1168: 'orw', 662: '.ser0', 663: '.ser1', 964: '.subw1', 154: 'SYS_Reset_36', 667: 'vBlankSample', 1180: 'xorw', 1050: '.addi1', 1696: 'SYS_LSLW4_46', 1222: 'SYS_LSRW8_24', 678: 'vBlankNormal', 173: 'SYS_Exec_88', 1710: '.sysLsrl4', 64175: 'Screen', 1202: '.sysRnd0', 1203: '.sysRnd1', 55070: '.sysRacer2', 1209: 'SYS_LSRW7_30', 1724: 'txReturn', 703: '.restart0', 710: '.restart1', 711: '.restart2', 714: '.restart3', 1229: 'SYS_LSLW8_24', 719: '.select0', 720: '.select1', 1236: 'SYS_Draw4_30', 1251: '.vdb0', 1257: '.vdb1', 1258: '.vdb2', 244: 'SYS_Out_22', 1781: '.sysCc0', 760: '.sysNbi', 249: 'SYS_In_24', 24320: 'packedBaboon', 767: 'ENTER', 256: 'videoA', 769: 'NEXT', 771: '.next2', 267: 'pixels', 785: 'LDWI', 55060: '.sysRacer0', 55062: '.sysRacer1', 1785: '.sysCc1', 794: 'LD', 8960: 'packedParrot', 55071: '.sysRacer3', 801: 'LDW', 1561: 'SYS_LSRW2_52', 811: 'STW', 63281: 'Credits', 821: 'BCC', 1792: 'font32up', 991: 'ALLOC', 829: '.cond1', 831: 'EQ', 832: '.cond2', 835: '.cond3', 837: '.cond4', 840: '.cond5', 842: '.cond6', 845: 'GT', 848: 'LT', 851: 'GE', 55124: '.sysPi1', 854: 'LE', 857: 'LDI', 1679: '.sysLsrw6a', 862: 'ST', 867: 'POP', 1086: 'stlw', 877: 'next1', 61806: 'Mandelbrot', 882: 'NE', 915: 'INC', 574: '.leds2', 1598: '.sysLsrw3a', 895: 'LUP', 898: 'ANDI', 904: 'ORI', 2304: 'notesTable', 908: 'XORI', 912: 'BRA', 579: '.leds4', 64405: 'Main', 63895: 'Loader', 921: 'ADDW', 58268: 'Snake', 1008: 'POKE', 931: '.addw0', 933: '.addw1', 1692: '.sysLsrw6b', 429: 'soundF', 943: 'retry', 432: 'sound2', 1011: 'DOKE', 948: 'SYS', 1097: 'ldlw', 952: 'SUBW', 961: '.subw0', 451: '.sound2a', 452: '.sound2b', 457: 'videoB', 971: 'REENTER', 973: 'DEF', 975: 'CALL', 468: 'videoC', 885: 'PUSH', 479: 'videoD', 55120: '.sysPi0', 995: 'ADDI', 998: 'SUBI', 1001: 'LSLW', 490: '.last', 1004: 'STLW', 1006: 'LDLW', 496: 'videoE', 499: 'videoF', 1014: 'DEEK', 1016: 'ANDW', 505: '.notlast', 1018: 'ORW', 507: '.join', 1020: 'XORW', 1023: 'RET'};

let rom = fs.readFileSync('ROMv1.rom');
let cpu = new Gigatron({
    rom: rom,
    symtab: symtab,
    log2ram: 16,
    horizontal: { frontPorch: 16, backPorch: 48, visible: 640 },
    vertical:   { frontPorch: 10, backPorch: 33, visible: 480 },
});

cpu.trace = false;

while(1) {
    //if (cpu.pc == 0x37) cpu.trace = true;
    //if (cpu.pc == 0x9a) cpu.trace = true;
    cpu.step();
}

