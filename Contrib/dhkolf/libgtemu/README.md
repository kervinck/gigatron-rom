
libgtemu -- David Kolf's Library for Gigatron Emulation
=======================================================

Version 0.2.0

About
-----

libgtemu provides functions for emulating the [Gigatron TTL Color Computer][gt]
and rendering its graphics through the [SDL library][sdl].

It can be used as a lightweight stand-alone emulator as well as being
controlled by a scripting language such as [Lua][lua].

[gt]:  https://gigatron.io/
[sdl]: http://www.libsdl.org/
[lua]: https://www.lua.org/

License
-------

Copyright (C) 2019 [David Heiko Kolf](http://dkolf.de/).

Published under the [BSD 2-Clause License][lic].

[lic]: https://opensource.org/licenses/BSD-2-Clause

Compiling
---------

You will probably have to modify the provided Makefile to fit to your
system, but there are only a few files and few dependencies.  As an example
for compiling Lua bindings you might look at [LPEG][lpeg].

[lpeg]: http://www.inf.puc-rio.br/~roberto/lpeg/#download

### Files

  *  Main library: gtemu.c, gtloader.c and gtsdl.c
  *  Example application: gtmain.c
  *  Lua bindings: luagt.c

The main library could also be compiled without gtsdl.c if you want to
use a different library for graphics and sound.

### Dependencies

  *  [Core SDL][sdl] at least version 2.0.4.
  *  [Lua][lua] version 5.3 (for the Lua bindings).

Future
------

The following features might be nice to have:

  *  Python bindings: Python is used for most other scripts related to the
     Gigatron, so it would be nice to be able to use this emulator from
     Python as well.
  *  Proper Makefile and precompiled binaries for Windows.
  *  More RAM aliases in the script bindings.
  *  IIR audio filters to replicate the sound output of the Gigatron more
     precisely.  According to the [Gigatron schematics][scm] (page 8/8)
     there are two single order filters in series, a low pass at 700 Hz
     and a high pass at 160 Hz that overlap to reduce the peak volume by a
     factor of 5.

[scm]: https://cdn.hackaday.io/files/20781889094304/Schematics.pdf

Stand-alone emulator
--------------------

The stand-alone emulator offers the following basic features on the
command line:

	usage: ./gtrun [-h] [-l filename.gt1] [-64] [filename.rom]

	Arguments:
	 -h               Display this help.
	 -l filename.gt1  File to be sent with Ctrl-F2.
	 -64              Expand RAM to 64k.
	    filename.rom  ROM file (default name: gigatron.rom).

	Special keys:
	    Ctrl-F2       Send designated GT1 file.
	    Alt-L         Perform hard reset and select loader.
	    ESC           Close the emulation.


Structure and C-API
-------------------

This library is split into two parts: pure emulation functions without
dependencies and SDL rendering functions.  You can use the emulation
functions directly and use other means to output graphics and sound.

### Emulation and peripherals

The core emulation is written without external dependencies and just
performs the raw computations.  Its functions are defined in the header
file gtemu.h.

gtemu.h defines two major structures: `GTState` and `GTPeriph`.  `GTState`
containes the state of the CPU and its fields are open for inspection and
manipulation:

	struct GTRomEntry {
		unsigned char i;
		unsigned char d;
	};

	struct GTState {
		int pc;
		unsigned char ir, d, ac, x, y, out, in;
		struct GTRomEntry *rom;
		size_t romcount;
		unsigned char *ram;
		unsigned int rammask;
	};

`GTPeriph` on the other hand contains the state of the peripherals; the
board, the video output, the audio output, the GT1 loader and the serial
output.  The variables are implementation details and should not be
accessed directly.

	struct GTPeriph;

#### gtemu_init

	extern void gtemu_init (struct GTState *gt,
		struct GTRomEntry *rom, size_t romsize,
		unsigned char *ram, size_t ramsize);

Initializes the contents of the `GTState` structure and sets
pointers to the given RAM and ROM arrays.  You can (and should) initialize
the content of those arrays.  `romsize` is the size in bytes, not the number
of entries.

#### gtemu_initperiph

	extern void gtemu_initperiph (struct GTPeriph *ph, int audiofreq,
		unsigned long randseed);

Initializes the state of the simulated peripherals.
`audiofreq` should be a valid value even if you do not want to output the
sound (just set it to 48000).  Otherwise you can get it from the SDL
functions.  `randseed` is a seed value for the random number generator.

#### gtemu_randomizemem

	unsigned long gtemu_randomizemem (unsigned long seed,
		void *mem, size_t size);

Randomizes a designated area of memory (like the RAM or the ROM). The
return value is the new state of the xorshift32-random number generator.

#### gtemu_getclock

	extern unsigned long long gtemu_getclock (struct GTPeriph *ph);

Returns the total number of instructions executed so far.

#### gtemu_getxout

	extern unsigned char gtemu_getxout (struct GTPeriph *ph);

Returns the current state of the XOUT register (the "blinkenlights"
and the sound output).

#### gtemu_processtick

	extern int gtemu_processtick (struct GTState *gt, struct GTPeriph *ph);

Advances the emulation one tick further.  This does not output any graphics
or sound but still keeps track of the position of the video beam.

It returns the value of rising edges on the synchronization signals in OUT.

#### gtemu_processscreen

	extern int gtemu_processscreen (struct GTState *gt, struct GTPeriph *ph,
		void *pixels, int pitch,
		unsigned short *samples, size_t maxsamples, size_t *nsamples);

Advances the simulation until either the next rising edge of the VSync
signal was detected or 110000 steps were processed.

This function is called by internally by `gtsdl_render` (and
`gtsdl_runuiframe`), you only need to call it when not using one of those
functions.

It will output the pixels in a 16-bit RGB444 format, the pitch variable
determines the length of one row in bytes.  The pixel and pitch arguments
are chosen to be compatible with the `SDL_LockTexture` function from
[SDL][sdl].

The sound samples are expected to be 16-bit in native byte order. This
function will output only positive 15-bit numbers to be compatible with
both unsigned and signed formats.

The pointers `pixels` and `samples` may be NULL if you are not interested
in the output.

`nsamples` points to a variable with the current position inside the samples
array and must not be NULL, even if no sound is being played.  Just let
it point to a dummy variable on the stack.

The return value is the number of instructions that were executed.

#### gtemu_placelights

	void gtemu_placelights (struct GTPeriph *ph, void *pixels, int pitch,
		int power);

Places LEDs as pixels on the bottom of the screen.  The positions are
chosen to not obscure any pixel of the emulation completely and the colors
are chosen from outside the palette available to Gigatron applications.

This function is called internally by `gtsdl_render` (and
`gtsdl_runuiframe`), you only need to use it when not using one of those
functions.

`power` indicates the state of the power LED.  The default rendering function
uses it to indicate that the frame rate is at the expected 60 FPS.

#### gtserialout_setbuffer

	void gtserialout_setbuffer (struct GTPeriph *ph, char *buffer,
		size_t buffersize, size_t *bufferpos);

Sets the buffer into which emulated programs can write data by manipulating
the synchronization signals.  `bufferpos` points to a variable containing
the current index into the buffer.  You can change that variable at any
time to make place for new data.

TinyBasic on the Gigatron sends an empty newline (`\n`) when it wants to
clear the buffer and start a new output.  In this emulation the empty line
is just placed into the buffer as well without having any built-in special
effects.

#### gtloader_sendgt1

	extern int gtloader_sendgt1 (struct GTPeriph *ph,
		const char *data, size_t datasize);

Instructs the emulation to send the contents of a GT1 file to the emulated
Gigatron that can be evaluated by the Loader program.  This function does
not check whether the GT1 is valid and expects the Loader application to be
running already.

The memory pointed to by `data` must stay valid until everything is sent.

It returns 1 on success and 0 on failure when there is still previous data
that was not completely sent.

#### gtloader_isactive

	extern int gtloader_isactive (struct GTPeriph *ph);

Returns whether the loader is active at the moment.

#### gtloader_validategt1

	extern int gtloader_validategt1 (const char *data, size_t datasize);

Validates a GT1 file.

The return value is 1 if the contents were valid, 0 otherwise.

### SDL

The necessary resources for SDL and some state are kept in the GTSDLState
structure.  You should access its content only through the provided
functions.

	struct GTSDLState;

#### gtsdl_openwindow

	extern int gtsdl_openwindow (struct GTSDLState *s, const char *title);

Initializes the necessary SDL subsystems (you should call `SDL_Init` before)
and creates a standalone window.

It returns 1 in case of success and 0 in case of errors.  The error can
be requested using `SDL_GetError`.

#### gtsdl_setup

	extern int gtsdl_setup (struct GTSDLState *s, SDL_Renderer *renderer,
		SDL_AudioDeviceID audiodev, SDL_AudioSpec *audiospec);

This function is an alternative to `gtsdl_openwindow` in case your
application initialized SDL and created an window on its own.

The audiospec has to be 16-bit integer without a callback function.

In case you want to mix the audio you could avoid the provided SDL
functions of this library and just call `gtemu_processscreen` directly in
your own sound and video rendering function.

It returns 1 in case of success and 0 in case of errors.  The error can
be requested using `SDL_GetError`.

#### gtsdl_close

	extern void gtsdl_close (struct GTSDLState *s);

Closes all SDL resources that were requested by this library.  In case of
the `gtsdl_openwindow` function it will free all resources and you just need
to call `SDL_Close`.  In case of the `gtsdl_setup` function it will not free
resources allocated outside of the library.

#### gtsdl_getaudiofreq

	extern int gtsdl_getaudiofreq (struct GTSDLState *s);

Returns the frequency of the used audio device.

#### gtsdl_render

	extern SDL_Texture *gtsdl_render (struct GTSDLState *s, struct GTState *gt,
		struct GTPeriph *ph);

Emulates and renders a single frame, queues the audio and returns the
finished texture.

This function should be used when the library was initialized with
`gtsdl_setup`.  You need to keep control of the frame rate yourself,
otherwise the emulation might get ahead of the audio output.

#### gtsdl_runuiframe

	extern int gtsdl_runuiframe (struct GTSDLState *s, struct GTState *gt,
		struct GTPeriph *ph, SDL_Event *ev);

Renders one frame in the standalone window created by
`gtsdl_openwindow` and checks for `SDL_Events` using `SDL_PollEvent`.

In case there is a event this function returns immediately without any
further actions with the return value 1.  The SDL_Event structure contains
the current event.

When there is no event it will emulate and render one frame and will call
`SDL_Delay` to limit the frame rate to 60 FPS. In this case the return
value is 0;

#### gtsdl_handleevent

	extern int gtsdl_handleevent (struct GTSDLState *s, struct GTState *gt,
		SDL_Event *ev);

Processes keyboard events and updates the IN register accordingly. It
returns 1 if it was a handled event, 0 otherwise.

Lua bindings
------------

### About Lua

[Lua][lua] is a lightweight scripting language that can be embedded into
other applications as well as being used as a standalone interpreter with
an interactive prompt.

The Lua bindings for the emulation library can be loaded using

	gtemu = require "gtemu"

assuming that the DLL is in the packages search path.

### API

#### gtemu.initsdl ()

Calls `SDL_Init` and registers `SDL_Close` for `atexit`.

#### gtemu.openwindow (title)

Initializes the SDL subsystems and creates a window.  A window object is
returned.

#### window:runloop (emulation, eventhandlers)

Runs an event loop for the specified emulation (see below) until either the
event `SQL_QUIT` was received, one of the eventhandlers requested a break
or the Escape key was detected.

`eventhandlers` is an optional table where the callback functions
`onkeydown (keyname, mods, scancode, keycode)`,
`onkeyup (keyname, mods, scancode, keycode)` and
`ontextinput (text)` can be defined.

The functions can return either `true`, `false` or the string `"break"`.
When `true` is returned the event handling is finished, for `false` the
default event handling is executed, for `break` the loop is interrupted. 

#### window:close ()

Closes the window and releases the SDL subsystems.  This function is called
automatically when the window object is garbage collected or at the end
of the application.

#### gtemu.newemulation (window [, ramsize])

Initializes the emulation. `ramsize` can be either 32 or 64.  When
omitted it defaults to 32.

This function returns an emulation object.

Example of setting up the application and starting an emulation:

	gtemu = require "gtemu"

	gtemu.initsdl()
	window = gtemu.openwindow("Gigatron in Lua")
	emulation = gtemu.newemulation(window, 64)

	f = assert(io.open("gigatron.rom", "rb"))
	emulation:loadrom(f:read("*a"))
	f:close()

	window:runloop(emulation)

#### Properties of emulation

In the emulation the following registers can be both read and modified:

	pc, ir, d, ac, x, y, out, inp

For example:

	if emulation.pc == 0x0123 then
		emulation.inp = 0xff
	end

The contents of the RAM can be accessed using the array notation:

	if emulation[0x0123] == 0x10 then
		emulation[0x1234] = 0x20
	end

There is a special variable that maps the the vPC memory location in
ram:

	if emulation.vpc >= 0x5a0c then
		-- An application (probably the Loader) is running inside
		-- screen memory
	end

	emulation.vpc = 0x200

This is equivalent to manual access to the RAM locations 0x16 and 0x17.
Further aliases might be defined in future versions.

The following read-only values are also available:

	clock, xout

#### emulation:loadrom (data)

Loads a string into ROM:

	f = assert(io.open("gigatron.rom", "rb"))
	emulation:loadrom(f:read("*a"))
	f:close()

#### emulation:processtick ()

Advances the simulation for a single tick without any output.

#### emulation:processscreen ()

Advances the simulation for an entire screen without any output.

#### emulation:sendgt1 (data)

Sends data from a GT1 file to the Loader application (assuming it is
running):

	f = assert(io.open("Overworld.gt1", "rb"))
	gt1 = f:read("*a")
	f:close()

	if emulation.vpc >= 0x5a0c then
		emulation:sendgt1(gt1)
	end

The data is automatically verified before sending, an error will be raised
for invalid data.

#### emulation:createbuffer (size)

Creates a buffer the Gigatron can send output to.

#### emulation:getbuffer ()

Requests the current contents of the buffer.

#### emulation:resetbuffer ()

Clears the buffer.

