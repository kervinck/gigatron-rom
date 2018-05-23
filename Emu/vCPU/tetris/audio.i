resetChannels   LDWI    giga_soundChan1
                STW     scratch
                LDI     0x04
                ST      ii

resetC_loop     LDI     giga_soundChan1     ; reset low byte
                ST      scratch
                LDW     scratch
                POKE    0x00                ; wavA
                INC     scratch
                POKE    0x03                ; wavX
                INC     scratch
                POKE    0x00                ; keyL
                INC     scratch
                POKE    0x00                ; keyH
                INC     scratch
                POKE    0x00                ; oscL
                INC     scratch
                POKE    0x00                ; oscH
                LDWI    0x0100              ; increment high byte
                ADDW    scratch
                STW     scratch
                LoopCounter ii resetC_loop
                RET
