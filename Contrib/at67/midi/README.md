# Gigamidi
Takes the bin output from Miditones https://github.com/LenShustek/miditones and generates source<br/>
code data that you can include in your projects for MIDI scores.<br/>

## Building
A single source file using any modern C\+\+11 or higher compiler, trivial to build at a command prompt.</br>

## Usage
- gigamidi \<input filename\> \<output filename\> \<format 0, 1, 2, 3\> \<address in hex\> \<offset in hex\></br>
\<int count> \<int line_length> \<float timing_adjust><br/></br>
- Input: Miditones binary file produced with miditones:
  e.g.  miditones -t4 -b \<filename\>.bin, 4 channels, binary.<br/>
        miditones -t4 -b -s2 -pi \<filename\>.bin, 4 channels, binary, use method2 for prioritising notes, disable drums.<br/>
- Format: 0 = vCPU ASM, 1 = GCL, 2 = C/C++, 3 = Python<br/>

## Format
The output format is very similar to the Miditones output format except for a few crucial differences.<br/>
1) The Gigatron's Maximum channels, (tone generators), is limited to 4, so you **_must_** use the -t4 option<br/>
   with Miditones.<br/>
2) The Gigatron does not support volume or instrument changes, so you **_cannot_** use the -i or -v options<br/>
   with Miditones.<br/>
3) The wait or delay command has been changed from 2 bytes and 1ms resolution to a variable length 1 byte stream<br/>
   and 16.66667ms resolution; this has an important impact.<br/>
   - Very short delays will either be rounded down to 0ms or rounded up to 16.6666667ms, this **_will_** affect<br/>
     timing, how much of a problem it causes is completely dependent on the MIDI score and your playback architecture.<br/>
   - The limit of 2116ms maximum delay has been lifted, gigamidi and the GCL and vASM players both support a variable<br/>
     length byte stream of delays.
   - Sequences of delays produced by miditones are coalesced into a single delay and then saved as a variable length<br/>
     byte stream.
   - Zero size delays produced by miditones are ignored.

## Address
The address, **_(specified in hex)_**, is the start address in RAM of where the MIDI byte sequence will be loaded to.<br/>

## Offset
The offset, **_(specified in hex)_**, is the offset used to determine the starting location of each new segment.<br/>

## Count
The count is the maximum number of bytes contained within each segment.

## Line length
The line length specifies the maximum size of each line of output source code.<br/>

## Timing Adjust
The timing adjust specifies a delta that attempts to adjust the overall timing to more closely match the original timing.<br/>
Normal values are {0.0...1.5}, results will vary depending on many factors, experimentation is key.

## Example
gigamidi game_over.bin game_over.i 0 0x08A0 0x100 96 100<br/>
This would segment the MIDI stream into the unused areas of video memory, the segments are linked together<br/>
through the **_0xD0_** Segment command, (see below), and are automatically fetched and played in sequence.<br/>


## Player

There is currently a MIDI player written in vCPU ASM and GCL that implements all the functionality within this<br/> specification.<br/>

Once a player has been written, it expects to be called at least once every 16.66666667ms, the simplest way to achieve this,<br/>
is to wait for **_VBlank_** and then call the player; this will work perfectly unless other parts of your code spend more than<br/>
one VBlank processing. If this is the case, these hot spots will need to have a **_Check VBlank Call Player_** function within<br/>
their loops or code.<br/>

**_GCL Player_**
~~~
gcl0x

[def { ResetAudio -- reset audio hardware }
  0 midiCommand=
  0 midiDelay=
  0 midiNote=
  0 frameCountPrev=
  $01FC midiChannel=
  $8000 midiStreamPtr=
  $01FA scratch= 

  3 ii=
  [do
    $FA scratch<.   { reset low byte }
    $0300 scratch:  { Doke $0300 into wavA, wavX}  
    scratch<++      { scratch + 0x0002 }
    scratch<++
    $0000 scratch:  { Doke $0000 into keyL, keyH }
    scratch<++      { scratch + 0x0002 }
    scratch<++
    scratch:        { Doke vAC into oscL, oscH }
    scratch>++      { scratch + 0x0100 }

  ii 1- ii=
  if>=0 loop]  

  ret
] ResetAudio=


[def { PlayMidiAsync -- play MIDI stream asynchronous to VBlank; use this one in processing loops that take longer than a VSync period }
  push
  [\frameCount, frameCountPrev- 
    if<>0 PlayMidiSync!
    \frameCount, frameCountPrev=]         { if frameCount has changed call PlayMidiSync }
    pop ret
] PlayMidiAsync=


[def { PlayMidiSync -- plays MIDI stream, use this after your main VBlank loop }
  push
  $01 \soundTimer=                        { keep pumping soundTimer, so that global sound stays alive }
  [midiDelay 
    if>0 midiDelay 1- midiDelay=          { if midiDelay>0 midiDelay-- if(midiDelay>0 return }
    if>0 pop ret]

  [do
    midiStreamPtr, midiCommand=           { midiCommand = Peek(midiStreamPtr) }
    midiStreamPtr 1+ midiStreamPtr=       { midiStreamPtr++                   }
    midiCommand $F0& command=
    [command $80& if=0 midiCommand midiDelay= pop ret]
    [command $90^ if=0 MidiStartNote! else
    [command $80^ if=0 MidiEndNote!   else
    [command $D0^ if=0 MidiSegment!]]]
  loop]
] PlayMidiSync=


[def { MidiEndNote -- ends a MIDI note }
  push
  midiCommand $03& scratch>.              { scratch high = channel }
  $0 scratch<.                            { scratch low = $00      }
  scratch midiChannel+ scratch=           { scratch += midiChannel }
  $0000 scratch:                          { Doke(scratch, $0000)   }
  pop ret
] MidiEndNote=


[def { MidiSegment -- jumps to a new MIDI segment }
  push
  midiStreamPtr; midiStreamPtr=
  pop ret
] MidiSegment=


\vLR>++ ret
$0300:
[def { MidiStartNote -- starts a MIDI note }
  push
  \notesTable scratch=                    { scratch = \notesTable                  }
  midiStreamPtr, 10- 1<< 2-               { vAC = (Peek(midiStreamPtr) - 10)*2 - 2 }
  scratch+ scratch=                       { scratch = scratch + vAC                }
  0? midiNote<.                           { midiNote low = LUP(scratch + 0)        }
  scratch 1? midiNote>.                   { midiNote high = LUP(scratch + 1)       }
  midiCommand $03& scratch>.              { scratch high = channel                 }
  $0 scratch<.                            { scratch low = $00                      }
  scratch midiChannel+ scratch=           { scratch += midiChannel                 }
  midiNote scratch:                       { Doke(scratch, midiNote)                }
  midiStreamPtr 1+ midiStreamPtr=         { midiStreamPtr++                        }
  pop ret
] MidiStartNote=


{ Main }
\vLR>++ ret
$0400:
{ Reset audio hardware }
ResetAudio!

{ Loop forever }
[do
  { Wait for VBlank }
  [do \frameCount, frameCountPrev- if=0 loop]
  \frameCount, frameCountPrev=
  PlayMidiSync!                           { play MIDI stream synchronous to VBlank }
                                          { use this every VBlank                  }

  {PlayMidiAsync!}                        { play MIDI stream asynchronous to VBlank }
loop]                                     { use this in long processing loops       }
~~~

**_VASM Player_**
~~~
resetAudio      LDWI    0x0000
                STW     midiCommand
                STW     midiDelay
                STW     midiNote
                LDWI    giga_soundChan1 + 2 ; keyL, keyH
                STW     midiChannel
                STW     scratch
                LDWI    title_screenMidi00  ; midi score
                STW     midiStreamPtr

                LDI     0x04
                ST      ii

resetA_loop     LDI     giga_soundChan1     ; reset low byte
                ST      scratch
                LDWI    0x0300              
                DOKE    scratch             ; wavA and wavX
                INC     scratch
                INC     scratch    
                LDWI    0x0000
                DOKE    scratch             ; keyL and keyH
                INC     scratch
                INC     scratch
                DOKE    scratch             ; oscL and oscH
                INC     scratch + 1         ; increment high byte
                LoopCounter ii resetA_loop
                RET


playMidiAsync   LD      giga_frameCount
                SUBW    frameCountPrev
                BEQ     playMV_exit
                LD      giga_frameCount
                STW     frameCountPrev
                PUSH
                CALL    playMidi
                POP
playMV_exit     RET


playMidi        LDI     0x01                ; keep pumping soundTimer, so that global sound stays alive
                ST      giga_soundTimer
                LDW     midiDelay
                BEQ     playM_process
                SUBI    0x01
                STW     midiDelay
                BEQ     playM_process    
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
                BRA     playM_process
                
playM_endnote   LDW     scratch 
                XORI    0x80                ; check for end note
                BNE     playM_segment

                PUSH
                CALL    midiEndNote         ; end note
                POP
                BRA     playM_process


playM_segment   LDW     scratch
                XORI    0xD0                ; check for new segment
                BNE     playM_delay

                PUSH
                CALL    midiSegment         ; new midi segment
                POP
                BRA     playM_process

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
                    DB      0x90 0x53 0x91 0x47 0x1d 0x80 0x81 0xD0 0xA0 0x09
~~~
- GCL
~~~
$8000:
[def
  $90# $53# $91# $47# $07# $90# $52# $91# $46# $07# $90# $53# $91# $47# $07# $90# $52# $91# $46#
  $07# $90# $53# $91# $47# $07# $90# $54# $91# $48# $07# $90# $53# $91# $47# $07# $90# $52#
  $91# $46# $07# $90# $53# $91# $47# $1d# $80# $81# $d0# $a0# $09#
] game_overMidi=
~~~
- C++
~~~
uint8_t game_overMidi[] =
{
    0x90,0x53,0x91,0x47,0x07,0x90,0x52,0x91,0x46,0x07,0x90,0x53,0x91,0x47,0x07,0x90,0x52,0x91,0x46,
    0x07,0x90,0x53,0x91,0x47,0x07,0x90,0x54,0x91,0x48,0x07,0x90,0x53,0x91,0x47,0x07,0x90,0x52,
    0x91,0x46,0x07,0x90,0x53,0x91,0x47,0x1d,0x80,0x81,0xD0,0xA0,0x09,
};
~~~

- Python
~~~
game_overMidi = bytearray([
    0x90,0x53,0x91,0x47,0x07,0x90,0x52,0x91,0x46,0x07,0x90,0x53,0x91,0x47,0x07,0x90,0x52,0x91,0x46,
    0x07,0x90,0x53,0x91,0x47,0x07,0x90,0x54,0x91,0x48,0x07,0x90,0x53,0x91,0x47,0x07,0x90,0x52,
    0x91,0x46,0x07,0x90,0x53,0x91,0x47,0x1d,0x80,0x81,0xD0,0xA0,0x09,
])
~~~