# gtmakerom
Takes a normal 16bit gigatron .**_rom_** file, a set of split .**_gt1_** files and merges them into an output ROM file.<br/>
Input file has to be 128KBytes in length, if greater, then only the first 128KBytes will be read and used.</br>

## Building
- CMake 3.7 or higher is required for building, has been tested on Windows with Visual Studio and gcc/mingw32<br/>
  and also built and tested under Linux.<br/>
- A C++ compiler that supports modern STL.<br/>

## Usage
gtmakerom \<input ROM filename\> \<input GT1 ROM filename\> \<output ROM filename\> \<title\> \<menu name\><br/>
          \<int menu 0 to 5\> \<uint16_t start address\><br/>

## Example
gtmakerom ROMv1.rom tetris.rom test.rom \"TTL micromputer test v0\" Tetris 5 0x0B00<br/>

## GT1 ROM Filename
The GT1 ROM filename must have be created with **_gt1torom_**, which will create two files, **_\<name\>.rom\_ti_** and<br/>
**_\<name\>.rom\_td_**. These two files contain the separate instructions and data with correct trampolines of the<br/>
original .**_gt1 file_**. Both these files must exist for **_gtmakerom_** to function correctly.<br/>

## Title
The title string is a text field that is used to label this ROM in the Gigatron main menu.<br/>

## Menu Name
The menu name is a text field that becomes the new menu item label.<br/>

## Menu
The menu is an int field from 0 to 5 that represents the actual internal GT1 module to replace.<br/>
Snake = 0, Racer = 1, Mandelbrot = 2, Pictures = 3, Credits = 4, Loader = 5.<br/>

## Address
The address, (**_specified in hex_**), is the address of where the .**_gt1_** file is merged to.<br/>

## Output
The output is always one merged ROM file of 128KBytes in length and two split files each 64KBytes in length.<br/>
i.e. from the above example, output would be **_test.rom_**, **_test.rom\_i_** and **_test.rom\_d_**.<br/>
The merged output file may be used as the input ROM file for the next invocation of gtmakerom, this<br/>
allows you to merge multiple GT1 ROM files into a base ROM file by repeatedly calling gtmakerom on each<br/>
successive output.<br/>
e.g.<br/>
~~~
gtmakerom ROMv1.rom starfield.rom test.rom "TTL microcomputer ROM at67" Starfield 2 0x0B00
gtmakerom test.rom Life.rom test.rom "TTL microcomputer ROM at67" Life 5 0x0f00
gtmakerom test.rom Lines.rom test.rom "TTL microcomputer ROM at67" Lines 0 0x1100
gtmakerom test.rom Gigatris.rom test.rom "TTL microcomputer ROM at67" Gigatris 3 0x1300
gtmakerom test.rom tetris.rom test.rom "TTL microcomputer ROM at67" Tetris 4 0x3000
gtmakerom test.rom miditest.rom test.rom "TTL microcomputer ROM at67" Midi 1 0x5800
~~~
