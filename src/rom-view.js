import {
    rpad,
    toHex,
} from './utils.js';

/** A view of ROM */
export class RomView {
    /** Create a new RomView
     * @param {HTMLDivElement} view
     * @param {Uint16Array} rom
     */
    constructor(view, rom) {
        this.view = view;
        this.rom = rom;
        this.disassembler = new Disassembler();
        this.scroller = new Scroller(view, {
            createRow: (index) => this._createRow(index),
            numRows: rom.length,
        });
        this.hilights = {};
    }

    /** create a row for the scroller
     * @param {number} index
     * @return {HTMLElement}
     */
    _createRow(index) {
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

    /** redraw the view at the given start address
     * @param {number} startAddress
     */
    render(startAddress) {
        let top = Math.floor(startAddress - this.scroller.numVisibleRows / 2);
        this.scroller.scrollTop(top);
    }
}
