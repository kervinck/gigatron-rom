'use strict';

/* exported setup, keyPressed, keyReleased */

const HZ = 6250000;

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
		this.startTime = Date.now();
	}

	/** advance simulation by one tick */
	tick() {
		/* update real frequency every second of simulated time */
		if (this.cycles++ > HZ) {
			let endTime = Date.now();
			let mhz = this.cycles / (1000 * (endTime-this.startTime));
			this.elt.text(mhz.toFixed(3) + 'MHz');
			this.startTime = endTime;
			this.cycles = 0;
		}
	}
}

function rpad(value, width, pad) {
	pad = pad || ' ';
	return value + (value.length < width ? pad.repeat(width-value.length) : '');
}

function lpad(value, width, pad) {
	pad = pad || ' ';
	return (value.length < width ? pad.repeat(width-value.length) : '') + value;
}

function toHex(value, width) {
	return lpad(value.toString(16), width, '0');
}

$(function() {
	$('[data-toggle="tooltip"]').tooltip({
		delay: {
			show: 500,
			hide: 100,
		},
	});

	let mhzText = $('#mhz');
	let playButton = $('#play');
	let pauseButton = $('#pause');
	let stepButton = $('#step');
	let resetButton = $('#reset');
	let loadButton = $('#load');
	let muteButton = $('#mute');
	let unmuteButton = $('#unmute');
	let volumeSlider = $('#volume-slider');
	let vgaCanvas = $('#vga').get(0);
	let blinkenLightsCanvas = $('#blinken-lights').get(0);
	let romFileInput = $('#rom-file');
	let regsTable = $('#regs-table');
	let ramTextarea = $('#ram-textarea').get(0);
	let romDiv = $('#rom-div');

	perf = new Perf(mhzText);

	cpu = new Gigatron({
		hz: HZ,
		romAddressWidth: 16,
		ramAddressWidth: 15,
	});

	vga = new Vga(vgaCanvas, cpu, {
		horizontal: {frontPorch: 16, backPorch: 48, visible: 640},
		vertical: {frontPorch: 10, backPorch: 34, visible: 480},
	});

	blinkenLights = new BlinkenLights(blinkenLightsCanvas, cpu);

	audio = new Audio(cpu);

	gamepad = new Gamepad(cpu, {
		up:     'ArrowUp',
		down:   'ArrowDown',
		left:   'ArrowLeft',
		right:  'ArrowRight',
		select: 'q',
		start:  'w',
		a:      'a',
		b:      's',
	});

	let ramView = new RamView(ramTextarea, cpu.ram);
	let romView = new RomView(romDiv, cpu.rom);

	let gdb = {
		timer: null,
	};

	function updateView() {
		regsTable.empty();
		for (let [reg, width, click] of [['pc',     4, () => romView.render(cpu.pc)],
													    			 ['nextpc', 4, () => romView.render(cpu.nextpc)],
																		 ['ac',     2],
 																		 ['x',      2],
																		 ['y',      2],
																		 ['out',    2],
																		 ['outx',   2]]) {
			let row = $('<tr>').append([
				$('<th>').text(reg),
				$('<td>').text(toHex(cpu[reg], width)),
			])
			.appendTo(regsTable);
			if (click) {
				row.css('cursor', 'pointer');
				row.click(click);
			}
		}
		ramView.render();
		romView.hilights = {
			[cpu.pc]: 'bg-success text-dark',
			[cpu.nextpc]: 'bg-secondary text-dark',
		};
		romView.render(cpu.pc);
		vga.render();
	}
	updateView();

	let loader = new Loader(cpu);
	loadButton.click(function() { load(loader); });

	resetButton.click(function() {
		cpu.reset();
		updateView();
	});

	playButton.click(function() {
		gdb.timer = setInterval(function ticks() {
			/* advance the simulation until the audio queue is full,
			 * or 100ms of simulated time has passed.
			 */
			let cycles = this.cpu.hz/10;
			audio.drain();
			while (cycles-- >= 0 && audio.scheduled < 4) {
				perf.tick();
				cpu.tick();
				vga.tick();
				audio.tick();
			}
			/* blinkenlights don't need to redraw every tick */
			blinkenLights.tick();
		}, audio.duration);
		$([playButton, pauseButton]).toggleClass('d-none');
	});

	pauseButton.click(function() {
		clearTimeout(gdb.timer);
		gdb.timer = null;
		$([playButton, pauseButton]).toggleClass('d-none');
		updateView();
	});

	stepButton.click(function() {
		perf.tick();
		cpu.tick();
		vga.tick();
		blinkenLights.tick();
		updateView();
	});

	muteButton.click(function() {
		audio.mute = true;
		$([muteButton, unmuteButton]).toggleClass('d-none');
	});

	unmuteButton.click(function() {
		audio.mute = false;
		$([muteButton, unmuteButton]).toggleClass('d-none');
	});

	volumeSlider.on('input', function() {
		this.labels[0].textContent = this.value+'%';
		audio.volume = this.value / 100;
	});
	volumeSlider.trigger('input');

	romFileInput.change(function() {
		let file = this.files[0];
		this.labels[0].textContent = file.name;
		let fileReader = new FileReader();
		fileReader.onload = function() {
			let buffer = this.result;
			let n = buffer.byteLength >> 1;
			let view = new DataView(buffer);
			for (let i = 0; i < n; i++) {
				cpu.rom[i] = view.getUint16(i<<1);
			}
			cpu.romMask = file.size-1;
			updateView();
		};
		fileReader.readAsArrayBuffer(file);
	});
});

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
