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
        BGT     _label_+2
        LDWI    _label
_label_ CALL    giga_vAC
%ENDM

%MACRO  ForNextLoopUp_CALLI _var _label _end
        INC     _var
        LD      _var
        SUBI    _end
        BGT     _label_+3
_label_ CALLI   _label
%ENDM

%MACRO  ForNextLoopDown _var _label _end
        LD      _var
        SUBI    1
        ST      _var
        SUBI    _end
        BLT     _label_+2
        LDWI    _label
_label_ CALL    giga_vAC
%ENDM

%MACRO  ForNextLoopDown_CALLI _var _label _end
        LD      _var
        SUBI    1
        ST      _var
        SUBI    _end
        BLT     _label_+3
_label_ CALLI   _label
%ENDM

%MACRO  ForNextLoopInit _start _end _step _vStart _vEnd _vStep
        LDWI    _start
        STW     _vStart
        LDWI    _end
        STW     _vEnd
        LDWI    _step
        STW     _vStep
%ENDM

%MACRO  ForNextLoopStepUp _var _label _vEnd _vStep
        LDW     _var
        ADDW    _vStep
        STW     _var
        SUBW    _vEnd
        BGT     _label_+2
        LDWI    _label
_label_ CALL    giga_vAC
%ENDM

%MACRO  ForNextLoopStepUp_CALLI _var _label _vEnd _vStep
        LDW     _var
        ADDW    _vStep
        STW     _var
        SUBW    _vEnd
        BGT     _label_+3
_label_ CALLI   _label
%ENDM

%MACRO  ForNextLoopStepDown _var _label _vEnd _vStep
        LDW     _var
        ADDW    _vStep
        STW     _var
        SUBW    _vEnd
        BLT     _label_+2
        LDWI    _label
_label_ CALL    giga_vAC
%ENDM

%MACRO  ForNextLoopStepDown_CALLI _var _label _vEnd _vStep
        LDW     _var
        ADDW    _vStep
        STW     _var
        SUBW    _vEnd
        BLT     _label_+3
_label_ CALLI   _label
%ENDM

%MACRO  PrintChar _chr
        LDI     _chr
        ST      textChr
        LDWI    printChar
        CALL    giga_vAC
%ENDM

%MACRO  PrintChar_CALLI _chr
        LDI     _chr
        ST      textChr_CALLI
        CALLI   printChar_CALLI
%ENDM

%MACRO  PrintAcChar
        ST      textChr
        LDWI    printChar
        CALL    giga_vAC
%ENDM

%MACRO  PrintAcChar_CALLI
        ST      textChr_CALLI
        CALLI   printChar_CALLI
%ENDM

%MACRO  PrintVarChar _var
        LD      _var
        ST      textChr
        LDWI    printChar
        CALL    giga_vAC
%ENDM

%MACRO  PrintVarChar_CALLI _var
        LD      _var
        ST      textChr_CALLI
        CALLI   printChar_CALLI
%ENDM

%MACRO  PrintAcHexByte
        ST      textHex
        LDWI    printHexByte
        CALL    giga_vAC
%ENDM

%MACRO  PrintAcHexByte_CALLI
        ST      textHex_CALLI
        CALLI   printHexByte_CALLI
%ENDM

%MACRO  PrintVarHexByte _var
        LD      _var
        ST      textHex
        LDWI    printHexByte
        CALL    giga_vAC
%ENDM

%MACRO  PrintVarHexByte_CALLI _var
        LD      _var
        ST      textHex_CALLI
        CALLI   printHexByte_CALLI
%ENDM

%MACRO  PrintAcHexWord
        STW     textHex
        LDWI    printHexWord
        CALL    giga_vAC
%ENDM

%MACRO  PrintAcHexWord_CALLI
        STW     textHex_CALLI
        CALLI   printHexWord_CALLI
%ENDM

%MACRO  PrintVarHexWord _var
        LDW     _var
        STW     textHex
        LDWI    printHexWord
        CALL    giga_vAC
%ENDM

%MACRO  PrintVarHexWord_CALLI _var
        LDW     _var
        STW     textHex_CALLI
        CALLI   printHexWord_CALLI
%ENDM

%MACRO  PrintString _str
        LDWI    _str
        STW     textStr
        LDWI    printText
        CALL    giga_vAC
%ENDM

%MACRO  PrintString_CALLI _str
        LDWI    _str
        STW     textStr_CALLI
        CALLI   printText_CALLI
%ENDM

%MACRO  PrintInt16 _int
        LDWI    _int
        STW     textNum    
        LDWI    printVarInt16
        CALL    giga_vAC
%ENDM

%MACRO  PrintInt16_CALLI _int
        LDWI    _int
        STW     textNum_CALLI
        CALLI   printVarInt16_CALLI
%ENDM

%MACRO  PrintAcInt16
        STW     textNum    
        LDWI    printVarInt16
        CALL    giga_vAC
%ENDM

%MACRO  PrintAcInt16_CALLI
        STW     textNum_CALLI
        CALLI   printVarInt16_CALLI
%ENDM

%MACRO  PrintVarInt16 _var
        LDW     _var
        STW     textNum    
        LDWI    printVarInt16
        CALL    giga_vAC
%ENDM

%MACRO  PrintVarInt16_CALLI _var
        LDW     _var
        STW     textNum_CALLI
        CALLI   printVarInt16_CALLI
%ENDM

%MACRO  Initialise
        ClearRegionInit 0x2020 0 0 giga_xres giga_yres
        LDWI    clearRegion
        CALL    giga_vAC

        LDWI    0x0F20          ; yellow on blue
        STW     textColour
        LDWI    0x0802          ; starting cursor position
        STW     cursorXY
%ENDM

%MACRO  Initialise_CALLI
        ClearRegionInit 0x2020 0 0 giga_xres giga_yres
        CALLI   clearRegion

        LDWI    0x0F20          ; yellow on blue
        STW     textColour
        LDWI    0x0802          ; starting cursor position
        STW     cursorXY
%ENDM

%MACRO  InitialiseCcOps
        LDWI    convertEqOp     ; (0x00E2 <-> 0x00ED), critical routines that can't straddle page boundaries
        STW     convertEqOpAddr
        LDWI    convertNeOp
        STW     convertNeOpAddr
        LDWI    convertLeOp
        STW     convertLeOpAddr
        LDWI    convertGeOp
        STW     convertGeOpAddr
        LDWI    convertLtOp
        STW     convertLtOpAddr
        LDWI    convertGtOp
        STW     convertGtOpAddr
%ENDM

%MACRO  ClearRegionInit  _colour _x _y _w _h
        LDWI    _colour
        STW     giga_sysArg0                            ; 4 pixels of colour
        STW     giga_sysArg2

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
%ENDM
