'use strict';

/* exported Disassembler */

class Disassembler {
  constructor(symtab) {
    this.symtab = symtab || {};
  }

  disassemble(instruction) {
    let op = (instruction >> 13) & 7;
    let mode = (instruction >> 10) & 7;
    let bus = (instruction >> 8) & 3;
    let d = (instruction >> 0) & 0xff;

      switch (op) {
          case 0: return this.decode_alu(mode, bus, d, 'ld');
          case 1: return this.decode_alu(mode, bus, d, 'anda');
          case 2: return this.decode_alu(mode, bus, d, 'ora');
          case 3: return this.decode_alu(mode, bus, d, 'xora');
          case 4: return this.decode_alu(mode, bus, d, 'adda');
          case 5: return this.decode_alu(mode, bus, d, 'suba');
          case 6: return this.decode_store(mode, bus, d);
          case 7: return this.decode_branch(mode, bus, d);
      }
  }

  decode_addr(mode, d)
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

  decode_alu(mode, bus, d, mnemonic) {
    let operands = [];

    switch (bus) {
      case 0: operands.push('$' + toHex(d, 2)); break;
      case 1: operands.push('[' + this.decode_addr(mode, d) + ']'); break;
      case 2: operands.push('ac'); break;
      case 3: operands.push('in'); break;
    }

    switch (mode) {
      case 4: operands.push('x'); break;
      case 5: operands.push('y'); break;
      case 6:
      case 7: operands.push('out'); break;
    }

    return {
      mnemonic: mnemonic,
      operands: operands,
    };
  }

  decode_store(mode, bus, d)
  {
    let operands = [];

    switch (bus) {
      case 0: operands.push('$' + toHex(d, 2)); break;
      case 1: operands.push('$??'); break;
      case 3: operands.push('in'); break;
    }

    operands.push('[' + this.decode_addr(mode, d) + ']');

    switch (mode) {
      case 4: operands.push('x'); break;
      case 5: operands.push('y'); break;
    }

    return {
      mnemonic: 'st',
      operands: operands,
    };
  }

  decode_branch(mode, bus, d)
  {
    let operands = [];
    let mnemonic;

    switch (mode) {
      case 0: mnemonic = 'jmp'; operands.push('y'); break;
      case 1: mnemonic = 'bgt'; break;
      case 2: mnemonic = 'bge'; break;
      case 3: mnemonic = 'bne'; break;
      case 4: mnemonic = 'beq'; break;
      case 5: mnemonic = 'blt'; break;
      case 6: mnemonic = 'ble'; break;
      case 7: mnemonic = 'bra'; break;
    }

    switch (bus) {
      case 0: operands.push('$' + toHex(d, 2)); break;
      case 1: operands.push('[$' + toHex(d, 2) + ']'); break;
      case 2: operands.push('ac'); break;
      case 3: operands.push('in'); break;
    }

    return {
      mnemonic: mnemonic,
      operands: operands,
    };
  }
}

//let symtab = {1536: 'SYS_LSRW1_48', 1280: 'shiftTable', 4: '.countMem0', 517: 'vBlankStart', 1031: 'def', 1544: '.sysLsrw1a', 1671: 'SYS_LSRW6_48', 685: 'vBlankLast', 1042: 'addi', 19: '.countMem1', 20: '.debounce', 1557: '.sysLsrw1b', 55129: '.sysPi2', 1048: '.addi0', 537: '.rnd0', 538: '.rnd1', 32: '.initEnt0', 1057: 'subi', 546: '.leds0', 36: '.initEnt1', 1063: '.subi0', 41: '.initEnt2', 1066: '.subi1', 59948: 'Racer', 1073: 'lslw', 1586: '.sysLsrw2b', 1590: 'SYS_LSRW3_52', 570: '.leds1', 1119: 'peek', 60: 'warm', 62: 'cold', 575: '.leds3', 65090: 'Reset', 779: 'EXIT', 2048: 'font82up', 585: '.leds5', 98: '.initTri0', 1614: '.sysLsrw3b', 1618: 'SYS_LSRW4_50', 1108: 'poke', 63061: 'Pictures', 2560: 'invTable', 1721: 'SYS_Read3_40', 600: '.snd0', 601: '.snd1', 1626: '.sysLsrw4a', 92: '.loop0', 2816: 'gigatronRaw', 607: '.snd2', 608: '.snd3', 610: 'sound1', 100: '.initTri1', 1125: 'lupReturn', 941: 'PEEK', 106: '.initPul', 1569: '.sysLsrw2a', 1645: 'SYS_LSRW5_50', 116: '.loop1', 1653: '.sysLsrw5a', 1143: 'deek', 633: '.sound1a', 634: '.sound1b', 1130: 'doke', 1667: '.sysLsrw5b', 1156: 'andw', 1641: '.sysLsrw4b', 649: 'vSync0', 653: 'vSync1', 654: 'vSync2', 655: 'vSync3', 1168: 'orw', 662: '.ser0', 663: '.ser1', 964: '.subw1', 154: 'SYS_Reset_36', 667: 'vBlankSample', 1180: 'xorw', 1050: '.addi1', 1696: 'SYS_LSLW4_46', 1222: 'SYS_LSRW8_24', 678: 'vBlankNormal', 173: 'SYS_Exec_88', 1710: '.sysLsrl4', 64175: 'Screen', 1202: '.sysRnd0', 1203: '.sysRnd1', 55070: '.sysRacer2', 1209: 'SYS_LSRW7_30', 1724: 'txReturn', 703: '.restart0', 710: '.restart1', 711: '.restart2', 714: '.restart3', 1229: 'SYS_LSLW8_24', 719: '.select0', 720: '.select1', 1236: 'SYS_Draw4_30', 1251: '.vdb0', 1257: '.vdb1', 1258: '.vdb2', 244: 'SYS_Out_22', 1781: '.sysCc0', 760: '.sysNbi', 249: 'SYS_In_24', 24320: 'packedBaboon', 767: 'ENTER', 256: 'videoA', 769: 'NEXT', 771: '.next2', 267: 'pixels', 785: 'LDWI', 55060: '.sysRacer0', 55062: '.sysRacer1', 1785: '.sysCc1', 794: 'LD', 8960: 'packedParrot', 55071: '.sysRacer3', 801: 'LDW', 1561: 'SYS_LSRW2_52', 811: 'STW', 63281: 'Credits', 821: 'BCC', 1792: 'font32up', 991: 'ALLOC', 829: '.cond1', 831: 'EQ', 832: '.cond2', 835: '.cond3', 837: '.cond4', 840: '.cond5', 842: '.cond6', 845: 'GT', 848: 'LT', 851: 'GE', 55124: '.sysPi1', 854: 'LE', 857: 'LDI', 1679: '.sysLsrw6a', 862: 'ST', 867: 'POP', 1086: 'stlw', 877: 'next1', 61806: 'Mandelbrot', 882: 'NE', 915: 'INC', 574: '.leds2', 1598: '.sysLsrw3a', 895: 'LUP', 898: 'ANDI', 904: 'ORI', 2304: 'notesTable', 908: 'XORI', 912: 'BRA', 579: '.leds4', 64405: 'Main', 63895: 'Loader', 921: 'ADDW', 58268: 'Snake', 1008: 'POKE', 931: '.addw0', 933: '.addw1', 1692: '.sysLsrw6b', 429: 'soundF', 943: 'retry', 432: 'sound2', 1011: 'DOKE', 948: 'SYS', 1097: 'ldlw', 952: 'SUBW', 961: '.subw0', 451: '.sound2a', 452: '.sound2b', 457: 'videoB', 971: 'REENTER', 973: 'DEF', 975: 'CALL', 468: 'videoC', 885: 'PUSH', 479: 'videoD', 55120: '.sysPi0', 995: 'ADDI', 998: 'SUBI', 1001: 'LSLW', 490: '.last', 1004: 'STLW', 1006: 'LDLW', 496: 'videoE', 499: 'videoF', 1014: 'DEEK', 1016: 'ANDW', 505: '.notlast', 1018: 'ORW', 507: '.join', 1020: 'XORW', 1023: 'RET'};
