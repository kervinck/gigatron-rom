# gtsplitrom
Takes a gigatron .**_rom_** file and splits it into two individual .**_rom_** files, one for instructions and one</br>
for data. Input file has to be 128KBytes in length, if greater, then only the first 128KBytes will be</br>
read and split.</br>

## Building
- CMake 3.7 or higher is required for building, has been tested on Windows with Visual Studio and gcc/mingw32<br/>
  and also built and tested under Linux.<br/>
- A C++ compiler that supports modern STL.<br/>

## Usage
gtsplitrom \<input filename\> \<output filename\></br>

## Example
gtsplitrom ROMv1.rom test.rom<br/>

## Output
Output is always two files each 64KBytes in length, i.e. from the above example, output would be **_test.rom\_i_**<br/>
and **_test.rom\_d_**.<br/>
