mathX           EQU     register0
mathY           EQU     register1
mathSum         EQU     register2
mathMask        EQU     register3
mathRem         EQU     register4
mathSign        EQU     register5
mathScratch     EQU     register6


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