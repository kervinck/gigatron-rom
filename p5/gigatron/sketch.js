"use strict";

var cpu;
var vga;
var blinkenLights;
var gamepad;
var perf;

class Perf {
	constructor(elt) {
		this.elt = elt;
		this.cycles = 0;
		this.startTime = millis();
	}

	refresh() {
		if (this.cycles > 1000000) {
			let endTime = millis();
			let mhz = this.cycles / (1000 * (endTime-this.startTime));
			this.elt.html(nf(mhz, 1, 3) + "MHz");
			this.startTime = endTime;
			this.cycles = 0;
		}
	}
}

function setup() {
	let mhzText = createElement('h2', "--");
	perf = new Perf(mhzText);

	createCanvas(640, 480 + 44);
	noLoop();

	vga = new Vga({
		horizontal: { frontPorch: 16, backPorch: 48, visible: 640 },
		vertical:   { frontPorch: 10, backPorch: 34, visible: 480 },
	});

  blinkenLights = new BlinkenLights();

	cpu = new Gigatron({
		log2rom: 16,
		log2ram: 15,
		onOut:   out => vga.onOut(out),
		onOutx:  outx => blinkenLights.onOutx(outx),
	});

	gamepad = new Gamepad(cpu, {
		up:     UP_ARROW,
		down:   DOWN_ARROW,
		left:   LEFT_ARROW,
		right:  RIGHT_ARROW,
		select: RETURN,
		start:  " ".codePointAt(0),
		a:      "A".codePointAt(0),
		b:      "B".codePointAt(0),
	});

	loadRom(cpu);
}

function start() {
	setInterval(tick, 1000/60);
}

function tick() {
	vga.vsyncOccurred = false;

	// step simulation until next vsync (hope there is one!)
	while(!vga.vsyncOccurred) {
		cpu.tick();
		vga.tick();
		perf.cycles++;
	}

	vga.refresh();
	perf.refresh();
}

function keyPressed() {	return gamepad.keyPressed(keyCode); }
function keyReleased() { return gamepad.keyReleased(keyCode); }

function loadRom(cpu) {
	let oReq = new XMLHttpRequest();
	oReq.open("GET", "ROMv1.rom", true);
	oReq.responseType = "arraybuffer";

	oReq.onload = function (oEvent) {
		let buffer = oReq.response;
		if (buffer) {
			let n = buffer.byteLength >> 1;
			let view = new DataView(buffer);
			for (let i = 0; i < n; i++) {
				cpu.rom[i] = view.getUint16(i<<1);
			}
			console.log("ROM loaded");
			start();
		}
	};

	oReq.send(null);
}
