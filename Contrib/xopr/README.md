# NetTerm
This gigatron program is an attempt to create a terminal that talks over SPI1 to the outside world.
Every keypress on the gigatron will be transfered over SPI, and every byte (apart from ANSI escapecodes) goes from SPI to the screen.
For the first run, execute `make lcc` in the *gigatron-rom* root folder.
run `make Contrib/xopr/NetTerm/NetTerm.gt1` in the root folder to build a new version.

If you want to test the NetTerm without expander hardware (i.e., emulator), you can comment out the following define:
```
//#define IOPORT SS1
```
**Note:** 
This application is still experimental and written to work best with a [Gigatron expander card](https://forum.gigatron.io/viewtopic.php?f=4&t=64)  combined with a patched *dev.rom* that enables drawing of the 3x5 'tinyfont'.
To apply the  patch, run `git apply Contrib/xopr/NetTerm/SYS_VDrawWord.patch` followed by `make dev.rom` and `make burn` to compile and burn the DEVROM to EPRPOM.

The patch itself was created using:
```
git add Core/dev.py
git commit -m "Added SYS_VDrawWord function"
git format-patch -1 master --stdout > Contrib/xopr/NetTerm/SYS_VDrawWord.patch
git reset HEAD^
git checkout -- Core/dev.py
```

# SPI_slave
This Arduino program acts like a SPI slave that transfers bytes from and to the (USB) serial connection.
It can be used to hook the gigatron up to the PC as a terminal client, or powered without USB, used as TTL serial (pins 0 and 1).

**Note:**
Make sure you disable the Arduino reset when using it as a USB serial terminal.
For programming, either re-enable Arduino reset or reset it manually directly after clicking upload.
Newer Arduinos have a scratch/solder pad for that (which one can scratch through and solder a jumper on): it should be in the middle of an Arduino uno with a label "Reset on".

## Run a terminal server
To run a terminal server for the (Arduino) Serial port, verify which port it uses (in my case, it was /dev/**ttyACM0**.

On a systemd linux machine:

* run: `systemctl enable serial-getty@ttyACM0.service` (or the appropriate Arduino serial device)
 * edit: `/etc/systemd/system/getty.target.wants/serial-getty@ttyACM0.service`
    * and change `ExecStart=...... $TERM` into `ExecStart=...... vt100`
 * then run: `systemctl daemon-reload`
 * and: `systemctl restart serial-getty@ttyACM0.service`


