# Gigatron Running on FPGA
Verilog implementation of the Gigatron for the Tang Nano 9K FPGA board.

## Installation
Download and install the free *GOWIN Programmer Vx.x.x. Education* or *Gowin Vx.x.x.Beta-4 Education* if you want to make changes to the code: https://www.gowinsemi.com/en/support/download_eda/ (Prior registration is required)

Note: If you intend run the non-education version of the programs, you need to apply for a license. The education versions are fully sufficient here.

## Flashing the Tang Nano
- Double-click the cell below "Operation" to open the Device configuration dialog.
- Select the bitstream file "gigatron.fs" and the flash initialization file "fpga_games.fi"


    <img src="images/programmer.JPG" width="800">

- Then save and click the "Configure/Program" button to write it all to the FPGA.

## Adapter Board
A VGA monitor, game controller and audio can be connected through the adapter board.
<img src="images/board.png" width="800">

Find schematics and Gerber files of the adapter board here: [Schematic](schematic)

## Building the User Flash File From Source
The user flash file is basically the Gigatron's rom file, but in a format that's understood by the Gowin Programmer (ending: .fi). Unlike the gigatron's rom file which has a maximum size of 64K, the maximm size of the user flash file is 38K. This is due to the size of the embedded user flash of 608 KBit = 38K 16-bit words.
If you want to (re)build it, type "make games.rom" in a Linux command shell. I was using WSL/Ubuntu under Windows. Cygwin, MinGW, MSYS etc. do not work (at least not for me) 

