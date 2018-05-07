/** A class to manage scrolling a dynamically generated list */
export class Scroller {
    /** Create a new Scroller
     * @param {HTMLElement} container
     * @param {{createRow: function, numRows: number}} options
     */
    constructor(container, options) {
        this.container = container;
        this.createRow = options.createRow;
        this.numRows = options.numRows;
        this.numVisibleRows = 0;
        this.scrollIndex = 0;
        container.on('wheel', (event) => this.onwheel(event));
    }

    /** redraw the rows */
    render() {
        let container = this.container;

        container.empty();

        let height = container.height();
        let index = this.scrollIndex;

        this.numVisibleRows = 0;

        while (height > 0) {
            let row = this.createRow(index);
            if (row == null) {
                break;
            }
            container.append(row);
            height -= row.height();
            index++;
            this.numVisibleRows++;
        }

        if (height < 0) {
            this.numVisibleRows--;
        }
    }

    /** set the index of the topmost visible row
     * @param {number} index
     */
    scrollTop(index) {
        this.scrollIndex = Math.max(0,
            Math.min(this.numRows - this.numVisibleRows,
                index));
        this.render();
    }

    /** respond to wheel events
     * @param {Event} event
     */
    onwheel(event) {
        event.preventDefault();

        let magnitude = (event.altKey && event.shiftKey) ? 0x1000 :
            event.altKey ? 0x0100 :
            event.shiftKey ? 0x0010 :
            0x0001;

        let delta = Math.sign(event.originalEvent.deltaY) * magnitude;
        this.scrollTop(this.scrollIndex + delta);
    }
}
