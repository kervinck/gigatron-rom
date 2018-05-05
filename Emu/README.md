# gtemuSDL
gtemuSDL is an emulator for the Gigatron TTL microcomputer, written in C++ using SDL2.<br/>
This project provides support for Microsoft Windows and should be compatible with Linux, MacOS<br/>
and possibly even Android. As of ver0.3.2 it has only been tested on Microsoft Windows10.<br/>

## Features
- Variable timing from a minimum of 60FPS up to whatever your PC can handle.<br/>
- Synchronisation of audio with video at any FPS at or above 60FPS.<br/>
- Gigatron TTL emulator using SDL2, tested on Windows 10 x64, compiled with VS2017.<br/>
- Supports fullscreen optimised rendering.<br/>
- Supports Gigatron TTL input buttons.<br/>
- Supports Gigatron TTL LED display.<br/>
- Supports Gigatron TTL audio channels.<br/>
- Supports Gigatron TTL loading of external vCPU code with a file browser.<br/>
- Displays Gigatron TTL IN and XOUT registers.<br/>
- Displays memory monitor of RAM, (**_RAM is editable_**), ROM0 and ROM1 at any start address.<br/>
- Displays read only contents of vCPU variable space.<br/>
- Can execute hand crafted code within the Hex Editor.<br/>
- Three seperate editable start addresses are provided; memory display address,<br/>
  vCPU vars display address and load start address.<br/>
- A built in assembler can now assemble vCPU as well as Native mnemonics.<br/>
- A debugging mode that lets you pause the simulation or single step through vCPU code.<br/>
- Upload assembled vCPU code to RAM.<br/>
- Upload assembled Native code to ROM, (**_emulation only_**).<br/>
- Supports the Gigatron TTL's GT1 object file format.<br/>

## YouTube
- https://www.youtube.com/edit?o=U&video_id=pH4st5dz7Go<br/>
- https://www.youtube.com/watch?v=u5AQCd47IcI<br/>

## References
- gigatron-rom https://github.com/kervinck/gigatron-rom<br/>
- SDL2 graphics https://github.com/kervinck/gigatron-rom/pull/1<br/>
- http://talkchess.com/forum/viewtopic.php?t=65944&start=11<br/>
- https://github.com/tronkko/dirent/releases<br/>
- https://stackoverflow.com/questions/9329406/evaluating-arithmetic-expressions-from-string-in-c<br/>

## Building
- CMake 3.7 or higher is required for building, has only been tested on Windows so far.<br/>
- A C++ compiler that supports modern STL.<br/>
- Requires the latest version of SDL2 and it's appropriate include/library/shared files.<br/>

## Installation
- After building, copy the executable, SDL2 shared library/DLL, **_"theloop2.rom"_** and<br/>
  **_"EmuFont-96x48.bmp"_** to an appropriate directory; run the executable from there.<br/>
- Create a **_"vCPU"_** directory within the above directory and add any user generated vCPU<br/>
  or GT1 files that you want to upload to the emulator.<br/>

## Controls
|Key        | Function                                                                          |
|:---------:|-----------------------------------------------------------------------------------|
|ESC        | Quits the application.                                                            |
|L or l     | Loads external vCPU files within the vCPU directory, this code is uploaded to     |
|           | an editable load address. Loading user vCPU code to system critical addresses     |
|           | can cause the emulator to hang, 0x0200 is guaranteed to be safe.                  |
|R or r     | Switches Hex Editor between RAM, ROM(0) and ROM(1).                               |
|F1         | Fast reset, performs the same action as a long hold of Start.                     |
|F5         | Executes whatever code is present at the load address.                            |
|F6         | Toggles debugging mode, simulation will pause and allow you to single step using  |
|           | F10.                                                                              |
|F10        | Only functions in debugging mode, will single step the simulation based on a      |
|           | memory location changing it's value.                                              |
|ENTER/CR   | Directly loads external vCPU code if editor is in file browse mode, otherwise     |
|           | switches to edit mode.                                                            |
|-/+        | Decrease/increase the speed of the emulation, from a minimum of 60FPS to a        |
|           | maximum determined by your PC's CPU.                                              |
|           |                                                                                   |
|Left       | Navigate the Hex editor one byte at a time or the file browser one file at a time.|
|Right      |                                                                                   |
|Up/Down    |                                                                                   |
|           |                                                                                   |
|PgUp       | Scroll the hex editor and file browser one page at a time.                        |
|PgDn       |                                                                                   |
|           |                                                                                   |
|Mouse Wheel| Scroll the hex editor and file browser one line/file at a time.                   |
|           |                                                                                   |
|D or d     | Right.                                                                            |
|A or a     | Left.                                                                             |
|W or w     | Up.                                                                               |
|S or s     | Down.                                                                             |
|SPACE      | Start.                                                                            |
|Z or z     | Select.                                                                           |
|/          | B.                                                                                |
|.          | A.                                                                                |

## Debugger
- The debugger allows you to single step through your vCPU code based on a variable changing<br/>
  its value. This variable by default is videoY located at address 0x0009, it counts horizontal<br/>
  scan lines and thus should give decent resolution for most .vasm and .gt1 files.<br/>
- The variable can be altered within a .vasm file like this:<br/>
~~~
_singleStepWatch_   EQU     xyPos
~~~
- In this example xyPos is a pointer into zero page memory, pointing to a variable that changes<br/>
  its value often.<br/>
- **_F6_** toggles debugging on and off and may be used as a pause or freeze.<br/>
- **_F10_** single steps the currently loaded code based on the _singleStepWatch variable.<br/>
- All other keys function normally as in the main editor mode, except for **_L_**, **_F1_**<br/>
  and **_F5_** which are ignored.<br/>

## Errors
- All error reporting is sent to stderr, under Windows a seperate console is opened, (that is<br/>
  probably hiding under the main display), that will contain any error messages.<br/>

## Limitations
- RAM is modifiable between 32K and 64K, any other value causes the simulation to fail.<br/>
- Controls and VSync are modifiable in the code, VSync currently seems to have little value<br/>
  as an option as the main loop is synchronised using the performance counters.<br/>

## TODO
- Test under Linux, MAC and Android.<br/>
- Add an inbuilt vCPU and native code disassembler.<br/>
- Add a help menu.<br/>
