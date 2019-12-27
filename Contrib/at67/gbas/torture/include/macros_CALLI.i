%MACRO  LoopCounterTo1 _counter _label
        LD      _counter
        SUBI    0x01
        ST      _counter
        BGT     _label
%ENDM

%MACRO  LoopCounterTo0 _counter _label
        LD      _counter
        SUBI    0x01
        ST      _counter
        BGE     _label
%ENDM

%MACRO  LoopCounter _counter _label
        LD      _counter
        SUBI    0x01
        ST      _counter
        BNE     _label
%ENDM

%MACRO  LoopCounter1 _counter _label
        LD      _counter
        SUBI    0x01
        ST      _counter
        BGE     _label
%ENDM

%MACRO  LutPeek _lut _index
        LDWI    _lut
        DEEK
        ADDW    _index
        PEEK
%ENDM

%MACRO  LutDeek _lut _index _scratch
        LDWI    _lut
        DEEK
        STW     _scratch
        LDW     _index
        LSLW
        ADDW    _scratch
        DEEK
%ENDM

%MACRO  ForNextLoopUp _var _label _end
        INC     _var
        LD      _var
        SUBI    _end
        BGT     _label_ + 3
_label_ CALLI   _label
%ENDM

%MACRO  ForNextLoopDown _var _label _end
        LD      _var
        SUBI    1
        ST      _var
        SUBI    _end
        BLT     _label_ + 3
_label_ CALLI   _label
%ENDM

%MACRO  ForNextLoopStepUp _var _label _vEnd _vStep
        LDW     _var
        ADDW    _vStep
        STW     _var
        SUBW    _vEnd
        BGT     _label_ + 3
_label_ CALLI   _label
%ENDM

%MACRO  ForNextLoopStepDown _var _label _vEnd _vStep
        LDW     _var
        ADDW    _vStep
        STW     _var
        SUBW    _vEnd
        BLT     _label_ + 3
_label_ CALLI   _label
%ENDM

%MACRO  PrintChar _chr
        LDI     _chr
        CALLI   printChr
%ENDM

%MACRO  PrintAcChar
        CALLI   printChr
%ENDM

%MACRO  PrintVarChar _var
        LD      _var
        CALLI   printChr
%ENDM

%MACRO  PrintAcHexByte
        CALLI   printHexByte
%ENDM

%MACRO  PrintVarHexByte _var
        LD      _var
        CALLI   printHexByte
%ENDM

%MACRO  PrintAcHexWord
        CALLI   printHexWord
%ENDM

%MACRO  PrintVarHexWord _var
        LDW     _var
        CALLI   printHexWord
%ENDM

%MACRO  PrintString _str
        LDWI    _str
        CALLI   printText
%ENDM

%MACRO  PrintAcString
        CALLI   printText
%ENDM

%MACRO  PrintVarString _var
        LDW     _var
        CALLI   printText
%ENDM

%MACRO  PrintAcLeft
        CALLI   printLeft
%ENDM

%MACRO  PrintAcRight
        CALLI   printRight
%ENDM

%MACRO  PrintAcMid
        CALLI   printMid
%ENDM

%MACRO  PrintInt16 _int
        LDWI    _int
        CALLI   printInt16
%ENDM

%MACRO  PrintAcInt16
        CALLI   printInt16
%ENDM

%MACRO  PrintVarInt16 _var
        LDW     _var
        CALLI   printInt16
%ENDM

%MACRO  StringChr
        CALLI   stringChr
%ENDM

%MACRO  StringHex
        CALLI   stringHex
%ENDM

%MACRO  StringHexw
        CALLI   stringHexw
%ENDM

%MACRO  StringCopy
        CALLI   stringCopy
%ENDM

%MACRO  StringAdd
        CALLI   stringAdd
%ENDM

%MACRO  StringMid
        CALLI   stringMid
%ENDM

%MACRO  StringLeft
        CALLI   stringLeft
%ENDM

%MACRO  StringRight
        CALLI   stringRight
%ENDM

%MACRO  Rand
        CALLI   rand16bit
%ENDM

%MACRO  RandMod
        STW     mathY
        CALLI   randMod16bit
%ENDM

%MACRO  ShiftLeft4bit
        CALLI   shiftLeft4bit
%ENDM

%MACRO  ShiftLeft8bit
        CALLI   shiftLeft8bit
%ENDM

%MACRO  ShiftRight1bit
        CALLI   shiftRight1bit
%ENDM

%MACRO  ShiftRight2bit
        CALLI   shiftRight2bit
%ENDM

%MACRO  ShiftRight3bit
        CALLI   shiftRight3bit
%ENDM

%MACRO  ShiftRight4bit
        CALLI   shiftRight4bit
%ENDM

%MACRO  ShiftRight5bit
        CALLI   shiftRight5bit
%ENDM

%MACRO  ShiftRight6bit
        CALLI   shiftRight6bit
%ENDM

%MACRO  ShiftRight7bit
        CALLI   shiftRight7bit
%ENDM

%MACRO  ShiftRight8bit
        CALLI   shiftRight8bit
%ENDM

%MACRO  ShiftRightSgn1bit
        CALLI   shiftRightSgn1bit
%ENDM

%MACRO  ShiftRightSgn2bit
        CALLI   shiftRightSgn2bit
%ENDM

%MACRO  ShiftRightSgn3bit
        CALLI   shiftRightSgn3bit
%ENDM

%MACRO  ShiftRightSgn4bit
        CALLI   shiftRightSgn4bit
%ENDM

%MACRO  ShiftRightSgn5bit
        CALLI    shiftRightSgn5bit
%ENDM

%MACRO  ShiftRightSgn6bit
        CALLI    shiftRightSgn6bit
%ENDM

%MACRO  ShiftRightSgn7bit
        CALLI   shiftRightSgn7bit
%ENDM

%MACRO  ShiftRightSgn8bit
        CALLI   shiftRightSgn8bit
%ENDM

%MACRO  ScanlineMode
        CALLI   scanlineMode
%ENDM

%MACRO  WaitVBlank
        CALLI   waitVBlank
%ENDM

%MACRO  DrawLine
        CALLI   drawLine
%ENDM

%MACRO  DrawVTLine
        CALLI   drawVTLine
%ENDM

%MACRO  DrawHLine
        CALLI   drawHLine
%ENDM

%MACRO  DrawVLine
        CALLI   drawVLine
%ENDM

%MACRO  AtLineCursor
        CALLI   atLineCursor
%ENDM

%MACRO  PlayMidi
        STW     midiStream
        CALLI   resetAudio
        LDWI    realTimeProc + 2
        STW     register0
        LDWI    playMidi
        DOKE    register0                               ; self modifying code, replaces realTimeProc stub with playMidi routine
%ENDM

%MACRO  TickMidi
        CALLI   playMidi
%ENDM

%MACRO  JumpFalse _label id
        BNE     _id_ + 3                                ; unique id is used as an internal macro label
_id_    CALLI   _label
%ENDM

%MACRO  JumpEQ _label id
        BEQ     _id_ + 3
_id_    CALLI   _label
%ENDM

%MACRO  JumpNE _label id
        BNE     _id_ + 3
_id_    CALLI   _label
%ENDM

%MACRO  JumpLE _label id
        BLE     _id_ + 3
_id_    CALLI   _label
%ENDM

%MACRO  JumpGE _label id
        BGE     _id_ + 3
_id_    CALLI   _label
%ENDM

%MACRO  JumpLT _label id
        BLT     _id_ + 3
_id_    CALLI   _label
%ENDM

%MACRO  JumpGT _label id
        BGT     _id_ + 3
_id_    CALLI   _label
%ENDM

%MACRO  Initialise
        LDWI    0x0F20
        STW     fgbgColour                              ; yellow on blue

        LDWI    0x0001
        STW     miscFlags                               ; reset flags

        LDWI    0xFF00
        STW     highByteMask
        
        LDWI    0x0000
        STW     midiStream                              ; reset MIDI
        LDI     0x00
        ST      giga_soundTimer                         ; reset soundTimer, (stops any current Audio)

        CALLI   initClearFuncs
%ENDM

%MACRO  ClearRegion _x _y _w _h
        LDI     _h / 2
        ST      ycount
        LDI     _w / 4
        ST      xcount
        ST      xreset

        LDWI    _y*256 + _x + giga_vram                 ; top line
        STW     treset
        STW     top
        LDWI    ((_h - 1) + _y)*256 + _x + giga_vram    ; bottom line
        STW     breset
        STW     bot
        CALLI   clearRegion
%ENDM
