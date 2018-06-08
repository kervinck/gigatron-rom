# gtasm
Takes a **_.vasm_** or **_.asm_** or **_.s_** vCPU assembly file and builds into a **_.gt1_** output file.</br>

## Building
- CMake 3.7 or higher is required for building, has been tested on Windows with Visual Studio and<br/>
  gcc/mingw32, (thanks to Cwiiis https://github.com/Cwiiis)<br/>
- A C++ compiler that supports modern STL.<br/>

## Usage
gtasm \<input filename\> \<start address in hex\></br>

## Address
The address, **_(specified in hex)_**, is the start address of the vCPU assembly code.<br/>

## Output
gtasm outputs a standard **_.gt1_** file, containing the start address and segments of the assembled code.<br/>

## Logging
Warnings and errors are output to **_stderr_**, (console under main window in Windows).

## Example
gtasm starfield.vasm 0x0200<br/>
~~~
********************************************************************************
starfield.vasm : 0x0200 :   787 bytes :   9 segments
********************************************************************************
Segment000 : 0x00e0 :    16 bytes
Segment001 : 0x0200 :    88 bytes
Segment002 : 0x0300 :   169 bytes
Segment003 : 0x0400 :   182 bytes
Segment004 : 0x0500 :   144 bytes
Segment005 : 0x0590 :    16 bytes
Segment006 : 0x05a0 :    48 bytes
Segment007 : 0x08a1 :    50 bytes
Segment008 : 0x09a1 :    74 bytes
~~~
