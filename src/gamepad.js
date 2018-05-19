/** map from controller button name to inReg bit */
const buttonMap = {
    a: 0x80,
    b: 0x40,
    select: 0x20,
    start: 0x10,
    up: 0x08,
    down: 0x04,
    left: 0x02,
    right: 0x01,
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
    }

    /** start handling key events */
    start() {
        $(document)
            .on('keydown', (event) => {
                let bit = this.keyMap[event.key];
                if (bit) {
                    this.pressed |= bit;
                    event.preventDefault();
                }
            })
            .on('keyup', (event) => {
                let bit = this.keyMap[event.key];
                if (bit) {
                    this.pressed &= ~bit;
                    event.preventDefault();
                }
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
