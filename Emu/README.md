# gtemuSDL
gtemuSDL is an emulator for the Gigatron TTL microcomputer, written in C++ using SDL2.<br/>
This project provides Microsoft Windows support and should be compatible with Linux, MacOS<br/>
and possibly even Android. As of ver0.3.2 it has only been tested on Microsoft Windows10.<br/>

# Features
- Variable timing from a minimum of 60FPS up to whatever your PC can handle.<br/>
- Synchronisation of audio with video at any FPS at or above 60FPS.<br/>
- Gigatron TTL emulator using SDL2, tested on Windows 10 x64, compiled with VS2017.<br/>
- Supports fullscreen optimised rendering.<br/>
- Supports Gigatron TTL input buttons.<br/>
- Supports Gigatron TTL LED display.<br/>
- Supports Gigatron TTL audio channels.<br/>
- Supports Gigatron TTL loading of external vCPU code with a file browser.<br/>
- Displays Gigatron TTL IN and XOUT registers.<br/>
- Displays memory monitor of RAM, [RAM is editable], ROM0 and ROM1 at any start address.<br/>
- Displays read only contents of vCPU variable space.<br/>
- Can execute hand crafted code within the Hex Editor.<br/>
- Three seperate editable start addresses are provided; memory display address,<br/>
  vCPU vars display address and load start address.<br/>
- A built in assembler can now assemble vCPU as well as Native mnemonics.<br/>
- A debugging mode that lets you pause the simulation or single step through vCPU code.<br/>
- Upload assembled vCPU code to RAM.<br/>
- Upload assembled Native code to ROM, [emulation only].<br/>
- Supports the Gigatron TTL's GT1 object file format.<br/>

## YouTube
- [https://www.youtube.com/edit?o=U&video_id=pH4st5dz7Go]<br/>
- [https://www.youtube.com/watch?v=u5AQCd47IcI]<br/>

## Based on
- gigatron-rom [https://github.com/kervinck/gigatron-rom]<br/>
- SDL[2] graphics [https://github.com/kervinck/gigatron-rom/pull/1]<br/>
- [http://talkchess.com/forum/viewtopic.php?t=65944&start=11]<br/>
- [https://github.com/tronkko/dirent/releases]<br/>
- [https://stackoverflow.com/questions/9329406/evaluating-arithmetic-expressions-from-string-in-c]<br/>

## Building
- CMake 3.7 or higher is required for building, has only been tested on Windows so far.<br/>
- A C++ compiler that supports modern STL.<br/>
- Requires the latest version of SDL2 and it's appropriate include/library/shared files.<br/>

## Installation
- After building, copy the executable, SDL2 shared library/DLL, "theloop2.rom" and<br/>
  "EmuFont-96x48.bmp" to an appropriate directory; run the executable from there.<br/>
- Create a vCPU directory within the above directory and add any user generated vCPU<br/>
  or GT1 files that you want to upload to the emulator.<br/>

## Controls
|Key       | Function                                                                          |
|----------|-----------------------------------------------------------------------------------|
|<ESC>     | Quits the application.                                                            |
|<l>       | Loads external vCPU files within the vCPU directory, this code is uploaded to     |
|          | an editable load address. Loading user vCPU code to system critical addresses     |
|          | can cause the emulator to hang, 0x0200 is guaranteed to be safe.                  |
|<r>       | Switches Hex Editor between RAM, ROM(0) and ROM(1).                               |
|<F5>      | Executes whatever code is present at the load address.                            |
|<F6>      | Toggles debugging mode, simulation will pause, only 3 keys currently work in this |
|          | mode, F6, F10 and ESC.                                                            |
|<F10>     | Only functions in debugging mode, will single step the simulation based on a      |
|          | memory location changing it's value.                                              |
|<CR>      | Directly loads external vCPU code if editor is in file browse mode, otherwise     |
|          | switches to edit mode.                                                            |
|<-/+>     | Decrease/increase the speed of the emulation, from a minimum of 60FPS to a        |
|          | maximum determined by your PC's CPU.                                              |
|          |                                                                                   |
|<Left>    | Navigate the Hex editor one byte at a time or the file browser one file at a time.|
|<Right>   |                                                                                   |
|<Up/Down> |                                                                                   |
|          |                                                                                   |
|<PgUp>    | Scroll the hex editor and file browser one page at a time.                        |
|<PgDn>    |                                                                                   |
|          |                                                                                   |
|<Wheel>   | Scroll the hex editor and file browser one line/file at a time.                   |
|          |                                                                                   |
|<d>       | Right.                                                                            |
|<a>       | Left.                                                                             |
|<w>       | Up.                                                                               |
|<s>       | Down.                                                                             |
|<SPACE>   | Start.                                                                            |
|<z>       | Select.                                                                           |
|</>       | B.                                                                                |
|<.>       | A.                                                                                |

## Limitations
- RAM is modifiable between 32K and 64K, any other value causes the simulation to fail.<br/>
- Controls and VSync are modifiable in the code, VSync currently seems to have little value<br/>
  as an option as the main loop is synchronised using the performance counters.<br/>

## TODO
- Test under Linux, MAC and Android.<br/>
- Allow for uploading of vCPU payloads larger than 60 bytes in size.<br/>
- Add a inbuilt vCPU and native code disassembler.<br/>
- Add a help menu.<br/>
