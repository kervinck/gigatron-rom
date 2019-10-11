                ; update score and level
updateScore     LDWI    scoringLut          ; get score for number of lines
                STW     scratch
                LDW     numLines
                SUBI    0x01
                ADDW    scratch
                PEEK   
                STW     scoreScratch        ; scoringLut[numLines - 1]

                LD      scoreLevel          ;  scoreDelta += scoringLut[numLines - 1] * (scoreLevel + 1)
                ADDI    0x01
                ST      ii
                LDWI    0x0000
                STW     scratch
updateS_delta   LDW     scratch        
                ADDW    scoreScratch
                STW     scratch        
                LoopCounter ii updateS_delta
                LDW     scratch        
                STW     scoreScratch        
                ADDW    scoreDelta
                STW     scoreDelta

                LDWI    levellingLut        ; check for level increase
                STW     scratch
                LDW     scoreLevel
                LSLW                        ; points to words
                ADDW    scratch
                DEEK
                SUBW    scoreDelta          ; if(levellingLut[scoreLevel] > scoreDelta) goto updateS_score
                BGT     updateS_score
                
                STW     scoreDelta          ; -ve remainder
                LDWI    0x0000
                SUBW    scoreDelta          ; invert
                STW     scoreDelta          

                LD      scoreLevel          ; if(scoreLevel == maxLevel) goto updateS_score
                XORI    maxLevel
                BEQ     updateS_score
     
                LDW     frameTicksLevel     ; speed up the game per level
                SUBI    deltaTicks
                STW     frameTicksLevel
                STW     frameTicks

                INC     scoreLevel          ; level up

                gprintf("Level %d : %d %d %s", *scoreLevel, *scoreScratch, *scoreDelta, *score_string)

                PUSH                        ; increment level and multiplier strings
                CALL    incrementLevel
                POP
                LDWI    level_string        ; print level string
                STW     textStr
                LDWI    levelPos
                STW     textPos
                PUSH
                CALL    printDigits
                POP
                LDWI    mult_string        ; print mult string
                STW     textStr
                LDWI    multPos
                STW     textPos
                PUSH
                CALL    printDigits
                POP

                ; increment score until it reaches (score + scoreDelta)
updateS_score   PUSH                        ; increment score string
                CALL    incrementScore      
                POP
                LDWI    score_string        ; print score string
                STW     textStr
                LDWI    scorePos
                STW     textPos

                PUSH
                CALL    printDigits
                POP

                LDW     scoreScratch        ; increment score scoreScratch/5 times, (score is a multiple of 5)
                SUBI    0x05
                STW     scoreScratch
                BGT     updateS_score

                gprintf("%d %d %s", *scoreScratch, *scoreDelta, *score_string)
                RET


                ; resets 6 digit score string and updates high score
updateHighScore LDWI    high_string + 1     ; starting at most significant digit
                STW     scoreScratch
                LDWI    score_string + 1
                STW     scratch
                LDI     0x06
                ST      ii

updateHS_loop0  LDW     scratch
                PEEK
                STW     scoreUpdate
                LDW     scoreScratch
                PEEK
                SUBW    scoreUpdate
                BGT     updateHS_exit       ; if high score digit > score digit skip update high score
                BLT     updateHS_update     ; if high score digit < score digit update high score
                INC     scoreScratch
                INC     scratch
                LoopCounter ii updateHS_loop0   ; if scores are equal, check next digit

updateHS_update LDWI    high_string + 1     ; starting at most significant digit
                STW     scoreScratch
                LDWI    score_string + 1
                STW     scratch
                LDI     0x06
                ST      ii

updateHS_loop1  LDW     scratch
                PEEK
                POKE    scoreScratch
                INC     scoreScratch
                INC     scratch
                LoopCounter ii updateHS_loop1

                LDWI    high_string        ; print updated high score string
                STW     textStr
                LDWI    highPos
                STW     textPos
                PUSH
                CALL    printDigits
                POP
updateHS_exit   RET


                ; resets level string
resetLevel      LDWI    level_string + 2    ;LDWI    level_string + 7
                STW     scratch
                LDI     48
                POKE    scratch
                LDWI    level_string        ; print level
                STW     textStr
                LDWI    levelPos
                STW     textPos
                PUSH
                CALL    printDigits
                POP

                LDWI    mult_string + 2
                STW     scratch
                LDI     49
                POKE    scratch
                LDWI    mult_string        ; print mult string
                STW     textStr
                LDWI    multPos
                STW     textPos
                PUSH
                CALL    printDigits
                POP
                RET


resetScore      LDWI    score_string + 1
                STW     scratch
                LDI     0x06
                ST      ii
                
resetS_loop1    LDI     48
                POKE    scratch
                INC     scratch
                LoopCounter ii resetS_loop1

                LDWI    score_string        ; print score
                STW     textStr
                LDWI    scorePos
                STW     textPos
                PUSH
                CALL    printDigits
                POP

                LDWI    high_string        ; print reset/loaded high score string
                STW     textStr
                LDWI    highPos
                STW     textPos
                PUSH
                CALL    printDigits
                POP
                RET

                
                ; increments level string and multiplier string
incrementLevel  LDWI    level_string + 2    ;LDWI    level_string + 7
                STW     scratch
                PEEK
                ADDI    0x01
                ST      ii
                XORI    58                  ; 48 = '0', 57 = '9', 58 = ':'
                BEQ     incL_exit
                LD      ii
                POKE    scratch
                LDWI    mult_string + 2
                STW     scratch
                PEEK
                ADDI    0x01
                POKE    scratch
incL_exit       RET


                ; increments 6 digit score string
incrementScore  LDWI    score_string + 6
                STW     scratch
                PEEK
                ADDI    0x05
                ST      ii
                XORI    58                  ; 48 = '0', 57 = '9', 58 = ':'
                BEQ     incS_digit1
                LD      ii
                POKE    scratch
                RET

incS_digit1     LDI     48
                POKE    scratch

                LDWI    score_string + 5
                STW     scratch
                PEEK
                ADDI    0x01
                ST      ii
                XORI    58                  ; 48 = '0', 57 = '9', 58 = ':'
                BEQ     incS_digit2
                LD      ii
                POKE    scratch
                RET

incS_digit2     LDI     48
                POKE    scratch

                LDWI    score_string + 4
                STW     scratch
                PEEK
                ADDI    0x01
                ST      ii
                XORI    58                  ; 48 = '0', 57 = '9', 58 = ':'
                BEQ     incS_digit3
                LD      ii
                POKE    scratch
                RET

incS_digit3     LDI     48
                POKE    scratch

                LDWI    score_string + 3
                STW     scratch
                PEEK
                ADDI    0x01
                ST      ii
                XORI    58                  ; 48 = '0', 57 = '9', 58 = ':'
                BEQ     incS_digit4
                LD      ii
                POKE    scratch
                RET

incS_digit4     LDI     48
                POKE    scratch

                LDWI    score_string + 2
                STW     scratch
                PEEK
                ADDI    0x01
                ST      ii
                XORI    58                  ; 48 = '0', 57 = '9', 58 = ':'
                BEQ     incS_digit5
                LD      ii
                POKE    scratch
                RET

incS_digit5     LDI     48
                POKE    scratch

                LDWI    score_string + 1
                STW     scratch
                PEEK
                ADDI    0x01
                ST      ii
                XORI    58                  ; 48 = '0', 57 = '9', 58 = ':'
                BEQ     incS_exit
                LD      ii
                POKE    scratch
incS_exit       RET
