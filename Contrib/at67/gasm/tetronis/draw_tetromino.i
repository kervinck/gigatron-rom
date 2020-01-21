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
                SYS     30

                PUSH
                CALL    playMidi
                POP
                
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


                ; draw next tetromino
drawNextTet     GetTetrominoBase indexNext rotationNext
                DEEK                    ; get first 2 bytes, (colour)
                STW     colour

                LDW     tetrominoBase   ; get w, h
                ADDI    0x02
                DEEK
                STW     kk

                LDW     tetrominoBase   ; get ox, oy
                ADDI    0x04
                DEEK
                STW     mm

                LDW     tetrominoBase   ; skip to tetromino offsets
                ADDI    0x05
                STW     tetrominoBase

                LDI     0x04            ; 4 pixels per tetromino
                ST      ii
                
drawN_loop      LDWI    giga_vram + xOffset + xPixels-3 + (yOffset-5)*256
                STW     xx

                PUSH
                CALL    drawNextAdjust
                POP

drawN_skip      LD      xx
                STW     scratch
                LDW     tetrominoBase   ; x position
                ADDI    0x01
                STW     tetrominoBase
                PEEK
                ADDW    scratch
                ST      xx

                LD      yy
                STW     scratch
                LDW     tetrominoBase   ; y position
                ADDI    0x01
                STW     tetrominoBase
                PEEK
                ADDW    scratch
                ST      yy

                LDW     colour
                POKE    xx

                PUSH
                CALL    playMidi
                POP
                
                LoopCounter ii drawN_loop
                RET


drawTetrisField LDWI    bgColourW
                STW     colour

                ; top horizontal      
                PUSH
                CALL    drawTFtopH
                POP
                LDWI    0xFF
                STW     ty
                LDWI    0xFF
                STW     tx
                LDI     xTetris + 2
                ST      ii
                PUSH
                CALL    drawTFhoriz
                POP

                ; bottom horizontal                 
                LDWI    0x14
                STW     ty
                LDWI    0xFF
                STW     tx
                LDI     xTetris + 2
                ST      ii
                PUSH
                CALL    drawTFhoriz
                POP

                PUSH
                CALL    drawTFbotH
                POP

                ; left vertical
                LDWI    0xFF
                STW     ty
                LDWI    0xFF
                STW     tx
                LDI     yTetris + 1
                ST      ii
                PUSH
                CALL    drawTFvert
                POP

                ; right vertical
                LDWI    0xFF
                STW     ty
                LDWI    xTetris
                STW     tx
                LDI     yTetris + 1
                ST      ii
                PUSH
                CALL    drawTFvert
                POP
                RET


drawTFtopH      LDWI    giga_vram + xOffset - 4 + (yOffset-7)*256
                STW     xx
                LDWI    giga_vram + xOffset - 4 + (yOffset-6)*256
                STW     kk
                LDWI    giga_vram + xOffset - 4 + (yOffset-5)*256
                STW     mm

                LDI     xPixels + 8
                ST      ii

drawTF_tl       LDI     bgColourB
                POKE    xx
                POKE    kk
                POKE    mm
                INC     xx
                INC     kk
                INC     mm
                LoopCounter ii drawTF_tl
                RET


drawTFhoriz     LDW     ty
                ST      yy
                LDW     tx
                ST      xx
                PUSH
                CALL    setTetrisBlock
                POP
                INC     tx
                LoopCounter ii drawTFhoriz
                RET


drawTFvert      LDW     ty
                ST      yy
                LDW     tx
                ST      xx
                PUSH
                CALL    setTetrisBlock
                POP
                INC     ty
                LoopCounter ii drawTFvert
                RET


drawTFbotH      LDWI    giga_vram + xOffset - 4 + (yOffset+yPixels + 4)*256
                STW     xx
                LDWI    giga_vram + xOffset - 4 + (yOffset+yPixels + 5)*256
                STW     kk
                LDWI    giga_vram + xOffset - 4 + (yOffset+yPixels + 6)*256
                STW     mm

                LDI     xPixels + 8
                ST      ii

drawTF_bl       LDI     bgColourB
                POKE    xx
                POKE    kk
                POKE    mm
                INC     xx
                INC     kk
                INC     mm
                LoopCounter ii drawTF_bl
                RET


drawNextAdjust  LDW     kk              ; widthheight - oxoy
                SUBW    mm
                STW     scratch

                LD      scratch         ; if width == 2 xx++
                XORI    0x02
                BNE     drawNA_height
                INC     xx

drawNA_height   LD      scratch + 1     ; if height == 2 yy++
                XORI    0x02
                BNE     drawNA_skip
                INC     yy

drawNA_skip     RET