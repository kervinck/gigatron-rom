                ; clears the viewable screen
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
                LUP     0x00
                ST      giga_sysArg2        ; draw slice
                SYS     0xCB
                INC     scratch             ; next slice
                INC     textPos             ; next x
                LoopCounter ii printT_slice
    
                INC     textStr             ; next char
                INC     textPos             ; 1 pixel space between chars
                LoopCounter tt printT_chr
                RET