                ; draw one tetromino block
setTetrisBlock  LDWI    SYS_Draw4_30        ; setup 4 pixel SYS routine
                STW     giga_sysFn
                LDW     colour
                STW     giga_sysArg0
                STW     giga_sysArg2

                LDI     0x04            ; 4 lines of 4 pixels per block
                ST      jj

                LD      xx              ; x*4
                LSLW
                LSLW
                ADDI    xOffset         ; add origin
                ST      xx

                LD      yy              ; y * 4
                LSLW
                LSLW
                ADDI    yOffset         ; add origin
                ST      yy

setT_loop       LDW     vbase           ; vram address
                ADDW    xx
                STW     giga_sysArg4
                SYS     0xFF            ; SYS_Draw4_30, 270 - 30/2 = 0xFF

                LD      yy              ; yy++
                ADDI    0x01
                ST      yy

                LoopCounter jj setT_loop
                RET


                ; draw complete tetromino
drawTetromino   GetTetrominoBase index rotation
                DEEK                    ; get first 2 bytes, (colour)
                STW     colour

                LDW     tetrominoBase   ; skip w, h, xo, yo
                ADDI    0x05
                STW     tetrominoBase

                LDI     0x04            ; 4 blocks per tetromino
                ST      ii
                
drawT_loop      LDW     tetrominoBase   ; x position
                ADDI    0x01
                STW     tetrominoBase
                PEEK
                ADDW    tx
                ST      xx
                BLT     drawT_skip      ; < 0 skip
                SUBI    xTetris
                BGE     drawT_skip      ; >= xTetris skip

                LDW     tetrominoBase   ; y position
                ADDI    0x01
                STW     tetrominoBase
                PEEK
                ADDW    ty
                SUBW    tv              ; yy = ty - tv, (offset within tetromino)
                ST      yy
                BLT     drawT_skip      ; < 0 skip
                SUBI    yTetris
                BGE     drawT_skip      ; >= yTetris skip

                PUSH                    ; save return address, (this is needed if you nest CALL's)
                CALL    setTetrisBlock
                POP                     ; restore return address

drawT_skip      LoopCounter ii drawT_loop
                RET


drawTetrisField LDWI    0x1515
                STW     colour

                ; top horizontal                
                LDWI    0xFF
                STW     ty
                LDWI    0xFF
                STW     tx
                LDI     xTetris + 2
                ST      ii
                PUSH
                CALL    drawTF_hloop
                POP

                ; bottom horizontal                 
                LDWI    0x14
                STW     ty
                LDWI    0xFF
                STW     tx
                LDI     xTetris + 2
                ST      ii
                PUSH
                CALL    drawTF_hloop
                POP

                ; left vertical
                LDWI    0xFF
                STW     ty
                LDWI    0xFF
                STW     tx
                LDI     yTetris + 1
                ST      ii
                PUSH
                CALL    drawTF_vloop
                POP

                ; right vertical
                LDWI    0xFF
                STW     ty
                LDWI    xTetris
                STW     tx
                LDI     yTetris + 1
                ST      ii
                PUSH
                CALL    drawTF_vloop
                POP
                RET

drawTF_hloop    LDW     ty
                ST      yy
                LDW     tx
                ST      xx
                PUSH
                CALL    setTetrisBlock
                POP
                INC     tx
                LoopCounter ii drawTF_hloop
                RET

drawTF_vloop    LDW     ty
                ST      yy
                LDW     tx
                ST      xx
                PUSH
                CALL    setTetrisBlock
                POP
                INC     ty
                LoopCounter ii drawTF_vloop
                RET
