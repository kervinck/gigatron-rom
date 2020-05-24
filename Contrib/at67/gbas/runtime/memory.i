%SUB                getArrayByte
                    ; get 8bit value from array
getArrayByte        LDW     memAddr
                    ADDW    memIndex0
                    PEEK
                    RET
%ENDS

%SUB                setArrayByte
                    ; set 8bit value from array
setArrayByte        LDW     memAddr
                    ADDW    memIndex0
                    STW     memAddr
                    LDW     memValue
                    POKE    memAddr
                    RET
%ENDS

%SUB                getArrayInt16
                    ; get 16bit value from array
getArrayInt16       LDW     memAddr
                    ADDW    memIndex0
                    ADDW    memIndex0
                    DEEK
                    RET
%ENDS

%SUB                setArrayInt16
                    ; set 16bit value from array
setArrayInt16       LDW     memAddr
                    ADDW    memIndex0
                    ADDW    memIndex0
                    STW     memAddr
                    LDW     memValue
                    DOKE    memAddr
                    RET
%ENDS

%SUB                getArrayInt16Low
                    ; get low byte from 16bit array value
getArrayInt16Low    LDW     memAddr
                    ADDW    memIndex0
                    ADDW    memIndex0
                    PEEK
                    RET
%ENDS

%SUB                setArrayInt16Low
                    ; set low byte from 16bit array value
setArrayInt16Low    LDW     memAddr
                    ADDW    memIndex0
                    ADDW    memIndex0
                    STW     memAddr
                    LDW     memValue
                    POKE    memAddr
                    RET
%ENDS

%SUB                getArrayInt16High
                    ; get High byte from 16bit array value
getArrayInt16High   LDW     memAddr
                    ADDW    memIndex0
                    ADDW    memIndex0
                    ADDI    1
                    PEEK
                    RET
%ENDS

%SUB                setArrayInt16High
                    ; set High byte from 16bit array value
setArrayInt16High   LDW     memAddr
                    ADDW    memIndex0
                    ADDW    memIndex0
                    ADDI    1
                    STW     memAddr
                    LDW     memValue
                    POKE    memAddr
                    RET
%ENDS

%SUB                convertArr2d
convertArr2d        ADDW    memIndex0
                    ADDW    memIndex0
                    DEEK
                    ADDW    memIndex1
                    ADDW    memIndex1
                    STW     memAddr
                    RET
%ENDS

%SUB                convertArr3d
convertArr3d        ADDW    memIndex0
                    ADDW    memIndex0
                    DEEK
                    ADDW    memIndex1
                    ADDW    memIndex1
                    DEEK
                    ADDW    memIndex2
                    ADDW    memIndex2
                    STW     memAddr
                    RET
%ENDS

%SUB                readIntVar
readIntVar          LDWI    _dataIndex_
                    STW     memAddr
                    DEEK
                    
readIV_loop         STW     memIndex0
                    ADDI    1
                    DOKE    memAddr
                    LDWI    _data_
                    ADDW    memIndex0
                    ADDW    memIndex0
                    DEEK
                    BEQ     readIV_loop                     ; reached end of list, so restore
                    RET
%ENDS

%SUB                readStrVar
readStrVar          PUSH
                    LDWI    readIntVar
                    CALL    giga_vAC
                    STW     strSrcAddr
                    LDWI    stringCopy
                    CALL    giga_vAC
                    POP
                    RET
%ENDS
