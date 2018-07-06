textStr         EQU     register0
textDigits      EQU     register0
textLen         EQU     register1
textBase        EQU     register2
textChr         EQU     register3
textSlice       EQU     register4
scanLine        EQU     register5
digitMult       EQU     register6
digitIndex      EQU     register7


                ; prints text using the inbuilt font
printText       PUSH
                LDW     textStr             
                PEEK
                ST      textLen             ; first byte is length

printT_char     INC     textStr             ; next char
                LDW     textStr             
                PEEK
                CALL    getFontChar
                CALL    drawChar
printT_loop     LoopCounter textLen printT_char
                POP
                RET


printDigit      PUSH
                LDW     textDigits
printD_index    SUBW    digitMult
                BLT     printD_cont
                STW     textDigits
                INC     digitIndex
                BRA     printD_index

printD_cont     LD      digitIndex
                ;BEQ     printD_exit
                ORI     0x30
                CALL    getFontChar
                CALL    drawChar
                LDI     0x30
                ST      digitIndex
printD_exit     POP
                RET


printChar       PUSH
                LD      textChr
                CALL    getFontChar
                CALL    drawChar
                POP
                RET


printVarInt16   PUSH
                LDI     0
                ST      digitIndex
                LDW     textDigits
                BGE     printVI16_pos
                LDI     0x2D
                CALL    getFontChar
                CALL    drawChar
                LDWI    0
                SUBW    textDigits
printVI16_pos   STW     textDigits    

                LDWI    10000
                STW     digitMult
                CALL    printDigit
                LDWI    1000
                STW     digitMult
                CALL    printDigit
                LDWI    100
                STW     digitMult
                CALL    printDigit
                LDWI    10
                STW     digitMult
                CALL    printDigit
                LD      textDigits
                ORI     0x30
                CALL    getFontChar
                CALL    drawChar
                POP
                RET


drawChar        LDW     textColour
                STW     giga_sysArg0
                LDWI    SYS_VDrawBits_134
                STW     giga_sysFn
                LDI     0x05
                ST      textSlice
drawC_slice     LDW     textPosition
                STW     giga_sysArg4        ; xy
                LDW     textBase            ; text font slice base address
                LUP     0x00                ; get ROM slice
                ST      giga_sysArg2        
                SYS     0xCB                ; draw vertical slice, SYS_VDrawBits_134, 270 - 134/2 = 0xCB
                INC     textBase            ; next vertical slice
                INC     textPosition        ; next x
                LoopCounter textSlice drawC_slice

                INC     textPosition        ; 1 pixel space between chars
                LD      textPosition
                SUBI    158
                BLT     drawC_exit
                PUSH
                CALL    updateCursorY       ; next row, scroll at bottom
                POP
drawC_exit      RET


                ; accumulator = char
getFontChar     SUBI    32                  ; (char - 32)*5 + 0x0700
                BGE     getFC_cont
                LDI     0

getFC_cont      ANDI    0x7F                ; 32 >= char <= 127
                STW     textChr
                STW     textBase
                LSLW    textBase
                LSLW    textBase
                ADDW    textChr
                STW     textBase             
                LDWI    giga_text32
                ADDW    textBase
                STW     textBase            ; text font slice base address for chars 32-81

                LDW     textChr
                SUBI    50
                BLT     getFC_exit
                LDW     textBase
                ADDI    0x06
                STW     textBase            ; text font slice base address for chars 82+
getFC_exit      RET


locateCursor    LDW     cursorXY
                STW     textPosition
                RET

                
                ; print from top row to bottom row, then start scrolling 
updateCursorY   LD      cursorXY+1
                ANDI    0x80            ; on bottom row flag
                BNE     updateCY_cont
                LD      cursorXY+1
                ADDI    8
                ST      cursorXY+1
                SUBI    128
                BLT     updateCY_exit

updateCY_cont   LDWI    giga_videoTable
                STW     scanLine

                ; scroll all scan lines by 8 through 0x08 to 0x7F
updateCY_scroll LDW     scanLine
                PEEK
                ADDI    8
                ANDI    0x7F
                SUBI    8
                BGE     updateCY_adjust
                ADDI    8
updateCY_adjust ADDI    8
                POKE    scanLine
                INC     scanLine    ; scan line pointers are 16bits
                INC     scanLine
                LD      scanLine
                SUBI    0xF0        ; scanline pointers end at 0x01EE
                BLT     updateCY_scroll

                ; read scan line pointer for last char row, use this as cursor position
                LDWI    giga_videoTable + 0x00E0
                PEEK
                ORI     0x80        ; on bottom row
                ST      cursorXY+1

updateCY_exit   LDWI    0x7FFF      ; mask out bottom row flag
                ANDW    cursorXY
                STW     textPosition
                LDWI    0x0002      ; start of video ram, offset in x slightly
                ADDW    textPosition
                STW     textPosition
                RET
