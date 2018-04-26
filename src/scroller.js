'use strict';

class Scroller {
  constructor(container, options) {
    this.container = container;
    this.createRow = options.createRow;
    this.rowCount = options.rowCount;
    this.rows = [];
    this.scrollTopIndex = 0;
    this.scrollTop = 0;
    container.scroll(() => this.scroll());
  }

  render() {
    let rows = [];
    let container = this.container;

    container.empty();

    let height = container.height();
    let index = this.scrollTopIndex;

    for (let i = 0; i < 2; i++) {
      let spacer = $('<div>').height(0);
      rows.push(spacer);
      spacer.appendTo(container);
    }

    while (height > 0 && index < this.rowCount) {
      let row = this.createRow(index);
      rows.splice(rows.length-1, 0, row);
      row.insertBefore(rows[rows.length-1]);
      height -= row.height();
      index++;
    }

    // set spacer heights
    rows[0].height(this.scrollTopIndex * rows[1].height());
    rows[rows.length-1].height((this.rowCount - this.scrollTopIndex - rows.length + 2) * rows[rows.length-2].height());

    this.scrollTop = rows[1].position().top - container.position().top;
    container.scrollTop(this.scrollTop);
    this.rows = rows;
  }

  scroll() {
    let rows = this.rows;
    let container = this.container;
    let scrollTop = container.scrollTop();
    if (scrollTop != this.scrollTop) {
      this.scrollTopIndex = Math.max(0, Math.floor(scrollTop / rows[1].height()));
      this.render();
      this.scrollTop = scrollTop;
      container.scrollTop(scrollTop);
    }
  }
}
