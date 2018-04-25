'use strict';

/* exported RomView */

class RomView {
  constructor(textarea, rom) {
    this.textarea = textarea;
    this.rom = rom;
    this.disassembler = new Disassembler();
  }

  render() {
    let rows = [];
    let rom = this.rom;
    let disassembler = this.disassembler;

    for (let address = 0; address < rom.length; address++) {
      let instruction = rom[address];
      let decode = disassembler.disassemble(instruction);
      rows.push(` ${toHex(address, 4)} ${toHex(instruction, 4)}  ${rpad(decode.mnemonic, 4)} ${decode.operands.join(',')}`);
    }

    this.textarea.value = rows.join('\n');
  }
}
