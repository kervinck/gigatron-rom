                ; get tetromino block
getTetrisBlock  LD      xx              ; xx * 4
                LSLW
                LSLW
                ADDI    xOffset         ; add origin
                ST      xx

                LD      yy              ; yy * 4
                LSLW
                LSLW
                ADDI    yOffset         ; add origin
                ST      yy

                LDW     vbase           ; vram address
                ADDW    xx
                PEEK
                STW     result          ; return pixel

                PUSH
                CALL    playMidi
                POP

                RET


                ; checks a complete tetromino, result returns with 0x00 if no blocks are occupied or 0xFF if any blocks are occupied
checkTetromino  GetTetrominoBase index rotation
                ADDI    0x05            ; skip w, h, xo, yo
                STW     tetrominoBase

                LDWI    0x00            ; reset result
                STW     result          

                LDI     0x04
                ST      ii
                
checkT_loop     LDW     tetrominoBase   ; x position
                ADDI    0x01
                STW     tetrominoBase
                PEEK
                ADDW    tx
                ST      xx
                BLT     checkT_skip     ; < 0 skip
                SUBI    xTetris
                BGE     checkT_skip     ; >= xTetris skip

                LDW     tetrominoBase   ; y position
                ADDI    0x01
                STW     tetrominoBase
                PEEK
                ADDW    ty
                SUBW    tv              ; yy = ty - tv, (offset within tetromino)
                ST      yy
                BLT     checkT_skip     ; < 0 skip
                SUBI    yTetris
                BGE     checkT_skip     ; >= yTetris skip

                PUSH                    ; save return address, (this is needed if you nest CALL's)
                CALL    getTetrisBlock
                POP                     ; restore return address

                LDW     result          ; check result, continue if empty
                BEQ     checkT_skip

                LDW     ty              ; blocked, check for game over, if(ty - tv == 0)
                SUBW    tv
                BNE     checkT_blocked

                LDWI    0x01            ; game over returns 1, (blocks can never be equal to 1)
                STW     result
checkT_blocked  RET

checkT_skip     LoopCounter ii checkT_loop
                RET
