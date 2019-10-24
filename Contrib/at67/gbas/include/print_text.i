textStr         EQU     register0
textNum         EQU     register0
textBak         EQU     register0
textLen         EQU     register1
textFont        EQU     register2
textChr         EQU     register3
textHex         EQU     register4
textSlice       EQU     register5
scanLine        EQU     register6
digitMult       EQU     register7
digitIndex      EQU     register8
clearLoop       EQU     register9


                ; clears the top 8 lines of pixels in preparation of text scrolling
clearCursorRow  LDWI    0x2020
                STW     giga_sysArg0        ; 4 pixels of colour
                STW     giga_sysArg2
                LDWI    SYS_Draw4_30        ; setup 4 pixel SYS routine
                STW     giga_sysFn

                LDI     8
                ST      clearLoop

                LDWI    giga_videoTable     ; current cursor position
                PEEK
                ST      giga_sysArg4 + 1

clearCR_loopy   LDI     giga_xres
                
clearCR_loopx   SUBI    4                   ; loop is unrolled 4 times
                ST      giga_sysArg4
                SYS     0xFF                ; SYS_Draw4_30, 270 - 30/2 = 0xFF
                SUBI    4
                ST      giga_sysArg4
                SYS     0xFF                ; SYS_Draw4_30, 270 - 30/2 = 0xFF
                SUBI    4
                ST      giga_sysArg4
                SYS     0xFF                ; SYS_Draw4_30, 270 - 30/2 = 0xFF
                SUBI    4
                ST      giga_sysArg4
                SYS     0xFF                ; SYS_Draw4_30, 270 - 30/2 = 0xFF
                BGT     clearCR_loopx

                INC     giga_sysArg4 + 1    ; next line                
                LoopCounter clearLoop clearCR_loopy
                RET
                
                ; prints text using the inbuilt font
printText       PUSH
                LDW     textStr             
                PEEK
                ST      textLen             ; first byte is length

printT_char     INC     textStr             ; next char
                LDW     textStr             
                PEEK
                ST      textChr
                LDWI    printChar
                CALL    giga_vAC
printT_loop     LoopCounter textLen printT_char
                POP
                RET

                
printDigit      PUSH
                LDW     textNum
printD_index    SUBW    digitMult
                BLT     printD_cont
                STW     textNum
                INC     digitIndex
                BRA     printD_index

printD_cont     LD      digitIndex
                BEQ     printD_exit
                ORI     0x30
                ST      textChr
                LDWI    printChar
                CALL    giga_vAC
                LDI     0x30
                ST      digitIndex
printD_exit     POP
                RET


printVarInt16   PUSH
                LDI     0
                ST      digitIndex
                LDW     textNum
                BGE     printVI16_pos
                LDI     0x2D
                ST      textChr
                LDWI    printChar
                CALL    giga_vAC
                LDWI    0
                SUBW    textNum
printVI16_pos   STW     textNum    

                LDWI    10000
                STW     digitMult
                LDWI    printDigit
                CALL    giga_vAC
                LDWI    1000
                STW     digitMult
                LDWI    printDigit
                CALL    giga_vAC
                LDWI    100
                STW     digitMult
                LDWI    printDigit
                CALL    giga_vAC
                LDWI    10
                STW     digitMult
                LDWI    printDigit
                CALL    giga_vAC
                LD      textNum
                ORI     0x30
                ST      textChr
                LDWI    printChar
                CALL    giga_vAC
                POP
                RET


                ; char in accumulator
printChar       PUSH
                LD      textChr             ; (char-32)*5 + 0x0700
                SUBI    32
                STW     textChr
                STW     textFont
                LSLW    
                LSLW    
                ADDW    textChr
                STW     textFont             
                LDWI    giga_text32
                ADDW    textFont
                STW     textFont            ; text font slice base address for chars 32-81

                LDW     textChr
                SUBI    50
                BLT     printC_draw
                LDW     textFont
                ADDI    0x06
                STW     textFont            ; text font slice base address for chars 82+

printC_draw     LDWI    SYS_VDrawBits_134
                STW     giga_sysFn
                LDW     textColour
                STW     giga_sysArg0
                LDWI    0x7FFF              ; mask out bottom row flag
                ANDW    cursorXY
                STW     giga_sysArg4        ; xy position

                LDI     0x05
                ST      textSlice

printC_slice    LDW     textFont            ; text font slice base address
                LUP     0x00                ; get ROM slice
                ST      giga_sysArg2        
                SYS     0xCB                ; draw vertical slice, SYS_VDrawBits_134, 270 - 134/2 = 0xCB
                INC     textFont            ; next vertical slice
                INC     giga_sysArg4        ; next x
                LoopCounter textSlice printC_slice

                LD      cursorXY
                ADDI    0x06
                ST      cursorXY
                SUBI    158
                BLT     printC_exit
                LDWI    newLineScroll       ; next row, scroll at bottom
                CALL    giga_vAC
printC_exit     POP
                RET


                ; print from top row to bottom row, then start scrolling 
newLineScroll   PUSH
                LDI     0x02                ; x offset slightly
                ST      cursorXY
                LD      cursorXY+1
                ANDI    0x80                ; on bottom row flag
                BNE     newLS_cont
                LD      cursorXY+1
                ADDI    8
                ST      cursorXY+1
                SUBI    128
                BLT     newLS_exit
                
newLS_cont      LDWI    clearCursorRow
                CALL    giga_vAC
                
                LDWI    giga_videoTable
                STW     scanLine

                ; scroll all scan lines by 8 through 0x08 to 0x7F
newLS_scroll    LDW     scanLine
                PEEK
                ADDI    8
                ANDI    0x7F
                SUBI    8
                BGE     newLS_adjust
                ADDI    8
newLS_adjust    ADDI    8
                POKE    scanLine
                INC     scanLine            ; scan line pointers are 16bits
                INC     scanLine
                LD      scanLine
                SUBI    0xF0                ; scanline pointers end at 0x01EE
                BLT     newLS_scroll

                ; read scan line pointer for last char row, use this as cursor position
                LDWI    giga_videoTable + 0x00E0
                PEEK
                ORI     0x80                ; on bottom row
                ST      cursorXY+1
newLS_exit      POP
                RET

                
printHexByte    PUSH
                LDWI    SYS_LSRW4_50    ; shift right by 4 SYS routine
                STW     giga_sysFn
                LD      textHex
                SYS     0xF5            ; SYS_LSRW4_50, 270 - 50/2 = 0xF5
                SUBI    10
                BLT     printH_skip0
                ADDI    7
printH_skip0    ADDI    0x3A
                ST      textChr
                LDWI    printChar
                CALL    giga_vAC
                LD      textHex
                ANDI    0x0F
                SUBI    10
                BLT     printH_skip1
                ADDI    7
printH_skip1    ADDI    0x3A
                ST      textChr
                LDWI    printChar
                CALL    giga_vAC
                POP
                RET
                
                
printHexWord    PUSH
                LD      textHex
                ST      textBak
                LD      textHex + 1
                ST      textHex
                LDWI    printHexByte
                CALL    giga_vAC
                LD      textBak
                ST      textHex
                LDWI    printHexByte
                CALL    giga_vAC
                POP
                RET

                

