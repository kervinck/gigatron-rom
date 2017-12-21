Software for Gigatron ROM
=========================

Files
=====
```
theloop.py                      Video/audio/io/interpreter loops
                                Built-in vCPU applications and data
                                Execute theloop.py to build ROM files
gcl.txt                         Gigatron Control Language explanation
gtemu.c                         Emulator / executable instruction set definition
LICENSE                         2-Clause BSD License
Makefile                        Marcel's Makefile
README.md                       This file...
```

Files processed by theloop.py
=============================
```
main.gcl                        Application code
gcl.py                          Module: GCL to vCPU compiler
asm.py                          Module: Assembler functions
font.py                         Module: Gigatron font definition
Images/Baboon-160x120.rgb       Raw RGB image file (source: Baboon-160x120.png)
Images/Jupiter-160x120.rgb      Raw RGB image file (source: Jupiter-160x120.png)
Images/Parrot-160x120.rgb       Raw RGB image file (source: Parrot-160x120.png)
```

Files generated by theloop.py
=============================
```
theloop.asm                     Annotated disassembly, with labels and comments
theloop.0.rom                   ROM file for 28C256 #0 (breadboard)
theloop.1.rom                   ROM file for 28C256 #1 (breadboard)
theloop.2.rom                   ROM file for 27C1024 (PCB versions)
```

Memory map (RAM)
================
```
             +------------------------------------+---------------------+
page 0       | System and program variables     <-|-> vCPU stack at top |
             +------------------------------------+--+------------------+
page 1       | Video frame indirection table         | Channel 1 at top |
             +---------------------------------------+------------------+
page 2       | Shift table for sound                 | Channel 2 at top |
             +---------------------------------------+------------------+
page 3       | vCPU code                          248| Channel 3 at top |
             |                                       +------------------+
page 4       |                                       | Channel 4 at top |
             |                                       +------------------+
page 5-7     |0                       159 160                        255|
             +---------------------------+------------------------------+
page 8-127   | 120 lines of 160 pixels   | Extra video/code/data at top |
             +---------------------------+------------------------------+
page 128-255 | Not used in the 32K system: mirror of page 0-127         |
             +----------------------------------------------------------+
```
