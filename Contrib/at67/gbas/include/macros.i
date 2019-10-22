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

%MACRO  ForNextLoopStepDown _var _label _vEnd _vStep
        LDW     _var
        ADDW    _vStep
        STW     _var
        SUBW    _vEnd
        BLT     _label_+2
        LDWI    _label
_label_ CALL    giga_vAC
%ENDM

%MACRO  PrintChar _chr
        LDI     _chr
        ST      textChr
        LDWI    validChar
        CALL    giga_vAC
        LDWI    printChar
        CALL    giga_vAC
%ENDM

%MACRO  PrintAcChar
        ST      textChr
        LDWI    validChar
        CALL    giga_vAC
        LDWI    printChar
        CALL    giga_vAC
%ENDM

%MACRO  PrintVarChar _var
        LD      _var
        ST      textChr
        LDWI    validChar
        CALL    giga_vAC
        LDWI    printChar
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

%MACRO  PrintInt16 _int
        LDWI    _int
        STW     textNum    
        LDWI    printVarInt16
        CALL    giga_vAC
%ENDM

%MACRO  PrintAcInt16
        STW     textNum    
        LDWI    printVarInt16
        CALL    giga_vAC
%ENDM

%MACRO  PrintVarInt16 _var
        LDW     _var
        STW     textNum    
        LDWI    printVarInt16
        CALL    giga_vAC
%ENDM

%MACRO  Initialise
        ClearRegion 0x2020 0 0 giga_xres giga_yres
        LDWI    0x0F20          ; yellow on blue
        STW     textColour
        LDWI    0x0802          ; starting cursor position
        STW     cursorXY
%ENDM

%MACRO  ClearRegion  _colour _x _y _w _h
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
        LDWI    clearRegion
        CALL    giga_vAC
%ENDM
