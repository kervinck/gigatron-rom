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
                SUBI    0x05
                STW     frameTicksLevel
                STW     frameTicks

                INC     scoreLevel          ; level up

                ;gprintf("Level %d : %d %d %s", *scoreLevel, *scoreScratch, *scoreDelta, *score_string)

                PUSH                        ; increment level string
                CALL    incrementLevel
                POP
                LDWI    level_string        ; print level string
                STW     textStr
                LDWI    0x713C
                STW     textPos
                PUSH
                CALL    printText
                POP

                ; increment score until it reaches (score + scoreDelta)
updateS_score   PUSH                        ; increment score string
                CALL    incrementScore      
                POP
                LDWI    score_string        ; print score string
                STW     textStr
                LDWI    0x103F
                STW     textPos
                PUSH
                CALL    printText
                POP

                LDW     scoreScratch        ; increment score scoreScratch/5 times, (score is a multiple of 5)
                SUBI    0x05
                STW     scoreScratch
                BGT     updateS_score

                ;gprintf("%d %d %s", *scoreScratch, *scoreDelta, *score_string)
                RET


                ; increments level string
incrementLevel  LDWI    level_string + 7
                STW     scratch
                PEEK
                ADDI    0x01
                ST      ii
                XORI    58              ; 48 = '0', 57 = '9', 58 = ':'
                BEQ     incL_exit
                LD      ii
                POKE    scratch
incL_exit       RET


                ; resets level string
resetLevel      LDWI    level_string + 7
                STW     scratch
                LDI     48
                POKE    scratch

                LDWI    level_string        ; print level
                STW     textStr
                LDWI    0x713C
                STW     textPos
                PUSH
                CALL    printText
                POP
                RET


                ; resets 6 digit score string
resetScore      LDWI    score_string + 1
                STW     scratch
                LDI     48
                POKE    scratch

                INC     scratch
                LDI     48
                POKE    scratch

                INC     scratch
                LDI     48
                POKE    scratch

                INC     scratch
                LDI     48
                POKE    scratch

                INC     scratch
                LDI     48
                POKE    scratch

                INC     scratch
                LDI     48
                POKE    scratch

                LDWI    score_string    ; print score
                STW     textStr
                LDWI    0x103F
                STW     textPos
                PUSH
                CALL    printText
                POP
                RET


                ; increments 6 digit score string
incrementScore  LDWI    score_string + 6
                STW     scratch
                PEEK
                ADDI    0x05
                ST      ii
                XORI    58              ; 48 = '0', 57 = '9', 58 = ':'
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
                XORI    58              ; 48 = '0', 57 = '9', 58 = ':'
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
                XORI    58              ; 48 = '0', 57 = '9', 58 = ':'
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
                XORI    58              ; 48 = '0', 57 = '9', 58 = ':'
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
                XORI    58              ; 48 = '0', 57 = '9', 58 = ':'
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
                XORI    58              ; 48 = '0', 57 = '9', 58 = ':'
                BEQ     incS_exit
                LD      ii
                POKE    scratch
incS_exit       RET
