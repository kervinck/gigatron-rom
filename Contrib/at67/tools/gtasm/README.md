# gtasm
Takes a .**_vasm_** or .**_asm_** or .**_s_** assembly file, (**_vCPU_**), and assembles it into a .**_gt1_** output file.</br>

## Building
- CMake 3.7 or higher is required for building, has been tested on Windows with Visual Studio and gcc/mingw32<br/>
  and also built and tested under Linux.<br/>
- A C++ compiler that supports modern STL.<br/>

## Usage
gtasm \<input filename\> \<start address in hex\></br>

## Address
The address, (**_specified in hex_**), is the start address of the vCPU assembly code.<br/>

## Output
gtasm outputs a standard .**_gt1_** file, containing the start address and segments of the assembled code.<br/>

## Logging
Warnings and errors are output to **_stderr_**, (console under main window in Windows).

## Example
gtasm starfield.vasm 0x0200<br/>
~~~
************************************************************
* starfield.gt1 : 0x0200 :   787 bytes :   9 segments
************************************************************
* Segment :  Type  : Address : Memory Used
************************************************************
*     0   :  RAM   : 0x00e0  :    16 bytes
*     1   :  RAM   : 0x0200  :    88 bytes
*     2   :  RAM   : 0x0300  :   169 bytes
*     3   :  RAM   : 0x0400  :   182 bytes
*     4   :  RAM   : 0x0500  :   144 bytes
*     5   :  RAM   : 0x0590  :    16 bytes
*     6   :  RAM   : 0x05a0  :    48 bytes
*     7   :  RAM   : 0x08a1  :    50 bytes
*     8   :  RAM   : 0x09a1  :    74 bytes
************************************************************
* Free RAM after loading: 44763
************************************************************
~~~
