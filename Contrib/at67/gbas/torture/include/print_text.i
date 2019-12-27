; do *NOT* use register4 to register7 during time slicing if you call realTimeProc
textStr             EQU     register0
textNum             EQU     register0
textBak             EQU     register0
textLen             EQU     register1
textOfs             EQU     register2
textChr             EQU     register3
textHex             EQU     register8
textFont            EQU     register9
textSlice           EQU     register10
scanLine            EQU     register11
digitMult           EQU     register12
digitIndex          EQU     register13
clearLoop           EQU     register14
    
    
%SUB                clearCursorRow
                    ; clears the top 8 lines of pixels in preparation of text scrolling
clearCursorRow      PUSH
                    LD      fgbgColour
                    ST      giga_sysArg0
                    ST      giga_sysArg0 + 1
                    ST      giga_sysArg2
                    ST      giga_sysArg2 + 1                ; 4 pixels of colour
    
                    LDWI    SYS_Draw4_30                    ; setup 4 pixel SYS routine
                    STW     giga_sysFn
    
                    LDWI    giga_videoTable                 ; current cursor position
                    PEEK
                    ST      giga_sysArg4 + 1
                    LDI     8

clearCR_loopy       ST      clearLoop
                    CALL    realTimeProcAddr
                    LDI     giga_xres
                    
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
                    LD      clearLoop
                    SUBI    1
                    BNE     clearCR_loopy

                    LDWI    printInit
                    CALL    giga_vAC                        ; re-initialise the SYS registers
                    POP
                    RET
%ENDS

%SUB                printInit
printInit           LDWI    SYS_VDrawBits_134
                    STW     giga_sysFn
                    LDW     fgbgColour
                    STW     giga_sysArg0
                    
                    LDWI    giga_videoTable
                    STW     giga_sysArg4
                    LD      cursorXY + 1
                    LSLW
                    ADDW    giga_sysArg4
                    PEEK
                    ST      giga_sysArg4 + 1
                    LD      cursorXY
                    ST      giga_sysArg4                    ; xy position
                    RET
%ENDS
                    
%SUB                printText
                    ; prints text string pointed to by the accumulator
printText           PUSH
                    LDWI    printInit
                    CALL    giga_vAC
                    LDW     textStr             
                    PEEK                                    ; first byte is length
                    BEQ     printT_exit
    
printT_char         ST      textLen
                    INC     textStr                         ; next char
                    LDW     textStr             
                    PEEK
                    ST      textChr
                    LDWI    printChar
                    CALL    giga_vAC

                    LD      textLen
                    SUBI    1
                    BNE     printT_char
printT_exit         POP
                    RET
%ENDS   

%SUB                printLeft
                    ; prints left sub string pointed to by the accumulator
printLeft           PUSH
                    LDWI    printInit
                    CALL    giga_vAC
                    LD      textLen
                    BEQ     printL_exit
    
printL_char         ST      textLen
                    INC     textStr                         ; next char
                    LDW     textStr             
                    PEEK
                    ST      textChr
                    LDWI    printChar
                    CALL    giga_vAC

                    LD      textLen
                    SUBI    1
                    BNE     printL_char
printL_exit         POP
                    RET
%ENDS   

%SUB                printRight
                    ; prints right sub string pointed to by the accumulator
printRight          PUSH
                    LDWI    printInit
                    CALL    giga_vAC
                    LDW     textStr
                    PEEK                                    ; text length
                    ADDW    textStr
                    SUBW    textLen
                    STW     textStr                         ; text offset
                    LD      textLen
                    BEQ     printR_exit
    
printR_char         ST      textLen
                    INC     textStr                         ; next char
                    LDW     textStr             
                    PEEK
                    ST      textChr
                    LDWI    printChar
                    CALL    giga_vAC

                    LD      textLen
                    SUBI    1
                    BNE     printR_char
printR_exit         POP
                    RET
%ENDS   

%SUB                printMid
                    ; prints sub string pointed to by the accumulator
printMid            PUSH
                    LDWI    printInit
                    CALL    giga_vAC
                    LDW     textStr
                    ADDW    textOfs
                    STW     textStr                         ; textStr += textOfs
                    LD      textLen
                    BEQ     printM_exit
    
printM_char         ST      textLen
                    INC     textStr                         ; next char
                    LDW     textStr             
                    PEEK
                    ST      textChr
                    LDWI    printChar
                    CALL    giga_vAC

                    LD      textLen
                    SUBI    1
                    BNE     printM_char
printM_exit         POP
                    RET
%ENDS   
        
%SUB                printDigit
                    ; prints single digit in textNum
printDigit          PUSH
                    LDW     textNum
printD_index        SUBW    digitMult
                    BLT     printD_cont
                    STW     textNum
                    INC     digitIndex
                    BRA     printD_index
    
printD_cont         LD      digitIndex
                    BEQ     printD_exit
                    ORI     0x30
                    ST      textChr
                    LDWI    printChar
                    CALL    giga_vAC
                    LDI     0x30
                    ST      digitIndex
printD_exit         POP
                    RET
%ENDS   
    
%SUB                printInt16
                    ; prints 16bit int in textNum
printInt16          PUSH
                    LDWI    printInit
                    CALL    giga_vAC
                    LDI     0
                    ST      digitIndex
                    LDW     textNum
                    BGE     printI16_pos
                    LDI     0x2D
                    ST      textChr
                    LDWI    printChar
                    CALL    giga_vAC
                    LDWI    0
                    SUBW    textNum
                    STW     textNum    
    
printI16_pos        LDWI    10000
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
%ENDS

%SUB                printHexByte
                    ; print hex byte in textHex
printHexByte        PUSH
                    LDWI    SYS_LSRW4_50                    ; shift right by 4 SYS routine
                    STW     giga_sysFn
                    LD      textHex
                    SYS     0xF5                            ; SYS_LSRW4_50, 270 - 50/2 = 0xF5
                    SUBI    10
                    BLT     printH_skip0
                    ADDI    7
printH_skip0        ADDI    0x3A
                    ST      textChr
                    LDWI    printInit
                    CALL    giga_vAC
                    LDWI    printChar
                    CALL    giga_vAC
                    LD      textHex
                    ANDI    0x0F
                    SUBI    10
                    BLT     printH_skip1
                    ADDI    7
printH_skip1        ADDI    0x3A
                    ST      textChr
                    LDWI    printChar
                    CALL    giga_vAC
                    POP
                    RET
%ENDS                    
        
%SUB                printHexWord     
                    ; print hex word in textHex
printHexWord        PUSH
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
%ENDS   

%SUB                printChr
                    ; prints char in textChr for standalone calls
printChr            PUSH
                    LDWI    printInit
                    CALL    giga_vAC
                    LDWI    printChar
                    CALL    giga_vAC
                    POP
                    RET
%ENDS

%SUB                printChar
                    ; prints char in textChr
printChar           LD      textChr
                    ANDI    0x7F                            ; char can't be bigger than 127
                    SUBI    82
                    BGE     printC_text82
                    ADDI    50
                    BLT     printC_exit                     ; char<32 exit
                    STW     textChr                         ; char-32
                    LDWI    giga_text32                     ; text font slice base address for chars 32-81
                    BRA     printC_font
                    
printC_text82       STW     textChr                         ; char-82
                    LDWI    giga_text82                     ; text font slice base address for chars 82+
                    
printC_font         STW     textFont
                    LD      textChr
                    LSLW    
                    LSLW    
                    ADDW    textChr
                    ADDW    textFont
                    STW     textFont                        ; char*5 + textFont
                    LDI     0x05

printC_slice        ST      textSlice
                    LDW     textFont                        ; text font slice base address
                    LUP     0x00                            ; get ROM slice
                    ST      giga_sysArg2
                    SYS     0xCB                            ; draw vertical slice, SYS_VDrawBits_134, 270 - 134/2 = 0xCB
                    INC     textFont                        ; next vertical slice
                    INC     giga_sysArg4                    ; next x
                    LD      textSlice
                    SUBI    1
                    BNE     printC_slice
                    
                    ST      giga_sysArg2                    ; result of printC_slice is 0
                    SYS     0xCB                            ; draw last blank slice
                    INC     giga_sysArg4                    ; using sysArg4 as a temporary cursor address for multiple char prints
                    
                    PUSH
                    CALL    realTimeProcAddr
                    LD      cursorXY
                    ADDI    0x06
                    ST      cursorXY
                    SUBI    giga_xres - 5                   ; giga_xres - 6, (154), is last possible char in row
                    BLT     printC_pop
                    LDWI    newLineScroll                   ; next row, scroll at bottom
                    CALL    giga_vAC
                    
printC_pop          POP

printC_exit         RET
%ENDS   
    
%SUB                newLineScroll
                    ; print from top row to bottom row, then start scrolling 
newLineScroll       LDI     0x02                            ; x offset slightly
                    ST      cursorXY
                    ST      giga_sysArg4
                    LDWI    0x0001
                    ANDW    miscFlags
                    BNE     newLS_cont0                     ; scroll on or off
                    RET
                    
newLS_cont0         PUSH
                    LDWI    0x8000
                    ANDW    miscFlags                       ; on bottom row flag
                    BNE     newLS_cont1
                    LD      cursorXY + 1
                    ADDI    8
                    ST      cursorXY + 1
                    SUBI    giga_yres
                    BLT     newLS_exit
                    LDI     giga_yres - 8
                    ST      cursorXY + 1
                    
newLS_cont1         LDWI    clearCursorRow
                    CALL    giga_vAC
                    LDWI    giga_videoTable
                    STW     scanLine
    
newLS_scroll        CALL    realTimeProcAddr
                    LDW     scanLine
                    PEEK
                    ADDI    8
                    ANDI    0x7F
                    SUBI    8
                    BGE     newLS_adjust
                    ADDI    8
                    
newLS_adjust        ADDI    8
                    POKE    scanLine
                    INC     scanLine                        ; scanline pointers are 16bits
                    INC     scanLine
                    LD      scanLine
                    SUBI    0xF0                            ; scanline pointers end at 0x01EE
                    BLT     newLS_scroll
                    
                    LDWI    0x8000
                    ORW     miscFlags
                    STW     miscFlags                       ; set on bottom row flag
                    
newLS_exit          LDWI    printInit
                    CALL    giga_vAC                        ; re-initialise the SYS registers
                    POP
                    RET
%ENDS   

%SUB                atTextCursor
atTextCursor        LD      cursorXY
                    SUBI    giga_xres
                    BLT     atTC_skip0
                    LDI     0
                    ST      cursorXY
                    
atTC_skip0          LD      cursorXY + 1
                    SUBI    giga_yres
                    BLT     atTC_skip1
                    LDI     giga_yres - 1
                    ST      cursorXY + 1
                    
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