# py-gtemu: A Python extension, wrapping gtemu.c for testing and debugging

gtemu is a module which exposes the Gigatron emulator in gtemu.c to Python. It is meant to be useful for testing and debugging Gigatron software written using a Python-based toolchain. It has grown out my effort to make a native Forth implementation for the Gigatron. It uses an extension module (_gtemu) which is built using cffi, and provides a high-level API on top of that.

## Core functionality

The main entry-point to the module is an object called `Emulator`. This is a singleton - there is only meant to be one instance of this. This is because the C source uses static arrays for the RAM and ROM, and so we can't have multiple emulators at the same time. Perhaps this should be changed in future, but I quite like that I'm using the unmodified C source, even if it was never meant to be used like this.

`Emulator` provides low-level methods to single-step, run for a number of cycles, run up to a certain address, to set and get the next execution address, and get and set any of the registers (these are properties, so `Emulator.AC` etc.). There are also objects in the module ROM and RAM which expose the data.

There are also higher-level methods which make certain assumptions about the software in the ROM, these include running up to the next horizontal and vertical blank period, emulating sending a byte of data in through the game controller port for one frame only, and reading serial output sent with SYS_SendSerial_v3_80.

The Emulator also has some support for the vCPU: you can single-step, run up to a certain address, get and set any of the (virtual) registers. This could probably be much improved.

The Emulator object has a flag called _print. When this is set to True, many methods will print a representation of the relevant emulator state on completion. This is useful for debugging. The string that they print is always accessible through the `Emulator.state` and `Emulator.vcpu_state` properties.

Look at the source in gtemu.py for a list of the available methods.

### Unit testing Gigatron code

The main usecase is to enable unit-testing of Gigatron code for a better development cycle. I use pytest for unit testing, often with Hypothesis for test case generation, but those details are not vital.

This usually follows a normal Arrange, Act, Assert process, where the arrange phase sets up the emulator state, the act phase runs the emulator, and the assert phase verifies that whatever the code was meant to do has been done, usually by reading the RAM and registers.

TODO: add an example here, possibly testing a SYS function.

You can look at the tests in this directory for some ideas, and also my Forth for more - although in Forth I've written a lot of wrappers, so the use may not be so obvious.

### Debugging Gigatron code

This module is not (yet) a fully fledged debugger. If that is what you need, perhaps at67's tools might be a better fit.
However when hitting a failing test, its very useful to drop into pdb, set `Emulator._print = True` and run `Emulator.step()` or other methods repeatedly (pdb repeats the last command if you just press enter). This gives a very debugger like experience.

## API stability

The interface has been driven solely by my needs, and I make no promises not to change it in future. Sorry! However if you're using this module in your tests, and they are available publicly, let me know and I'll endeavour not to break your tests, or perhaps provide pull-requests to update them.


## Build / Installation

In order to build the module you need to have Python 3.6 or later, the headers for your Python, a C compiler and the C library headers. The build also depends on a Python package called CFFI, but *I think* you shouldn't need to do anything about that.

On Debian or Ubuntu, running `apt-get install build-essential python3-dev python3-pip python3-venv` should get you what you need.

On Windows I think it's sufficient to install Python from Python.org or the Microsoft Store and the Visual Studio Build Tools from the Visual Studio Installer. I believe that this is free. If you already have Visual Studio installed, you don't need to install the build tools.

On other platforms, I'm afraid you're on your own.

Once you have these dependencies in place, you should be able to use `pip` to install the modules, by pointing it at this directory. It assumes that this directory is part of a full Gigatron source tree - i.e. gtemu.c is in ..\..\..\Docs\ If you have struggles with the extension module (error messages involving difficulties importing cffi for example), check that you are using an up to date pip, as I think all of the metadata that should tell pip to install cffi is in place.

As always when working with external dependencies with Python it is very strongly advisable to use an isolated Python environment to avoid messing up your Python installation, particularly on Linux distributions where it is a core part of your operating system.

For development of py-gtemu itself you will need to install cffi with pip, and to run the tests pytest as well. Check psakefile.ps1 to see how I manage this - even if you don't use psake to build, it should give the right idea.

## Development

This was spun out from the Forth project when I needed to add the ability to test routines that perform serial IO. The code I added to read the serial output was complicated enough that I was sure it would have bugs, and I couldn't really debug both the system under test and the test harness at the same time. That explains why that is the only area with anything like tests.

My idea was to have various GCL programs which could be built in to complete ROM images by making them the Main entry point, as I have with echo.gcl. psakefile.ps1 contains code to generate a ROM from each .gcl file in the scripts directory, and the tests can then load these.

## Future developments

The following might be worth doing in future:

* A Makefile for Linux / macOS development.
* The ability to capture and restore emulator state for better fixtures
* Make a full PDB style debugger, using the cmd module. This should be pretty easy.
* A pytest plugin.
* Some Hypothesis integration: I currently use pytest fixtures, but Hypothesis warns that they're not compatible. Actually the 'incompatible' behaviour is perfectly fine for me, but it would be possible to do better.