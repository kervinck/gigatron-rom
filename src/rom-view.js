'use strict';

/* exported RomView */

class RomView {
  constructor(view, rom) {
    this.view = view;
    this.rom = rom;
    this.disassembler = new Disassembler();
  }

  render(startAddress, hilights) {
    this.view.empty();

    let rom = this.rom;
    let disassembler = this.disassembler;

    for (let address = startAddress; address < startAddress+50 /*rom.length*/; address++) {
      let instruction = rom[address];
      let decode = disassembler.disassemble(instruction);
        $('<tr>')
          .addClass(hilights[address] || '')
          .append([
            $('<td>').text(toHex(address, 4)),
            $('<td>').text(toHex(instruction, 4)),
            $('<td>').text(`${rpad(decode.mnemonic, 4)} ${decode.operands.join(',')}`)])
          .appendTo(this.view);
    }
  }
}
