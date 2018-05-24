# Gigamidi
Takes the bin output from Miditones https://github.com/LenShustek/miditones and generates source<br/>
code data that you can include in your projects for MIDI scores.<br/>

## Building
A single source file using any modern C\+\+11 or higher compiler, trivial to build at a command prompt.</br>

## Usage
- gigamidi \<input filename\> \<output filename\> \<format 0, 1, 2, 3\> \<address in hex\> \<offset in hex\> \<count\> \<line length\><br/>
- Input: Miditones binary file produced with miditones, e.g. miditones -t4 -b \<filename\>.bin<br/>
- Format: 0 = vCPU ASM, 1 = GCL, 2 = C/C++, 3 = Python<br/>

## Address
The address, **_(specified in hex)_**, is the start address in RAM of where the MIDI byte sequence will be loaded to.<br/>

## Offset
The offset, **_(specified in hex)_**, is the offset used to determine the starting location of each new segment.<br/>

## Count
The count is the maximum number of bytes contained within each segment.

## Line length
The line length specifies the maximum size of each line of output source code.<br/>

## Example
gigamidi game_over.bin game_over.i 0 0x08A0 0x100 96 100<br/>
This would segment the MIDI stream into the unused areas of video memory, the segments are linked together<br/>
through the **_0xD0_** Segment command, (see below), and are automatically fetched and played in sequence.<br/>

## Format
The output format is very similar to the Miditones output format except for a few crucial differences.<br/>
1) The Gigatron's Maximum channels, (tone generators), is limited to 4, so you **_must_** use the -t4 option<br/>
with Miditones.<br/>
2) The Gigatron does not support volume or instrument changes, so you **_cannot_** use the -i or -v options<br/>
with Miditones.<br/>
3) The wait or delay command has been changed from 2 bytes and 1ms resolution to 1 byte and 16.66667ms<br/>
resolution; this has a number of important impacts.
    - You cannot have a delay longer than 2116ms, so any MIDI score that does will have serious timing issues.<br/>
    - Very short delays will either be rounded down to 0ms or rounded up to 16.6666667ms, this **_will_** affect<br/>
timing, how much of a problem it causes is completely dependent on the MIDI score and your playback architecture.<br/>

## Player

There is currently a MIDI player written in vCPU ASM that implements all the functionality within this specification.<br/>
Other languages are yet to be implemented as of the date of this document, but it is not a difficult task to accomplish.<br/>

Once a player has been written, it expects to be called at least once every 16.66666667ms, the simplest way to achieve this,<br/>
is to wait for **_VBlank_** and then call the player; this will work perfectly unless other parts of your code spend more than<br/>
one VBlank processing. If this is the case, these hot spots will need to have a **_Check VBlank Call Player_** function within<br/>
their loops or code.<br/>
~~~
playMidi        LDI     0x02                ; keep pumping soundTimer, so that global sound stays alive
                ST      giga_soundTimer
                LDW     midiDelay
                BEQ     playM_process

                SUBI    0x01
                STW     midiDelay
                RET

playM_process   LDW     midiStreamPtr
                PEEK                        ; get midi stream byte
                STW     midiCommand
                LDW     midiStreamPtr
                ADDI    0x01
                STW     midiStreamPtr
                LDW     midiCommand
                ANDI    0xF0
                STW     scratch
                XORI    0x90                ; check for start note
                BNE     playM_endnote

                PUSH                    
                CALL    midiStartNote       ; start note
                POP
                BRA     playMidi
                
playM_endnote   LDW     scratch 
                XORI    0x80                ; check for end note
                BNE     playM_segment

                PUSH
                CALL    midiEndNote         ; end note
                POP
                BRA     playMidi


playM_segment   LDW     scratch
                XORI    0xD0                ; check for new segment
                BNE     playM_delay

                PUSH
                CALL    midiSegment         ; new midi segment
                POP
                BRA     playMidi

playM_delay     LDW     midiCommand         ; all that is left is delay
                STW     midiDelay
                RET


midiStartNote   LDWI    giga_notesTable     ; note table in ROM
                STW     scratch
                LDW     midiStreamPtr       ; midi score
                PEEK
                SUBI    10
                LSLW
                SUBI    2
                ADDW    scratch
                STW     scratch
                LUP     0x00                ; get ROM midi note low byte
                ST      midiNote
                LDW     scratch
                LUP     0x01                ; get ROM midi note high byte
                ST      midiNote + 1
                LDW     midiCommand
                ANDI    0x03                ; get channel
                ST      scratch + 1
                LDI     0x00
                ST      scratch
                LDW     scratch
                ADDW    midiChannel         ; channel address
                STW     scratch
                LDW     midiNote
                DOKE    scratch             ; set note
                LDW     midiStreamPtr
                ADDI    0x01                ; midiStreamPtr++
                STW     midiStreamPtr
                RET


midiEndNote     LDW     midiCommand
                ANDI    0x03                ; get channel
                ST      scratch + 1
                LDI     0x00
                ST      scratch
                LDW     scratch
                ADDW    midiChannel         ; channel address
                STW     scratch
                LDWI    0x0000
                DOKE    scratch             ; end note
                RET


midiSegment     LDW     midiStreamPtr       ; midi score
                DEEK
                STW     midiStreamPtr       ; 0xD0 new midi segment address
                RET
~~~

## Stream
The byte stream produced by Miditones is composed of a number of commands, these commands are differentiated<br/>
from delays by the most significant bit of the command byte. The only commands that are supported by the Gigatron<br/>
and hence Gigamidi are the following:<br/>
1) **_Note On_**        $9t $nn play note **_nn_** on tone generator **_t_**.<br/>
2) **_Note Off_**       $8t stop playing on tone generator **_t_**.<br/>
3) **_Segment_**        $D0 $nnnn contains the absolute 16bit address of the next segment.<br/>
4) **_Wait_**           $nn waits **_nn_** x 16.666666667ms before processing the next command in<br/>
the stream. So the maximum delay is 0x7F \* 16.66666667ms.<br/>

The Segment command, **_(0xD0)_** is a powerful **_(Gigatron only)_**, command embedded within the MIDI byte stream,<br/>
(generated automatically by Gigamidi), that not only allows the MIDI data to be spread over multiple fragmented areas<br/>
of memory, but also allows you to sequence and chain multiple MIDI streams together without writing any code.<br/>
~~~
$08A0:
[def
  $90# $53# $91# $47# $07# $90# $52# $91# $46# $07# $90# $53# $91# $47# $07# $90# $52# $91# $46#
  $07# $90# $53# $91# $47# $07# $90# $54# $91# $48# $07# $90# $53# $91# $47# $07# $90# $52#
  $91# $46# $07# $90# $53# $91# $47# $1d# $80# $81# $d0# $a0# $09#
] game_overMidi=
~~~
The last command **_($d0# $a0# $09#)_** within the game over MID byte stream is a Segment address that points to the<br/>
next segment of MIDI data to process; in this example it would probably point back to the title MIDI byte stream.<br/>
If the game_overMIDI byte stream did not fit in that section of memory, then 0xD0 commands would be used to chain<br/>
multiple segments of the byte stream together.<br/>

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