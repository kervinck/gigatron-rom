import {
    toHex,
} from './utils.js';

/** Gigatron Disassembler */
export class Disassembler {
    /**
     * The result of disassembly
     * @typedef {Object} Parts
     * @property {string} mnemonic
     * @property {string[]} operands
     */

    /** disassemble instruction
     * @param {number} instruction
     * @return {Parts}
     */
    disassemble(instruction) {
        let op = (instruction >> 13) & 7;
        let mode = (instruction >> 10) & 7;
        let bus = (instruction >> 8) & 3;
        let d = (instruction >> 0) & 0xff;

        switch (op) {
            case 0:
                return this._decodeAlu(mode, bus, d, 'ld');
            case 1:
                return this._decodeAlu(mode, bus, d, 'anda');
            case 2:
                return this._decodeAlu(mode, bus, d, 'ora');
            case 3:
                return this._decodeAlu(mode, bus, d, 'xora');
            case 4:
                return this._decodeAlu(mode, bus, d, 'adda');
            case 5:
                return this._decodeAlu(mode, bus, d, 'suba');
            case 6:
                return this._decodeStore(mode, bus, d);
            case 7:
                return this._decodeBranch(mode, bus, d);
        }
    }

    /** decode a ram address
     * @param {number} mode
     * @param {number} d
     * @return {string}
     */
    _decodeAddr(mode, d) {
        switch (mode) {
            case 0:
            case 4:
            case 5:
            case 6:
                return '$' + toHex(d, 2);
            case 1:
                return 'x';
            case 2:
                return 'y,' + '$' + toHex(d, 2);
            case 3:
                return 'y,x';
            case 7:
                return 'y,x++';
        }
    }

    /** decode an alu op
     * @param {number} mode
     * @param {number} bus
     * @param {number} d
     * @param {number} mnemonic
     * @return {Parts}
     */
    _decodeAlu(mode, bus, d, mnemonic) {
        let operands = [];

        switch (bus) {
            case 0:
                operands.push('$' + toHex(d, 2));
                break;
            case 1:
                operands.push('[' + this._decodeAddr(mode, d) + ']');
                break;
            case 2:
                operands.push('ac');
                break;
            case 3:
                operands.push('in');
                break;
        }

        switch (mode) {
            case 4:
                operands.push('x');
                break;
            case 5:
                operands.push('y');
                break;
            case 6:
            case 7:
                operands.push('out');
                break;
        }

        return {
            mnemonic: mnemonic,
            operands: operands,
        };
    }

    /** decode a store op
     * @param {number} mode
     * @param {number} bus
     * @param {number} d
     * @return {Parts}
     */
    _decodeStore(mode, bus, d) {
        let operands = [];

        switch (bus) {
            case 0:
                operands.push('$' + toHex(d, 2));
                break;
            case 1:
                operands.push('$??');
                break;
            case 3:
                operands.push('in');
                break;
        }

        operands.push('[' + this._decodeAddr(mode, d) + ']');

        switch (mode) {
            case 4:
                operands.push('x');
                break;
            case 5:
                operands.push('y');
                break;
        }

        return {
            mnemonic: 'st',
            operands: operands,
        };
    }

    /** decode a branch op
     * @param {number} mode
     * @param {number} bus
     * @param {number} d
     * @return {Parts}
     */
    _decodeBranch(mode, bus, d) {
        let operands = [];
        let mnemonic;

        switch (mode) {
            case 0:
                mnemonic = 'jmp';
                operands.push('y');
                break;
            case 1:
                mnemonic = 'bgt';
                break;
            case 2:
                mnemonic = 'bge';
                break;
            case 3:
                mnemonic = 'bne';
                break;
            case 4:
                mnemonic = 'beq';
                break;
            case 5:
                mnemonic = 'blt';
                break;
            case 6:
                mnemonic = 'ble';
                break;
            case 7:
                mnemonic = 'bra';
                break;
        }

        switch (bus) {
            case 0:
                operands.push('$' + toHex(d, 2));
                break;
            case 1:
                operands.push('[$' + toHex(d, 2) + ']');
                break;
            case 2:
                operands.push('ac');
                break;
            case 3:
                operands.push('in');
                break;
        }

        return {
            mnemonic: mnemonic,
            operands: operands,
        };
    }
}
