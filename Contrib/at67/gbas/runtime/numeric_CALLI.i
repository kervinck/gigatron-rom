; do *NOT* use register4 to register7 during time slicing if you call realTimeProc
intSrcAddr          EQU     register8
intDigit            EQU     register9
intResult           EQU     register10
intNegative         EQU     register11
intTemp0            EQU     register12
intTemp1            EQU     register15


%SUB                integerStr
                    ; converts a string to a +/- integer, assumes string pointer is pointing to first char and not the string length, (no overflow or underflow checks)
integerStr          LDI     0
                    ST      intNegative
                    STW     intResult
                    LDW     intSrcAddr
                    PEEK
                    SUBI    45                  ; -ve
                    BNE     integerS_loop
                    LDI     1
                    ST      intNegative
                    INC     intSrcAddr          ; skip -ve

integerS_loop       LDW     intSrcAddr
                    PEEK
                    SUBI    48                  ; str[i] - '0'
                    BLT     integerS_neg
                    STW     intDigit
                    SUBI    9
                    BGT     integerS_neg
                    LDW     intResult
                    LSLW
                    LSLW
                    ADDW    intResult
                    LSLW
                    ADDW    intDigit
                    STW     intResult           ; result = result*10 + digit
                    INC     intSrcAddr
                    BRA     integerS_loop
          
integerS_neg        LD      intNegative
                    BEQ     integerS_exit
                    LDI     0
                    SUBW    intResult           ; result *= -1
                    RET
                    
integerS_exit       LDW     intResult
                    RET
%ENDS