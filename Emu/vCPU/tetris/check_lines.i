                ; checks the tetris playfield for lines
checkLines      LDI     maxLines - 1
                ST      ii
                LDWI    0x00
                STW     numLines
                
ii_loop         LDI     yTetris - 1
                ST      ll

ll_loop         LDI     xTetris - 1
                ST      kk

                ; check for solid line
kk_loop0        LDW     kk
                STW     xx
                PUSH                    ; save return address, (this is needed if you nest CALL's)
                CALL    getTetrisBlock  ; check for occupied block
                POP                     ; restore return address
               
                LDW     result
                BEQ     skip_kk
                LoopCounter1 kk kk_loop0

                ; found a line
                INC     numLines
                PUSH
                CALL    moveLines
                POP
       
skip_kk         LoopCounter1 ll ll_loop
                LoopCounter1 ii ii_loop

                LDI     0x00
                POKE    xScroll
                RET


                ; copy previous line to current line
moveLines       LD      ll
                ST      nn
nn_loop         LDI     xTetris - 1
                ST      mm
mm_loop         LD      nn              ; get block colour
                SUBI    0x01
                ST      yy
                LD      mm
                ST      xx
                PUSH
                CALL    getTetrisBlock
                POP
            
                LD      result          ; set block colour
                ST      colour
                ST      colour + 1

                LDW     mm
                STW     xx
                PUSH                    ; save return address, (this is needed if you nest CALL's)
                CALL    setTetrisBlock
                POP                     ; restore return address
                
                LoopCounter1 mm mm_loop

                PUSH
                CALL    shakeScreen
                POP

                LD      nn
                SUBI    0x01
                ST      nn
                ;XORI    0x01
                BNE     nn_loop

                ; erase top line
                LDI     xTetris - 1
                ST      kk
kk_loop1        LDI     0x00
                ST      yy
                LD      kk
                ST      xx

                LDWI    0x0000
                STW     colour

                PUSH                    ; save return address, (this is needed if you nest CALL's)
                CALL    setTetrisBlock
                POP                     ; restore return address

                LoopCounter1 kk kk_loop1
                RET


shakeScreen     LD      giga_rand0
                ANDI    0x03
                BEQ     shake_left
                XORI    0x01
                BEQ     shake_right

                LDI     0x00
                POKE    xScroll
                RET

shake_right     LDI     0x01
                POKE    xScroll
                RET

shake_left      LDI     0xFF
                POKE    xScroll
                RET
