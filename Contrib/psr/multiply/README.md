# Multiplication Routines

This directory includes two implementations of byte by byte multiplication
in native code. It's pretty fast, but comes at the cost of a very large 
(two pages) lookup table.

The algorithm used is [Quarter square multiplication](https://en.wikipedia.org/wiki/Multiplication_algorithm#Quarter_square_multiplication)
with some caveats: multiplying two 8-bit values would require a four page
lookup table, and I couldn't find a way to implement that.
However it was possible to implement a two-page table sufficient for
multiplying 7-bit numbers.
This is then extended by other means.

This code has not been tested on real hardware,
and it uses instructions that do not seem to be used elsewhere
(bne ac and bge ac). gtemu.c and asm.py seem happy with them,
so I do expect them to work.

The purpose of this code is threefold:

* Just for fun. I read about the algorithm and wanted to see how hard it was to implement, and if it turned out to be fast.
* As a demonstration of an approach to native Gigatron programming. I've tried to make the code relatively easy to read, more below.
* As a testbed and demonstration of py-gtemu

## What's here

There are two implementations, and a Mandelbrot demo.

### The proof of concept

The first implementation,
and the one you should read if you want to understand the code is src/multiplication.asm.py.

This is a file that contains nothing but the multiplication routine.
It was where I developed the code.
If loaded on a Gigatron it would do nothing at all,
it is exercised only by the tests in test/test_multiplication.asm.py

If you want to understand how it works,
I suggest reading the commits that affected those files,
rather than reading it all in one go.
It should break the implementation into logical chunks:
first looking up the low-byte of a quarter square,
then looking up the high-byte,
then implementing the full routine for 7-bit numbers,
then extending it to 8-bit numbers.
As the code got bigger the whole file became harder to understand,
but I tried to keep the diffs readable.

### The SYS function

The second implementation is the same thing, but with some optimisations,
and implemented as a SYS function.
This is built in to a modified version of the ROMv5a.asm.py script. 
It can be found in sys/ROM.asm.py.
Again, the diffs are probably the best way to view the code,
as there is naturally a lot of unrelated code in there.
The reason that this had to be written in, 
and not a seperate .py file, 
is that I need code in the page after the shift table.

The major differences between the two versions are that in the SYS function:

* I neglected comments
* I store the lookup table inverted, because
* I do the subtraction step before the addition. This makes things a bit faster
* I do the 7-bit multiplication first instead of last.
  This means that the control flow splits but does not merge again,
  which is better for SYS functions.
* The cycle counting is done "Marcel style" with each instruction being numbered.

The sys function takes 120 cycles,
or 92 if both bytes are known to be <128, 
i.e. they have the top bit clear.
I haven't used a slot to allow for the code moving, 
but just started in the middle of the lookup table.
This saves some cycles, and this is just a demo.

The SYS function takes the two bytes in sysArgs0 and sysArgs1,
and returns a 16-bit product in vAC.
Neither parameter is modified,
to make it easy to reuse them on subsequent calls, 
but sysArgs2-5 are trampled.

### Mandelbrot

In order to see how performant the SYS function is,
I modified Mandelbrot.gcl to make use of it.

The answer is quite a lot faster than the vCPU shift-and-add implementation,
but not as much as I might have hoped.
In the default graphics mode it completes in under 8 minutes to render the first view,
going by the clock on the screen.
This takes something like 17 minutes using Marcel's code.

I suspect that there is scope for improvement,
either by avoiding calling the SYS function with zero as a parameter,
or rearranging code to minimise stalls.
I'd like to do some profiling to see where time is being spent.

## How to run it

### To run the Mandelbrot demo
Assuming you are in this directory,
you can add ../../../Core to your Python module search path (e.g. set PYTHONPATH) and run

    python3 ./sys/ROM.asm.py "Reset=../../../Core/Reset_v5.gcl" "Main=mandelbrot/Mandelbrot.gcl" "Boot=../../../Apps/CardTest/CardBoot.gcl"

(convert slashes as appropriate on your platform)

This should generate a ROM.rom and ROM.lst file, which you can run in your preferred emulator

### To run the tests
As above you need the Core directory on the search path, and you will also need:

* py-gtemu (follow the instructions in Contrib/psr/py-gtemu)
* pytest
* hypothesis

With these available you can just run pytest from this directory

You can see my approach to this
(creating a virtualenv, with a .pth file for Core, and installing dependencies)
in psakefile.ps1.

## Future work

I'd like to do the following

* Do more acurate benchmarking of Mandelbrot
* Move the code for loading Mandelbrot.gcl in the test to py-gtemu
* Do some profiling of MulShift8, and see if it can be sped up
* See if using PyPy helps make any of the above faster
