resetAudio      LDWI    0x0000
                STW     midiCommand
                STW     midiDelay
                STW     midiNote
                LDWI    giga_soundChan1 + 2 ; keyL, keyH
                STW     midiChannel
                STW     scratch
                LDWI    title_screenMidi00  ; midi score
                STW     midiStreamPtr

                LDI     0x04
                ST      ii

resetA_loop     LDI     giga_soundChan1     ; reset low byte
                ST      scratch
                LDWI    0x0300              
                DOKE    scratch             ; wavA and wavX
                INC     scratch
                INC     scratch    
                LDWI    0x0000
                DOKE    scratch             ; keyL and keyH
                INC     scratch
                INC     scratch
                DOKE    scratch             ; oscL and oscH
                LDWI    0x0100              ; increment high byte
                ADDW    scratch
                STW     scratch
                LoopCounter ii resetA_loop
                RET


playMidiVBlank  LD      giga_frameCount
                SUBW    frameCountPrev
                BEQ     playMV_exit
                LD      giga_frameCount
                STW     frameCountPrev
                PUSH
                CALL    playMidi
                POP
playMV_exit     RET


playMidi        LDI     0x02                ; keep pumping soundTimer, so that global sound stays alive
                ST      giga_soundTimer
                LDW     midiDelay
                BEQ     playM_process

                SUBI    0x01
                STW     midiDelay
                RET

playM_process   LDW     midiStreamPtr
                PEEK                        ; get midi stream byte
                STW     midiCommand
                LDW     midiStreamPtr
                ADDI    0x01
                STW     midiStreamPtr
                LDW     midiCommand
                ANDI    0xF0
                STW     scratch
                XORI    0x90                ; check for start note
                BNE     playM_endnote

                PUSH                    
                CALL    midiStartNote       ; start note
                POP
                BRA     playMidi
                
playM_endnote   LDW     scratch 
                XORI    0x80                ; check for end note
                BNE     playM_segment

                PUSH
                CALL    midiEndNote         ; end note
                POP
                BRA     playMidi


playM_segment   LDW     scratch
                XORI    0xD0                ; check for new segment
                BNE     playM_delay

                PUSH
                CALL    midiSegment         ; new midi segment
                POP
                BRA     playMidi

playM_delay     LDW     midiCommand         ; all that is left is delay
                STW     midiDelay
                RET


midiStartNote   LDWI    giga_notesTable     ; note table in ROM
                STW     scratch
                LDW     midiStreamPtr       ; midi score
                PEEK
                SUBI    10
                LSLW
                SUBI    2
                ADDW    scratch
                STW     scratch
                LUP     0x00                ; get ROM midi note low byte
                ST      midiNote
                LDW     scratch
                LUP     0x01                ; get ROM midi note high byte
                ST      midiNote + 1
                LDW     midiCommand
                ANDI    0x03                ; get channel
                ST      scratch + 1
                LDI     0x00
                ST      scratch
                LDW     scratch
                ADDW    midiChannel         ; channel address
                STW     scratch
                LDW     midiNote
                DOKE    scratch             ; set note
                LDW     midiStreamPtr
                ADDI    0x01                ; midiStreamPtr++
                STW     midiStreamPtr
                RET


midiEndNote     LDW     midiCommand
                ANDI    0x03                ; get channel
                ST      scratch + 1
                LDI     0x00
                ST      scratch
                LDW     scratch
                ADDW    midiChannel         ; channel address
                STW     scratch
                LDWI    0x0000
                DOKE    scratch             ; end note
                RET


midiSegment     LDW     midiStreamPtr       ; midi score
                DEEK
                STW     midiStreamPtr       ; 0xD0 new midi segment address
                RET
