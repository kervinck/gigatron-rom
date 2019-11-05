numericLabel        EQU     register0
lutLabs             EQU     register1
lutAddrs            EQU     register2
lutIndex            EQU     register3
lutLabel            EQU     register4


%SUB                gotoNumericLabel
                    ; find numeric label and jump to it
gotoNumericLabel    LDWI    lut_numericLabs
                    STW     lutLabs
                    STW     lutIndex
                    LDWI    lut_numericAddrs
                    STW     lutAddrs
                    
gotoNL_loop         LDW     lutIndex
                    DEEK
                    STW     lutLabel
                    LD      lutLabel + 1
                    ANDI    0x80                            ; check for -1
                    BEQ     gotoNL_cont
                    RET
                    
gotoNL_cont         LDW     lutLabel
                    SUBW    numericLabel
                    BEQ     gotoNL_found
                    INC     lutIndex
                    INC     lutIndex                        ; loop through lut until found or -1
                    BRA     gotoNL_loop
                    
gotoNL_found        LDW     lutIndex
                    SUBW    lutLabs
                    ADDW    lutAddrs
                    DEEK
                    CALL    giga_vAC                        ; fetch label address and jump, (note we never return from here)
%ENDS

%SUB                gosubNumericLabel
                    ; find numeric label and call it, (it had better return or welcome to lala land)
gosubNumericLabel   LDWI    lut_numericLabs
                    STW     lutLabs
                    STW     lutIndex
                    LDWI    lut_numericAddrs
                    STW     lutAddrs
                    
gosubNL_loop        LDW     lutIndex
                    DEEK
                    STW     lutLabel
                    LD      lutLabel + 1
                    ANDI    0x80                            ; check for -1
                    BEQ     gosubNL_cont
                    RET
                    
gosubNL_cont        LDW     lutLabel
                    SUBW    numericLabel
                    BEQ     gosubNL_found
                    INC     lutIndex
                    INC     lutIndex                        ; loop through lut until found or -1
                    BRA     gosubNL_loop
                    
gosubNL_found       LDW     lutIndex
                    SUBW    lutLabs
                    ADDW    lutAddrs
                    DEEK
                    PUSH
                    CALL    giga_vAC                        ; fetch label address and call
                    POP
                    RET
%ENDS