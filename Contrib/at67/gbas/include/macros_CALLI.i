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
        BGT     _label_+3
_label_ CALLI   _label
%ENDM

%MACRO  ForNextLoopDown _var _label _end
        LD      _var
        SUBI    1
        ST      _var
        SUBI    _end
        BLT     _label_+3
_label_ CALLI   _label
%ENDM

%MACRO  ForNextLoopStepUp _var _label _vEnd _vStep
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
        BLT     _label_+3
_label_ CALLI   _label
%ENDM

%MACRO  PrintChar _chr
        LDI     _chr
        CALLI   printChar
%ENDM

%MACRO  PrintAcChar
        CALLI   printChar
%ENDM

%MACRO  PrintVarChar _var
        LD      _var
        CALLI   printChar
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

%MACRO  Random
        CALLI   random16bit
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

%MACRO  ScanlineMode
        CALLI   scanlineMode
%ENDM

%MACRO  WaitVBlank
        CALLI   waitVBlank
%ENDM

%MACRO  DrawLine
        CALLI    drawLine
%ENDM

%MACRO  Initialise
        ClearRegionInit 0x2020 0 0 giga_xres giga_yres
        CALLI   clearRegion

        LDWI    0x0F20          ; yellow on blue
        STW     fgbgColour
        STW     giga_sysArg0
        LDWI    0x0802          ; starting cursor position
        STW     cursorXY
        LDWI    0x0001          ; text scrolling enabled by default
        STW     textScroll
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
