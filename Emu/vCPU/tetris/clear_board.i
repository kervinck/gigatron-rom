                ; clears the tetris playfield
clearBoard      LDWI    SYS_Draw4_30    ; setup 4 pixel SYS routine
                STW     giga_sysFn

                LDWI    0x0000          ; 4 pixels of colour
                STW     giga_sysArg0
                STW     giga_sysArg2

                LDI     yOffset         ; origin
                ST      yy
                LDI     xOffset
                ST      xx

                LDI     yTetris*4       ; counters
                ST      jj
                LDI     xTetris
                ST      ii

clearB_loop     LDW     vbase           ; vram address
                ADDW    xx
                STW     giga_sysArg4
                SYS     0xFF            ; SYS_Draw4_30, 270 - 30/2 = 0xFF

                LD      xx              ; 4 horizontal pixels
                ADDI    0x04
                ST      xx

                LoopCounter ii clearB_loop

                LD      yy              ; next line
                ADDI    0x01
                ST      yy
                LDI     xOffset
                ST      xx

                LDI     xTetris
                ST      ii
                LoopCounter jj clearB_loop
                RET


                ; clears the viewable screen
clearScreen     LDWI    SYS_Draw4_30    ; setup 4 pixel SYS routine
                STW     giga_sysFn

                LDWI    0x0000          ; 4 pixels of colour
                STW     giga_sysArg0
                STW     giga_sysArg2

                LDI     giga_yres       ; counters
                ST      jj
                LDI     giga_xres / 4
                ST      ii

                LDWI    0x0000
                STW     xx

clearS_loop     LDW     vbase           ; vram address
                ADDW    xx
                STW     giga_sysArg4
                SYS     0xFF            ; SYS_Draw4_30, 270 - 30/2 = 0xFF

                LD      xx              ; 4 horizontal pixels
                ADDI    0x04
                ST      xx

                LoopCounter ii clearS_loop

                ; clear left and right edge single pixel vertical strips
                LDW     vbase
                ADDW    xx
                STW     scratch
                LDI     0x00
                POKE    scratch
                LDW     scratch
                ADDI    0x5F    
                STW     scratch
                LDI     0x00
                POKE    scratch

                INC     yy              ; next line
                LDI     0x00
                ST      xx
                LDI     giga_xres / 4
                ST      ii
                LoopCounter jj clearS_loop
                RET