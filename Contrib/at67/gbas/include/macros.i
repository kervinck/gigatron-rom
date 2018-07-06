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

%MACRO  ForNextInit _counter _start _end _step
        LDWI    _start
        STW     _counter
        LDWI    _end
        STW     kk
        LDWI    _step
        STW     xx
%ENDM

%MACRO  ForNextStepP _counter _label
        LDW     _counter
        ADDW    xx
        STW     _counter
        SUBW    kk
        BLT     _label
%ENDM

%MACRO  ForNextStepN _counter _label
        LDW     _counter
        ADDW    xx
        STW     _counter
        SUBW    kk
        BGT     _label
%ENDM

%MACRO  PrintString _textStr
        LDWI    _textStr
        STW     textStr
        CALL    printText
%ENDM

%MACRO  PrintVarInt16 _var
        LDW     _var
        STW     textDigits    
        CALL    printVarInt16
%ENDM

%MACRO  PrintInt16 _int
        LDWI    _int
        STW     textDigits    
        CALL    printVarInt16
%ENDM
