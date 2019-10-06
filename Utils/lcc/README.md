LCC for Gigatron
================

C compiler targetting the Gigatron TTL computer

ANSI C cross-compiler (C89) that generates .gt1 files. These files
can be loaded with the Loader application, embedded into an EPROM,
or loaded into a Gigatron emulator.

Based on Pat Gavlin's excellent initial work on adding a Gigatron backend to
`lcc`. Requires Python 3.6+.

This is still very much work in progress, some of the many
restrictions include:
 * No floating point (this will probably be the case for a _very_ long time)
 * Sometimes hangs, or crashes with a stack trace instead of a nice message
 * Especially on 'complex' expressions. We still have a registor allocation issue
 * No large structs and arrays (they must typically be much smaller than 100 bytes)
 * Work on libraries has barely started. No malloc yet. No file system.
 * Comparisons are still broken with respect to overflow
 * 'long' and 'long long' are 16-bit
 * ...

See the open GitHub issues with "lcc:" in their title for the current status.
Link: https://github.com/kervinck/gigatron-rom/issues?&q=is%3Aissue+is%3Aopen+lcc%3A

Resources:
 * Forum thread: https://forum.gigatron.io/viewtopic.php?f=4&t=116
 * Gigatron website: https://gigatron.io/
 * LCC website: http://sites.google.com/site/lccretargetablecompiler/
 * Original LCC sources: https://github.com/drh/lcc

Instructions
============

Build `lcc` from the root directory of the gigatron-rom/ repro:

```
$ make lcc
mkdir -p Utils/lcc/build
cd Utils/lcc && env HOSTFILE=etc/gt1h.c make all
cc -g -c -Isrc -o build/main.o src/main.c
cc -g -c -Isrc -o build/alloc.o src/alloc.c
cc -g -c -Isrc -o build/bind.o src/bind.c
cc -g -c -Isrc -o build/dag.o src/dag.c
[
 ...lots of output and compiler warnings snipped...
]
cp rt.py build/rt.py
cp asm.py build/asm.py
$
```

All of `lcc` is now in the directory Utils/lcc/build/. The compiler
has many subprograms and files, with `lcc` itself being the driver
for it all. We can simply keep everything there.

Next compile the test program. The Gigatron backend requires Python
3.6 or up to be installed on the system.

```
$ make Libs/Example.gt1
Utils/lcc/build/lcc -ILibs -c Libs/sys/ClearScreen.c -o Libs/sys/ClearScreen.o
Utils/lcc/build/lcc -ILibs -c Libs/sys/Newline.c -o Libs/sys/Newline.o
Utils/lcc/build/lcc -ILibs -c Libs/sys/Random.c -o Libs/sys/Random.o
Utils/lcc/build/lcc -ILibs -c Libs/stdio/putchar.c -o Libs/stdio/putchar.o
Utils/lcc/build/lcc -ILibs -c Libs/stdio/puts.c -o Libs/stdio/puts.o
Utils/lcc/build/lcc -ILibs -c Libs/Example.c -o Libs/Example.o
Utils/lcc/build/lcc -ILibs Libs/sys/ClearScreen.o Libs/sys/Newline.o Libs/sys/Random.o Libs/stdio/putchar.o Libs/stdio/puts.o Libs/Example.o -o Libs/Example.gt1
$ ls -l Libs/Example.gt1
-rw-r-----  1 marcelk  staff  743 Apr 29 22:59 Libs/Example.gt1
```

Now we have a program that we can send to a Gigatron with sendFile.py
(Python 2...!), or load into directly into one of the emulators.

Original README from lcc project
================================

This hierarchy is the distribution for lcc version 4.2.

lcc version 3.x is described in the book "A Retargetable C Compiler:
Design and Implementation" (Addison-Wesley, 1995, ISBN 0-8053-1670-1).
There are significant differences between 3.x and 4.x, most notably in
the intermediate code. For details, see
http://storage.webhop.net/documents/interface4.pdf.

VERSION 4.2 IS INCOMPATIBLE WITH EARLIER VERSIONS OF LCC. DO NOT
UNLOAD THIS DISTRIBUTION ON TOP OF A 3.X DISTRIBUTION.

LOG describes the changes since the last release.

CPYRIGHT describes the conditions under you can use, copy, modify, and
distribute lcc or works derived from lcc.

doc/install.html is an HTML file that gives a complete description of
the distribution and installation instructions.

Chris Fraser / cwf@aya.yale.edu
David Hanson / drh@drhanson.net
$Revision$ $Date$
