                ; prints text using the inbuilt font and SYS routine
printText       LDWI    SYS_VDrawBits_134   ; setup 8 vertical pixel SYS routine
                STW     giga_sysFn

                LDI     0x00                ; background colour
                ST      giga_sysArg0
                LDI     0x3F                ; foreground colour
                ST      giga_sysArg1

                LDW     textStr             ; first byte is length
                PEEK
                ST      tt
                INC     textStr

printT_chr      LDW     textStr             
                PEEK
                SUBI    32                  ; (char - 32)*5 + 0x0700
                STW     textChr
                STW     scratch
                LSLW    scratch
                LSLW    scratch
                ADDW    textChr
                ADDW    tbase
                STW     scratch             ; text font slice base address for chars 32-81

                LDW     textChr
                SUBI    50
                BLT     printT_draw
                LDW     scratch
                ADDI    0x06
                STW     scratch             ; text font slice base address for chars 82+

printT_draw     LDI     0x05
                ST      ii
printT_slice    LDW     textPos
                STW     giga_sysArg4        ; xy
                LDW     scratch             ; text font slice base address
                LUP     0x00                ; get ROM slice
                ST      giga_sysArg2        
                SYS     134
                INC     scratch             ; next vertical slice
                INC     textPos             ; next x
                LoopCounter ii printT_slice
    
                INC     textStr             ; next char
                INC     textPos             ; 1 pixel space between chars
                LoopCounter tt printT_chr
                RET


                ; prints digits using a tiny 3x5 font
printDigits     LDWI    SYS_Draw4_30        ; setup 4 pixel SYS routine
                STW     giga_sysFn

                LDW     textStr             ; first byte is length
                PEEK
                ST      tt
                INC     textStr

printD_chr      LDW     textStr             
                PEEK
                SUBI    48                  ; (char - 48)*4 + digit_font0
                STW     scratch
                LSLW    scratch
                LSLW    scratch
                ADDW    dbase
                STW     scratch
                LDI     0x05
                ST      ii

printD_slice    LDW     scratch
                DEEK
                STW     giga_sysArg0        ; first 2 pixels from digit font LUT
                LDW     scratch
                ADDI    0x02
                PEEK
                ST      giga_sysArg2        ; third pixel from digit font LUT
                LDI     bgColourB
                ST      giga_sysArg3        ; fourth pixel is background colour
                LDW     textPos
                STW     giga_sysArg4        ; xy
                SYS     30

                PUSH
                CALL    playMidi
                POP

                INC     scratch + 1         ; next horizontal slice
                INC     textPos + 1         ; next y
                LoopCounter ii printD_slice

                INC     textStr             ; next char
                LD      textPos             ; x += 4
                ADDI    0x04
                ST      textPos
                LD      textPos + 1         ; y -= 5
                SUBI    0x05
                ST      textPos + 1
                LoopCounter tt printD_chr
                RET

