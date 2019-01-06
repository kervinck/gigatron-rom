import {
    toHex,
} from './utils.js';

/** determine if a character code is printable
 * @param {number} code - Character code to test
 * @return {boolean}
 */
function isprint(code) {
    return code >= 0x20 && code < 0x7f;
}

/** A view of RAM */
export class RamView {
    /** Create a new RamView
     * @param {HTMLTextAreaElement} textarea
     * @param {Uint8Array} memory
     */
    constructor(textarea, memory) {
        this.textarea = textarea;
        this.memory = memory;
        this.address = 0;
        this.bytesPerRow = 16;
        this.render();
    }

    /** fill the textarea with dump of ram */
    render() {
        let rows = [];
        let address = this.address;
        let memory = this.memory;

        // it seems fast enough to just dump the entire contents
        // of memory into the textarea!!
        while (address < memory.length) {
            let cols = [];
            let chars = [];
            cols.push(' ' + toHex(address, 4) + ':');
            for (let col = 0; col < this.bytesPerRow; col++) {
                if (address < memory.length) {
                    let char = String.fromCharCode(memory[address]);
                    cols.push(' ' + toHex(memory[address], 2));
                    chars.push(isprint(char.charCodeAt(0)) ? char : '.');
                    address++;
                } else {
                    cols.push('   ');
                }
            }
            cols.push(' |');
            cols = cols.concat(chars);
            cols.push('|');
            rows.push(cols.join(''));
        }
        this.textarea.value = rows.join('\n');
    }
}
