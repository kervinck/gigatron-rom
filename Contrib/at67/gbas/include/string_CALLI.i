; do *NOT* use register4 to register7 during time slicing if you call realTimeProc
strChr              EQU     register0
strHex              EQU     register1
strLength           EQU     register0
strFinish           EQU     register0
strSrcAddr          EQU     register1
strDstAddr          EQU     register2
strSrcLen           EQU     register3
strOffset           EQU     register8
strSrcAddr2         EQU     register9
strTmpAddr          EQU     register10


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
                    CALLI   createHex
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
                    CALLI   createHex
                    LD      strHex
                    ST      strChr
                    CALLI   createHex
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
                    SYS     0xF5                                ; SYS_LSRW4_50, 270 - 50/2 = 0xF5
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

%SUB                stringAdd
                    ; concatenates two strings together
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
                    BGE     stringA_exit
                    INC     strSrcAddr2                         ; skips length first time
                    LDW     strSrcAddr2
                    PEEK
                    BEQ     stringA_exit
                    POKE    strDstAddr                          ; copy char
                    INC     strDstAddr
                    INC     strLength
                    BRA     stringA_copy1                       ; copy char until terminating char

stringA_exit        LDW     strLength
                    POKE    strTmpAddr                          ; save concatenated string length
                    LDI     0
                    POKE    strDstAddr                          ; terminating zero
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