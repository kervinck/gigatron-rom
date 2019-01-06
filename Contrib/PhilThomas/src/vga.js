export const VSYNC = 0x80;
export const HSYNC = 0x40;

/** Vga */
export class Vga {
    /** Create a new Vga
     * @param {HTMLCanvasElement} canvas
     * @param {Gigatron} cpu
     * @param {Object} options
     */
    constructor(canvas, cpu, options) {
        this.canvas = canvas;
        this.canvas.width = options.horizontal.visible;
        this.canvas.height = options.vertical.visible;
        this.ctx = canvas.getContext('2d', {
            alpha: false,
        });
        this.imageData = this.ctx.getImageData(
            0, 0,
            canvas.width, canvas.height);
        this.pixels = this.imageData.data;
        this.cpu = cpu;
        this.row = 0;
        this.minRow = options.vertical.backPorch + options.vertical.pulse;
        this.maxRow = this.minRow + options.vertical.visible;
        this.col = 0;
        this.minCol = options.horizontal.backPorch + options.horizontal.pulse;
        this.maxCol = this.minCol + options.horizontal.visible;
        this.pixel = 0;
        this.out = 0;
        // turn all pixels black with full alpha
        for (let i = 0; i < this.pixels.length; i++) {
            this.pixels[i] = (i % 4) == 3 ? 255 : 0;
        }
        this.render();
    }

    /** draw the pixels into the canvas */
    render() {
        this.ctx.putImageData(this.imageData, 0, 0);
    }

    /** advance simulation by one tick */
    tick() {
        let out = this.cpu.out;
        let falling = this.out & ~out;

        if (falling & VSYNC) {
            this.row = -1; // After 4 more CPU cycles HSYNC increments row to 0
            this.pixel = 0;
            this.render();
        }

        if (falling & HSYNC) {
            this.col = 0;
            this.row++;
        }

        // Chrome optimizer put this before the falling calculation
        // if it follows immediately after it, so it got moved down here
        this.out = out;

        if ((this.row >= this.minRow && this.row < this.maxRow) &&
            (this.col >= this.minCol && this.col < this.maxCol)) {
            let pixels = this.pixels;
            let pixel = this.pixel;
            let r = (out     ) & 3;
            let g = (out >> 2) & 3;
            let b = (out >> 4) & 3;

            for (let i = 0; i < 4; i++) {
                pixels[pixel++] = 85 * r;
                pixels[pixel++] = 85 * g;
                pixels[pixel++] = 85 * b;
                pixel++;
            }

            this.pixel = pixel;
        }

        this.col += 4;
    }
}
