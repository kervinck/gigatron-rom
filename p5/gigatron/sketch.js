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

function toHex(value, width) {
	let s = value.toString(16);
	if (s.length < width) {
		s = '0'.repeat(width-s.length) + s;
	}
	return s;
}

/** p5 setup function */
function setup() {
	let mhzText = select('#mhz');
	let runButton = select('#run');
	let stepButton = select('#step');
	let resetButton = select('#reset');
	let loadButton = select('#load');
	let muteButton = select('#mute');
	let volumeSlider = select('#volume');
	let volumeLabel = select('#volume-label');
	let vgaCanvas = select('#vga');
	let vgaCtx = vgaCanvas.elt.getContext('2d');
	let blinkenLightsCanvas = select('#blinken-lights').elt;
	let romFileInput = select('#rom-file');
	let pcSpan = select('#reg-pc');
	let nextpcSpan = select('#reg-nextpc');
	let acSpan = select('#reg-ac');
	let xSpan = select('#reg-x');
	let ySpan = select('#reg-y');
	let outSpan = select('#reg-out');
	let outxSpan = select('#reg-outx');

	noLoop();

	perf = new Perf(mhzText);

	cpu = new Gigatron({
		log2rom: 16,
		log2ram: 15,
	});

	vga = new Vga(vgaCtx, cpu, {
		horizontal: {frontPorch: 16, backPorch: 48, visible: 640},
		vertical: {frontPorch: 10, backPorch: 34, visible: 480},
	});

	blinkenLights = new BlinkenLights(blinkenLightsCanvas, cpu);

	audio = new Audio(cpu);

	gamepad = new Gamepad(cpu, {
		up: UP_ARROW,
		down: DOWN_ARROW,
		left: LEFT_ARROW,
		right: RIGHT_ARROW,
		select: 'Q'.codePointAt(0),
		start: 'W'.codePointAt(0),
		a: 'A'.codePointAt(0),
		b: 'S'.codePointAt(0),
	});

	let gdb = {
		timer: null,
	};

	function updateRegs() {
		pcSpan.html('$'+toHex(cpu.pc, 4));
		nextpcSpan.html('$'+toHex(cpu.nextpc, 4));
		acSpan.html('$'+toHex(cpu.ac, 2));
		xSpan.html('$'+toHex(cpu.x, 2));
		ySpan.html('$'+toHex(cpu.y, 2));
		outSpan.html('$'+toHex(cpu.out, 2));
		outxSpan.html('$'+toHex(cpu.out, 2));
	}

	updateRegs();

	let loader = new Loader(cpu);
	loadButton.mousePressed(function() { load(loader); });
	resetButton.mousePressed(function() {
		cpu.reset();
		updateRegs();
	});

	runButton.mousePressed(function() {
		if (gdb.timer) {
			clearTimeout(gdb.timer);
			gdb.timer = null;
			this.elt.textContent = 'Go';
			this.removeClass('btn-danger');
			this.addClass('btn-success');
			updateRegs();
		}
		else {
			gdb.timer = setInterval(ticks, audio.duration);
			this.elt.textContent = "Stop";
			this.addClass('btn-danger');
			this.removeClass('btn-success');
		}
	});

	stepButton.mousePressed(function() {
		perf.tick();
		cpu.tick();
		vga.tick();
		blinkenLights.tick();
		updateRegs();
	});

	muteButton.mousePressed(function() {
		audio.mute = !audio.mute;
		if (audio.mute) {
			this.elt.textContent = 'Unmute';
			this.removeClass('btn-danger');
			this.addClass('btn-success');
		}
		else {
			this.elt.textContent = 'Mute';
			this.addClass('btn-danger');
			this.removeClass('btn-success');
		}
	});

	volumeSlider.input(function() {
		volumeLabel.html(volumeSlider.value()+'%');
		audio.volume = volumeSlider.value() / 100;
	});

	romFileInput.input(function() {
		let file = this.elt.files[0];
		this.elt.labels[0].textContent = file.name;
		let fileReader = new FileReader();
		fileReader.onload = function() {
			let buffer = this.result;
			let n = buffer.byteLength >> 1;
			let view = new DataView(buffer);
			for (let i = 0; i < n; i++) {
				cpu.rom[i] = view.getUint16(i<<1);
			}
			cpu.romMask = file.size-1;
		};
		fileReader.readAsArrayBuffer(file);
	});
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

/** advance the simulation by many ticks */
function ticks() {
	while (audio.scheduled < 4) {
		perf.tick();
		cpu.tick();
		vga.tick();
		audio.tick();
	}
	blinkenLights.tick();
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
