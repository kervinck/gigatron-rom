graphicsMode    EQU     register0


%SUB            scanlineMode
scanlineMode    LDW     giga_romType
                ANDI    0xF8
                SUBI    romTypeValue_ROMv2
                BGE     scanlineM_cont
                RET

scanlineM_cont  LDWI    SYS_SetMode_v2_80
                STW     giga_sysFn
                LDW     graphicsMode
                SYS     0xE6                ; 270 - max(14,80/2)
                RET
%ENDS
