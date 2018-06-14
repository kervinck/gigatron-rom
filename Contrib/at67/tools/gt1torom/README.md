# gt1torom
Takes a gigatron .**_gt1_** file and splits it into two individual .**_rom_** files, one for instructions and one</br>
for data. Output ROM files will contain the .**_gt1_** file in correct loading format with correct ROM trampolines.</br>

## Building
- CMake 3.7 or higher is required for building, has been tested on Windows with Visual Studio and gcc/mingw32<br/>
  and also built and tested under Linux.<br/>
- A C++ compiler that supports modern STL.<br/>

## Usage
gt1torom \<input filename\> \<output filename\> \<start address in hex\></br>

## Address
The address, (**_specified in hex_**), is the start address of the ROM trampoline code.<br/>

## Example
gt1torom test.gt1 test.rom 0x8000<br/>

## Output
Output is always two files, one for the instruction ROM and one for the data ROM, i.e. from the above example, output<br/>
would be **_test.rom\_ti_** and **_test.rom\_td_**.<br/>
