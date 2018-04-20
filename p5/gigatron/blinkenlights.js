"use strict";

class BlinkenLights {
	onOutx(outx) {
		for (let i = 0; i < 4; i++) {
			fill((outx & (1 << i)) ? 255 : 100, 0, 0);
			noStroke();
			ellipse(320 + 32*(i-2), 490 + 17, 30, 30);
		}
	}
}
