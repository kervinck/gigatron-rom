                ; erase complete tetromino
eraseTetromino  GetTetrominoBase oindex orotation        
                LDWI    0x0000
                STW     colour

                LDW     tetrominoBase   ; skip w, h, xo, yo
                ADDI    0x05
                STW     tetrominoBase

                LDI     0x04            ; 4 blocks per tetromino
                ST      ii
                
eraseT_loop     LDW     tetrominoBase   ; x position
                ADDI    0x01
                STW     tetrominoBase
                PEEK
                ADDW    ox
                ST      xx
                BLT     eraseT_skip      ; < 0 skip
                SUBI    xTetris
                BGE     eraseT_skip      ; >= xTetris skip

                LDW     tetrominoBase   ; y position
                ADDI    0x01
                STW     tetrominoBase
                PEEK
                ADDW    oy
                SUBW    ov              ; yy = oy - ov, (offset within tetromino)
                ST      yy
                BLT     eraseT_skip      ; < 0 skip
                SUBI    yTetris
                BGE     eraseT_skip      ; >= yTetris skip

                PUSH                    ; save return address, (this is needed if you nest CALL's)
                CALL    setTetrisBlock
                POP                     ; restore return address

eraseT_skip     LoopCounter ii eraseT_loop
                RET


                ; erase top right corner
eraseTrCorner   LDWI    SYS_Draw4_30    ; setup 4 pixel SYS routine
                STW     giga_sysFn
                LDWI    bgColourW
                STW     giga_sysArg0
                STW     giga_sysArg2

                LDI     6
                ST      ii
                LDI     xOffset + xPixels - 2
                ST      giga_sysArg4    
                LDI     (yOffset-6) + giga_vram/256
                ST      giga_sysArg4 + 1

eraseTr_loop    SYS     30
                LDW     giga_sysArg4
                SUBI    0x01
                STW     scratch
                LDI     bgColourB
                POKE    scratch
                LDW     scratch
                SUBI    0x01
                STW     scratch
                LDI     bgColourB
                POKE    scratch
                LD      giga_sysArg4 + 1
                ADDI    0x01
                ST      giga_sysArg4 + 1
                LoopCounter ii eraseTr_loop

                RET
