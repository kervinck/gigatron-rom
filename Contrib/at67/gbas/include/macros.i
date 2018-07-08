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

%MACRO  ForNextInit _var _start _end _step _varEnd _varStep
        LDWI    _start
        STW     _var
        LDWI    _end
        STW     _varEnd
        LDWI    _step
        STW     _varStep
%ENDM

%MACRO  ForNextStepP _var _label _varEnd _varStep
        LDW     _var
        ADDW    _varStep
        STW     _var
        SUBW    _varEnd
        BLE     _label
%ENDM

%MACRO  ForNextStepN _var _label _varEnd _varStep
        LDW     _var
        ADDW    _varStep
        STW     _var
        SUBW    _varEnd
        BGE     _label
%ENDM

%MACRO  PrintChar _chr
        LDI     _chr
        CALL    validChar
        CALL    printChar
%ENDM

%MACRO  PrintVarChar _var
        LD      _var
        CALL    validChar
        CALL    printChar
%ENDM

%MACRO  PrintString _str
        LDWI    _str
        STW     textStr
        CALL    printText
%ENDM

%MACRO  PrintInt16 _int
        LDWI    _int
        STW     textDigits    
        CALL    printVarInt16
%ENDM

%MACRO  PrintVarInt16 _var
        LDW     _var
        STW     textDigits    
        CALL    printVarInt16
%ENDM

%MACRO  Initialise
        LDWI    0x2020          ; blue background
        CALL    clearScreen
        LDWI    0x0F20          ; yellow on blue
        STW     textColour
        LDWI    0x0802          ; starting cursor position
        STW     cursorXY
%ENDM
