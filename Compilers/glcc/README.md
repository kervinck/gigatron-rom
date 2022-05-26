

# GLCC


This LCC--derived compiler and C library targets the [Gigatron](http://gigatron.io) VCPU.
It keeps many of the ideas of the previous attempt to port LCC to the
Gigatron (pgavlin's).  For instance it outputs assembly code that can be parsed by
Python and it features a linker writen in Python that can directly
read these files. It also differs in important ways. For instance the
code generator is fundamentally different.

## 1. Status

This project provides a complete toolchain and C library for ANSI C 1989.

### 1.1. Implementation details and caveats

Some useful things to know:

  * Types `short` and `int` are 16 bits long.  Type `long` is 32 bits
	long. Types `float` and `double` are 40 bits long, using the
	Microsoft Basic floating point format. Both long arithmetic or
	floating point arithmetic incur a significant speed penalty.
	
  * Type `char` is unsigned by default. This is more efficient because
	the C language always promotes `char` values into `int` values to
	perform arithmetic. Promoting a signed byte involves a clumsy sign
	extension. Promoting an unsigned byte comes for free with most
	VCPU opcodes. If you really want signed chars, use `signed char`
	or maybe use the compiler option `-Wf-unsigned_char=0`. This
	is not recommended. 

* The traditional C standard library offers feature rich functions
  like `printf()` and `scanf()`. These functions are present but their
  implementation requires a lot of space. Instead one can call some of
  the lower level functions that either are standard like `fputs()` or
  non standard functions such as `itoa()`, `dtoa()` whose prototypes
  are provided by [`<gigatron/libc.h>`](include/gigatron/gigatron/libc.h).
  
* Alternatively one can completely bypass stdio and use the 
  low-level console functions whose prototypes are provided in
  [`<gigatron/console.h>`](include/gigatron/gigatron/console.h).
  Even lower-level functions are provided in 
  [`<gigatron/sys.h>`](include/gigatron/gigatron/sys.h).
  
* Many parts of the main library can be overriden to provide special 
  functionalities. For instance the console has the usual 15x26 characters, 
  but this can be changed by linking with a library that redefines 
  what is in `cons_geom.c`. This is what happens when one uses argument
  `-map=conx` to save memory with a 10x26 console. Another more 
  important example is the standard i/o library. By default
  it is only connected to the console and `fopen()` always fail.
  But one just has to redefine a few functions to change that.
  This is one happens when one compiles with `-map=sim` which
  [forwards](gigatron/mapsim/libsim) all the stdio calls to 
  the emulator. Note that such binaries only run in `gtsim`
  because they attempt to communicate with `gtsim` to forward
  the stdio calls.
  
* Over time the linker `glink` has accumulated 
  a lot of capabilites. It supports common symbols,
  weak symbols, and conditional imports. It can synthetize magic lists
  such as a list of initialization functions that are called before main,
  a list of data segments to be cleared before running main, a list
  of memory segments that can be used as heap by `malloc()`, or a list
  of finalization functions called when the program exits.
  Using `glink -h` provides some help. Using `glink --info` provides
  help for specific memory maps. But the more advanced functions
  are documented by comments in the [source](gigatron/glink.py)
  or comments in the library source files that use them...
  
### 1.2. What remains to be done?

*  The library implements ANSI C 89 but few Gigatron specific functions.
   For instance, one could add more Gigatron SYS stubs into `gigatron/libc/gigatron.s`,
   one could provide a graphic and sprite library, etc.

*   The compiler could also be improved but with diminishing returns.
    A way to begin could be, for instance, to rewrite the python
    compiler driver `glcc` to be self-contained. The current version
    delegates a lot of work to the historical lcc driver `lcc` which
    then calls the python assembler and linker. Cleaning this up 
    would make option processing simpler to understand. This
    is harder than it seems.

## 2. Compiling and installing

You can build GLCC from source using two methods.

* The first method relies on the traditional `make` command
  using the usual Posix command line utilities. This is the method used
  for development and therefore is the recommended method
  for Linux machines or Macs. It can also be used
  on Windows when compiling with `cygwin` (https://cygwin.org) 
  or `mingw64/msys2` (https://www.mingw-w64.org/). 
  
* The second method relies on `cmake` (https://cmake.org)
  and supports many different toolchains such as Microsoft
  Visual Studio, etc.
  
### 2.1 Building gigatron-lcc with Make

Because the primary GLCC development platform is Linux. 
building `gigatron-lcc` with make on a Unix platform 
should be very easy provided that a C compiler, bison, 
gnu-make >= 4.0, and python >= 3.8 are installed.
Simply type:
```
$ git clone https://github.com/lb3361/gigatron-lcc.git
$ cd gigatron-lcc
$ make
```
Then you can either invoke the compiler from its build location `./build/glcc` or
install it into your system with command
```
$ make PREFIX=/usr/local install
```
where variable `PREFIX` indicates where the compiler should be installed.
This command copies the compiler files into `${PREFIX}/lib/gigatron-lcc/` 
and symlinks the compiler driver `glcc`, the linker driver `glink`,
and the simulator `gtsim` into `${PREFIX}/bin`. All the other files are located under 
`${PREFIX}/lib/gigatron-lcc`. Note that this directory can be relocated
elsewhere in the system as long as its contents is preserved.
You just need to either invoke `glcc` using the full path of
the `gigatron-lcc` directory. Alternatively you can place symbolic links
to these files somewhere in the executable search path.

There is also 
```
$ make test
```
to run the current test suite. 

### 2.2 Building gigatron-lcc with CMake

The prerequisites are python >= 3.8 and cmake >= 3.16.

In order to compile with cmake, you must first create a build directory
and invoke CMake from that build directory. 
For instance, on a Unix machine, 
```
$ git clone https://github.com/lb3361/gigatron-lcc.git
$ cd gigatron-lcc
$ mkdir build
$ cd build
$ cmake .. 
```

This operation creates a Makefile in the build directory.
You can then compile with `make`
```
$ make
```
Then you can either invoke the compiler from its build location `./glcc` or
install it into your system with command
```
$ cmake --install .  -DCMAKE_INSTALL_PREFIX=/usr/local
```
where variable `CMAKE_INSTALL_PREFIX` is the directory where glcc
should be installed.  Note that this directory can be relocated
elsewhere as long as the relative position of the glcc files
is preserved.


### 2.3 Windows notes

#### 2.3.1 Cygwin GLCC

Thanks to the feedback of axelb, you can use the `make` method
to compile gigatron-lcc under cygwin. For this, you must first
install cygwin >= 3.2 from http://cygwin.org, make sure to 
select the packages `gcc-core`, `make`, `bison`, `git`, and `python3`, 
then issue the `make`, `make install`, or `make test` 
command from the cygwin shell.
The main drawback of building `gigatron-lcc` under cygwin is
that you have to execute it from the cygwin shell as well since
it depends on the cygwin infrastructure.

#### 2.3.2 Native Windows GLCC 

For this, you need a native version of Python (https://python.org).
It is also highly recommended to install Git-for-Windows (https://gitforwindows.org/)
and a version of GNU make for windows, for instance using 
Chocolatey (https://community.chocolatey.org/packages/make).

* A first option is to use the mingw64 compiler (https://mingw-w64.org).
  This compiler comes in various guises. After a bit of experimentation,
  the recommended approach is to download the 32 bits version of 
  [Git for Windows SDK](https://github.com/git-for-windows/build-extra/releases/latest).
  This is certainly an overkill, but a very reliable one.
  You can then start the Git for Windows SDK shell, clone the Gigatron LCC repository,
  and use the `make` method discussed above. A precompiled version with installation 
  and usage instructions can be found in the forum 
  post https://forum.gigatron.io/viewtopic.php?p=2484#p2484.
  
* A second option is to use the CMake approach with any supported toolchain.
  For this you need to create a build directory and run the CMake program
  to generate project files. Please follow the instructions that come
  with CMake to select the proper toolchain, compile the project, and
  optionally set CMAKE_INSTALL_PREFIX and trigger the installation target.

Both options create a `bin` directory with Window batch files, e.g
`glcc.cmd`, etc., that can be used to invoke GLCC from the DOS command
line, from PowerShell, or from the Git Bash. These batch files rely on
the `py` launcher that is installed by default with recent versions of
Python for windows. Just add this directory to the executable search
path, e.g.  `PATH=/c/glcc/bin;%PATH%` at the DOS command line or
`PATH=/c/glcc/bin:$PATH` at the Git Bash command line. T

## 3 Compiler invocation

Besides the options listed in the [lcc manual page](doc/lcc.1),
the compiler driver `glcc` recognizes a few Gigatron-specific options.
Additional options recognized by the assembler/linker `glink`
are documented by typing `glink -h`
 	
  * Option `-rom=<romversion>` is passed to the linked and helps
	selecting runtime code that uses the SYS functions implemented by
	the indicated rom version. The default is `v5a` which does not
	provide much support at this point.
	
  * Option `-cpu=[456]` indicates which VCPU version should be
    targeted.  Version 5 adds the instructions `CALLI`, `CMPHS` and
    `CMPHU` that came with ROMv5a. Version 6 will support AT67's new
    instruction once finalized. The default CPU is the one implemented
    by the selected ROM.

  * Option `-map=<memorymap>{,<overlay>}` is also passed to the linker
    and specifya memory layout for the generated code. The default
    map, `32k` uses all little bits of memory available on a 32KB
    Gigatron, starting with the video memory holes `[0x?a0-0x?ff]`,
    the low memory `[0x200-0x6ff]`. There is also a `64k` map and a
    `conx` map which uses a reduced console to save memory.
    Additional information about each map can be displayed by 
    using option `-info` as in `glcc -map=sim -info`
	
    Maps can also manipulate the linker arguments, insert libraries,
	and define the initialization function that checks the rom type
	and the ram configuration. For instance, map `sim` produces gt1
	files that only run in the emulator [`gtsim`](gigatron/mapsim) with a
	library that redirects `printf` and all standard i/o functions to
	the emulator itself. This is my main debugging tool.
	

## 3. Examples

### 3.1. Running the LCC 8 queens program:

```
$ ./build/glcc -map=sim tst/8q.c 
tst/8q.c:30: warning: missing return value
tst/8q.c:37: warning: implicit declaration of function `printf'
tst/8q.c:39: warning: missing return value
$ ./build/gtsim -rom gigatron/roms/dev.rom a.gt1 
1 5 8 6 3 7 2 4 
1 6 8 3 7 4 2 5 
1 7 4 6 8 2 5 3 
1 7 5 8 2 4 6 3 
2 4 6 8 3 1 7 5 
2 5 7 1 3 8 6 4 
2 5 7 4 1 8 6 3 
2 6 1 7 4 8 3 5 
2 6 8 3 1 4 7 5 
2 7 3 6 8 5 1 4 
2 7 5 8 1 4 6 3 
2 8 6 1 3 5 7 4 
...
```

### 3.2. Capturing signals:
```
$ cat gigatron/libc/tst/TSTsignal.c 
#include <string.h>
#include <stdio.h>
#include <signal.h>

int a = 3;
long b = 323421L;
volatile int vblcount = 0;
extern char frameCount;

int handler(int signo, int fpeinfo)
{
	printf("handle %d %d\n", signo, fpeinfo);
	return 1234;
}

long lhandler(int signo, int fpeinfo)
{
	printf("handle %d %d\n", signo, fpeinfo);
	return 1234L;
}

void vhandler(int signo)
{
	printf("SIGVIRQ(%d): count=%d\n", signo, vblcount++);
	frameCount=255;
	signal(SIGVIRQ, vhandler);
}

int main()
{
	signal(SIGFPE, (sig_handler_t)handler);
	printf("%d/0 = %d\n", a, a / 0);
	signal(SIGFPE, (sig_handler_t)lhandler);
	printf("%ld/0 = %ld\n", b , b / 0);
	signal(SIGVIRQ, vhandler);
	while (vblcount < 10) { 
		b = b * b;
	}
	return 0;
}
$ ./build/glcc -map=sim  gigatron/libc/tst/TSTsignal.c 
$ ./build/gtsim  -rom gigatron/roms/dev.rom a.gt1 
handle 4 1
3/0 = 1234
handle 4 1
323421/0 = 1234
SIGVIRQ(7): count=0
SIGVIRQ(7): count=1
SIGVIRQ(7): count=2
SIGVIRQ(7): count=3
SIGVIRQ(7): count=4
SIGVIRQ(7): count=5
SIGVIRQ(7): count=6
SIGVIRQ(7): count=7
SIGVIRQ(7): count=8
SIGVIRQ(7): count=9
```

### 3.3. Running Marcel's simple chess program:

I found this program when studying the previous incarnation 
of LCC for the Gigatron, with old forums posts where Marcel
mentionned it as a "stretch goal" for the compiler. The main
issue is that MSCP takes about 25KB of code and 25KB of data
meaning that we need to use the video memory. My main change 
was to reduce the size of the opening book,
but this is not enough. One could think about using
the 128KB memory extention but this will require a lot
of changes to the code. In the mean time. we can
run it with the `gtsim` emulator which has no screen
but can forward stdio...

```
$ cp stuff/mscp.c .
$ cp stuff/book.txt .

# Using map sim with overlay allout commits all the memory
$ ./build/glcc -map=sim,allout mscp.c -o mscp.gt1
```

Now we can run it. Option -f in `gtsim` allows mscp to 
open and read the opening book file `book.txt`.
Be patient...

```
$ ./build/gtsim -f -rom gigatron/roms/dev.rom  mscp.gt1 

This is MSCP 1.4 (Marcel's Simple Chess Program)

Copyright (C)1998-2003 Marcel van Kervinck
This program is distributed under the GNU General Public License.
(See file COPYING or http://combinational.com/mscp/ for details.)

Type 'help' for a list of commands

8  r n b q k b n r
7  p p p p p p p p
6  - - - - - - - -
5  - - - - - - - -
4  - - - - - - - -
3  - - - - - - - -
2  P P P P P P P P
1  R N B Q K B N R
   a b c d e f g h
1. White to move. KQkq 
Compacted book 256 -> 57
Compacted book 256 -> 94
Compacted book 256 -> 131
Compacted book 256 -> 155
Compacted book 256 -> 171
Compacted book 256 -> 182
Compacted book 256 -> 194
Compacted book 256 -> 204
Compacted book 256 -> 216
Compacted book 256 -> 231
Compacted book 256 -> 231
Compacted book 256 -> 242
Compacted book 256 -> 247
Compacted book 256 -> 251
Compacted book 256 -> 253
Compacted book 256 -> 253
Compacted book 256 -> 256
Compacted book 256 -> 256
mscp> 
```

Now you can type `both` to make it play against itself...

```
mscp> both 
book: (88)e4
1. ... e2e4
8  r n b q k b n r
7  p p p p p p p p
6  - - - - - - - -
5  - - - - - - - -
4  - - - - P - - -
3  - - - - - - - -
2  P P P P - P P P
1  R N B Q K B N R
   a b c d e f g h
1. Black to move. KQkq 
book: (88)c5
1. ... c7c5
8  r n b q k b n r
7  p p - p p p p p
```
This slows down a lot when we leave the opening book.
But it plays!

## 4. Internals

The code generator uses two block of respectively 16 and 48 bytes in page zero.

  *  The first block is dedicated to runtime routines that implement
	 long and float arithmetic. It is located at fixed addres `0xc0-0xcf`
	 because it is expected that runtime support will be progressively
	 supported by ROM features. The long accumulator `LAC` occupies
	 addresses `0xc4-0xc7`. The float accumulator `FAC` overlaps `LAC` and
	 uses locations `0xc0-0xc7`. The remaining eight bytes at `0xc8-0xcf`
     are working space for these runtime routines. They are also known as
     registers `T0` to `T3` and are occasionally used as scratch
     registers by the code generator.
     
  *  The second block contains 24 general purpose word registers named
     `R0` to `R23`. This block is by default located at addresses
     `0x50-0x7f` but can be displaced using the command line option
     `--register-base=0x90` for instance. Register pairs named `L0` to
     `L22` can hold longs.  Register triplets named `F0` to `F21` can
     hold floats. Registers `R0` to `R7` are callee-saved and are
     often used for local variables. Registers `R8` to `R15` are used
     to pass arguments to functions. Registers `R15` to `R22` are used
     for temporaries. Register `R23` or `SP` is the stack pointer.

The function prologue first saves `vLR` and constructs a stack frame
by adjusting `SP`. It then saves the callee-saved registers onto the
stack.  Nonleaf functions save 'vLR' in the stack frame and copy the
argument passed in a registers to their final location. In contrast,
leaf functions keep arguments passed in registers where they are
because these registers are no longer needed for further calls.  In
the same vein, nonleaf functions allocate callee-saved registers for
local variables, whereas leaf functions use callee-saved registers in
last resord and often avoid having to construct a stack-frame
alltogether.  Leaf functions that do not need to allocate space on the
stack can use a register to save VLR and become entirely frameless.
Sometimes one can help this by using `register` when declaring local
variables. I have to find a way to make lcc more aggressive in that
respect.

Saving `vLR` allows us to use `CALLI` as a long jump
without fearing to erase the function return address.
This is especially useful when one needs to hop over page boundaries.

The VCPU accumulator `vAC` is not treated by the compiler as a normal 
register because there is essentially nothing the VCPU can do once the 
accumulator is allocated to represent a particular variable or a temporary.
This would force the compiler to spill its content to a stack location
in ways that not only produce less efficient code, but often result
in an infinite loop because the spilling code must itself use `vAC`.
Instead, the GLCC code generator produces VCPU instructions in bursts 
that are packed on a single line of the generated assembly code. 
Each burst is in fact what LCC calls one instruction. Bursts are
produced by subverting the mechanisms defined by LCC to construct 
various parts of a typical CPU instruction such as the mnemonic, 
the address mode, etc. The VCPU accumulator `vAC` is treated as a scratch 
register inside a burst. Meanwhile LCC allocates zero page registers 
to pass data across bursts. This approach avoid the spilling problems
but sometimes needs improving because we do not keep track
of what data is left on the accumulator after each burst.
This has been improved by adding code that opportunistically
allows using `vAC` to pass data between bursts.

The compiler produces a python file that first define a function for each
code or data fragment. The file then constructs a module that
holds a list of all the fragments, as well as all the imported and
exported symbols. The linker/assembler `glink` can read such files
or can read a library file that is merely the concatenation
of individual modules.  Each fragment is represented as a function
that calls predefined functions whose uppercase name mirrors the name
of the instruction they emit. Additional functions implement synthetic
opcodes that can be implemented differently by different VCPU versions.
More predefined functions are used to define labels or control
when to check for a page boundary. The source of truth for 
all this is the file `glink.py`.

The linker collects all the code and data fragments generated by the compiler.
It then analyzes import and exports to determine which ones should be
kept. It tries hard to place short functions into single segments in order
to avoid costly hops. Then it iterates until all symbols are resolved and
all symbol value dependent code is stabilized. Finally it produces a
familar `GT1` file.
