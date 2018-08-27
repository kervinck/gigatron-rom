import {
    Vga,
} from './vga.js';
import {
    BlinkenLights,
} from './blinkenlights.js';
import {
    Gigatron,
} from './gigatron.js';
import {
    Gamepad,
} from './gamepad.js';
import {
    Audio,
} from './audio.js';
import {
    Loader,
} from './loader.js';

const {
    finalize,
} = rxjs.operators;

const HZ = 6250000;
const romUrl = 'gigatron.rom';

$(function() {
    $('[data-toggle="tooltip"]').tooltip();

    let muteButton = $('#mute');
    let unmuteButton = $('#unmute');
    let volumeSlider = $('#volume-slider');
    let vgaCanvas = $('#vga-canvas');
    let loadFileInput = $('#load-file-input');

    /** Trigger a keydown/keyup event in response to a mousedown/mouseup event
     * @param {JQuery} $button
     * @param {string} key
     */
    function bindKeyToButton($button, key) {
        $button
            .on('mousedown', (event) => {
                event.preventDefault();
                document.dispatchEvent(new KeyboardEvent('keydown', {
                    'key': key,
                }));
                $button.addClass('pressed');
            })
            .on('mouseenter', (event) => {
                event.preventDefault();
                if (event.originalEvent.buttons & 1) {
                    document.dispatchEvent(new KeyboardEvent('keydown', {
                        'key': key,
                    }));
                    $button.addClass('pressed');
                }
            })
            .on('mouseup mouseleave', (event) => {
                event.preventDefault();
                document.dispatchEvent(new KeyboardEvent('keyup', {
                    'key': key,
                }));
                $button.removeClass('pressed');
            });
    }

    bindKeyToButton($('.gamepad-btn-a'), 'Delete');
    bindKeyToButton($('.gamepad-btn-b'), 'Insert');
    bindKeyToButton($('.gamepad-btn-start'), 'PageUp');
    bindKeyToButton($('.gamepad-btn-select'), 'PageDown');
    bindKeyToButton($('.gamepad-btn-up'), 'ArrowUp');
    bindKeyToButton($('.gamepad-btn-down'), 'ArrowDown');
    bindKeyToButton($('.gamepad-btn-left'), 'ArrowLeft');
    bindKeyToButton($('.gamepad-btn-right'), 'ArrowRight');

    // jQuery targets of current touches indexed by touch identifier
    let $touchTargets = {};

    // track touches within the fc30 and map them to mouse events
    $('.gamepad')
        .on('touchstart', (event) => {
            event.preventDefault();
            for (let touch of event.changedTouches) {
                let $currTarget = $(document.elementFromPoint(
                        touch.clientX, touch.clientY))
                    .filter('.gamepad-btn');
                $touchTargets[touch.identifier] = $currTarget;
                $currTarget.trigger('mousedown');
                if ($currTarget.length > 0 && navigator.vibrate) {
                    navigator.vibrate(20);
                }
            }
        })
        .on('touchmove', (event) => {
            event.preventDefault();
            for (let touch of event.changedTouches) {
                let $prevTarget = $touchTargets[touch.identifier];
                let $currTarget = $(document.elementFromPoint(
                        touch.clientX, touch.clientY))
                    .filter('.gamepad-btn');
                if ($prevTarget.get(0) != $currTarget.get(0)) {
                    $prevTarget.trigger('mouseup');
                    $touchTargets[touch.identifier] = $currTarget;
                    $currTarget.trigger('mousedown');
                    if ($currTarget.length > 0 && navigator.vibrate) {
                        navigator.vibrate(20);
                    }
                }
            }
        })
        .on('touchend touchcancel', (event) => {
            event.preventDefault();
            for (let touch of event.changedTouches) {
                let $prevTarget = $touchTargets[touch.identifier];
                $prevTarget.trigger('mouseup');
                delete $touchTargets[touch.identifier];
            }
        });

    /** display the error modal with the given message
     * @param {JQuery} body
     */
    function showError(body) {
        $('#error-modal-body')
            .empty()
            .append(body);
        $('#error-modal').modal();
    }

    let cpu = new Gigatron({
        hz: HZ,
        romAddressWidth: 16,
        ramAddressWidth: 15,
    });

    let vga = new Vga(vgaCanvas.get(0), cpu, {
        horizontal: {
            frontPorch: 16,
            pulse: 96,
            backPorch: 48,
            visible: 640,
        },
        vertical: {
            frontPorch: 6,
            pulse: 8,
            backPorch: 27,
            visible: 480,
        },
    });

    let blinkenLights = new BlinkenLights(cpu);

    let audio = new Audio(cpu);

    let gamepad = new Gamepad(cpu, {
        up: ['ArrowUp'],
        down: ['ArrowDown'],
        left: ['ArrowLeft'],
        right: ['ArrowRight'],
        select: ['PageDown'],
        start: ['PageUp'],
        a: ['Delete', 'Backspace', 'End'],
        b: ['Insert', 'Home'],
    });

    let loader = new Loader(cpu);

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

    /** load a GT1 file
     * @param {File} file
     */
    function loadGt1(file) {
        gamepad.stop();
        loader.load(file)
            .pipe(finalize(() => {
                gamepad.start();
            }))
            .subscribe({
                error: (error) => showError($(`\
                <p>\
                    Could not load GT1 from <code>${file.name}</code>\
                </p>\
                <hr>\
                <p class="alert alert-danger">\
                    <span class="oi oi-warning"></span> ${error.message}\
                </p>`)),
            });
    }

    loadFileInput
        .on('click', (event) => {
            loadFileInput.closest('form').get(0).reset();
        })
        .on('change', (event) => {
            let target = event.target;
            if (target.files.length != 0) {
                let file = target.files[0];
                loadGt1(file);
            }
        });

    $(document)
        .on('dragenter', (event) => {
            event.preventDefault();
            event.stopPropagation();
            let dataTransfer = event.originalEvent.dataTransfer;
            dataTransfer.dropEffect = 'link';
        })
        .on('dragover', (event) => {
            event.preventDefault();
            event.stopPropagation();
        })
        .on('drop', (event) => {
            let dataTransfer = event.originalEvent.dataTransfer;
            if (dataTransfer) {
                let files = dataTransfer.files;
                if (files.length != 0) {
                    event.preventDefault();
                    event.stopPropagation();
                    loadGt1(files[0]);
                }
            }
        });

    /** start the simulation loop */
    function startRunLoop() {
        gamepad.start();

        timer = setInterval(() => {
            /* advance the simulation until the audio queue is full,
             * or 10ms of simulated time has passed.
             */
            let cycles = cpu.hz / 100;
            audio.drain();
            while (cycles-- >= 0 && !audio.full) {
                cpu.tick();
                vga.tick();
                audio.tick();
                loader.tick();
            }
            blinkenLights.tick(); // don't need realtime update
            gamepad.tick();
        }, audio.duration);

        audio.context.resume();

        // Chrome suspends the AudioContext on reload
        // and doesn't allow it to be resumed unless there
        // is user interaction
        if (audio.context.state === 'suspended') {
            vga.ctx.fillStyle = 'white';
            vga.ctx.textAlign = 'center';
            vga.ctx.textBaseline = 'middle';
            vga.ctx.font = '4em sans-serif';
            vga.ctx.fillText('Click to start', 320, 240);
            vgaCanvas.on('click', (event) => {
                audio.context.resume();
                vgaCanvas.off('click');
            });
        }
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
