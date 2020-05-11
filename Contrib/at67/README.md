# gtemuAT67
**_gtemuAT67_** is an emulator, a cross assembler for both vCPU and Native asm code, a debugger, a monitor<br/>
and a real time controller for Gigatron TTL hardware; written in C++ using SDL2.<br/>
This project provides support for Microsoft Windows and should be compatible with Linux, MacOS<br/>
and possibly even Android. As of v0.8.17 it has been tested on Microsoft Windows10, some flavours of Linux and MacOS.<br/>

## Features
- Can control real Gigatron hardware through an Arduino interface.<br/>
- Can upload vCPU, GCL and GT1 files to real Gigatron hardware through an Arduino interface.<br/>
- Emulates a PS2 Keyboard for vCPU, GCL code that can use PS2 hardware, such as WozMon.gt1<br/>
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
- Three separate editable start addresses are provided; memory display address,<br/>
  vCPU vars display address and load start address.<br/>
- A built in assembler can now assemble vCPU as well as Native mnemonics.<br/>
- A preprocessor that is able to include files and expand parameterised macros.<br/>
- A debugging mode that lets you pause the simulation or single step through vCPU code.<br/>
- Upload assembled vCPU code to RAM.<br/>
- Upload assembled Native code to ROM, (**_emulation only_**).<br/>
- Supports the Gigatron TTL's .**_gt1_** object file format.<br/>
- An inbuilt file and directory browser for uploading.<br/>
- Multiple scanline disable options, (default key **_F3_**, **_emulation only_**), will disable scanlines<br/>
  offering large amounts of extra processor time for vCPU code.<br/>
- Input keys easily configurable through an INI file.<br/>
- On screen help menu, (accessible with 'H/h' by default), that fully explains each key and it's function.<br/>
- Optional configuration files, "**_graphics_config.ini_**", "**_input_config.ini_**", "**_loader_config.ini_**" and<br/> "**_high_scores.ini_**".<br/>
- You may now start the executable anywhere, the default ROM file and default font are now built into the<br/>
  executable.<br/>

## YouTube
https://www.youtube.com/channel/UCIDqjK ... ewAtLbfwQ/

## References
- Gigatron TTL Home: https://gigatron.io/<br/>
- Gigatron TTL Forums: https://forum.gigatron.io/index.php<br/>
- Gigatron TTL Repo: https://github.com/kervinck/gigatron-rom<br/>
- SDL2 graphics:  https://github.com/kervinck/gigatron-rom/pull/1<br/>
- Early Gigatron emulation:  http://talkchess.com/forum/viewtopic.php?t=65944&start=11<br/>
- Dirent for Windows:  https://github.com/tronkko/dirent<br/>
- Recursive Descent Parser:  https://stackoverflow.com/questions/9329406/evaluating-arithmetic-expressions-from-string-in-c<br/>
- INI Not Invented Here:  https://github.com/jtilly/inih<br/>
- rs232: https://github.com/Marzac/rs232<br/>

## Contributors
- at67 https://github.com/at67<br/>
- kervinck https://github.com/kervinck<br/>
- Cwiiis https://github.com/Cwiiis<br/>
- xxxbxxx https://github.com/xxxbxxx<br/>

## Building
- CMake 3.7 or higher is required for building, has been tested on Windows with Visual Studio and gcc/mingw32<br/>
  and also built and tested under Linux.<br/>
- A C++ compiler that supports modern STL.<br/>
- Requires the latest version of SDL2 and it's appropriate include/library/shared files.<br/>
- For detailed instructions for Windows, Linux and macOS, see this thread in the Gigatron forum:<br/>
  https://forum.gigatron.io/viewtopic.php?p=368#p368<br/>

## Windows
~~~
  Download and install cmake
  Download sdl2 development libraries https://www.libsdl.org/download-2.0.php
  Clone or download https://github.com/kervinck/gigatron-rom
  cd gigatron-rom\Contrib\at67
  cmake .
~~~
- Open **_gtemuAT67.sln_** into Visual Studio, compile and execute.<br/>
- **Optional:** if you want to be able to develop using SDL2 without having to configure the include and lib variables<br/>
  for each new project, then add an environment variable **_SDL2DIR_** pointing to the directory you installed SDL2 into.<br/>
  
## Linux  
~~~
  sudo apt-get -y install cmake libsdl2-dev
  git clone https://github.com/kervinck/gigatron-rom
  cd gigatron-rom/Contrib/at67
  cmake .
  make
  ./gtemuAT67 &
~~~

## Installation
- After building, copy the executable and SDL2 shared library\/DLL to an appropriate directory;<br/>
  run the executable from there.<br/>
- The default ROM file and default font are now built into the executable., (thanks to  Cwiiis for<br/>
  the idea). The only dependency is the shared library\/DLL from SDL2 that either must be in the current<br/>
  working directory path or in the appropriate system directory.<br/>

## Configuration
- The emulator may be placed anywhere in any directory as long as it has access to the SDL2 shared library.<br/>
- The emulator will search for and use a file named "**_graphics_config.ini_**" in it's current<br/>
  working directory. This file allows the emulator's graphics and video options to be completely user<br/>
  configured:<br/>
~~~  
  Fullscreen = 1, will create a full sized screen that minimises when it loses focus
  Fullscreen = 0, will create a window that does not minimise when it loses focus
~~~
  
- The emulator will search for and use a file named "**_input_config.ini_**" in it's current<br/>
  working directory. This file allows the emulator's keys to be completely user configured. The on<br/>
  screen help menu also uses this file to display help instructions. See the file for help on input<br/>
  configuration.<br/>

- The emulator will search for and use a file named "**_loader_config.ini_**" in it's current<br/>
  working directory. This file allows the emulator's com port to be user configured for communicating<br/>
  with real Gigatron hardware through an Arduino adapter. Custom ROM's are also supported:<br/>
~~~
  BaudRate    = 115200   ; arduino software stack doesn't like > 115200
  ComPort     = COM3     ; can be an index or a name, eg: ComPort = 0 or ComPort = COM5
  Timeout     = 5.0      ; maximum seconds to wait for Gigatron to respond
  GclBuild    = D:/Projects/Gigatron TTL/gigatron-rom ; must be an absolute path, can contain spaces
  RomName     = ROMv1.rom  
~~~
  
- The emulator will search for and use an optional file named "**_high_scores.ini_**" in it's current<br/>
  working directory. This file allows the emulator to load and save segments of memory and have them<br/>
  regularly updated, (can be used for debugging, replays, high scores, etc), see the file for help.<br/>
  Files ending in .**_dat_** will be created in the current working directory of the emulator for<br/>
  every entry that is made and successfully parsed within "**_high_scores.ini_**". These .**_dat_**<br/>
  files contain the individual memory segments loaded and saved to disk for each game/application.<br/>

## Controls
|Key        | Function                                                                          |
|:---------:|-----------------------------------------------------------------------------------|
|CTRL + H   | Displays a help screen showing the currently configured keys.                     |
|CTRL + Q   | Quits the application.                                                            |
|CTRL + R   | Switches between ROM types, (v1 to v4 are built in).                              |
|L or l     | Loads external vCPU files within the vCPU directory, this code is uploaded to     |
|           | an editable load address. Loading user vCPU code to system critical addresses     |
|           | can cause the emulator to hang, 0x0200 is guaranteed to be safe.                  |
|R or r     | Switches Hex Editor between RAM, ROM(0) and ROM(1).                               |
|CTRL + F1  | Fast reset, performs the same action as a long hold of Start.                     |
|ALT  + F1  | Fast reset of real Gigatron hardware, if connected to an Arduino interface.       |
|CTRL + F3  | Toggles scanline modes between, Normal, VideoB and VideoBC, only for ROMv1.       |
|CTRL + F5  | Executes whatever code is present at the load address.                            |
|CTRL + F6  | Toggles debug mode, simulation will pause and allow you to single step using F7.  |
|CTRL + F7  | Only functions in debug mode, will single step the simulation based on a memory   |
|           | location changing it's value.                                                     |
|CTRL + F9  | switches to Hex mode from any other mode                                          |
|CTRL + F10 | Toggles PS2 keyboard emulation on and off.                                        |
|CTRL + F11 | Toggles Gigatron input between emulator and hardware.                             |
|CTRL + F12 | Toggles PS2 keyboard emulation between emulator and hardware                      |
|CTRL + CR  | Loads vCPU code if editor is in file browse mode, otherwise switches to edit mode.|
|ALT  + CR  | Uploads vCPU code to real Gigatron hardware, if connected to an Arduino interface.|
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
  run. All you have to do is load the .**_vasm_** file within the file browser, (press **_L_** to enable<br/>
  the browser), upon loading your file will automatically be assembled and any errors printed to<br/>
  **_stderr_**. If there are no errors then a .**_gt1_** file will automatically be produced as long as there is<br/>
  no native code within the .**_vasm_** file; the code is then executed within the Gigatron CPU emulator.<br/>
- The assembler is able to assemble vCPU as well as native code.<br/>
- GCL code is now able to be compiled and uploaded to the emulator or real hardware as long as you have Python 2.7<br/>
  installed, see **_README.md_** in Contrib/at67/gcl for details.<br/>
- vCPU code is assembly code that is variable length Von Neumann instructions supporting 16bit operations<br/>
  that are executed by the inbuilt interpreter of the Gigatron TTL firmware and are stored in RAM.<br/>
- Native code is based on a Harvard Architecture that exists only in ROM on both the Gigatron TTL<br/>
  hardware and within the emulator.<br/>
- The ROM code on the Gigatron TTL hardware is currently not modifiable without an EPROM programmer,<br/>
  but the ROM within the emulator is able to be modified and experimented with, which this assembler<br/>
  lets you do very easily.<br/>
- The Assembler differentiates between the two instruction sets, (**_vCPU_** and **_Native_**), by preceding<br/>
  Native instructions with a period '**_\._**'<br/>
- The Assembler supports Labels, Equates, Expressions and self modifying code.<br/>
- The Assembler recognises the following reserved words:<br/>
    - **_\_startAddress\__** : entry point for the code, if this is missing defaults to 0x0200.<br/>
    - **_\_callTable\__** : grows downwards as you use more CALL's, it must exist in page 0 RAM.<br/>
    - **_\_singleStepWatch\__** : the single step debugger watches this variable location to decide when to step.<br/>
    - **_\_disableUpload\__** : disables all writes to RAM/ROM, used for testing and verification.<br/>
    - **_\_cpuUsageAddressA\__** : the start of a vCPU exclusion zone for measuring vCPU load, (emulation only).<br/>
    - **_\_cpuUsageAddressB\__** : the end of a vCPU exclusion zone for measuring vCPU load, (emulation only).<br/>
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
- Comments can go anywhere on a line and commence with '**_\;_**' or '**_\#_**'<br/>
- You can include files into your projects using the %**_include_** command.<br/>
- A fully parameterised macro system has been added that can expand in place code using the %**_MACRO_** and<br/>
  %**_ENDM_** commands.<br/>

## Debugger
- The debugger allows you to single step through your vCPU code based on a variable changing<br/>
  its value. This variable by default is **_videoY_** located at address 0x0009, it counts horizontal<br/>
  scan lines and thus should give decent resolution for most .**_vasm_** and .**_gt1_** files.<br/>
- The variable can be altered within a .**_vasm_** file like this:<br/>
~~~
_singleStepWatch_   EQU     xyPos
~~~
- In this example xyPos is a pointer into zero page memory, pointing to a variable that changes<br/>
  its value often.<br/>
- **_F6_** toggles debugging on and off and may be used as a pause or freeze.<br/>
- **_F10_** single steps the currently loaded code based on the _singleStepWatch_ variable.<br/>
- All other keys function normally as in the main editor mode, except for **_L_**, **_F1_**<br/>
  and **_F5_** which are ignored.<br/>
- Real time logging with the gprintf command, (similar syntax to the standard printf); this feature<br/>
  allows you to print any variable, label or expression to stderr whilst the emulator is running vCPU<br/>
  code. The vPC address of the current instruction that the gprintf aligns with is used to decide when to<br/>
  send the logging information to the console as the vCPU code is interpreted in real time.<br/>
~~~
  gprintf("%c %d $%04x $%04x $%04x b%016b b%08b o%04o $%04x %s", 48, 45000, 0xDEAD, 0xBEEF,
                                                                 resetLevel, frameCountPrev, 
                                                                 *frameCounter, maxTicks + 1,
                                                                 vBlank, *level_string)
~~~  
- Indirection is specified using '**_\*_**' to access and print the variable's data.<br/>
- You can specify field width, fill chars and use the same numeric literals as the rest of the Assembler. e.g:<br/>
    - %c will print a char.<br/>
    - %d will print an int.<br/>
    - %04x will print a zero padded four digit hex number.<br/>
    - %016b will print a zero padded 16 digit binary number.<br/>
    - %04o will print a zero padded 4 digit octal number.<br/>
    - %s will print a maximum 255 sequence of chars, with the first byte of the string expected to be the length.<br/>

## MIDI
- See **_gtmidi_** and **_README.md_** in the Contrib/at67/midi directory.<br/>

## Tools
- The following command line tools that break out some of the functionality of the emulator are contained within<br/>
  the following folder, **_Contrib/at67/tools_**, see their respective **_README.md_** files for detailed documentation:<br/>
    - **_gtasm_**:      can assemble .**_vasm_** assembly code into a .**_gt1_** file.<br/>
    - **_gt1torom_**:   splits a .**_gt1_** file into two separate .**_rom_** files, one for data and one for instructions.<br/>
    - **_gtmakerom_**:  takes a normal 16bit Gigatron ROM and merges split .**_gt1_** roms into it.<br/>
    - **_gtsplitrom_**: takes a normal 16bit Gigatron ROM and splits it into data and instruction .**_rom_** files.<br/>

## Memory and State saving
- Real time saving of Gigatron and applications/games memory and state without any involvement of software<br/>
  on the Gigatron, (it's firmware or it's applications). This generic interface is controlled with **_INI_**<br/>
  files, (see "**_high_scores.ini_**" for an example.

## Errors
- All error reporting is sent to **_stderr_**, under Windows a separate console is opened, (that is<br/>
  probably hiding under the main display), that will contain any error messages.<br/>

## Limitations
- RAM is modifiable between 32K and 64K, any other value causes the simulation to fail.<br/>
- Controls and VSync are modifiable in the code, VSync currently seems to have little value<br/>
  as an option as the main loop is synchronised using the performance counters.<br/>

## TODO
- Build and test on Android.<br/>
