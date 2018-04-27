'use strict';

/* exported RomView */

class RomView {
  constructor(view, rom) {
    this.view = view;
    this.rom = rom;
    this.disassembler = new Disassembler();
    this.scroller = new Scroller(view, {
      createRow: (index) => this.createRow(index),
      numRows: rom.length,
    });
    this.hilights = {};
  }

  createRow(index) {
    if (index < 0 || index >= this.rom.length) {
      return null;
    }

    let instruction = this.rom[index];
    let decode = this.disassembler.disassemble(instruction);

    return $('<div>')
      .addClass(this.hilights[index] || '')
      .html(toHex(index, 4) + '&nbsp;' +
            toHex(instruction, 4) + '&nbsp;&nbsp;' +
            rpad(decode.mnemonic, 4, '&nbsp;') + '&nbsp;' +
            decode.operands.join(','));
  }

  render(startAddress) {
    this.scroller.scrollTop(Math.floor(startAddress-this.scroller.numVisibleRows/2));
  }
}
