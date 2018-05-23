# Gigamidi
Takes the bin output from Miditones https://github.com/LenShustek/miditones and generates source<br/>
code data that you can include in your projects for MIDI scores.<br/>

## Building
A single source file using any modern C\+\+11 or higher compiler, trivial to build at a command prompt.</br>

## Usage
- gigamidi <input filename> <output filename> <format 0, 1, 2, 3> <address> <line length><br/>
- Input: Miditones binary file produced with miditones, e.g. miditones -t4 -b <filename>.bin<br/>
- Format: 0 = vCPU ASM, 1 = GCL, 2 = C/C++, 3 = Python<br/>

## Example
gigamidi game_over.bin game_over.i 0 0x8000 100<br/>

## Format
The output format is very similar to the Miditones output format except for a few crucial differences.<br/>
1) The Gigatron's Maximum channels, (tone generators), is limited to 4, so you **_must_** use the -t4 option<br/>
with Miditones.<br/>
2) The Gigatron does not support volume or instrument changes, so you **_cannot_** use the -i or -v options<br/>
with Miditones.<br/>
3) The wait or delay command has been changed from 2 bytes and 1ms resolution to 1 byte and 16.66667ms<br/>
resolution; this has a number of important impacts.
    - You cannot have a delay longer than 2116ms, so any MIDI score that does will have serious timing issues.<br/>
    - Very short delays will either be rounded down to 0ms or 16.6666667ms, this **_will_** affect timing,<br/>
how much of a problem it causes is completely dependent on the MIDI score and your playback<br/>
architecture.<br/>

## Stream
The byte stream produced by Miditones is composed of a number of commands, these commands are differentiated<br/>
from delays by the most significant bit of the command byte. The only commands that are supported<br/>
by the Gigatron and hence Gigamidi are the following:<br/>
1) **_Note On_**        $9t $nn, play note **_nn_** on tone generator **_t_**.<br/>
2) **_Note Off_**       $8t, stop playing on tone generator **_t_**.<br/>
3) **_Stop playing_**   $F0, stops playing on all channels.<br/>
4) **_Restart_**        $E0, reset stream to the beginning and restart the MIDI score.<br/>
5) **_Wait_**           $nn, waits **_nn_** x 16.666666667ms before processing the next command in<br/>
the stream. So the maximum delay is 0x7F x 16.66666667ms.<br/>

## Output
- vCPU ASM
~~~
game_overMidi       EQU     0x8000
game_overMidi       DB      0x90 0x53 0x91 0x47 0x07 0x90 0x52 0x91 0x46 0x07 0x90 0x53 0x91 0x47
                    DB      0x07 0x90 0x52 0x91 0x46 0x07 0x90 0x53 0x91 0x47 0x07 0x90 0x54
                    DB      0x91 0x48 0x07 0x90 0x53 0x91 0x47 0x07 0x90 0x52 0x91 0x46 0x07
                    DB      0x90 0x53 0x91 0x47 0x1d 0x80 0x81 0xf0
~~~
- GCL
~~~
$8000:
[def
  $90# $53# $91# $47# $07# $90# $52# $91# $46# $07# $90# $53# $91# $47# $07# $90# $52# $91# $46#
  $07# $90# $53# $91# $47# $07# $90# $54# $91# $48# $07# $90# $53# $91# $47# $07# $90# $52#
  $91# $46# $07# $90# $53# $91# $47# $1d# $80# $81# $f0#
] game_overMidi=
~~~
- C++
~~~
uint8_t game_overMidi[] =
{
    0x90,0x53,0x91,0x47,0x07,0x90,0x52,0x91,0x46,0x07,0x90,0x53,0x91,0x47,0x07,0x90,0x52,0x91,0x46,
    0x07,0x90,0x53,0x91,0x47,0x07,0x90,0x54,0x91,0x48,0x07,0x90,0x53,0x91,0x47,0x07,0x90,0x52,
    0x91,0x46,0x07,0x90,0x53,0x91,0x47,0x1d,0x80,0x81,0xf0,
};
~~~

- Python
~~~
game_overMidi = bytearray([
    0x90,0x53,0x91,0x47,0x07,0x90,0x52,0x91,0x46,0x07,0x90,0x53,0x91,0x47,0x07,0x90,0x52,0x91,0x46,
    0x07,0x90,0x53,0x91,0x47,0x07,0x90,0x54,0x91,0x48,0x07,0x90,0x53,0x91,0x47,0x07,0x90,0x52,
    0x91,0x46,0x07,0x90,0x53,0x91,0x47,0x1d,0x80,0x81,0xf0,
])
~~~