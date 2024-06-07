
# RUNJS 128K

This version of the online gigatron emulator differs from the original one by Phil Thomas in the following ways:

* fix rare opcodes such as `st(IN,[d],X)`.
* small changes to help JavaScript JIT compilers.
* make audio and self-adjusted timing more robust.
* implement 128k RAM&IO expansion board.
* emulates a SD card connected to the expansion SPI.
* optionally uses python3's web server

The development source of this version of runjs is at 
[https://github.com/lb3361/gigatron-rom/tree/master/Utils/runjs](https://github.com/lb3361/gigatron-rom/tree/master/Utils/runjs) and will be synced here from time to time. This is the emulator available at [http://gigatron128k.com](http://gigatron128k.com).

## Usage

### Without a web server

Assuming you have python >= 3.10, type

```
$ python3 http-server.py
```
and point your browser to url
`http://127.0.0.1:8000`.


### With a web server

Copy the contents of the html directory into your server pages. 

Note that the disk image `sd.vhd.gz` comes compressed. To serve such a compressed image with Apache, use the contents of `apache-htaccess` as a `.htaccess` file. Otherwise you have to uncompress this file.

## How to

### How to emulate a Gigatron 32k or 64k

Search for the following in file `html/main.js`

```
    let cpu = new Gigatron({
        hz: HZ,
        romAddressWidth: 16,
        ramAddressWidth: 17,
    });
```
and change `ramAddressWidth` to 15 for a Gigatron 32k and 16 for a Gigatron 64k. This disables both the emulation of the RAM&IO expansion board and the SD card.

### How to use a different ROM

Change the symbolic link `html/gigatron.rom`. Alternate roms can be found in directory `html/roms`.

### How to use a different SD card

The SD cards are represented by VHD files containing images of a FAT32 filesystem. Such images can be created with a variety of tools such as [https://github.com/lb3361/dosfs](https://github.com/lb3361/dosfs).

The simplest way is to mount a VHD file using the "Mount VHD" button or by dragging a VHD file onto the emulator.

The symbolic link `html/sd.vhd` or `html/sd.vhd.gz` points to a disk image that is preloaded when the emulator starts. Note that the default image is compressed and require installing a special `.htaccess` file in an actual web server. Alternatively, uncompress the image and name it `sd.vhd` instead of `sd.vhd.gz`.



## License

The original program is under Phil's license in file LICENSE.
The modifications are contributed under the same license.




