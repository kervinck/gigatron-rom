### pytron

First working, but slow attempt of a python port of the `C` Gigatron emulator in `../../Docs/gtemu.c`.

### Gigatron Mac

Fully featured Gigatron emulator for Mac OS X, written in Objective-C and
using XCode. 

Features:
- simulated 50Hz VGA output
- 44.1kHz audio output
- keyboard and simulated controller input
- load .gt1 files directly into RAM
- watch .gt1 files for changes and re-load automatically
- switch between 32kB / 64kB RAM
- rudimentary debugging with disassembly of native CPU and vCPU code, instruction stepping and vCPU stepping
- watch CPU and vCPU state
- RAM gadget (256x256 pixel screen to display RAM content, one pixel for every byte)

Known issues:
- Simulated keypresses sometimes hang
- vCPU stepping interrupts on every change of vPC, but vPC sometimes changes during execution of a vCPU call (e.g. jumps)
- Window position and configuration data is not properly saved and restored on restart of the application
- Due to the author not having written code for some years, the code could certainly be better in a lot of places ... :-)


### gasm

Gigatron assembly code. You need to first build at67's assembler (by using
cmake in Contrib/at67) and then running e.g.
```
../../at67/tools/gtasm/gtasm ./maze.gasm 0x0200
```
- maze.gasm : Every platform needs a maze builder, this is an iterative
  implementation instead of a recursive one (our call stack is not that big...)

### tools

- img2gasm.py : Reads an image (should be roughly in 4:3 format)
  converts it to the gigatron color space and outputs a gigatron assembly program. The
  resulting program can then be compiled and will be loaded directly into
  video RAM. In order not to interfere with the loader itself, three lines
  will be left out during loading and reconstructed from another memory area
  after loading.   


### Licence

Copyright 2018-2019 Tobias Braun
Mail: tobias@braun-online.org
Web: https://home.640k.de

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, 
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
this list of conditions and the following disclaimer in the documentation 
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
