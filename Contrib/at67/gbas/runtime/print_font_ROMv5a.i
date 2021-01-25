; do *NOT* use register4 to register7 during time slicing
textStr             EQU     register0
textNum             EQU     register0
textBak             EQU     register0
textHex             EQU     register1
textSpc             EQU     register1
textLen             EQU     register2
textOfs             EQU     register3
textChr             EQU     register8
scanLine            EQU     register11
digitMult           EQU     register12
digitIndex          EQU     register13
clearLoop           EQU     register14
fontId              EQU     register9
fontAddrs           EQU     register10
fontBase            EQU     register11
fontPosXY           EQU     register15


%SUB                clearCursorRow
                    ; clears the top giga_yfont lines of pixels in preparation of text scrolling
clearCursorRow      PUSH
                    LDWI    SYS_SetMemory_v2_54
                    STW     giga_sysFn                      ; setup fill memory SYS routine
                    LD      fgbgColour
                    ST      giga_sysArg1                    ; fill value
                    LDWI    giga_videoTable
                    PEEK
                    ST      giga_sysArg3                    ; row0 high byte address
                    LDI     giga_yfont

clearCR_loopy       ST      clearLoop                    
                    LDI     giga_xres
                    ST      giga_sysArg0
                    LDI     0
                    ST      giga_sysArg2                    ; low start address
                    SYS     54                              ; fill memory
                    INC     giga_sysArg3                    ; next line
                    LD      clearLoop
                    SUBI    1
                    BNE     clearCR_loopy
                    CALLI   printInit                       ; re-initialise the SYS registers
                    POP
                    RET
%ENDS

%SUB                printInit
printInit           LDWI    SYS_Sprite6_v3_64
                    STW     giga_sysFn
                    LD      cursorXY + 1                    ; xy = peek(256+2*y)*256 + x
                    LSLW
                    INC     giga_vAC + 1
                    PEEK
                    ST      fontPosXY + 1
                    LD      cursorXY
                    ST      fontPosXY                       ; xy position
                    RET
%ENDS

%SUB                printText
                    ; prints text string pointed to by textStr
printText           PUSH
                    STW     textStr
                    CALLI   printInit

                    ; first byte is length
printT_char         INC     textStr                         ; next char
                    LDW     textStr             
                    PEEK
                    BEQ     printT_exit                     ; check for delimiting zero
                    ST      textChr
                    CALLI   printChar
                    BRA     printT_char
                    
printT_exit         POP
                    RET
%ENDS   

%SUB                printLeft
                    ; prints left sub string pointed to by the accumulator
printLeft           PUSH
                    CALLI   printInit
                    LD      textLen
                    BEQ     printL_exit
    
printL_char         ST      textLen
                    INC     textStr                         ; next char
                    LDW     textStr             
                    PEEK
                    ST      textChr
                    CALLI   printChar

                    LD      textLen
                    SUBI    1
                    BNE     printL_char
printL_exit         POP
                    RET
%ENDS   

%SUB                printRight
                    ; prints right sub string pointed to by the accumulator
printRight          PUSH
                    CALLI   printInit
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
                    CALLI   printChar

                    LD      textLen
                    SUBI    1
                    BNE     printR_char
printR_exit         POP
                    RET
%ENDS   

%SUB                printMid
                    ; prints sub string pointed to by the accumulator
printMid            PUSH
                    CALLI   printInit
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
                    CALLI   printChar

                    LD      textLen
                    SUBI    1
                    BNE     printM_char
printM_exit         POP
                    RET
%ENDS   

%SUB                printLower
                    ; prints lower case version of textStr
printLower          PUSH
                    CALLI   printInit
    
printLo_next        INC     textStr                         ; next char, (skips length byte)
                    LDW     textStr
                    PEEK
                    BEQ     printLo_exit
                    ST      textChr
                    SUBI    65
                    BLT     printLo_char
                    LD      textChr
                    SUBI    90
                    BGT     printLo_char
                    LD      textChr                         ; >= 65 'A' and <= 90 'Z'
                    ADDI    32
                    ST      textChr
                    
printLo_char        CALLI   printChar
                    BRA     printLo_next
                    
printLo_exit        POP
                    RET
%ENDS

%SUB                printUpper
                    ; prints upper case version of textStr
printUpper          PUSH
                    CALLI   printInit
    
printUp_next        INC     textStr                         ; next char, (skips length byte)
                    LDW     textStr
                    PEEK
                    BEQ     printUp_exit
                    ST      textChr
                    SUBI    97
                    BLT     printUp_char
                    LD      textChr
                    SUBI    122
                    BGT     printUp_char
                    LD      textChr                         ; >= 97 'a' and <= 122 'z'
                    SUBI    32
                    ST      textChr
                    
printUp_char        CALLI   printChar
                    BRA     printUp_next
                    
printUp_exit        POP
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
                    ST      textChr
                    CALLI   printChar
                    LDI     0x30
                    ST      digitIndex
printD_exit         POP
                    RET
%ENDS   
    
%SUB                printInt16
                    ; prints 16bit int in textNum
printInt16          PUSH
                    STW     textNum
                    CALLI   printInit
                    LDI     0
                    ST      digitIndex
                    LDW     textNum
                    BGE     printI16_pos
                    LDI     0x2D
                    ST      textChr
                    CALLI   printChar
                    LDI     0
                    SUBW    textNum
                    STW     textNum    
    
printI16_pos        LDWI    10000
                    STW     digitMult
                    CALLI   printDigit
                    LDWI    1000
                    STW     digitMult
                    CALLI   printDigit
                    LDI     100
                    STW     digitMult
                    CALLI   printDigit
                    LDI     10
                    STW     digitMult
                    CALLI   printDigit
                    LD      textNum
                    ORI     0x30
                    ST      textChr
                    CALLI   printChar
                    POP
                    RET
%ENDS

%SUB                printChr
                    ; prints char in textChr for standalone calls
printChr            PUSH
                    ST      textChr
                    CALLI   printInit
                    CALLI   printChar
                    POP
                    RET
%ENDS

%SUB                printSpc
                    ; prints textSpc spaces
printSpc            PUSH
                    BEQ     printS_exit
                    ST      textSpc
                    CALLI   printInit
                    
printS_loop         LDI     32
                    STW     textChr
                    CALLI   printChar
                    LD      textSpc
                    SUBI    1
                    ST      textSpc
                    BNE     printS_loop
                    
printS_exit         POP
                    RET
%ENDS

%SUB                printHex
                    ; print textLen hex digits in textHex, (textStr, textHex, textLen = strAddr, strHex, strLen in string::stringHex)
printHex            PUSH
                    LDWI    textWorkArea
                    STW     strAddr
                    CALLI   stringHex
                    LDW     strAddr
                    CALLI   printText
                    POP
                    RET
%ENDS

%SUB                printChar
                    ; prints char in textChr
printChar           LD      textChr
                    ANDI    0x7F                            ; char can't be bigger than 127
                    SUBI    32
                    BLT     printC_exit
                    STW     textChr                         ; char-32                    
                    LDWI    _fontId_
                    PEEK
                    STW     fontId
                    LDWI    _fontsLut_                      ; fonts table
                    ADDW    fontId
                    ADDW    fontId
                    DEEK
                    STW     fontAddrs                       ; get font address table
                    INC     fontAddrs
                    INC     fontAddrs
                    DEEK                                    ; get font mapping table
                    BEQ     printC_noMap                    ; no mapping table means font contains all chars 32 -> 127 in the correct order
                    ADDW    textChr
                    PEEK
                    STW     textChr                         ; get mapped char
                    
printC_noMap        LDW     fontAddrs
                    DEEK
                    STW     fontBase                        ; baseline address, shared by all chars in a font
                    INC     fontAddrs
                    INC     fontAddrs
                    LDW     textChr
                    LSLW
                    ADDW    fontAddrs
                    DEEK                                    ; get char address
                    STW     giga_sysArg0
                    LDW     fontPosXY                       ; XY pos generated in printInit
                    SYS     64                              ; draw char
                    STW     fontPosXY
                    
                    LDW     fontBase
                    STW     giga_sysArg0
                    LDWI    0x0F00
                    ADDW    cursorXY
                    SYS     64                              ; draw baseline for char
                    
                    PUSH
                    CALLI   printClip
                    POP
                    
printC_exit         RET
%ENDS

%SUB                printClip
printClip           LD      cursorXY
                    ADDI    giga_xfont
                    ST      cursorXY
                    SUBI    giga_xres - giga_xfont          ; last possible char on line
                    BLE     printCl_exit
                    LDI     DISABLE_CLIP_BIT
                    ANDW    miscFlags                       ; is text clipping disabled?
                    BNE     printCl_exit
                    PUSH
                    CALLI   newLineScroll                   ; next row, scroll at bottom
                    POP
                    
printCl_exit        RET
%ENDS

%SUB                newLineScroll
                    ; print from top row to bottom row, then start scrolling 
newLineScroll       LDI     giga_CursorX                    ; cursor x start
                    ST      cursorXY
                    ST      fontPosXY
                    LDI     ENABLE_SCROLL_BIT
                    ANDW    miscFlags
                    BNE     newLS_cont0                     ; is scroll on or off?
                    RET
                    
newLS_cont0         PUSH
                    LDI     ON_BOTTOM_ROW_BIT
                    ANDW    miscFlags                       ; is on bottom row flag?
                    BNE     newLS_cont1
                    LD      cursorXY + 1
                    ADDI    giga_yfont
                    ST      cursorXY + 1
                    SUBI    giga_yres
                    BLT     newLS_exit
                    LDI     giga_yres - giga_yfont
                    ST      cursorXY + 1
                    
newLS_cont1         CALLI   clearCursorRow
                    LDWI    giga_videoTable
                    STW     scanLine
    
newLS_scroll        LDW     scanLine
                    PEEK
                    ADDI    giga_yfont
                    ANDI    0x7F
                    SUBI    giga_yfont
                    BGE     newLS_adjust
                    ADDI    giga_yfont
                    
newLS_adjust        ADDI    giga_yfont
                    POKE    scanLine
                    INC     scanLine                        ; scanline pointers are 16bits
                    INC     scanLine
                    LD      scanLine
                    SUBI    0xF0                            ; scanline pointers end at 0x01EE
                    BLT     newLS_scroll
                    
                    LDI     ON_BOTTOM_ROW_BIT
                    ORW     miscFlags
                    STW     miscFlags                       ; set on bottom row flag
                    
newLS_exit          CALLI   printInit                       ; re-initialise the SYS registers
                    POP
                    RET
%ENDS   

%SUB                atTextCursor
atTextCursor        LD      cursorXY
                    SUBI    giga_xres - giga_xfont
                    BLE     atTC_checkY
                    LDI     0
                    ST      cursorXY
                    
atTC_checkY         LD      cursorXY + 1
                    SUBI    giga_yres - giga_yfont
                    BLT     atTC_resbot
                    LDI     giga_yres - giga_yfont
                    ST      cursorXY + 1
                    LDI     ON_BOTTOM_ROW_BIT
                    ORW     miscFlags
                    STW     miscFlags                       ; set on bottom row flag
                    RET
                    
atTC_resbot         LDWI    ON_BOTTOM_ROW_MSK
                    ANDW    miscFlags
                    STW     miscFlags                       ; reset on bottom row flag
                    RET
%ENDS
