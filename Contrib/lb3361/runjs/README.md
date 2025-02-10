
The contents of the `html` directory is a variant of Phil Thomas'
Javascript Gigatron emulator with support for the emulation of a 128K
RAM&IO expansion board with a SD card on port SPI0.

File 'http-server.py' invokes the python http server with an extension
that looks for precompressed files and serves with a Content-Encoding
header. This is useful to serve large binary files.

## Usage

* GT1 files can be loaded using the "load gt1" button
  or by dropping a `.gt1` file onto the VGA emulated display.

* SD disk images can be loaded using the "mount vhd" button or by
  dropping a `.vhd` or `.img` file onto the VGA emulated display.


## License

The original program is under Phil's license in file LICENSE.
The modifications are contributed under the same license.




