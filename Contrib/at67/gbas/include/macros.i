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
        BGT     _label_ + 2
        LDWI    _label
_label_ CALL    giga_vAC
%ENDM

%MACRO  ForNextLoopDown _var _label _end
        LD      _var
        SUBI    1
        ST      _var
        SUBI    _end
        BLT     _label_ + 2
        LDWI    _label
_label_ CALL    giga_vAC
%ENDM

%MACRO  ForNextLoopStepUp _var _label _vEnd _vStep
        LDW     _var
        ADDW    _vStep
        STW     _var
        SUBW    _vEnd
        BGT     _label_ + 2
        LDWI    _label
_label_ CALL    giga_vAC
%ENDM

%MACRO  ForNextLoopStepDown _var _label _vEnd _vStep
        LDW     _var
        ADDW    _vStep
        STW     _var
        SUBW    _vEnd
        BLT     _label_ + 2
        LDWI    _label
_label_ CALL    giga_vAC
%ENDM

%MACRO  PrintChar _chr
        LDI     _chr
        ST      textChr
        LDWI    printChr
        CALL    giga_vAC
%ENDM

%MACRO  PrintAcChar
        ST      textChr
        LDWI    printChr
        CALL    giga_vAC
%ENDM

%MACRO  PrintVarChar _var
        LD      _var
        ST      textChr
        LDWI    printChr
        CALL    giga_vAC
%ENDM

%MACRO  PrintAcHexByte
        ST      textHex
        LDWI    printHexByte
        CALL    giga_vAC
%ENDM

%MACRO  PrintVarHexByte _var
        LD      _var
        ST      textHex
        LDWI    printHexByte
        CALL    giga_vAC
%ENDM

%MACRO  PrintAcHexWord
        STW     textHex
        LDWI    printHexWord
        CALL    giga_vAC
%ENDM

%MACRO  PrintVarHexWord _var
        LDW     _var
        STW     textHex
        LDWI    printHexWord
        CALL    giga_vAC
%ENDM

%MACRO  PrintString _str
        LDWI    _str
        STW     textStr
        LDWI    printText
        CALL    giga_vAC
%ENDM

%MACRO  PrintAcString
        STW     textStr
        LDWI    printText
        CALL    giga_vAC
%ENDM

%MACRO  PrintVarString _var
        LDW     _var
        STW     textStr
        LDWI    printText
        CALL    giga_vAC
%ENDM

%MACRO  PrintAcLeft
        STW     textStr
        LDWI    printLeft
        CALL    giga_vAC
%ENDM

%MACRO  PrintAcRight
        STW     textStr
        LDWI    printRight
        CALL    giga_vAC
%ENDM

%MACRO  PrintAcMid
        STW     textStr
        LDWI    printMid
        CALL    giga_vAC
%ENDM

%MACRO  PrintInt16 _int
        LDWI    _int
        STW     textNum    
        LDWI    printInt16
        CALL    giga_vAC
%ENDM

%MACRO  PrintAcInt16
        STW     textNum    
        LDWI    printInt16
        CALL    giga_vAC
%ENDM

%MACRO  PrintVarInt16 _var
        LDW     _var
        STW     textNum    
        LDWI    printInt16
        CALL    giga_vAC
%ENDM

%MACRO  StringChr
        LDWI    stringChr
        CALL    giga_vAC
%ENDM

%MACRO  StringHex
        LDWI    stringHex
        CALL    giga_vAC
%ENDM

%MACRO  StringHexw
        LDWI    stringHexw
        CALL    giga_vAC
%ENDM

%MACRO  StringCopy
        LDWI    stringCopy
        CALL    giga_vAC
%ENDM

%MACRO  StringAdd
        LDWI    stringAdd
        CALL    giga_vAC
%ENDM

%MACRO  StringMid
        LDWI    stringMid
        CALL    giga_vAC
%ENDM

%MACRO  StringLeft
        LDWI    stringLeft
        CALL    giga_vAC
%ENDM

%MACRO  StringRight
        LDWI    stringRight
        CALL    giga_vAC
%ENDM

%MACRO  Rand
        LDWI    rand16bit
        CALL    giga_vAC
%ENDM

%MACRO  RandMod
        STW     mathY
        LDWI    randMod16bit
        CALL    giga_vAC
%ENDM        

%MACRO  ShiftLeft4bit
        LDWI    shiftLeft4bit
        CALL    giga_vAC
%ENDM

%MACRO  ShiftLeft8bit
        LDWI    shiftLeft8bit
        CALL    giga_vAC
%ENDM

%MACRO  ShiftRight1bit
        LDWI    shiftRight1bit
        CALL    giga_vAC
%ENDM

%MACRO  ShiftRight2bit
        LDWI    shiftRight2bit
        CALL    giga_vAC
%ENDM

%MACRO  ShiftRight3bit
        LDWI    shiftRight3bit
        CALL    giga_vAC
%ENDM

%MACRO  ShiftRight4bit
        LDWI    shiftRight4bit
        CALL    giga_vAC
%ENDM

%MACRO  ShiftRight5bit
        LDWI    shiftRight5bit
        CALL    giga_vAC
%ENDM

%MACRO  ShiftRight6bit
        LDWI    shiftRight6bit
        CALL    giga_vAC
%ENDM

%MACRO  ShiftRight7bit
        LDWI    shiftRight7bit
        CALL    giga_vAC
%ENDM

%MACRO  ShiftRight8bit
        LDWI    shiftRight8bit
        CALL    giga_vAC
%ENDM

%MACRO  ShiftRightSgn1bit
        LDWI    shiftRightSgn1bit
        CALL    giga_vAC
%ENDM

%MACRO  ShiftRightSgn2bit
        LDWI    shiftRightSgn2bit
        CALL    giga_vAC
%ENDM

%MACRO  ShiftRightSgn3bit
        LDWI    shiftRightSgn3bit
        CALL    giga_vAC
%ENDM

%MACRO  ShiftRightSgn4bit
        LDWI    shiftRightSgn4bit
        CALL    giga_vAC
%ENDM

%MACRO  ShiftRightSgn5bit
        LDWI    shiftRightSgn5bit
        CALL    giga_vAC
%ENDM

%MACRO  ShiftRightSgn6bit
        LDWI    shiftRightSgn6bit
        CALL    giga_vAC
%ENDM

%MACRO  ShiftRightSgn7bit
        LDWI    shiftRightSgn7bit
        CALL    giga_vAC
%ENDM

%MACRO  ShiftRightSgn8bit
        LDWI    shiftRightSgn8bit
        CALL    giga_vAC
%ENDM

%MACRO  ScanlineMode
        LDWI    scanlineMode
        CALL    giga_vAC
%ENDM

%MACRO  WaitVBlank
        LDWI    waitVBlank
        CALL    giga_vAC
%ENDM

%MACRO  DrawLine
        LDWI    drawLine
        CALL    giga_vAC
%ENDM

%MACRO  DrawVTLine
        LDWI    drawVTLine
        CALL    giga_vAC
%ENDM

%MACRO  DrawHLine
        LDWI    drawHLine
        CALL    giga_vAC
%ENDM

%MACRO  DrawVLine
        LDWI    drawVLine
        CALL    giga_vAC
%ENDM

%MACRO  AtLineCursor
        LDWI    atLineCursor
        CALL    giga_vAC
%ENDM

%MACRO  PlayMidi
        STW     midiStream
        LDWI    resetAudio
        CALL    giga_vAC
        LDWI    realTimeProc + 2
        STW     register0
        LDWI    playMidi
        DOKE    register0                               ; self modifying code, replaces realTimeProc stub with playMidi routine
%ENDM

%MACRO  TickMidi
        LDWI    playMidi
        CALL    giga_vAC
%ENDM

%MACRO  JumpFalse _label id
        BNE     _id_ + 2                                ; unique id is used as an internal macro label
        LDWI    _label
_id_    CALL    giga_vAC
%ENDM

%MACRO  JumpEQ _label id
        BEQ     _id_ + 2
        LDWI    _label
_id_    CALL    giga_vAC
%ENDM

%MACRO  JumpNE _label id
        BNE     _id_ + 2
        LDWI    _label
_id_    CALL    giga_vAC
%ENDM

%MACRO  JumpLE _label id
        BLE     _id_ + 2
        LDWI    _label
_id_    CALL    giga_vAC
%ENDM

%MACRO  JumpGE _label id
        BGE     _id_ + 2
        LDWI    _label
_id_    CALL    giga_vAC
%ENDM

%MACRO  JumpLT _label id
        BLT     _id_ + 2
        LDWI    _label
_id_    CALL    giga_vAC
%ENDM

%MACRO  JumpGT _label id
        BGT     _id_ + 2
        LDWI    _label
_id_    CALL    giga_vAC
%ENDM

%MACRO  InitEqOp
        LDWI    convertEqOp
        STW     convertEqOpAddr
%ENDM
        
%MACRO  InitNeOp
        LDWI    convertNeOp
        STW     convertNeOpAddr
%ENDM

%MACRO  InitLeOp
        LDWI    convertLeOp
        STW     convertLeOpAddr
%ENDM
        
%MACRO  InitGeOp
        LDWI    convertGeOp
        STW     convertGeOpAddr
%ENDM
        
%MACRO  InitLtOp
        LDWI    convertLtOp
        STW     convertLtOpAddr
%ENDM
        
%MACRO  InitGtOp
        LDWI    convertGtOp
        STW     convertGtOpAddr
%ENDM

%MACRO  InitRealTimeProc
        LDWI    realTimeProc
        STW     realTimeProcAddr
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

        LDWI    initClearFuncs
        CALL    giga_vAC
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
        LDWI    clearRegion
        CALL    giga_vAC
%ENDM
