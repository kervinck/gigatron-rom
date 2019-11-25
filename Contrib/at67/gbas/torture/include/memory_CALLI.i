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