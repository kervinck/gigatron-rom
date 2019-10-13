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

%MACRO  ForNextInitVe _var _start _end _varEnd
        LDWI    _start
        STW     _var
        LDWI    _end
        STW     _varEnd
%ENDM

%MACRO  ForNextInitVs _var _start _step _varStep
        LDWI    _start
        STW     _var
        LDWI    _step
        STW     _varStep
%ENDM

%MACRO  ForNextInitVsVe _var _start _end _step _varEnd _varStep
        LDWI    _start
        STW     _var
        LDWI    _end
        STW     _varEnd
        LDWI    _step
        STW     _varStep
%ENDM

%MACRO  ForNextLoopP _var _label _end
        INC     _var
        LD      _var
        SUBI    _end
        BGT     _label_
        LDWI    _label
        CALL    giga_vAC
_label_ LD      giga_vAC        
%ENDM

%MACRO  ForNextLoopVsVeP _var _label _varEnd _varStep
        LDW     _var
        ADDW    _varStep
        STW     _var
        SUBW    _varEnd
        BLE     _label
%ENDM

%MACRO  ForNextLoopVsVeN _var _label _varEnd _varStep
        LDW     _var
        ADDW    _varStep
        STW     _var
        SUBW    _varEnd
        BGE     _label
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

%MACRO  PrintString _str
        LDWI    _str
        STW     textStr
        LDWI    printText
        CALL    giga_vAC
%ENDM

%MACRO  PrintInt16 _int
        LDWI    _int
        STW     textDigits    
        LDWI    printVarInt16
        CALL    giga_vAC
%ENDM

%MACRO  PrintAcInt16
        STW     textDigits    
        LDWI    printVarInt16
        CALL    giga_vAC
%ENDM

%MACRO  PrintVarInt16 _var
        LDW     _var
        STW     textDigits    
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
