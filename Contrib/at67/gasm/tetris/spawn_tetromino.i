                ; spawn new tetromino
spawnTetromino  LD      tetrominoNext   ; index points to 2byte entries so *2
                ANDI    0x07
                LSLW
                STW     index

                LD      tetrominoNext   ; rotation points to 16byte entries so *16
                ANDI    0x30
                STW     rotation
                PUSH
                CALL    updateTetromino
                POP

                LDWI    0xFF            ; y = -1
                STW     ty

                ; this will break if you modify TETRIS_XEXT, (xTetris)
                LDWI    xTetris         ; approximates x = rand() % (TETRIS_XEXT - w - 1) - u;
                SUBW    tw
                SUBI    0x01
                STW     scratch
                LD      giga_rand1           ; rand = {1..8}
                ANDI    0x07
                ADDI    0x01
                STW     rand
                SUBW    scratch
                STW     scratch         ; scratch = rand - (TETRIS_XEXT - w - 1)
                BLE     spawnT_tx       ; if(rand <= (TETRIS_XEXT - w - 1)) goto spawnT_tx

                LDW     rand            ; rand = rand - scratch
                SUBW    scratch
                STW     rand
spawnT_tx       LDW     rand            ; tx = rand - tu
                SUBW    tu
                STW     tx

                PUSH
                CALL    checkLines
                POP

                PUSH
                CALL    eraseTrCorner
                POP
                PUSH
                CALL    nextTetromino
                POP
                PUSH
                CALL    drawNextTet 
                POP

                LD      numLines
                BNE     spawnT_score
                RET

spawnT_score    PUSH
                CALL    updateScore
                POP
                PUSH
                CALL    updateHighScore
                POP
                RET


updateTetromino GetTetrominoBase index rotation
                GetTetrominoData 0x02 tw
                GetTetrominoData 0x03 th
                GetTetrominoData 0x04 tu
                GetTetrominoData 0x05 tv
                RET

nextTetromino   LD      giga_rand1      ; setup next tetromino
                ST      tetrominoNext
                ANDI    0x07
                LSLW
                STW     indexNext

                LD      tetrominoNext   ; rotation points to 16byte entries so *16
                ANDI    0x30
                STW     rotationNext
                RET
