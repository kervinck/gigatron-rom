# Gigatron specific files

This directory contains files that accompany the lcc proper to support the gigatron.

 * `glcc` : main compiler driver written in python. This code parses some options and calls the standard lcc compiler driver. What the standard compiler driver is also was customized in [`gigatron-lcc.c`](../etc/gigatron-lcc.c) as is normal for lcc. Then changes were also made in the lcc compiler driver to support the assembler/linker combination. It would be nice to consolidate all the compiler driver business into a single python file.
 * `glink`, `glink.py`: vCPU assembler and linker.  This is one of the most important part in this whole project. The other one is the gigatron machine description for lcc, [`gigatron.md` (not a markdown file)](https://raw.githubusercontent.com/lb3361/gigatron-lcc/master/src/gigatron.md). 
 * `interface.json`: the standard Gigatron interface file which is read by `glink` to define well known symbols.
 * `roms.json`: rom capability description file, which is read by both `glink` and `glcc`.
 * `libc`: intended to contain an implementation of the ANSI C libary. The driver `glcc` uses this library by default.
 * `runtime` : assembly routines that support the compiler (multiplications, divisions, long support, fp support.)
 * `map32k`, `map64k`, etc : directories that define linker maps. Use `glcc -map=64k --info` for a description of each of them.
 * `mapsim` : contains a gigatron emulator [`gtsim`](mapsim/gtsim.c) that redirects the gigatron program stdio to the simulator stdio. Useful for testing.
 * `tst` : gigatron outputs of the standard lcc test suite.
 * `install-sh` : a shell script that aids installation on unix machines. 

Note that the gigatron include files are in [`include/gigatron`](../include/gigatron) and not `gigatron/include`. 
