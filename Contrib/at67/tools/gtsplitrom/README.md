# gtsplitrom
Takes gigatron **_.rom_** file and splits it into two individual rom files, one for</br>
instructions and one for data. Input file has to be 128KBytes in length, if greater</br>
then only the first 128KBytes will be read and split.

## Building
- CMake 3.7 or higher is required for building, has been tested on Windows with Visual Studio and<br/>
  gcc/mingw32, (thanks to Cwiiis https://github.com/Cwiiis)<br/>
- A C++ compiler that supports modern STL.<br/>

## Usage
gtsplitrom \<input filename\> \<output filename\></br>

## Example
gtasm theloop.2.rom test.rom<br/>

## Output
Output is always two files each 64KBytes in length.
test.rom0 and test.rom1
