/** right pad a string
 * @param {string} value - The value to pad
 * @param {number} width - The width to pad to
 * @param {string} [pad=' '] - The pad string
 * @return {string} The padded string
 */
export function rpad(value, width, pad) {
    pad = pad || ' ';
    if (value.length < width) {
        value = value + pad.repeat(width - value.length);
    }
    return value;
}

/** left pad a string
 * @param {string} value - The value to pad
 * @param {number} width - The width to pad to
 * @param {string} [pad=' '] - The pad string
 * @return {string} The padded string
 */
export function lpad(value, width, pad) {
    pad = pad || ' ';
    if (value.length < width) {
        value = pad.repeat(width - value.length) + value;
    }
    return value;
}

/** convert a number to a zero-padded hex string
 * @param {number} value - The number to convert to hex
 * @param {number} width - The width of the result
 * @return {string} The padded hex string
 */
export function toHex(value, width) {
    return lpad(value.toString(16), width, '0');
}
