textStr         EQU     register0
textDigits      EQU     register0
textLen         EQU     register1
textFont        EQU     register2
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
                CALL    printChar
printT_loop     LoopCounter textLen printT_char
                POP
                RET

                
                ; arg in accumulator, result in accumulator and textChr
validChar       ANDI    0x7F                ; char = <32...127>
                ST      textChr
                SUBI    32
                BGE     validC_chr
                LDI     32
                ST      textChr
validC_chr      LD      textChr
                RET


printDigit      PUSH
                LDW     textDigits
printD_index    SUBW    digitMult
                BLT     printD_cont
                STW     textDigits
                INC     digitIndex
                BRA     printD_index

printD_cont     LD      digitIndex
                BEQ     printD_exit
                ORI     0x30
                CALL    printChar
                LDI     0x30
                ST      digitIndex
printD_exit     POP
                RET


printVarInt16   PUSH
                LDI     0
                ST      digitIndex
                LDW     textDigits
                BGE     printVI16_pos
                LDI     0x2D
                CALL    printChar
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
                CALL    printChar
                POP
                RET


                ; char in accumulator
printChar       SUBI    32                  ; (char - 32)*5 + 0x0700
                STW     textChr
                STW     textFont
                LSLW    textFont
                LSLW    textFont
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
                PUSH
                CALL    newLineScroll       ; next row, scroll at bottom
                POP
printC_exit     RET


                ; print from top row to bottom row, then start scrolling 
newLineScroll   LDI     0x02                ; x offset slightly
                ST      cursorXY
                LD      cursorXY+1
                ANDI    0x80                ; on bottom row flag
                BNE     newLS_cont
                LD      cursorXY+1
                ADDI    8
                ST      cursorXY+1
                SUBI    128
                BLT     newLS_exit
                
newLS_cont      LDWI    giga_videoTable
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

newLS_exit      RET
