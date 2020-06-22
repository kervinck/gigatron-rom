; do *NOT* use register4 to register7 during time slicing
intSrcA             EQU     register0
intSrcB             EQU     register1
intSrcX             EQU     register2
intSwap             EQU     register3
intSrcAddr          EQU     register8
intDigit            EQU     register9
intResult           EQU     register10
intNegative         EQU     register11
bcdLength           EQU     register8
bcdSrcAddr          EQU     register9
bcdDstAddr          EQU     register10
bcdSrcData          EQU     register11
bcdDstData          EQU     register11          ; alias to make code less confusing
bcdCarry            EQU     register12
bcdBorrow           EQU     register12          ; alias to make code less confusing
bcdValue            EQU     register0
bcdDigit            EQU     register1
bcdMult             EQU     register2


%SUB                integerMin
integerMin          LDW     intSrcA
                    SUBW    intSrcB
                    BLE     integerMi_A
                    LDW     intSrcB
                    RET

integerMi_A         LDW     intSrcA
                    RET
%ENDS

%SUB                integerMax
integerMax          LDW     intSrcA
                    SUBW    intSrcB
                    BGE     integerMa_A
                    LDW     intSrcB
                    RET

integerMa_A         LDW     intSrcA
                    RET
%ENDS

%SUB                integerClamp
integerClamp        LDW     intSrcX
                    SUBW    intSrcA
                    BGE     integerCl_X
                    BRA     integerCl_A0

integerCl_X         LDW     intSrcX
                    STW     intSrcA

integerCl_A0        LDW     intSrcA
                    SUBW    intSrcB
                    BLE     integerCl_A1
                    LDW     intSrcB
                    RET

integerCl_A1        LDW     intSrcA
                    RET
%ENDS

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

                    ; bcd values are stored unpacked lsd to msd
%SUB                bcdAdd
bcdAdd              LDI     0
                    STW     bcdCarry
                    
bcdA_loop           LDW     bcdDstAddr
                    PEEK                        ; expects unpacked byte values 0 to 9
                    STW     bcdDstData
                    LDW     bcdSrcAddr
                    PEEK                        ; expects unpacked byte values 0 to 9
                    ADDW    bcdDstData
                    ADDW    bcdCarry
                    STW     bcdDstData
                    SUBI    10                  ; no handling of values > 9
                    BLT     bcdA_nc
                    STW     bcdDstData
                    LDI     1
                    BRA     bcdA_cont
          
bcdA_nc             LDI     0
                    
bcdA_cont           STW     bcdCarry
          
                    LDW     bcdDstData
                    POKE    bcdDstAddr          ; modifies dst bcd value
                    INC     bcdDstAddr
                    INC     bcdSrcAddr
                    LD      bcdLength
                    SUBI    1
                    ST      bcdLength           ; expects src and dst lengths to be equal
                    BGT     bcdA_loop
                    RET
%ENDS

                    ; bcd values are stored unpacked lsd to msd
%SUB                bcdSub
bcdSub              LDI     0
                    STW     bcdBorrow
                    
bcdS_loop           LDW     bcdSrcAddr
                    PEEK                        ; expects unpacked byte values 0 to 9
                    STW     bcdSrcData
                    LDW     bcdDstAddr
                    PEEK                        ; expects unpacked byte values 0 to 9
                    SUBW    bcdSrcData
                    SUBW    bcdBorrow
                    STW     bcdDstData
                    BGE     bcdS_nb
                    ADDI    10
                    STW     bcdDstData
                    LDI     1
                    BRA     bcdS_cont
          
bcdS_nb             LDI     0
                    
bcdS_cont           STW     bcdBorrow
          
                    LDW     bcdDstData
                    POKE    bcdDstAddr          ; modifies dst bcd value
                    INC     bcdDstAddr
                    INC     bcdSrcAddr
                    LD      bcdLength
                    SUBI    1
                    ST      bcdLength           ; expects src and dst lengths to be equal
                    BGT     bcdS_loop
                    RET
%ENDS

%SUB                bcdDigits
bcdDigits           LDW     bcdValue

bcdD_index          SUBW    bcdMult
                    BLT     bcdD_cont
                    STW     bcdValue
                    INC     bcdDigit                            ; calculate digit
                    BRA     bcdD_index
    
bcdD_cont           LD      bcdDigit
                    POKE    bcdDstAddr                          ; store digit
                    LDW     bcdDstAddr
                    SUBI    1
                    STW     bcdDstAddr
                    LDI     0
                    ST      bcdDigit                            ; reset digit
                    
bcdD_exit           RET
%ENDS

%SUB                bcdInt
                    ; create a bcd value from a +ve int, (max 42767)
bcdInt              PUSH
                    LDW     bcdDstAddr
                    ADDI    4
                    STW     bcdDstAddr                          ; bcdDstAddr must point to >= 5 digit bcd value
                    LDI     0
                    STW     bcdDigit
                    LDWI    10000
                    STW     bcdMult
                    LDWI    bcdDigits
                    CALL    giga_vAC
                    LDWI    1000
                    STW     bcdMult
                    LDWI    bcdDigits
                    CALL    giga_vAC
                    LDI     100
                    STW     bcdMult
                    LDWI    bcdDigits
                    CALL    giga_vAC
                    LDI     10
                    STW     bcdMult
                    LDWI    bcdDigits
                    CALL    giga_vAC
                    LD      bcdValue
                    POKE    bcdDstAddr
                    POP
                    RET
%ENDS

                    ; bcd values are stored unpacked lsd to msd
                    ; cmp expects addrs to be pointing to msd!
%SUB                bcdCmp
bcdCmp              LDW     bcdDstAddr
                    PEEK                        ; expects unpacked byte values 0 to 9
                    STW     bcdDstData
                    LDW     bcdSrcAddr
                    PEEK                        ; expects unpacked byte values 0 to 9
                    SUBW    bcdDstData
                    BGT     bcdC_gt
                    BLT     bcdC_lt
                    PUSH
                    LDWI    bcdCmpExt
                    CALL    giga_vAC
                    POP
                    BGT     bcdCmp
                    LDI     0
                    RET

bcdC_gt             LDI     1
                    RET
                    
bcdC_lt             LDWI    -1
                    RET                    
%ENDS

%SUB                bcdCmpExt
bcdCmpExt           LDW     bcdDstAddr
                    SUBI    1
                    STW     bcdDstAddr
                    LDW     bcdSrcAddr
                    SUBI    1
                    STW     bcdSrcAddr
                    LD      bcdLength
                    SUBI    1
                    ST      bcdLength           ; expects src and dst lengths to be equal
                    RET
%ENDS

%SUB                bcdCpy
bcdCpy              LDW     bcdSrcAddr
                    PEEK
                    POKE    bcdDstAddr
                    INC     bcdSrcAddr
                    INC     bcdDstAddr
                    LD      bcdLength
                    SUBI    1
                    ST      bcdLength           ; expects src and dst lengths to be equal
                    BGT     bcdCpy
                    RET
%ENDS
