cpySrcAddr          EQU     register0
cpyDstAddr          EQU     register1
cpyCount            EQU     register2
cpyLoaderLut        EQU     register3
regsWork            EQU     giga_sysArg4                        ; use SYS arg registers to stop conflicts with time slicing/vblanks
regsAddr            EQU     giga_sysArg6                        ; use SYS arg registers to stop conflicts with time slicing/vblanks


%SUB                loadRegs
                    ; hard coded to save register8 to register15
loadRegs            LDWI    regsWorkArea
                    STW     regsWork
                    LDI     register8
                    STW     regsAddr

loadRegs_loop       LDW     regsWork
                    DEEK
                    DOKE    regsAddr
                    INC     regsAddr
                    INC     regsAddr
                    INC     regsWork
                    INC     regsWork
                    LDW     regsAddr
                    SUBI    register15
                    BLE     loadRegs_loop
                    RET
%ENDS

%SUB                saveRegs
                    ; hard coded to load register8 to register15
saveRegs            LDWI    regsWorkArea
                    STW     regsWork
                    LDI     register8
                    STW     regsAddr

saveRegs_loop       LDW     regsAddr
                    DEEK
                    DOKE    regsWork
                    INC     regsWork
                    INC     regsWork
                    INC     regsAddr
                    INC     regsAddr
                    LDW     regsAddr
                    SUBI    register15
                    BLE     saveRegs_loop
                    RET
%ENDS

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

%SUB                convert8Arr2d
convert8Arr2d       ADDW    memIndex0
                    ADDW    memIndex0
                    DEEK
                    ADDW    memIndex1
                    STW     memAddr
                    RET
%ENDS

%SUB                convert8Arr3d
convert8Arr3d       ADDW    memIndex0
                    ADDW    memIndex0
                    DEEK
                    ADDW    memIndex1
                    ADDW    memIndex1
                    DEEK
                    ADDW    memIndex2
                    STW     memAddr
                    RET
%ENDS

%SUB                convert16Arr2d
convert16Arr2d      ADDW    memIndex0
                    ADDW    memIndex0
                    DEEK
                    ADDW    memIndex1
                    ADDW    memIndex1
                    STW     memAddr
                    RET
%ENDS

%SUB                convert16Arr3d
convert16Arr3d      ADDW    memIndex0
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
                    ; read int16, used by DATA/READ
readIntVar          LDWI    _dataIndex_
                    STW     memAddr
                    DEEK
                    STW     memIndex0
                    ADDI    1
                    DOKE    memAddr
                    LDWI    _data_
                    ADDW    memIndex0
                    ADDW    memIndex0
                    DEEK
                    RET
%ENDS

%SUB                readStrVar
                    ; read string, used by DATA/READ
readStrVar          PUSH
                    LDWI    readIntVar
                    CALL    giga_vAC
                    STW     strSrcAddr
                    LDWI    stringCopy
                    CALL    giga_vAC
                    POP
                    RET
%ENDS

%SUB                copyBytes
copyBytes           LDW     cpySrcAddr
                    PEEK
                    POKE    cpyDstAddr
                    INC     cpySrcAddr
                    INC     cpyDstAddr
                    LDW     cpyCount
                    SUBI    1
                    STW     cpyCount
                    BGT     copyBytes
                    RET
%ENDS

%SUB                copyWords
copyWords           LDW     cpySrcAddr
                    DEEK
                    DOKE    cpyDstAddr
                    INC     cpySrcAddr
                    INC     cpySrcAddr
                    INC     cpyDstAddr
                    INC     cpyDstAddr
                    LDW     cpyCount
                    SUBI    1
                    STW     cpyCount
                    BGT     copyWords
                    RET
%ENDS

%SUB                copyLoaderImages
copyLoaderImages    PUSH
                    LDWI    _loader_image_chunksLut
                    STW     cpyLoaderLut
                    
copyLI_loop         LDW     cpyLoaderLut
                    DEEK
                    BEQ     copyLI_exit
                    STW     cpySrcAddr
                    INC     cpyLoaderLut
                    INC     cpyLoaderLut
                    LDW     cpyLoaderLut
                    DEEK
                    STW     cpyDstAddr
                    INC     cpyLoaderLut
                    INC     cpyLoaderLut
                    LDW     cpyLoaderLut
                    PEEK
                    STW     cpyCount
                    INC     cpyLoaderLut
                    LDWI    copyBytes
                    CALL    giga_vAC
                    BRA     copyLI_loop

copyLI_exit         POP
                    RET
%ENDS