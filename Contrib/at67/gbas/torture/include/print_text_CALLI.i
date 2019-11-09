textStr             EQU     register0
textNum             EQU     register0
textScratch         EQU     register0
textLen             EQU     register1
textFont            EQU     register2
textChr             EQU     register3
textHex             EQU     register4
textSlice           EQU     register5
scanLine            EQU     register6
digitMult           EQU     register7
digitIndex          EQU     register8
clearLoop           EQU     register9
    
    
%SUB                clearCursorRow
                    ; clears the top 8 lines of pixels in preparation of text scrolling
clearCursorRow      LD      fgbgColour
                    ST      giga_sysArg0
                    ST      giga_sysArg0 + 1
                    ST      giga_sysArg2
                    ST      giga_sysArg2 + 1                ; 4 pixels of colour
                    
                    LDWI    SYS_Draw4_30                    ; setup 4 pixel SYS routine
                    STW     giga_sysFn
    
                    LDI     8
                    ST      clearLoop
    
                    LDWI    giga_videoTable                 ; current cursor position
                    PEEK
                    ST      giga_sysArg4 + 1
    
clearCR_loopy       LDI     giga_xres
                    
clearCR_loopx       SUBI    4                               ; loop is unrolled 4 times
                    ST      giga_sysArg4
                    SYS     0xFF                            ; SYS_Draw4_30, 270 - 30/2 = 0xFF
                    SUBI    4
                    ST      giga_sysArg4
                    SYS     0xFF                            ; SYS_Draw4_30, 270 - 30/2 = 0xFF
                    SUBI    4
                    ST      giga_sysArg4
                    SYS     0xFF                            ; SYS_Draw4_30, 270 - 30/2 = 0xFF
                    SUBI    4
                    ST      giga_sysArg4
                    SYS     0xFF                            ; SYS_Draw4_30, 270 - 30/2 = 0xFF
                    BGT     clearCR_loopx
    
                    INC     giga_sysArg4 + 1                ; next line                
                    LoopCounter clearLoop clearCR_loopy
                    RET
%ENDS   

%SUB                printInit
printInit           LDWI    SYS_VDrawBits_134
                    STW     giga_sysFn
                    LDW     fgbgColour
                    STW     giga_sysArg0
                    LDWI    0x0800
                    ADDW    cursorXY
                    STW     giga_sysArg4                    ; xy position
                    RET
%ENDS

%SUB                printText
                    ; prints text string pointed to by the accumulator
printText           PUSH
                    STW     textStr             
                    PEEK
                    ST      textLen                         ; first byte is length
        
printT_char         INC     textStr                         ; next char
                    LDW     textStr             
                    PEEK
                    CALLI   printChar
                    LoopCounter textLen printT_char
                    POP
                    RET
%ENDS   
    
%SUB                printDigit
                    ; prints single digit in textNum
printDigit          PUSH
                    STW     digitMult
                    LDW     textNum
printD_index        SUBW    digitMult
                    BLT     printD_cont
                    STW     textNum
                    INC     digitIndex
                    BRA     printD_index
            
printD_cont         LD      digitIndex
                    BEQ     printD_exit
                    ORI     0x30
                    CALLI   printChar
                    LDI     0x30
                    ST      digitIndex
printD_exit         POP
                    RET
%ENDS       
        
%SUB                printInt16
                    ; prints 16bit int in the accumulator
printInt16          PUSH
                    STW     textNum
                    LDI     0
                    ST      digitIndex
                    LDW     textNum
                    BGE     printI16_pos
                    LDI     0x2D
                    CALLI   printChar
                    LDWI    0
                    SUBW    textNum
printI16_pos        STW     textNum    
            
                    LDWI    10000
                    CALLI   printDigit
                    LDWI    1000
                    CALLI   printDigit
                    LDWI    100
                    CALLI   printDigit
                    LDWI    10
                    CALLI   printDigit
                    LD      textNum
                    ORI     0x30
                    CALLI   printChar
                    POP
                    RET
%ENDS       

%SUB                printHexByte
                    ; print hex byte in the accumulator
printHexByte        PUSH
                    ST      textHex
                    LDWI    SYS_LSRW4_50                    ; shift right by 4 SYS routine
                    STW     giga_sysFn
                    LD      textHex
                    SYS     0xF5                            ; SYS_LSRW4_50, 270 - 50/2 = 0xF5
                    SUBI    10
                    BLT     printH_skip0
                    ADDI    7
printH_skip0        ADDI    0x3A
                    CALLI   printChar
                    LD      textHex
                    ANDI    0x0F
                    SUBI    10
                    BLT     printH_skip1
                    ADDI    7
printH_skip1        ADDI    0x3A
                    CALLI   printChar
                    POP
                    RET
%ENDS   
        
%SUB                printHexWord
                    ; print hex word in the accumulator
printHexWord        PUSH
                    STW     textScratch
                    LD      textScratch + 1
                    CALLI   printHexByte
                    LD      textScratch
                    CALLI   printHexByte
                    POP
                    RET
%ENDS   

%SUB                printChar
                    ; prints char in the accumulator
printChar           PUSH
                    ST      textChr                         ; (char-32)*5 + 0x0700
                    SUBI    32
                    STW     textChr
                    STW     textFont
                    LSLW    
                    LSLW    
                    ADDW    textChr
                    STW     textFont             
                    LDWI    giga_text32
                    ADDW    textFont
                    STW     textFont                        ; text font slice base address for chars 32-81
            
                    LDW     textChr
                    SUBI    50
                    BLT     printC_draw
                    LDW     textFont
                    ADDI    0x06
                    STW     textFont                        ; text font slice base address for chars 82+
            
printC_draw         CALLI   printInit
                    LDI     0x05
                    ST      textSlice
            
printC_slice        LDW     textFont                        ; text font slice base address
                    LUP     0x00                            ; get ROM slice
                    ST      giga_sysArg2        
                    SYS     0xCB                            ; draw vertical slice, SYS_VDrawBits_134, 270 - 134/2 = 0xCB
                    INC     textFont                        ; next vertical slice
                    INC     giga_sysArg4                    ; next x
                    LoopCounter textSlice printC_slice
                    ST      giga_sysArg2                    ; result of loopCounter is always 0
                    SYS     0xCB                            ; draw last blank slice

                    LD      cursorXY
                    ADDI    0x06
                    ST      cursorXY
                    SUBI    giga_xres - 5                   ; giga_xres - 6, (154), is last possible char in row
                    BLT     printC_exit
                    CALLI   newLineScroll ; next row, scroll at bottom
printC_exit         POP
                    RET
%ENDS   
        
%SUB                newLineScroll
                    ; print from top row to bottom row, then start scrolling 
newLineScroll       LDI     0x02                            ; x offset slightly
                    ST      cursorXY
                    LDWI    0x0001
                    ANDW    miscFlags
                    BNE     newLS_cont0                     ; scroll on or off
                    RET
                    
newLS_cont0         PUSH
                    LDWI    0x8000
                    ANDW    miscFlags                       ; on bottom row flag
                    BNE     newLS_cont1
                    LD      cursorXY+1
                    ADDI    8
                    ST      cursorXY+1
                    SUBI    giga_yres
                    BLT     newLS_exit
                            
newLS_cont1         CALLI   clearCursorRow
                            
                    LDWI    giga_videoTable
                    STW     scanLine
            
                    ; scroll all scan lines by 8 through 0x08 to 0x7F
newLS_scroll        LDW     scanLine
                    PEEK
                    ADDI    8
                    ANDI    0x7F
                    SUBI    8
                    BGE     newLS_adjust
                    ADDI    8
newLS_adjust        ADDI    8
                    POKE    scanLine
                    INC     scanLine                        ; scan line pointers are 16bits
                    INC     scanLine
                    LD      scanLine
                    SUBI    0xF0                            ; scanline pointers end at 0x01EE
                    BLT     newLS_scroll
    
                    LDWI    0x8000
                    ORW     miscFlags
                    STW     miscFlags                       ; set on bottom row flag
                    
                    ; read scan line pointer for last char row, use this as cursor position
                    LDWI    giga_videoTable + 0x00E0
                    PEEK
                    SUBI    0x08                            ; corrected in printChar
                    ST      cursorXY+1
newLS_exit          POP
                    RET
%ENDS   

%SUB                atTextCursor
atTextCursor        LD      cursorXY
                    SUBI    giga_xres - 5
                    BLT     atTC_skip0
                    LDI     0
                    STW     cursorXY
                    
atTC_skip0          LD      cursorXY + 1
                    SUBI    giga_yres - 7
                    BLT     atTC_skip1
                    LDI     giga_yres - 8
                    STW     cursorXY + 1
                    
atTC_skip1          LD      cursorXY + 1
                    SUBI    giga_yres - 8
                    BGE     atTC_skip2
                    LDWI    0x7FFF
                    ANDW    miscFlags
                    STW     miscFlags                       ; reset on bottom row flag
                    RET
                    
atTC_skip2          LDWI    0x8000
                    ORW     miscFlags
                    STW     miscFlags                       ; set on bottom row flag
                    RET
%ENDS