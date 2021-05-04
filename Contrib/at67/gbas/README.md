# TIPS and TRICKS

## Constants:
- The CONST keyword is a thing, it defines the thing following it as a literal thing that either takes up no memory space, (int vars), or is instanced, (strings).
- Use constants liberally, you can use expressions to evaluate constants making code more readable and easier to understand.
~~~
const fontStart = 0
const boingStart = fontStart + 27
load sprite, ../../images/gbas/Boing/Boing0.tga, boingStart + 0
load sprite, ../../images/gbas/Boing/Boing1.tga, boingStart + 1

' const allows you to easily reference an instanced copy of a literal string anywhere in your code
const NAME$ = "Insert Name Here"

print NAME$
if s$ = NAME$ then gosub blah
~~~

## Variables:
- Integer variable names MUST be alphanumeric, must begin with an alpha char and can use underscores everywhere except for the first and last char.
- Currently there are 40 int16_t, (2 byte signed integer), fast variables available for program use.
- These are always global, their lifetime spans from program start to program finish and across gosub subroutines.
- If you need more variables then you can use arrays, (DIM statement).
- If you need byte sized variables then you can use the DEF BYTE statement with POKE and PEEK.
- DEF WORD and DEEK/DOKE can be used as faster versions of arrays inside tight inner loops.
- Slow variables, (using main non zero page memory), will be added in the future.
- Local variables, procedures and the ability to easily recurse, *may* be added in the future.
- You can perform limited recursion right now with careful handling of variables/arrays inside subroutines.
- Arrays do not use any of page zero or fast variable space, so you may use as many as you like until you run out of RAM.

## Strings:
- String variable names MUST be alphanumeric, must begin with an alpha char, must end with a dollar sign and can use underscores everywhere except for the first and last char.
- The maximum length of a string is 94 bytes, (this allows the most efficient use of the Gigatron's fragmented memory architecture).
- Most of the string functions, (LEFT$, RIGHT$, MID$, HEX$, etc), respect the maximum length, but don't assume.
- You can assign and concatenate strings easily using '=' and '+'.
- Use STRCMP to compare two strings.
- Strings have a length byte preamble and a '0' terminator byte epilogue, this extra byte per string makes the runtime smaller and in some cases much faster.
- When PEEKing/POKEing strings, remember to skip the first length byte.
- A string length pragma that controls the maximum length of strings will be added at a future date.
- Constant/literal strings are instanced where possible, e.g. if you reference a space, " ", in multiple locations within your code there will only be 1 byte allocated for that space in memory.
- If you need non instanced copies of a string, then use string variables to represent that string, not constants or literals.
- String variables do not use any of page zero or fast variable space, so you may use as many as you like until you run out of RAM.

## Input:
- There are no INKEY or GETKEY funtions as it is trivial to perform any type of key/button processing you require using the GET function.
- e.g. you can use get("SERIALRAW") or get("BUTTONSTATE") and do your own edge/level and keyup/keydown detection.
~~~
kk = 255

loop:
    k = get("SERIALRAW")
    if kk &&= 255
        if k&&<>255 then gosub k
    endif
    kk = k
goto &loop

49: print "1" : return
50: print "2" : return
51: print "3" : return
~~~
- General purpose input can be obtained through the INPUT command.
- INPUT is more powerful than ye old BASIC's INPUT and provides a number of extra and advanced features.
- It supports a heading string, multiple int/str variables, semi-colon formatting and field widths.
- Input text will automatically scroll when reaching the edge of the screen or be limited to a pre-defined field width.
~~~
' INPUT <heading string>, <int/str var0>, <prompt string0>, ... <int/str varN>, <prompt stringN>
input "Testing INPUT:", cat$,"Cats Name ?"32;, hours,"H ?"2;, minutes,"M ?"2;, seconds,"S ?"2;
~~~

## Delays:
- You can wait a predermined period of time using the WAIT command.
- WAIT without any parameters waits for one vertical blank, (use this to sync your fast inner loops to VSYNC).
- WAIT <n> will wait 'n' vertical blank periods, where 'n' can 1 to 32767.

## Assembler:
- The compiler always compiles and links to vCPU mnemonics, the assembler then assembles the mnemonics into vCPU code and a .GT1 file.
- This is a different order of operation to a traditional compiler because linker files, (the gtBASIC runtime), are not object/binary files, they are source/text files.
- The compiler will always produce a .GASM file that you may then view to determine how good/bad a job the compiler has performed.
- The assembly code is fully annotated and you can directly see which gtBASIC statements have produced which vCPU mnemonics.
- The following code is a complete assembled version of Blinky, you can see all the register/memory allocations, the annotated code and the gtBASIC runtime linked.
<details>
 <summary>Summary</summary>

~~~
_startAddress_      EQU                     0x0200

; Internal variables
serialRawPrev       EQU                     0x0081
register0           EQU                     0x0082
register1           EQU                     register0 + 0x02
register2           EQU                     register0 + 0x04
register3           EQU                     register0 + 0x06
register4           EQU                     register0 + 0x08
register5           EQU                     register0 + 0x0A
register6           EQU                     register0 + 0x0C
register7           EQU                     register0 + 0x0E
register8           EQU                     register0 + 0x10
register9           EQU                     register0 + 0x12
register10          EQU                     register0 + 0x14
register11          EQU                     register0 + 0x16
register12          EQU                     register0 + 0x18
register13          EQU                     register0 + 0x1A
register14          EQU                     register0 + 0x1C
register15          EQU                     register0 + 0x1E
fgbgColour          EQU                     register0 + 0x20
cursorXY            EQU                     register0 + 0x22
midiStream          EQU                     register0 + 0x24
midiDelay           EQU                     register0 + 0x26
miscFlags           EQU                     register0 + 0x28
fontLutId           EQU                     0x00e0

; Internal buffers
textWorkArea        EQU                     0x7fa0

; Includes
%includePath        "../runtime"
%include            gigatron.i
%include            macros.i

; Labels
_entryPoint_        EQU                     0x0200
_10                 EQU                     0x0239
_20                 EQU                     0x0239
_30                 EQU                     0x0239
_40                 EQU                     0x0245
_50                 EQU                     0x0239
_60                 EQU                     0x025b
_end_0x0271         EQU                     0x0263

; Variables
_X                  EQU                     0x0030
_Y                  EQU                     0x0032
_I                  EQU                     0x0034
_P                  EQU                     0x0036

; Strings

; Define Bytes

; Define Words

; Define Images

; Define Sprites

; Define Fonts

; Lookup Tables

; Code
_entryPoint_        InitRealTimeProc        
                    InitEqOp                
                    InitNeOp                
                    InitLeOp                
                    InitGeOp                
                    InitLtOp                
                    InitGtOp                
                    Initialise                                              ; INIT

_20                 LDI                     80
                    STW                     _X
                    LDI                     60
                    STW                     _Y
                    LDI                     0
                    STW                     _I                              ; X=160/2:Y=120/2:I=0

_40                 LDWI                    256
                    ADDW                    _Y
                    ADDW                    _Y
                    PEEK                    
                    STW                     0xc2
                    LD                      0xc2
                    ST                      giga_vAC + 1
                    ORI                     0xFF
                    XORI                    0xFF
                    ADDW                    _X
                    STW                     _P                              ; P=(PEEK(256+Y+Y) LSL 8)+X

_60                 LDW                     _I
                    POKE                    _P
                    INC                     _I
                    BRA                     _60                             ; POKE P,I:INC I:GOTO &60

_end_0x0271         BRA                     _end_0x0271                     ; END



;****************************************************************************************************************************************
;****************************************************************************************************************************************
;* Internal runtime, DO NOT MODIFY PAST THIS POINT, modifications must be made in the original include files                            *
;****************************************************************************************************************************************
;****************************************************************************************************************************************

realTimeProc        EQU     0x7ef6
convertEqOp         EQU     0x7eed
convertNeOp         EQU     0x7ee4
convertLeOp         EQU     0x7edb
convertGeOp         EQU     0x7ed2
convertLtOp         EQU     0x7ec9
convertGtOp         EQU     0x7ec0
resetVideoTable     EQU     0x7db5
initClearFuncs      EQU     0x7ddd
realTimeProcAddr    EQU     0x00dc
convertEqOpAddr     EQU     0x00d0
convertNeOpAddr     EQU     0x00d2
convertLeOpAddr     EQU     0x00d4
convertGeOpAddr     EQU     0x00d6
convertLtOpAddr     EQU     0x00d8
convertGtOpAddr     EQU     0x00da


; do *NOT* use register4 to register7 during time slicing if you call realTimeProc
numericLabel        EQU     register0
defaultLabel        EQU     register1
lutLabs             EQU     register2
lutAddrs            EQU     register3
lutIndex            EQU     register8


                    ; runs real time, (time sliced), code at regular intervals
realTimeProc        PUSH
                    LDWI    realTimeStub                    ; realTimeStub gets replaced by MIDI routine
                    CALL    giga_vAC
                    POP
                    RET
                    
realTimeStub        RET


                    ; convert equal to into a boolean
convertEqOp         BEQ     convertEq_1
                    LDI     0
                    RET
convertEq_1         LDI     1
                    RET


                    ; convert not equal to into a boolean
convertNeOp         BNE     convertNe_1
                    LDI     0
                    RET
convertNe_1         LDI     1
                    RET


                    ; convert less than or equal to into a boolean
convertLeOp         BLE     convertLe_1
                    LDI     0
                    RET
convertLe_1         LDI     1
                    RET


                    ; convert greater than or equal to into a boolean
convertGeOp         BGE     convertGe_1
                    LDI     0
                    RET
convertGe_1         LDI     1
                    RET


                    ; convert less than into a boolean
convertLtOp         BLT     convertLt_1
                    LDI     0
                    RET
convertLt_1         LDI     1
                    RET


                    ; convert greater than into boolean
convertGtOp         BGT     convertGt_1
                    LDI     0
                    RET
convertGt_1         LDI     1
                    RET



; do *NOT* use register4 to register7 during time slicing if you call realTimeProc
xreset              EQU     register0
xcount              EQU     register1
ycount              EQU     register2
treset              EQU     register3
breset              EQU     register8
top                 EQU     register9
bot                 EQU     register10
vramAddr            EQU     register11
evenAddr            EQU     register12
clsAddress          EQU     register13
    
    
                    ; resets video table pointers
resetVideoTable     PUSH
                    LDI     8
                    STW     vramAddr
                    LDWI    giga_videoTable
                    STW     evenAddr
    
resetVT_loop        CALL    realTimeProcAddr
                    LDW     vramAddr
                    DOKE    evenAddr
                    INC     evenAddr
                    INC     evenAddr
                    INC     vramAddr
                    LD      vramAddr
                    SUBI    giga_yres + 8
                    BLT     resetVT_loop
                    
                    LDWI    giga_videoTop                       ; reset videoTop
                    STW     register0
                    LDI     0
                    POKE    register0
                    POP
                    RET


initClearFuncs      PUSH
                    LDWI    resetVideoTable
                    CALL    giga_vAC
    
                    LDI     0x02                                ; starting cursor position
                    STW     cursorXY
                    LDWI    0x7FFF
                    ANDW    miscFlags
                    STW     miscFlags                           ; reset on bottom row flag
            
                    LD      fgbgColour
                    ST      giga_sysArg0
                    ST      giga_sysArg0 + 1
                    ST      giga_sysArg2
                    ST      giga_sysArg2 + 1                    ; 4 pixels of fg colour
    
                    LDWI    SYS_Draw4_30                        ; setup 4 pixel SYS routine
                    STW     giga_sysFn
                    POP
                    RET
~~~
</details>

## Branching:
- Don't use 0 as a line number, 0 is used as a delimiter in GOTO/GOSUB LUT's.
- Line numbers are totally optional, (except for ON <VAR> GOTO/GOSUB and GOTO/GOSUB <VAR>).
- ON <VAR> GOTO/GOSUB <LINE#1>, <LINE#2>, ..... <LINE#n>, (no checks are performed so make sure your <VAR> does not send you off into lala land).
- GOTO/GOSUB <VAR>, <OPTIONAL DEFAULT LABEL>, allows for efficient switch type statements, e.g.
~~~
loop:
    a = rnd(2) + 1
    gosub a
    
    b = rnd(2) + 3
    goto b
goto &loop

' Numeric GOSUB identifies line numbers with ':'
1: print "1" : return
2: print "2" : return

' Numeric GOTO identifies line numbers with '!'
3! print "3" : goto loop
4! print "4" : goto loop
~~~
- Use GOTO to continue/break out of loops, (break and continue are not a thing in this compiler).
- Use labels and indentation abundantly and line numbers only when needed, e.g.
~~~
loop:
    set SOUNDTIMER, 5

    gosub vuMeter
    
    k = get("SERIALRAW")
    if kk &&= 255 then gosub k
    gosub state
    kk = k
goto &loop

midiOff:
    set MIDISTREAM, &h0000
return

1:  gosub midiOff
    gosub siren
    return
    
2:  gosub midiOff
    gosub crash
    return

3:  gosub midiOff
    gosub beep
    return
    
4:  if get("MIDISTREAM") &= &h0000
        play midiv, &h0AA0, 2
    else
        play midiv
    endif
    return
~~~

## Arithmetic:
- Multiply and especially divide/modulus are slow.
- Use shifts, masks, LUT's, cheats and hacks as much as possible, especially in your inner most or tightest loops.
~~~
' slow, rnd(<number>) performs a modulus with that number on the random result
' rnd(0) foregoes the modulus so that you can do the filtering yourself
x = rnd(160) : y = rnd(120)

' much faster, but gives a different type of noise that drastically drops off towards it's boundaries
x = (rnd(0) AND &h7F) + (rnd(0) AND &h1F) + (rnd(0) AND &h01)
y = (rnd(0) AND &h3F) + (rnd(0) AND &h1F) + (rnd(0) AND &h0F) + (rnd(0) AND &h07) + (rnd(0) AND &h03)

' x = 4 + 8*i : y = 3 + 6*i
math: sh = (i<<1)
      x = sh + sh + sh + sh + 4
      y = sh + sh + sh + 3
return

' samples a complex transcendental function into a 64 byte LUT, (use graphing calculators like Desmos to create your functions)
def byte(&h08A0, y, 0.0, 0.5, 64) = 63.0 - exp(-0.5*y)*(1.0-exp(-3.0*y))*1.6125*63.0
~~~

## Memory Manager:
- The host compiler, (your PC), has a fairly extensive memory manager that tracks every single byte on the Gigatron whilst compiling your code.
- There is no memory manager within the gtBASIC runtime, (i.e. on the Gigatron), so ALL memory allocations are static and happen BEFORE your code is run.
- This includes the following pragmas and commands:
~~~
_runTimeStart_
_spriteStripeChunks_
DIM
DEF
ALLOC
FREE
~~~
- These commands ALWAYS allocate memory before code and other commands gets access to memory, so you can choose the RAM addresses for the most efficient layout for your application. Everything else will fit around your high priority allocations.
- These commands can NOT be used in loops, branches or condition statements, they are read once each during an initial parse of the code and RAM is allocated accordingly.
- You can statically initialise arrays and defined areas without wasting precious memory and vCPU cycles using the following syntax.
~~~
' you can have less than the required initialisers, the missing values will be automatically zero'ed.
' DIM array elements are always 2 byte wide integers
dim colours(3) = &h3F, &h2A, &h15, &h00
' one initialiser fills the entire array
dim blah(20) = &hBEEF

def byte(&h10a0) = &h1e, &h90, &h4a, &h91, &h45, &h3d, &h80, &h81, &h1f, &h91, &h4a, &h90, &h45, &h3d, &h80

' you can use loops of complex floating point functions to initialise DEF BYTES and DEF WORDS areas of memory
' the y is a placeholder, use a variable name that does not clash with the transcendentals you are using
' <address> <var> <start> <end> <count>
' this fills the RAM starting at &h08A0 and ending at &h08FF, (offscreen memory), with the following decay function
def byte(&h08A0, y, 0.0, 0.5, 96) = 63.0 - exp(-0.5*y)*(1.0-exp(-3.0*y))*1.6125*63.0
~~~

## Address Of '@':
- Use the '@' symbol to obtain the address of any int, array or string variable, (works with constant strings as well).
~~~
dim arr(5) = 0
const cat$ = "cat"
dog$ = "dog"

a = 1
b = @a
c = @c + b + 1
d = @cat$ + @dog$ + @arr
~~~

## .LO and .HI:
- Use .LO/.HI when you want to access the low and high bytes of an integer or array variable.
~~~
a = 1
d = @a
a.hi = 1 : a.lo = 0
b = a.hi + 1
c = 0 : c.hi = 1
~~~

## LEN:
- Use the LEN function to obtain the length of any int, array or string variable, (works with constant strings as well).

## INC:
- Use the INC command when incrementing byte values.

## POKE and PEEK:
- You can POKE/PEEK the non parenthesis Gigatron system variables with complete freedom, (you shouldn't touch the ones in parenthesis noted in Marcel's main README.md as they are subject to change).
- Do not POKE/PEEK any of the gtBASIC variables as they will definitely change and sooner rather than later.
- Use the provided SET/GET functions, they cover all usable Gigatron and gtBASIC system variables.

## Optimisation Hints:
- The optimiser uses one or more '&' characters to control how branching and jumping is performed.
- When writing code you do not need to worry about page jumps and code spilling out of pages/sections, the compiler will automatically stitch all the fragmented areas of code together using page jumps and accumulator save/restore's.
- You can completely ignore these compiler hints and the code will work just fine, but if you want to, (sometimes drastically), reduce it's size and increase it's speed then you can use these hints as follows:
~~~
' No hints: allows full relational operator expressions, i.e. AND, OR, XOR, NOT, (use parenthesis so that order of precedence works correctly)
IF (a > 50) AND (b < 20) then gosub blah

' One hint: relational operator expressions will no longer work, jumping will be more efficient as it uses macros rather than subroutines
IF a &> 50
    IF b &< 20 then gosub blah
ENDIF    

' Two hints: relational operator expressions will no longer work, branches now instead of jumping, but will fail if branch destination is in another page, (validator will warn you if this occurs).
IF a &&> 50
    IF b &&< 20 then gosub blah
ENDIF    

' Uses a branch instead of a jump, which can fail as above
FOR i=0 &TO 10
    PRINT i
NEXT i

' Uses a branch instead of a jump, which can fail as above
goto &blah
~~~

## Optimiser:
- The optimiser currently only spans single lines and multi statement lines, (it will be expanded in the future); so use it effectively or disable it by spreading your code out over multiple lines.
- There are many sequences of instructions that are matched/replaced/relocated by smaller more efficient sequences, check the source code for a full run down.
- Optimising causes var/label names to contain meaningless address values because of code relocation.
- Do not rely on the address values when reading vCPU assembly code, their purpose is only to provide unique names.
- When a line of code uses the result of a previous line's calculations, the optimiser may be able to save some LDW instructions if you spread the code over a multi-line statement. 
~~~
' the LDW _p has been optimised out of this code sequence, it used to sit in between the STW _p and the 'SUBW _q'
                    LDW                     _p
                    ADDI                    4
                    STW                     _p
                    SUBW                    _q
                    BLT                     _repeat_0x0414                  ; p = p + 4 : until p &&>= q
~~~

## Variable Initialisation:
- Pack the initialisation of your variables into multi-statement lines if you can, your code will be smaller and faster.

### UNPACKED:
~~~
                    LDI                     0
                    STW                     _x                              ; x = 0

                    LDI                     0
                    STW                     _y                              ; y = 0

                    LDI                     0
                    STW                     _z                              ; z = 0
~~~

### PACKED:
~~~
                    LDI                     0
                    STW                     _x
                    STW                     _y
                    STW                     _z                              ; x = 0 : y = x : z = y
~~~

## Expression Handler:
- Short circuit complex literal calculations into simpler real instructions.

### BAD:
~~~
                    LDI                     20
                    STW                     _x                              ; x = 20
                    
                    LDW                     _x
                    STW                     mathX
                    LDI                     20
                    STW                     mathY
                    LDWI                    multiply16bit
                    CALL                    giga_vAC
                    ADDI                    53
                    SUBI                    12
                    SUBI                    9
                    STW                     _blah                           ; blah = x*20 + 53 - 12 - 9
~~~

### GOOD:
~~~
                    LDI                     20
                    STW                     _x
                    STW                     mathX
                    LDI                     20
                    STW                     mathY
                    LDWI                    multiply16bit
                    CALL                    giga_vAC
                    ADDI                    32
                    STW                     _blah                           ; x = 20 : blah = x*20 + (53 - 12 - 9)
~~~

- Use as much floating point calculations with literals, (including transcendentals), as you want; it will all be calculated at full double floating point precision and then the final answer rounded down into int16_t, (the native vCPU 16 bit format).
- Remember to ALWAYS use parenthesis around your complex literal calculations or you will calculate garbage.
- No variables can be used within the parenthesis containing the floating point calculations.
- Transcendentals use Degrees, NOT Radians.

~~~
                    LDI                     10
                    STW                     _x
                    STW                     mathX
                    LDI                     2
                    STW                     mathY
                    LDWI                    multiply16bit
                    CALL                    giga_vAC
                    STW                     mathX
                    LDWI                    -995
                    STW                     mathY
                    LDWI                    multiply16bit
                    CALL                    giga_vAC
                    STW                     _blah                           ; x = 10 : blah = x*2*(50*exp(-1.232455)*sin(45)*cos(57.324786234) - 1000.342876324)
~~~