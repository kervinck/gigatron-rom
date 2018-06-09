# gt1torom
Takes a gigatron **_.gt1_** file and splits it into two individual ROM files, one for instructions and one</br>
for data. Output ROM files will contain .gt1 file in correct loading format with correct ROM trampolines.</br>

## Building
- CMake 3.7 or higher is required for building, has been tested on Windows with Visual Studio and gcc/mingw32<br/>
  and also built and tested under Linux.<br/>
- A C++ compiler that supports modern STL.<br/>

## Usage
gt1torom \<input filename\> \<output filename\></br>

## Example
gt1torom test.gt1 test.rom<br/>

## Output
Output is always two files, one for the instruction ROM and one for the data ROM, i.e. from above example, output<br/>
would be **_test.rom0_** and **_test.rom1_**.<br/>
