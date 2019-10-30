mathX           EQU     register0
mathY           EQU     register1
mathSum         EQU     register2
mathMask        EQU     register3
mathRem         EQU     register4
mathSign        EQU     register5
mathScratch     EQU     register6
mathShift       EQU     register7


%SUB            multiply16bit
                ; accumulator = mathX * mathY, (result 16bit)
multiply16bit   LDI     0
                STW     mathSum
                LDI     1

multiply16_loop STW     mathMask
                ANDW    mathY
                BEQ     multiply16_skip
                LDW     mathSum
                ADDW    mathX
                STW     mathSum
                
multiply16_skip LDW     mathX
                ADDW    mathX
                STW     mathX
                LDW     mathMask
                ADDW    mathMask
                BNE     multiply16_loop
                
                LDW     mathSum
                RET
%ENDS

%SUB            divide16bit
                ; accumulator:mathRem = mathX / mathY, (results 16bit)
divide16bit     LDW     mathX
                XORW    mathY
                STW     mathSign
                LDW     mathX
                BGE     divide16_skip0
                LDI     0
                SUBW    mathX
                STW     mathX
                
divide16_skip0  LDW     mathY                     
                BGE     divide16_skip1
                LDI     0
                SUBW    mathY
                STW     mathY
                
divide16_skip1  LDI     0
                STW     mathRem

divide16_loop   STW     mathScratch
                LDW     mathRem
                ADDW    mathRem
                STW     mathRem
                LDW     mathX
                BGE     divide16_skip3
                INC     mathRem
                
divide16_skip3  LDW     mathX
                ADDW    mathX
                STW     mathX
                LDW     mathRem
                SUBW    mathY
                BLT     divide16_skip4
                STW     mathRem
                INC     mathX
                
divide16_skip4  LDW     mathScratch
                ADDI    1
                ANDI    0x0F
                BNE     divide16_loop
                LDW     mathSign
                BGE     divide16_exit
                LDI     0
                SUBW    mathX
                RET
                
divide16_exit   LDW     mathX
                RET
%ENDS

%SUB            random16bit
random16bit     LDWI    SYS_Random_34
                STW     giga_sysFn
                SYS     0xFD
                RET
%ENDS

%SUB            shiftLeft4bit
shiftLeft4bit   LDWI    SYS_LSLW4_46
                STW     giga_sysFn          ; 0xF7 = 270-max(14,46/2)
                LDW     mathShift
                SYS     0xF7
                RET
%ENDS

%SUB            shiftLeft8bit
shiftLeft8bit   LDWI    SYS_LSLW8_24
                STW     giga_sysFn          ; 0x00 = 270-max(14,24/2)
                LDW     mathShift
                SYS     0x00
                RET
%ENDS

%SUB            shiftRight1bit
shiftRight1bit  LDWI    SYS_LSRW1_48
                STW     giga_sysFn          ; 0xF6 = 270-max(14,48/2)
                LDW     mathShift
                SYS     0xF6
                RET
%ENDS

%SUB            shiftRight2bit
shiftRight2bit  LDWI    SYS_LSRW2_52
                STW     giga_sysFn          ; 0xF4 = 270-max(14,52/2)
                LDW     mathShift
                SYS     0xF4
                RET
%ENDS

%SUB            shiftRight3bit
shiftRight3bit  LDWI    SYS_LSRW3_52
                STW     giga_sysFn          ; 0xF4 = 270-max(14,52/2)
                LDW     mathShift
                SYS     0xF4
                RET
%ENDS

%SUB            shiftRight4bit
shiftRight4bit  LDWI    SYS_LSRW4_50
                STW     giga_sysFn          ; 0xF5 = 270-max(14,50/2)
                LDW     mathShift
                SYS     0xF5
                RET
%ENDS

%SUB            shiftRight5bit
shiftRight5bit  LDWI    SYS_LSRW5_50
                STW     giga_sysFn          ; 0xF5 = 270-max(14,50/2)
                LDW     mathShift
                SYS     0xF5
                RET
%ENDS

%SUB            shiftRight6bit
shiftRight6bit  LDWI    SYS_LSRW6_48
                STW     giga_sysFn          ; 0xF6 = 270-max(14,48/2)
                LDW     mathShift
                SYS     0xF6
                RET
%ENDS

%SUB            shiftRight7bit
shiftRight7bit  LDWI    SYS_LSRW7_30
                STW     giga_sysFn          ; 0xFF = 270-max(14,30/2)
                LDW     mathShift
                SYS     0xFF
                RET
%ENDS

%SUB            shiftRight8bit
shiftRight8bit  LDWI    SYS_LSRW8_24
                STW     giga_sysFn          ; 0x00 = 270-max(14,24/2)
                LDW     mathShift
                SYS     0x00
                RET
%ENDS
