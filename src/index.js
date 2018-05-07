'use strict';

/* exported rpad, lpad, toHex */

const HZ = 6250000;
const romUrl = 'theloop.2.rom';

var cpu;
var vga;
var blinkenLights;
var audio;
var gamepad;

$(function() {
    $('[data-toggle="tooltip"]').tooltip();

    let muteButton = $('#mute');
    let unmuteButton = $('#unmute');
    let volumeSlider = $('#volume-slider');
    let vgaCanvas = $('#vga').get(0);
    let blinkenLightsCanvas = $('#blinken-lights').get(0);
    let loadFileInput = $('#load-file-input');

    /** display the error modal with the given message
     * @param {JQuery} body
     */
    function showError(body) {
        $('#error-modal-body')
            .empty()
            .append(body);
        $('#error-modal').modal();
    }

    cpu = new Gigatron({
        hz: HZ,
        romAddressWidth: 16,
        ramAddressWidth: 15,
    });

    vga = new Vga(vgaCanvas, cpu, {
        horizontal: {
            frontPorch: 16,
            backPorch: 48,
            visible: 640,
        },
        vertical: {
            frontPorch: 10,
            backPorch: 34,
            visible: 480,
        },
    });

    blinkenLights = new BlinkenLights(blinkenLightsCanvas, cpu);

    audio = new Audio(cpu);

    gamepad = new Gamepad(cpu, {
        up: ['ArrowUp'],
        down: ['ArrowDown'],
        left: ['ArrowLeft'],
        right: ['ArrowRight'],
        select: ['Q', 'q'],
        start: ['W', 'w'],
        a: ['A', 'a'],
        b: ['S', 's'],
    });

    muteButton.click(function() {
        audio.mute = true;
        $([muteButton, unmuteButton]).toggleClass('d-none');
    });

    unmuteButton.click(function() {
        audio.mute = false;
        $([muteButton, unmuteButton]).toggleClass('d-none');
    });

    volumeSlider.val(100 * audio.volume);
    volumeSlider.on('input', function(event) {
        let target = event.target;
        target.labels[0].textContent = target.value + '%';
        audio.volume = target.value / 100;
    });
    volumeSlider.trigger('input');

    let timer;
    let loader;

    loadFileInput.on('input', (event) => {
        let target = event.target;
        let file = target.files[0];
        target.labels[0].textContent = file.name;
        loader = new Loader(cpu);
        loader.load(file).subscribe({
            error: (error) => showError($(`\
                <p>\
                    Could not load GT1 from <code>${file.name}</code>\
                </p>\
                <hr>\
                <p class="alert alert-danger">\
                    <span class="oi oi-warning"></span> ${error.message}\
                </p>`)),
            complete: () => loader = null,
        });
    });

    /** start the simulation loop */
    function startRunLoop() {
        gamepad.start();

        timer = setInterval(function ticks() {
            /* advance the simulation until the audio queue is full,
             * or 1000ms of simulated time has passed.
             */
            let cycles = cpu.hz / 100;
            audio.drain();
            while (cycles-- >= 0 && !audio.full) {
                cpu.tick();
                vga.tick();
                audio.tick();
                if (loader) {
                    if (loader.tick()) {
                        break;
                    }
                }
            }
            blinkenLights.tick(); // don't need realtime update
        }, audio.duration);
    }

    /** stop the simulation loop */
    function stopRunLoop() { // eslint-disable-line
        clearTimeout(timer);
        gamepad.stop();
    }

    /** load the ROM image
     * @param {string} url
     */
    function loadRom(url) {
        var req = new XMLHttpRequest();
        req.open('GET', url);
        req.responseType = 'arraybuffer';
        req.onload = (event) => {
            if (req.status != 200) {
                showError($(`\
                    <p>\
                        Could not load ROM from <code>${url}</code>\
                    </p>\
                    <hr>\
                    <p class="alert alert-danger">\
                        <span class="oi oi-warning"></span> ${req.statusText}\
                    </p>`));
            } else {
                let dataView = new DataView(req.response);
                let wordCount = dataView.byteLength >> 1;
                // convert to host endianess
                for (let wordIndex = 0; wordIndex < wordCount; wordIndex++) {
                    cpu.rom[wordIndex] = dataView.getUint16(2 * wordIndex);
                }
                startRunLoop();
            }
        };

        req.send(null);
    }

    loadRom(romUrl);
});
