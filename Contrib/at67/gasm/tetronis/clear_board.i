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
                SYS     30

                LD      xx              ; 4 horizontal pixels
                ADDI    0x04
                ST      xx

                LoopCounter ii clearB_loop

                LD      yy              ; next line
                ADDI    0x01
                ST      yy
                LDI     xOffset
                ST      xx

                PUSH
                CALL    playMidi
                POP

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

                LDI     giga_yres / 2   ; counters
                ST      jj
                LDI     giga_xres / 4
                ST      ii

                LDWI    0x0800          ; top line
                STW     xx
                LDWI    0x7F00          ; bottom line
                STW     kk
                PUSH                    ; save clearScreen's caller return address
                CALL    clearScreen_0   ; jump to clearScreen_0

                ; clear left and right edge single pixel vertical strips, (for screen shake)
clearStrips     STW     scratch
                LDI     0x00
                POKE    scratch
                LDW     scratch
                ADDI    0x5F    
                STW     scratch
                LDI     0x00
                POKE    scratch
                RET

clearScreen_0   POP                     ; restore clearScreen's caller return address
clearS_loop     LDW     xx
                STW     giga_sysArg4    ; top line
                SYS     30

                LDW     kk
                STW     giga_sysArg4    ; bottom line
                SYS     30

                LD      xx              ; 4 horizontal pixels
                ADDI    0x04
                ST      xx
                LD      kk              ; 4 horizontal pixels
                ADDI    0x04
                ST      kk
                LoopCounter ii clearS_loop

                ; clear one extra pixel wide stripes for screen shake
                PUSH                    ; top
                LDW     xx
                CALL    clearStrips
                POP
                PUSH                    ; bottom
                LDW     kk
                CALL    clearStrips
                POP

                INC     yy              ; next top line
                LD      ll              ; next bottom line
                SUBI    0x01
                ST      ll

                LDI     0x00            ; reset xx, kk and ii
                ST      xx
                ST      kk
                LDI     giga_xres / 4
                ST      ii
                LoopCounter jj clearS_loop
                RET
