numChannels         EQU     register0
audioPtr            EQU     register1
midiNote            EQU     register4           ; register4 to register7 are the only free registers during time slicing
midiCommand         EQU     register5
midiPtr             EQU     register6


%SUB                resetAudio
resetAudio          LDWI    0x0000
                    STW     midiDelay
                    LDWI    giga_soundChan1
                    STW     audioPtr
                    LDI     0x04
                    ST      numChannels
                    
resetA_loop         LDI     giga_soundChan1     ; reset low byte
                    ST      audioPtr
                    LDWI    0x0200              ; waveform type
                    DOKE    audioPtr            ; wavA and wavX
                    INC     audioPtr
                    INC     audioPtr    
                    LDWI    0x0000
                    DOKE    audioPtr            ; keyL and keyH
                    INC     audioPtr
                    INC     audioPtr
                    DOKE    audioPtr            ; oscL and oscH
                    INC     audioPtr + 1        ; increment high byte
                    LoopCounter numChannels resetA_loop
                    RET
%ENDS   
    
%SUB                playMidi
playMidi            LDW     midiStream
                    BEQ     playM_exit0         ; 0x0000 = stop
                    LDI     0x05                ; keep pumping soundTimer, so that global sound stays alive
                    ST      giga_soundTimer
                    LD      giga_frameCount
                    SUBW    midiDelay
                    BEQ     playM_start
playM_exit0         RET

playM_start         PUSH
playM_process       LDW     midiStream
                    PEEK                        ; get midi stream byte
                    STW     midiCommand
                    LDW     midiStream
                    ADDI    0x01
                    STW     midiStream
                    LDI     0xF0
                    ANDW    midiCommand
                    XORI    0x90                ; check for start note
                    BNE     playM_endnote
    
                    CALLI   midiStartNote       ; start note
                    BRA     playM_process
                    
playM_endnote       XORI    0x10                ; check for end note
                    BNE     playM_segment
    
                    CALLI   midiEndNote         ; end note
                    BRA     playM_process

playM_segment       XORI    0x50                ; check for new segment
                    BNE     playM_delay
    
                    LDW     midiStream          ; midi score
                    DEEK
                    STW     midiStream          ; 0xD0 new midi segment address
                    BEQ     playM_exit1         ; 0x0000 = stop
                    BRA     playM_process
    
playM_delay         LD      giga_frameCount     ; midiDelay = (midiCommand + peek(frameCount)) & 0x00FF 
                    ADDW    midiCommand
                    ST      midiDelay
playM_exit1         POP
                    RET
%ENDS

%SUB                midiStartNote
midiStartNote       LDWI    giga_notesTable     ; note table in ROM
                    STW     midiPtr
                    LDW     midiStream          ; midi score
                    PEEK
                    SUBI    11
                    LSLW
                    ADDW    midiPtr
                    STW     midiPtr
                    LUP     0x00                ; get ROM midi note low byte
                    ST      midiNote
                    LDW     midiPtr
                    LUP     0x01                ; get ROM midi note high byte
                    ST      midiNote + 1
                    LDW     midiCommand
                    ANDI    0x03                ; get channel
                    ADDI    0x01                
                    ST      midiPtr + 1
                    LDI     0xFC
                    ST      midiPtr             ; channels address 0x01FC <-> 0x04FC
                    LDW     midiNote
                    DOKE    midiPtr             ; set note
                    LDW     midiStream
                    ADDI    0x01                ; midiStream++
                    STW     midiStream
                    RET
                    
midiEndNote         LDW     midiCommand
                    ANDI    0x03                ; get channel
                    ADDI    0x01                
                    ST      midiPtr + 1
                    LDI     0xFC
                    ST      midiPtr             ; channels address 0x01FC <-> 0x04FC
                    LDWI    0x0000
                    DOKE    midiPtr             ; end note
                    RET
%ENDS
