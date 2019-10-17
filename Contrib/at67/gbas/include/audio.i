midiData        EQU     register0
midiNote        EQU     register1
midiCommand     EQU     register2
midiChannel     EQU     register3
numChannels     EQU     register4
audioBase       EQU     register5
channelsBase    EQU     register6
notesBase       EQU     register7


resetAudio      LDWI    0x0000
                STW     midiDelay
                
                LDI     0x04
                ST      numChannels
resetA_loop     LDI     giga_soundChan1     ; reset low byte
                ST      audioBase
                LDWI    0x0300              
                DOKE    audioBase           ; wavA and wavX
                INC     audioBase
                INC     audioBase    
                LDWI    0x0000
                DOKE    audioBase           ; keyL and keyH
                INC     audioBase
                INC     audioBase
                DOKE    audioBase           ; oscL and oscH
                INC     audioBase + 1       ; increment high byte
                LoopCounter numChannels resetA_loop
                RET


playMidiAsync   LD      giga_frameCount
                SUBW    frameCountPrev
                BEQ     playMV_exit
                LD      giga_frameCount
                STW     frameCountPrev
                PUSH
                LDWI    playMidi
                CALL    giga_vAC
                POP
playMV_exit     RET


playMidi        LDWI    giga_soundChan1 + 2 ; keyL, keyH
                STW     channelsBase
                LDI     0x01                ; keep pumping soundTimer, so that global sound stays alive
                ST      giga_soundTimer
                LDW     midiDelay
                BEQ     playM_process
                SUBI    0x01
                STW     midiDelay
                BEQ     playM_process    
                RET

playM_process   LDW     midiStreamPtr
                PEEK                        ; get midi stream byte
                STW     midiData
                LDW     midiStreamPtr
                ADDI    0x01
                STW     midiStreamPtr
                LDW     midiData
                ANDI    0xF0
                STW     midiCommand
                XORI    0x90                ; check for start note
                BNE     playM_endnote

                PUSH
                LDWI    midiStartNote
                CALL    giga_vAC            ; start note
                POP
                BRA     playM_process
                
playM_endnote   LDW     midiCommand 
                XORI    0x80                ; check for end note
                BNE     playM_segment

                PUSH
                LDWI    midiEndNote
                CALL    giga_vAC            ; end note
                POP
                BRA     playM_process


playM_segment   LDW     midiCommand
                XORI    0xD0                ; check for new segment
                BNE     playM_delay

                PUSH
                LDWI    midiSegment
                CALL    giga_vAC            ; new midi segment
                POP
                BRA     playM_process

playM_delay     LDW     midiData            ; all that is left is delay
                STW     midiDelay
                RET


midiStartNote   LDWI    giga_notesTable     ; note table in ROM
                STW     notesBase
                LDW     midiStreamPtr       ; midi score
                PEEK
                SUBI    10
                LSLW
                SUBI    2
                ADDW    notesBase
                STW     notesBase
                LUP     0x00                ; get ROM midi note low byte
                ST      midiNote
                LDW     notesBase
                LUP     0x01                ; get ROM midi note high byte
                ST      midiNote + 1
                LDW     midiData
                ANDI    0x03                ; get channel
                ST      midiChannel + 1
                LDI     0x00
                ST      midiChannel
                LDW     midiChannel
                ADDW    channelsBase        ; channel address
                STW     midiChannel
                LDW     midiNote
                DOKE    midiChannel         ; set note
                LDW     midiStreamPtr
                ADDI    0x01                ; midiStreamPtr++
                STW     midiStreamPtr
                RET


midiEndNote     LDW     midiData
                ANDI    0x03                ; get channel
                ST      midiChannel + 1
                LDI     0x00
                ST      midiChannel
                LDW     midiChannel
                ADDW    channelsBase        ; channel address
                STW     midiChannel
                LDWI    0x0000
                DOKE    midiChannel         ; end note
                RET


midiSegment     LDW     midiStreamPtr       ; midi score
                DEEK
                STW     midiStreamPtr       ; 0xD0 new midi segment address
                RET
