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