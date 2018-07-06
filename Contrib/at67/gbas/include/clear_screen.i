xcount          EQU     register0
ycount          EQU     register1
topline         EQU     register2
botline         EQU     register3


                ; clears the viewable screen
clearScreen     STW     giga_sysArg0    ; 4 pixels of colour
                STW     giga_sysArg2

                LDWI    SYS_Draw4_30    ; setup 4 pixel SYS routine
                STW     giga_sysFn

                LDI     giga_yres / 2   ; counters
                ST      ycount
                LDI     giga_xres / 4
                ST      xcount

                LDWI    0x0800          ; top line
                STW     topline
                LDWI    0x7F00          ; bottom line
                STW     botline

clearS_loop     LDW     topline
                STW     giga_sysArg4    ; top line
                SYS     0xFF            ; SYS_Draw4_30, 270 - 30/2 = 0xFF

                LDW     botline
                STW     giga_sysArg4    ; bottom line
                SYS     0xFF            ; SYS_Draw4_30, 270 - 30/2 = 0xFF

                LD      topline         ; 4 horizontal pixels
                ADDI    0x04
                ST      topline
                LD      botline         ; 4 horizontal pixels
                ADDI    0x04
                ST      botline
                LoopCounter xcount clearS_loop

                INC     topline + 1     ; next top line
                LD      botline + 1     ; next bottom line
                SUBI    0x01
                ST      botline + 1

                LDI     0x00            ; reset topline, botline and xcount
                ST      topline
                ST      botline
                LDI     giga_xres / 4
                ST      xcount
                LoopCounter ycount clearS_loop
                RET
