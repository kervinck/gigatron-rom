memValue            EQU     register0
memAddr             EQU     register1
memIndex            EQU     register2


%SUB                getArrayByte
                    ; get 8bit value from array, (expects memAddr in AC)
getArrayByte        ADDW    memIndex
                    PEEK
                    RET
%ENDS

%SUB                setArrayByte
                    ; set 8bit value from array, (expects memAddr in AC)
setArrayByte        ADDW    memIndex
                    STW     memAddr
                    LDW     memValue
                    POKE    memAddr
                    RET
%ENDS

%SUB                getArrayInt16
                    ; get 16bit value from array, (expects memAddr in AC)
getArrayInt16       ADDW    memIndex
                    ADDW    memIndex
                    DEEK
                    RET
%ENDS

%SUB                setArrayInt16
                    ; set 16bit value from array, (expects memAddr in AC)
setArrayInt16       ADDW    memIndex
                    ADDW    memIndex
                    STW     memAddr
                    LDW     memValue
                    DOKE    memAddr
                    RET
%ENDS

%SUB                getArrayInt16Low
                    ; get low byte from 16bit array value
getArrayInt16Low    ADDW    memIndex
                    ADDW    memIndex
                    PEEK
                    RET
%ENDS

%SUB                setArrayInt16Low
                    ; set low byte from 16bit array value
setArrayInt16Low    ADDW    memIndex
                    ADDW    memIndex
                    STW     memAddr
                    LDW     memValue
                    POKE    memAddr
                    RET
%ENDS

%SUB                getArrayInt16High
                    ; get High byte from 16bit array value
getArrayInt16High   ADDW    memIndex
                    ADDW    memIndex
                    ADDI    1
                    PEEK
                    RET
%ENDS

%SUB                setArrayInt16High
                    ; set High byte from 16bit array value
setArrayInt16High   ADDW    memIndex
                    ADDW    memIndex
                    ADDI    1
                    STW     memAddr
                    LDW     memValue
                    POKE    memAddr
                    RET
%ENDS