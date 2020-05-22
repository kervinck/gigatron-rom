; do *NOT* use register4 to register7 during time slicing if you call realTimeStub
strChr              EQU     register0
strHex              EQU     register1
strCmpRes           EQU     register2
strLength           EQU     register0
strFinish           EQU     register0
strSrcAddr          EQU     register1
strDstAddr          EQU     register2
strSrcLen           EQU     register3
strOffset           EQU     register8
strSrcAddr2         EQU     register9
strTmpAddr          EQU     register10
strLutAddr          EQU     register11
strBakAddr          EQU     register12
strInteger          EQU     register0
strDigit            EQU     register1
strMult             EQU     register3


%SUB                stringChr
                    ; create a char string, (parameter in strChr)
stringChr           LDI     1
                    POKE    strDstAddr                          ; set destination buffer length                    
                    INC     strDstAddr
                    LD      strChr
                    POKE    strDstAddr                          ; copy char
                    INC     strDstAddr
                    LDI     0
                    POKE    strDstAddr                          ; terminating 0
                    RET
%ENDS

%SUB                stringHex
                    ; create a hex byte string, (parameter in strChr)
stringHex           PUSH
                    LDI     2
                    POKE    strDstAddr                          ; set destination buffer length                    
                    INC     strDstAddr
                    LDWI    createHex
                    CALL    giga_vAC
                    LDI     0
                    POKE    strDstAddr                          ; terminating 0
                    POP
                    RET
%ENDS

%SUB                stringHexw
                    ; create a hex word string, (parameter in strHex)
stringHexw          PUSH
                    LDI     4
                    POKE    strDstAddr                          ; set destination buffer length                    
                    INC     strDstAddr
                    LD      strHex + 1
                    ST      strChr
                    LDWI    createHex
                    CALL    giga_vAC
                    LD      strHex
                    ST      strChr
                    LDWI    createHex
                    CALL    giga_vAC
                    LDI     0
                    POKE    strDstAddr                          ; terminating 0
                    POP
                    RET
%ENDS

%SUB                createHex
                    ; updates a hex byte
createHex           LDWI    SYS_LSRW4_50                        ; shift right by 4 SYS routine
                    STW     giga_sysFn
                    LD      strChr
                    SYS     50
                    SUBI    10
                    BLT     createH_skip0
                    ADDI    7

createH_skip0       ADDI    0x3A
                    POKE    strDstAddr                          ; save first char
                    INC     strDstAddr
                    LD      strChr
                    ANDI    0x0F
                    SUBI    10
                    BLT     createH_skip1
                    ADDI    7
                    
createH_skip1       ADDI    0x3A                    
                    POKE    strDstAddr                          ; save second char
                    INC     strDstAddr
                    RET
%ENDS

%SUB                stringCopy
                    ; copy one string to another
stringCopy          LDW     strSrcAddr
                    PEEK
                    POKE    strDstAddr
                    INC     strSrcAddr
                    INC     strDstAddr
                    BNE     stringCopy                          ; copy char until terminating char
                    RET
%ENDS

%SUB                stringCmp
                    ; compares two strings
stringCmp           LDI     0
                    STW     strCmpRes

stringC_cmp         LDW     strSrcAddr
                    PEEK
                    BEQ     stringC_one                         ; this assumes your strings are valid, (i.e. valid length and terminating bytes)
                    STW     strChr
                    LDW     strSrcAddr2
                    PEEK
                    SUBW    strChr
                    BNE     stringC_zero
                    INC     strSrcAddr
                    INC     strSrcAddr2
                    BRA     stringC_cmp
                    
stringC_one         INC     strCmpRes                           ; return 1

stringC_zero        LDW     strCmpRes
                    RET
%ENDS

%SUB                stringAdd
                    ; adds two strings together, (internal sub)
stringAdd           LDW     strDstAddr
                    STW     strTmpAddr
                    INC     strSrcAddr
                    INC     strDstAddr                          ; skip lengths
                    LDI     0
                    STW     strLength
                    
stringA_copy0       LDW     strSrcAddr
                    PEEK
                    BEQ     stringA_copy1
                    POKE    strDstAddr
                    INC     strSrcAddr
                    INC     strDstAddr
                    INC     strLength
                    BRA     stringA_copy0
                    
stringA_copy1       LDW     strLength
                    SUBI    94
                    BGE     stringA_exit                        ; maximum destination length reached
                    INC     strSrcAddr2                         ; skips length first time
                    LDW     strSrcAddr2
                    PEEK
                    BEQ     stringA_exit                        ; copy char until terminating char
                    POKE    strDstAddr                          ; copy char
                    INC     strDstAddr
                    INC     strLength
                    BRA     stringA_copy1

stringA_exit        LDW     strLength
                    POKE    strTmpAddr                          ; save concatenated string length
                    LDI     0
                    POKE    strDstAddr                          ; terminating zero
                    RET
%ENDS

%SUB                stringConcat
                    ; concatenates multiple strings together
stringConcat        PUSH
                    LDW     strLutAddr
                    DEEK
                    BEQ     stringCC_exit
                    STW     strSrcAddr
                    LDW     strDstAddr
                    STW     strBakAddr
                    
stringCC_loop       INC     strLutAddr
                    INC     strLutAddr
                    LDW     strLutAddr
                    DEEK
                    BEQ     stringCC_exit
                    STW     strSrcAddr2
                    LDWI    stringAdd
                    CALL    giga_vAC
                    LDW     strBakAddr
                    STW     strDstAddr
                    STW     strSrcAddr
                    BRA     stringCC_loop
                    
stringCC_exit       POP
                    RET
%ENDS

%SUB                stringLeft
                    ; copies sub string from left hand side of source string to destination string
stringLeft          LD      strLength
                    POKE    strDstAddr                          ; destination length
                    BEQ     stringL_exit                        ; exit if left length = 0
                    LDW     strSrcAddr
                    PEEK                                        ; get source length
                    STW     strSrcLen
                    SUBW    strLength
                    BGE     stringL_skip                        ; is left length <= source length
                    LD      strSrcLen
                    STW     strLength
                    POKE    strDstAddr                          ; new destination length
                    
stringL_skip        LDW     strSrcAddr
                    ADDW    strLength
                    STW     strFinish                           ; end source address
                    
stringL_loop        INC     strSrcAddr                          ; skip lengths the first time in
                    INC     strDstAddr
                    LDW     strSrcAddr
                    PEEK
                    POKE    strDstAddr                          ; copy char
                    LDW     strSrcAddr
                    SUBW    strFinish
                    BLT     stringL_loop                        ; until finished
                    
stringL_exit        INC     strDstAddr
                    LDI     0
                    POKE    strDstAddr                          ; terminating 0
                    RET
%ENDS

%SUB                stringRight
                    ; copies sub string from right hand side of source string to destination string
stringRight         LD      strLength
                    POKE    strDstAddr                          ; destination length
                    BEQ     stringR_exit                        ; exit if right length = 0
                    LDW     strSrcAddr
                    PEEK                                        ; get source length
                    STW     strSrcLen
                    SUBW    strLength
                    BGE     stringR_skip                        ; length <= srcLength
                    LD      strSrcLen
                    STW     strLength
                    POKE    strDstAddr                          ; new destination length
                    LDI     0
                    
stringR_skip        ADDW    strSrcAddr
                    STW     strSrcAddr                          ; copy from (source address + (source length - right length)) to destination address
                    ADDW    strLength
                    STW     strFinish                           ; end source address

stringR_loop        INC     strSrcAddr                          ; skip lengths the first time in
                    INC     strDstAddr
                    LDW     strSrcAddr
                    PEEK
                    POKE    strDstAddr                          ; copy char
                    LDW     strSrcAddr
                    SUBW    strFinish
                    BLT     stringR_loop                        ; until finished
                    
stringR_exit        INC     strDstAddr
                    LDI     0
                    POKE    strDstAddr                          ; terminating 0
                    RET
%ENDS

%SUB                stringMid
                    ; copies length sub string from source offset to destination string
stringMid           LD      strLength
                    POKE    strDstAddr                          ; destination length
                    BEQ     stringM_exit                        ; exit if right length = 0
                    LDW     strSrcAddr
                    PEEK                                        ; get source length
                    STW     strSrcLen
                    SUBW    strOffset                           
                    SUBW    strLength
                    BGE     stringM_skip                        ; length + offset <= srcLength
                    LD      strSrcLen
                    SUBW    strOffset
                    STW     strLength
                    POKE    strDstAddr                          ; new destination length
                    
stringM_skip        LDW     strSrcAddr
                    ADDW    strOffset
                    STW     strSrcAddr                          ; copy from (source address + (source length - right length)) to destination address
                    ADDW    strLength
                    STW     strFinish                           ; end source address

stringM_loop        INC     strSrcAddr                          ; skip lengths the first time in
                    INC     strDstAddr
                    LDW     strSrcAddr
                    PEEK
                    POKE    strDstAddr                          ; copy char
                    LDW     strSrcAddr
                    SUBW    strFinish
                    BLT     stringM_loop                        ; until finished
                    
stringM_exit        INC     strDstAddr
                    LDI     0
                    POKE    strDstAddr                          ; terminating 0
                    RET
%ENDS

%SUB                stringDigit
stringDigit         LDW     strInteger

stringD_index       SUBW    strMult
                    BLT     stringD_cont
                    STW     strInteger
                    INC     strDigit                            ; calculate digit
                    BRA     stringD_index
    
stringD_cont        LD      strDigit
                    BEQ     stringD_exit                        ; leading zero supressed
                    ORI     0x30
                    POKE    strTmpAddr                          ; store digit
                    INC     strTmpAddr
                    LDI     0x30
                    ST      strDigit                            ; reset digit
                    
stringD_exit        RET
%ENDS

%SUB                stringInt
                    ; create a string from an int
stringInt           PUSH
                    LDI     0
                    STW     strDigit
                    LDW     strDstAddr
                    STW     strTmpAddr
                    INC     strTmpAddr                          ; skip length byte
                    LDW     strInteger
                    BGE     stringI_pos
                    LDI     0x2D
                    POKE    strTmpAddr                          ; -ve sign
                    INC     strTmpAddr
                    LDI     0
                    SUBW    strInteger
                    STW     strInteger                          ; +ve number

stringI_pos         LDWI    10000
                    STW     strMult
                    LDWI    stringDigit
                    CALL    giga_vAC
                    LDWI    1000
                    STW     strMult
                    LDWI    stringDigit
                    CALL    giga_vAC
                    LDI     100
                    STW     strMult
                    LDWI    stringDigit
                    CALL    giga_vAC
                    LDI     10
                    STW     strMult
                    LDWI    stringDigit
                    CALL    giga_vAC
                    LD      strInteger
                    ORI     0x30
                    POKE    strTmpAddr                          ; 1's digit
                    LDW     strTmpAddr
                    SUBW    strDstAddr
                    POKE    strDstAddr                          ; length byte
                    INC     strTmpAddr
                    LDI     0
                    POKE    strTmpAddr                          ; terminating 0                    
                    POP
                    RET
%ENDS
