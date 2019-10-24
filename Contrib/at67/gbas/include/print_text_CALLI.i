textStr_CALLI       EQU     register0
textNum_CALLI       EQU     register0
textBak_CALLI       EQU     register0
textLen_CALLI       EQU     register1
textFont_CALLI      EQU     register2
textChr_CALLI       EQU     register3
textHex_CALLI       EQU     register4
textSlice_CALLI     EQU     register5
scanLine_CALLI      EQU     register6
digitMult_CALLI     EQU     register7
digitIndex_CALLI    EQU     register8


                    ; prints text using the inbuilt font
printText_CALLI     PUSH
                    LDW     textStr_CALLI             
                    PEEK
                    ST      textLen_CALLI       ; first byte is length

printT_C_char       INC     textStr_CALLI       ; next char
                    LDW     textStr_CALLI             
                    PEEK
                    ST      textChr_CALLI
                    CALLI   printChar_CALLI
                    LoopCounter textLen_CALLI printT_C_char
                    POP
                    RET


printDigit_CALLI    PUSH
                    LDW     textNum_CALLI
printD_C_index      SUBW    digitMult_CALLI
                    BLT     printD_C_cont
                    STW     textNum_CALLI
                    INC     digitIndex_CALLI
                    BRA     printD_C_index
    
printD_C_cont       LD      digitIndex_CALLI
                    BEQ     printD_C_exit
                    ORI     0x30
                    ST      textChr_CALLI
                    CALLI   printChar_CALLI
                    LDI     0x30
                    ST      digitIndex_CALLI
printD_C_exit       POP
                    RET
    
    
printVarInt16_CALLI PUSH
                    LDI     0
                    ST      digitIndex_CALLI
                    LDW     textNum_CALLI
                    BGE     printVI16_C_pos
                    LDI     0x2D
                    ST      textChr_CALLI
                    CALLI   printChar_CALLI
                    LDWI    0
                    SUBW    textNum_CALLI
printVI16_C_pos     STW     textNum_CALLI    
    
                    LDWI    10000
                    STW     digitMult_CALLI
                    CALLI   printDigit_CALLI
                    LDWI    1000
                    STW     digitMult_CALLI
                    CALLI   printDigit_CALLI
                    LDWI    100
                    STW     digitMult_CALLI
                    CALLI   printDigit_CALLI
                    LDWI    10
                    STW     digitMult_CALLI
                    CALLI   printDigit_CALLI
                    LD      textNum_CALLI
                    ORI     0x30
                    ST      textChr_CALLI
                    CALLI   printChar_CALLI
                    POP
                    RET
    
    
                    ; char in accumulator
printChar_CALLI     PUSH
                    LD      textChr_CALLI       ; (char-32)*5 + 0x0700
                    SUBI    32
                    STW     textChr_CALLI
                    STW     textFont_CALLI
                    LSLW    
                    LSLW    
                    ADDW    textChr_CALLI
                    STW     textFont_CALLI             
                    LDWI    giga_text32
                    ADDW    textFont_CALLI
                    STW     textFont_CALLI      ; text font slice base address for chars 32-81
    
                    LDW     textChr_CALLI
                    SUBI    50
                    BLT     printC_C_draw
                    LDW     textFont_CALLI
                    ADDI    0x06
                    STW     textFont_CALLI      ; text font slice base address for chars 82+
    
printC_C_draw       LDWI    SYS_VDrawBits_134
                    STW     giga_sysFn
                    LDW     textColour
                    STW     giga_sysArg0
                    LDWI    0x7FFF              ; mask out bottom row flag
                    ANDW    cursorXY
                    STW     giga_sysArg4        ; xy position
    
                    LDI     0x05
                    ST      textSlice_CALLI
    
printC_C_slice      LDW     textFont_CALLI      ; text font slice base address
                    LUP     0x00                ; get ROM slice
                    ST      giga_sysArg2        
                    SYS     0xCB                ; draw vertical slice, SYS_VDrawBits_134, 270 - 134/2 = 0xCB
                    INC     textFont_CALLI      ; next vertical slice
                    INC     giga_sysArg4        ; next x
                    LoopCounter textSlice_CALLI printC_C_slice
    
                    LD      cursorXY
                    ADDI    0x06
                    ST      cursorXY
                    SUBI    158
                    BLT     printC_C_exit
                    CALLI   newLineScroll_CALLI ; next row, scroll at bottom
printC_C_exit       POP
                    RET
    
    
                    ; print from top row to bottom row, then start scrolling 
newLineScroll_CALLI PUSH
                    LDI     0x02                ; x offset slightly
                    ST      cursorXY
                    LD      cursorXY+1
                    ANDI    0x80                ; on bottom row flag
                    BNE     newLS_C_cont
                    LD      cursorXY+1
                    ADDI    8
                    ST      cursorXY+1
                    SUBI    128
                    BLT     newLS_C_exit
                    
newLS_C_cont        CALLI   clearCursorRow
                    
                    LDWI    giga_videoTable
                    STW     scanLine_CALLI
    
                    ; scroll all scan lines by 8 through 0x08 to 0x7F
newLS_C_scroll      LDW     scanLine_CALLI
                    PEEK
                    ADDI    8
                    ANDI    0x7F
                    SUBI    8
                    BGE     newLS_C_adjust
                    ADDI    8
newLS_C_adjust      ADDI    8
                    POKE    scanLine_CALLI
                    INC     scanLine_CALLI      ; scan line pointers are 16bits
                    INC     scanLine_CALLI
                    LD      scanLine_CALLI
                    SUBI    0xF0                ; scanline pointers end at 0x01EE
                    BLT     newLS_C_scroll
    
                    ; read scan line pointer for last char row, use this as cursor position
                    LDWI    giga_videoTable + 0x00E0
                    PEEK
                    ORI     0x80                ; on bottom row
                    ST      cursorXY+1
newLS_C_exit        POP
                    RET
    
                    
printHexByte_CALLI  PUSH
                    LDWI    SYS_LSRW4_50    ; shift right by 4 SYS routine
                    STW     giga_sysFn
                    LD      textHex_CALLI
                    SYS     0xF5            ; SYS_LSRW4_50, 270 - 50/2 = 0xF5
                    SUBI    10
                    BLT     printH_C_skip0
                    ADDI    7
printH_C_skip0      ADDI    0x3A
                    ST      textChr_CALLI
                    CALLI   printChar_CALLI
                    LD      textHex_CALLI
                    ANDI    0x0F
                    SUBI    10
                    BLT     printH_C_skip1
                    ADDI    7
printH_C_skip1      ADDI    0x3A
                    ST      textChr_CALLI
                    CALLI   printChar_CALLI
                    POP
                    RET
                    
                    
printHexWord_CALLI  PUSH
                    LD      textHex_CALLI
                    ST      textBak_CALLI
                    LD      textHex_CALLI + 1
                    ST      textHex_CALLI
                    CALLI   printHexByte_CALLI
                    LD      textBak_CALLI
                    ST      textHex_CALLI
                    CALLI   printHexByte_CALLI
                    POP
                    RET

                

