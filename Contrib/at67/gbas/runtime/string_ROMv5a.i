; do *NOT* use register4 to register7 during time slicing
strChr              EQU     register0
strAddr             EQU     register0
strHex              EQU     register1
strLen              EQU     register2
strDstLen           EQU     register0
strFinish           EQU     register0
strSrcAddr          EQU     register1
strDstAddr          EQU     register2
strSrcLen           EQU     register3
strOffset           EQU     register8
strSrcAddr2         EQU     register9
strTmpAddr          EQU     register10
strLutAddr          EQU     register11
strBakAddr          EQU     register12
strSrcLen2          EQU     register13
strInteger          EQU     register0
strDigit            EQU     register1
strMult             EQU     register3


%SUB                stringChr
                    ; create a char string, (parameter in strChr)
stringChr           STW     strDstAddr
                    LDI     1
                    POKE    strDstAddr                          ; set destination buffer length                    
                    INC     strDstAddr
                    LD      strChr
                    POKE    strDstAddr                          ; copy char
                    INC     strDstAddr
                    LDI     0
                    POKE    strDstAddr                          ; terminating 0
                    RET
%ENDS

%SUB                stringSpc
                    ; create a spc string, (parameter in strLen)
stringSpc           LD      strLen
                    BEQ     stringS_exit
                    SUBI    94
                    BGT     stringS_exit
                    POKE    strAddr                             ; set destination buffer length
                    INC     strAddr
                    
stringS_loop        LDI     32
                    POKE    strAddr                             ; copy char
                    INC     strAddr
                    LD      strLen
                    SUBI    1
                    ST      strLen
                    BNE     stringS_loop
                    LDI     0
                    POKE    strAddr                             ; terminating 0
                    
stringS_exit        RET
%ENDS

%SUB                stringHex
                    ; creates a hex string at strAddr of strLen digits from strHex
stringHex           LDWI    SYS_LSRW4_50                    ; shift right by 4 SYS routine
                    STW     giga_sysFn
                    LDW     strAddr
                    STW     strTmpAddr                      ; store string start
                    LD      strLen
                    POKE    strAddr                         ; length byte
                    ADDI    1
                    ADDW    strAddr
                    STW     strAddr                         ; offset by length byte and zero delimeter
                    LDI     0
                    POKE    strAddr                         ; zero delimiter
                    
stringH_loop        LDW     strAddr
                    SUBI    1
                    STW     strAddr                         ; start at LSD and finish at MSD
                    SUBW    strTmpAddr
                    BEQ     stringH_done
                    LD      strHex
                    ANDI    0x0F
                    SUBI    10
                    BLT     stringH_skip
                    ADDI    7
                    
stringH_skip        ADDI    0x3A
                    POKE    strAddr
                    LDW     strHex
                    SYS     50
                    STW     strHex                          ; next nibble
                    BRA     stringH_loop
                    
stringH_done        RET
%ENDS

%SUB                stringCopy
                    ; copy one string to another
stringCopy          STW     strDstAddr

stringCp_loop       LDW     strSrcAddr
                    PEEK
                    POKE    strDstAddr
                    INC     strSrcAddr
                    INC     strDstAddr
                    BNE     stringCp_loop                       ; copy char until terminating char
                    RET
%ENDS

%SUB                stringCmp
                    ; compares two strings: returns 0 for smaller, 1 for equal and 2 for larger
stringCmp           STW     strSrcAddr2
                    LDW     strSrcAddr
                    PEEK
                    STW     strSrcLen                           ; save str length
                    LDW     strSrcAddr2
                    PEEK
                    STW     strSrcLen2                          ; save str length
                    INC     strSrcAddr
                    INC     strSrcAddr2                         ; skip lengths
                    
stringC_loop        LDW     strSrcAddr
                    PEEK
                    BEQ     stringC_equal                       ; this assumes your strings are valid, (i.e. valid length and terminating bytes)
                    STW     strChr
                    LDW     strSrcAddr2
                    PEEK
                    SUBW    strChr
                    BLT     stringC_larger
                    BGT     stringC_smaller
                    INC     strSrcAddr
                    INC     strSrcAddr2
                    BRA     stringC_loop

stringC_smaller     LDI     0
                    RET
                    
stringC_equal       LDW     strSrcLen
                    SUBW    strSrcLen2
                    BLT     stringC_smaller
                    BGT     stringC_larger                      ; if strings are equal, choose based on length
                    LDI     1
                    RET
                    
stringC_larger      LDI     2
                    RET
%ENDS

%SUB                stringAdd
                    ; adds two strings together, (internal sub)
stringAdd           LDW     strDstAddr
                    STW     strTmpAddr
                    XORW    strSrcAddr
                    BNE     stringA_diff
                    LDW     strDstAddr                          ; if src = dst then skip first copy
                    PEEK
                    STW     strDstLen
                    ADDW    strDstAddr
                    STW     strDstAddr                          ; skip length byte and point to end of dst
                    INC     strDstAddr
                    BRA     stringA_copy1

stringA_diff        INC     strSrcAddr
                    INC     strDstAddr                          ; skip lengths
                    LDI     0
                    STW     strDstLen
                    
stringA_copy0       LDW     strSrcAddr                          ; assumes strSrcAddr is a valid string <= 94 length
                    PEEK
                    BEQ     stringA_copy1
                    POKE    strDstAddr
                    INC     strSrcAddr
                    INC     strDstAddr
                    INC     strDstLen
                    BRA     stringA_copy0
                    
stringA_copy1       LDW     strDstLen
                    SUBI    94
                    BGE     stringA_exit                        ; maximum destination length reached
                    INC     strSrcAddr2                         ; skips length first time
                    LDW     strSrcAddr2
                    PEEK
                    BEQ     stringA_exit                        ; copy char until terminating char
                    POKE    strDstAddr                          ; copy char
                    INC     strDstAddr
                    INC     strDstLen
                    BRA     stringA_copy1

stringA_exit        LDW     strDstLen
                    POKE    strTmpAddr                          ; save concatenated string length
                    LDI     0
                    POKE    strDstAddr                          ; terminating zero
                    RET
%ENDS

%SUB                stringConcat
                    ; concatenates multiple strings together
stringConcat        PUSH
                    STW     strDstAddr
                    CALLI   stringAdd
                    POP
                    RET
%ENDS

%SUB                stringConcatLut
                    ; concatenates multiple strings together using a LUT of string addresses
stringConcatLut     PUSH
                    STW     strDstAddr
                    LDW     strLutAddr
                    DEEK
                    BEQ     stringCCL_exit
                    STW     strSrcAddr
                    LDW     strDstAddr
                    STW     strBakAddr
                    
stringCCL_loop      INC     strLutAddr
                    INC     strLutAddr
                    LDW     strLutAddr
                    DEEK
                    BEQ     stringCCL_exit
                    STW     strSrcAddr2
                    CALLI   stringAdd
                    LDW     strBakAddr
                    STW     strDstAddr
                    STW     strSrcAddr
                    BRA     stringCCL_loop
                    
stringCCL_exit      POP
                    RET
%ENDS

%SUB                stringLeft
                    ; copies sub string from left hand side of source string to destination string
stringLeft          STW     strDstAddr
                    LD      strDstLen
                    POKE    strDstAddr                          ; destination length
                    BEQ     stringL_exit                        ; exit if left length = 0
                    LDW     strSrcAddr
                    PEEK                                        ; get source length
                    STW     strSrcLen
                    SUBW    strDstLen
                    BGE     stringL_skip                        ; is left length <= source length
                    LD      strSrcLen
                    STW     strDstLen
                    POKE    strDstAddr                          ; new destination length
                    
stringL_skip        LDW     strSrcAddr
                    ADDW    strDstLen
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
stringRight         STW     strDstAddr
                    LD      strDstLen
                    POKE    strDstAddr                          ; destination length
                    BEQ     stringR_exit                        ; exit if right length = 0
                    LDW     strSrcAddr
                    PEEK                                        ; get source length
                    STW     strSrcLen
                    SUBW    strDstLen
                    BGE     stringR_skip                        ; length <= srcLength
                    LD      strSrcLen
                    STW     strDstLen
                    POKE    strDstAddr                          ; new destination length
                    LDI     0
                    
stringR_skip        ADDW    strSrcAddr
                    STW     strSrcAddr                          ; copy from (source address + (source length - right length)) to destination address
                    ADDW    strDstLen
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
stringMid           STW     strDstAddr
                    LD      strDstLen
                    POKE    strDstAddr                          ; destination length
                    BEQ     stringM_exit                        ; exit if right length = 0
                    LDW     strSrcAddr
                    PEEK                                        ; get source length
                    STW     strSrcLen
                    SUBW    strOffset                           
                    SUBW    strDstLen
                    BGE     stringM_skip                        ; length + offset <= srcLength
                    LD      strSrcLen
                    SUBW    strOffset
                    STW     strDstLen
                    POKE    strDstAddr                          ; new destination length
                    
stringM_skip        LDW     strSrcAddr
                    ADDW    strOffset
                    STW     strSrcAddr                          ; copy from (source address + (source length - right length)) to destination address
                    ADDW    strDstLen
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

%SUB                stringLower
                    ; creates a lower case string
stringLower         LDW     strSrcAddr
                    PEEK
                    POKE    strDstAddr                          ; dst length = src length
                    
stringLo_next       INC     strSrcAddr                          ; next char, (skips length byte)
                    INC     strDstAddr
                    LDW     strSrcAddr
                    PEEK
                    BEQ     stringLo_exit
                    ST      strChr
                    SUBI    65
                    BLT     stringLo_char
                    LD      strChr
                    SUBI    90
                    BGT     stringLo_char
                    LD      strChr                              ; >= 65 'A' and <= 90 'Z'
                    ADDI    32
                    ST      strChr
                    
stringLo_char       LD      strChr
                    POKE    strDstAddr                          ; lower case char
                    BRA     stringLo_next
                    
stringLo_exit       POKE    strDstAddr                          ; terminating 0
                    RET
%ENDS

%SUB                stringUpper
                    ; creates an upper case string
stringUpper         LDW     strSrcAddr
                    PEEK
                    POKE    strDstAddr                          ; dst length = src length
    
stringUp_next       INC     strSrcAddr                          ; next char, (skips length byte)
                    INC     strDstAddr
                    LDW     strSrcAddr
                    PEEK
                    BEQ     stringUp_exit
                    ST      strChr
                    SUBI    97
                    BLT     stringUp_char
                    LD      strChr
                    SUBI    122
                    BGT     stringUp_char
                    LD      strChr                              ; >= 97 'a' and <= 122 'z'
                    SUBI    32
                    ST      strChr
                    
stringUp_char       LD      strChr
                    POKE    strDstAddr                          ; upper case char
                    BRA     stringUp_next
                    
stringUp_exit       POKE    strDstAddr                          ; terminating 0
                    RET
%ENDS

%SUB                stringDigit
stringDigit         STW     strMult
                    LDW     strInteger
                    
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
                    STW     strDstAddr
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
                    CALLI   stringDigit
                    LDWI    1000
                    CALLI   stringDigit
                    LDI     100
                    CALLI   stringDigit
                    LDI     10
                    CALLI   stringDigit
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
