/** Blinkenlights on outx register */
export class BlinkenLights {
    /** Create a new BlinkenLights
     * @param {Gigatron} cpu
     */
    constructor(cpu) {
        this.cpu = cpu;
        this.outx = cpu.outx & 0x0f;
        this.timer = null;

        for (let i = 0; i < 4; i++) {
            $(`.led-${i}`).addClass(
                `led-${(this.outx & (1<<i)) ? 'on' : 'off'}`);
        }
    }

    /** advance simulation by one tick */
    tick() {
        let outx = this.cpu.outx & 0x0f;
        let changed = this.outx ^ outx;

        if (changed) {
            for (let i = 0; i < 4; i++) {
                if (changed & (1 << i)) {
                    $(`.led-${i}`).toggleClass('led-on led-off');
                }
            }
            this.outx = outx;
        }
    }
}
