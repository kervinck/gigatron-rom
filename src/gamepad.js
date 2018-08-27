export const BUTTON_A = 0x80;
export const BUTTON_B = 0x40;
export const BUTTON_SELECT = 0x20;
export const BUTTON_START = 0x10;
export const BUTTON_UP = 0x08;
export const BUTTON_DOWN = 0x04;
export const BUTTON_LEFT = 0x02;
export const BUTTON_RIGHT = 0x01;

/** map from controller button name to inReg bit */
const buttonMap = {
    a: BUTTON_A,
    b: BUTTON_B,
    select: BUTTON_SELECT,
    start: BUTTON_START,
    up: BUTTON_UP,
    down: BUTTON_DOWN,
    left: BUTTON_LEFT,
    right: BUTTON_RIGHT,
};

const axisThreshold = 0.5;

/** map from standard gamepad button index to inReg bit */
const gamepadButtonMap = {
    0: buttonMap.a,
    1: buttonMap.b,
    6: buttonMap.b,
    7: buttonMap.a,
    8: buttonMap.select,
    9: buttonMap.start,
    12: buttonMap.up,
    13: buttonMap.down,
    14: buttonMap.left,
    15: buttonMap.right,
};

/** map from standard gamepad axis index to negative and positive inReg bits */
const gamepadAxisMap = {
    0: [buttonMap.left, buttonMap.right],
    1: [buttonMap.up, buttonMap.down],
};

/** Gamepad device */
export class Gamepad {
    /** Create a Gamepad
     * @param {Gigatron} cpu - The cpu to control
     * @param {Object.<string,string[]>} keys - Map from controller button
     *  name to list of keys
     */
    constructor(cpu, keys) {
        this.cpu = cpu;
        this.enabled = false;
        this.pressed = 0;
        this.keyMap = {};

        /* build map from keyboard key to controller button name */
        for (let button of Object.keys(keys)) {
            for (let key of keys[button]) {
                this.keyMap[key] = buttonMap[button];
            }
        }

        /* build map of ASCII codes that the Gigatron understands as well */
        this.asciiMap = {
            'Tab': 9,
            'Enter': 10,
            'Escape': 27,
            'Esc': 27,
            'Delete': 127,
            'Backspace': 127,
        };
        for (let ascii=32; ascii<127; ascii++) {
            this.asciiMap[String.fromCharCode(ascii)] = ascii;
        }
        for (let fnKey=1; fnKey<=12; fnKey++) {
          this.asciiMap['F' + fnKey] = 0xc0 + fnKey;
        }
    }

    /** start handling key events */
    start() {
        $(document)
            .on('keydown', (event) => {
                let bit = this.keyMap[event.key];
                if (bit) {
                    this.pressed |= bit;
                    event.preventDefault();
                } else {
                    let ascii = this.asciiMap[event.key];
                    if (ascii) {
                        if (event.ctrlKey) {
                            // Control codes (e.g. Ctrl-C for ETX or BREAK)
                            if      (ascii == 63 /*'?'*/) ascii = 127;
                            else if (ascii == 32 /*' '*/) ascii = 0;
                            else                          ascii &= 31;
                        }
                        this.pressed = ascii ^ 0xff; /// will be inverted again in tick()
                        event.preventDefault();
                    }
                }
            })
            .on('keyup', (event) => {
                let bit = this.keyMap[event.key];
                if (bit) {
                    this.pressed &= ~bit;
                } else {
                    this.pressed = 0;
                }
                event.preventDefault();
            });
        this.enabled = true;
    }

    /** stop handling key events */
    stop() {
        $(document).off('keydown keyup');
        this.enabled = false;
    }

    /** check gamepads */
    tick() {
        let pressed = this.pressed;

        if (navigator.getGamepads) {
            let gamepads = navigator.getGamepads();
            for (let gamepad of gamepads) {
                if (gamepad) {
                    // check the axes
                    for (let axisIndex of Object.keys(gamepadAxisMap)) {
                        let axis = gamepad.axes[axisIndex];
                        let bits = gamepadAxisMap[axisIndex];
                        if (axis < -axisThreshold) {
                            pressed |= bits[0];
                        } else if (axis > axisThreshold) {
                            pressed |= bits[1];
                        }
                    }

                    // check the buttons
                    for (let buttonIndex of Object.keys(gamepadButtonMap)) {
                        let bit = gamepadButtonMap[buttonIndex];
                        if (gamepad.buttons[buttonIndex].pressed) {
                            pressed |= bit;
                        }
                    }
                }
            }
        }

        if (this.enabled) {
            this.cpu.inReg = pressed ^ 0xff; // active low
        }
    }
}
