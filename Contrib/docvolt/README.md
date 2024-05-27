# Gigatron running on FPGA
Verilog implementation of the Gigatron for the Tang Nano 9K FPGA board.

## Installation
Download and install the free *GOWIN Programmer Vx.x.x. Education* or *Gowin Vx.x.x.Beta-4 Education* if you want to make changes to the code: https://www.gowinsemi.com/en/support/download_eda/ (Prior registration is required)

Note: You need to apply for a license, if you run the non-education version of the programs. The education version is fully sufficient here.

## Flashing the Tang Nano
- Double-click the cell below "Operation" to open the Device configuration dialog.
- Select the bitstream file "gigatron.fs" and the flash initialization file "fpga_games.fi"


    <img src="images/programmer.JPG" width="800">

- Then save and click the "Configure/Program" button to write it all to the FPGA.

## Adapter board
A VGA monitor, game controller and audio can be connected through the adapter board.
<img src="images/board.png" width="800">

Find schematics and Gerber files of the adapter board here: [Schematic](schematic)

