%MACRO  GetTetrominoBase _index _rotation
    LDW     _index          ; tetromino type
    ADDW    tetrominoLut    
    DEEK                    ; get tetromino type address
    ADDW    _rotation       ; add tetromino rotation offset
    STW     tetrominoBase 
%ENDM

%MACRO  GetTetrominoData _offset _variable
    LDW     tetrominoBase
    ADDI    _offset
    PEEK
    STW     _variable
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