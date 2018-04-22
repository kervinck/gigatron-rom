'use strict';

/* exported setup, keyPressed, keyReleased */

var cpu;
var vga;
var blinkenLights;
var audio;
var gamepad;
var perf;

/** Performance Monitor */
class Perf {
	/** Create a Performance Monitor
	 * @param {Element} elt - The element in which to display performance
	 */
	constructor(elt) {
		this.elt = elt;
		this.cycles = 0;
		this.startTime = millis();
	}

	/** advance simulation by one tick */
	tick() {
		if (this.cycles++ > 5000000) {
			let endTime = millis();
			let mhz = this.cycles / (1000 * (endTime-this.startTime));
			this.elt.html(nf(mhz, 1, 3) + 'MHz');
			this.startTime = endTime;
			this.cycles = 0;
		}
	}
}

/** p5 setup function */
function setup() {
	let mhzText = createElement('h2', '--');
	let schedText = createElement('h2', '--');
	let loadButton = createButton('Load');
	let muteCheckbox = createCheckbox('Mute', false);

	createCanvas(640, 480 + 44);
	noLoop();

	perf = new Perf(mhzText);

	cpu = new Gigatron({
		log2rom: 16,
		log2ram: 15,
	});

	vga = new Vga(cpu, {
		horizontal: {frontPorch: 16, backPorch: 48, visible: 640},
		vertical: {frontPorch: 10, backPorch: 34, visible: 480},
	});

	blinkenLights = new BlinkenLights(cpu);

	audio = new Audio(cpu, schedText);

	gamepad = new Gamepad(cpu, {
		up: UP_ARROW,
		down: DOWN_ARROW,
		left: LEFT_ARROW,
		right: RIGHT_ARROW,
		select: RETURN,
		start: ' '.codePointAt(0),
		a: 'A'.codePointAt(0),
		b: 'B'.codePointAt(0),
	});

	let loader = new Loader(cpu);
	loadButton.mousePressed(() => load(loader));
	muteCheckbox.changed(function() {
		audio.mute = this.checked();
	});

	const romurl = 'theloop.2.rom';
	loadRom(romurl, cpu);
}

/** load blinky program
 * @param {Loader} loader
*/
function load(loader) {
	console.log('Loading');

	/* eslint-disable no-multi-spaces, max-len */
	loader.load({
		startAddress: 0x7f00,
		blocks: [
			{
				address: 0x7f00,
				bytes: [
					0x11, 0x50, 0x44, // 7f00 LDWI $4450  ; Load address of center of screen
					0x2b, 0x30,       // 7f03 STW  'p'    ; Store in variable 'p' (at $0030)
					0xf0, 0x30,       // 7f05 POKE 'p'    ; Write low byte of accumulator there
					0xe3, 0x01,       // 7f07 ADDI 1      ; Increment accumulator
					0x90, 0x03,       // 7f09 BRA  $7f05  ; Loop forever
                                      // 7f0b
				],
			},
		],
	});
	/* eslint-enable no-multi-spaces, max-len */

	console.log('Loaded');
}

/** start the periodic */
function start() {
	setInterval(ticks, 2*audio.buffers[0].duration);
}

/** advance the simulation by many ticks */
function ticks() {
	while (audio.scheduled < 4) {
		//perf.tick();
		cpu.tick();
		vga.tick();
		//blinkenLights.tick();
		audio.tick();
	}
	audio.drain();
}

/** KeyPressed event handler
 * @return {boolean} whether event should be default processed
*/
function keyPressed() {
	return gamepad.keyPressed(keyCode);
}

/** KeyReleased event handler
 * @return {boolean} whether event should be default processed
*/
function keyReleased() {
	return gamepad.keyReleased(keyCode);
}

/** async rom loader
 * @param {string} url - Url of rom file
 * @param {Gigatron} cpu - CPU to load
 */
function loadRom(url, cpu) {
	let oReq = new XMLHttpRequest();
	oReq.open('GET', url, true);
	oReq.responseType = 'arraybuffer';

	oReq.onload = function(oEvent) {
		let buffer = oReq.response;
		if (buffer) {
			let n = buffer.byteLength >> 1;
			let view = new DataView(buffer);
			for (let i = 0; i < n; i++) {
				cpu.rom[i] = view.getUint16(i<<1);
			}
			console.log('ROM loaded');
			start();
		}
	};

	oReq.send(null);
}
