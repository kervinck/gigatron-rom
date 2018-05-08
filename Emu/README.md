# gtemuSDL
gtemuSDL is an emulator for the Gigatron TTL microcomputer, written in C++ using SDL2.<br/>
This project provides support for Microsoft Windows and should be compatible with Linux, MacOS<br/>
and possibly even Android. As of ver0.3.8 it has only been tested on Microsoft Windows10.<br/>

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
- Supports the Gigatron TTL's **_GT1_** object file format.<br/>

## YouTube
- https://www.youtube.com/watch?v=pH4st5dz7Go<br/>
- https://www.youtube.com/watch?v=u5AQCd47IcI<br/>
- https://www.youtube.com/watch?v=sq8uCpBFwp8<br>

## References
- Gigatron-ROM https://github.com/kervinck/gigatron-rom<br/>
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
  or **_GT1_** files that you want to upload to the emulator.<br/>

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

## Assembler
- The assembler is built into the executable, there are no makefiles or command line programs to<br/>
  run. All you have to do is load the **_.vasm_** file within the file browser, (press **_L_** to enable<br/>
  the browser), upon loading your file will automatically be assembled and any errors printed to<br/>
  **_stderr_**. If there are no errors then a **_.gt1_** file will automatically be produced as long as there is<br/>
  no native code within the **_.vasm_** file; the code is then executed within the Gigatron CPU emulator.<br/>
- The assembler is able to assemble vCPU as well as native code.<br/>
- vCPU code is assembly code that is variable length Von Neumann instructions supporting 16bit operations<br/>
  that are executed by the inbuilt interpreter of the Gigatron TTL firmware and are stored in RAM.<br/>
- Native code is based on a Harvard Architecture that exists only in ROM on both the Gigatron TTL<br/>
  hardware and within the emulator.<br/>
- The ROM code on the Gigatron TTL hardware is currently not modifiable without an EPROM programmer,<br/>
  but the ROM within the emulator is able to be modified and experimented with, which this assembler<br/>
  lets you do very easily.<br/>
- The Assembler differentiates between the two instruction sets, (**_vCPU_** and **_Native_**), by preceding<br/>
  Native instructions with a period **_'.'_**<br/>
- The Assembler supports Labels, Equates, Expressions and self modifying code.<br/>
- The Assembler recognises the following reserved words:<br/>
    - **_\_startAddress\__** : entry point for the code, if this is missing defaults to 0x0200.<br/>
    - **_\_callTable\__** : grows downwards as you use more CALL's, it must exist in page 0 RAM.<br/>
    - **_\_singleStepWatch\__** : the single step debugger watches this variable location to decide when to step.<br/>
    - **_\_disableUpload\__** : disables all writes to RAM/ROM, used for testing and verification.<br/>
- The Assembler recognises the following reserved opcodes:<br/>
    - **_DB_** : Define Byte, a string of 8bit bytes declared within the source code that are written to RAM.<br/>
    - **_DW_** : Define Word, a string of 16bit words declared within the source code that are written to RAM.<br/>
    - **_DBR_** : Define Byte, a string of 8bit bytes declared within the source code that are written to ROM.<br/>
    - **_DWR_** : Define Word, a string of 16bit words declared within the source code that are written to ROM.<br/>
- Numeric literals can be of any of the following format:<br/>
    - **_hex_** : 0x0000 or $0000<br/>
    - **_dec_** : 1234<br/>
    - **_oct_** : 0o777 or 0q777<br/>
    - **_bin_** : 0b01011111<br/>
- Comments can go anywhere on a line and commence with **_';'_** or **_'#'_**<br/>

## Debugger
- The debugger allows you to single step through your vCPU code based on a variable changing<br/>
  its value. This variable by default is **_videoY_** located at address 0x0009, it counts horizontal<br/>
  scan lines and thus should give decent resolution for most **_.vasm_** and **_.gt1_** files.<br/>
- The variable can be altered within a **_.vasm_** file like this:<br/>
~~~
_singleStepWatch_   EQU     xyPos
~~~
- In this example xyPos is a pointer into zero page memory, pointing to a variable that changes<br/>
  its value often.<br/>
- **_F6_** toggles debugging on and off and may be used as a pause or freeze.<br/>
- **_F10_** single steps the currently loaded code based on the _singleStepWatch_ variable.<br/>
- All other keys function normally as in the main editor mode, except for **_L_**, **_F1_**<br/>
  and **_F5_** which are ignored.<br/>

## Errors
- All error reporting is sent to **_stderr_**, under Windows a separate console is opened, (that is<br/>
  probably hiding under the main display), that will contain any error messages.<br/>

## Limitations
- RAM is modifiable between 32K and 64K, any other value causes the simulation to fail.<br/>
- Controls and VSync are modifiable in the code, VSync currently seems to have little value<br/>
  as an option as the main loop is synchronised using the performance counters.<br/>

## TODO
- Test under Linux, MAC and Android.<br/>
- Add an inbuilt vCPU and native code disassembler.<br/>
- Add a help menu.<br/>
