numChannels         EQU     register0
audioAddr           EQU     register1
waveType            EQU     register2
midiNote            EQU     register4                       ; register4 to register7 are the only free registers during time slicing
midiCommand         EQU     register5
midiPtr             EQU     register6
sndChannel          EQU     register8
sndFrequency        EQU     register9
sndVolume           EQU     register10
sndWaveType         EQU     register11


%SUB                resetAudio
resetAudio          LD      giga_frameCount
                    ADDI    1
                    STW     midiDelay                       ; instant startup
                    LDWI    giga_soundChan1
                    STW     audioAddr
                    LD      waveType
                    ANDI    0x03
                    ST      waveType + 1
                    LDI     0x00
                    ST      waveType                        ; waveform type
                    LDI     0x04

resetA_loop         ST      numChannels
                    LDI     giga_soundChan1
                    ST      audioAddr                       ; reset low byte
                    LDW     waveType
                    DOKE    audioAddr                       ; wavA and wavX
                    INC     audioAddr
                    INC     audioAddr    
                    LDI     0
                    DOKE    audioAddr                       ; keyL and keyH
                    INC     audioAddr
                    INC     audioAddr
                    DOKE    audioAddr                       ; oscL and oscH
                    INC     audioAddr + 1                   ; increment high byte
                    
                    LD      numChannels
                    SUBI    1
                    BNE     resetA_loop
                    RET
%ENDS   

%SUB                playMidi
playMidi            LDW     midiStream
                    BEQ     playM_exit0                     ; 0x0000 = stop
                    LDI     0x08                            ; keep pumping soundTimer
                    ST      giga_soundTimer
                    LD      giga_frameCount
                    SUBW    midiDelay
                    BEQ     playM_start
playM_exit0         RET

playM_start         PUSH
playM_process       LDW     midiStream
                    PEEK                                    ; get midi stream byte
                    STW     midiCommand
                    LDW     midiStream
                    ADDI    0x01
                    STW     midiStream
                    LDI     0xF0
                    ANDW    midiCommand
                    XORI    0x90                            ; check for start note
                    BNE     playM_endnote
    
                    CALLI   midiStartNote                   ; start note
                    BRA     playM_process
                    
playM_endnote       XORI    0x10                            ; check for end note
                    BNE     playM_segment
    
                    CALLI   midiEndNote                     ; end note
                    BRA     playM_process

playM_segment       XORI    0x50                            ; check for new segment
                    BNE     playM_delay
    
                    LDW     midiStream                      ; midi score
                    DEEK
                    STW     midiStream                      ; 0xD0 new midi segment address
                    BEQ     playM_exit1                     ; 0x0000 = stop
                    BRA     playM_process
    
playM_delay         LD      giga_frameCount                 ; midiDelay = (midiCommand + peek(frameCount)) & 0x00FF 
                    ADDW    midiCommand
                    ST      midiDelay
playM_exit1         POP
                    RET
%ENDS

%SUB                playMidiVol
playMidiVol         LDW     midiStream
                    BEQ     playM_exit0                     ; 0x0000 = stop
                    LDI     0x08                            ; keep pumping soundTimer
                    ST      giga_soundTimer
                    LD      giga_frameCount
                    SUBW    midiDelay
                    BEQ     playM_start
playM_exit0         RET

playM_start         PUSH
playM_process       LDW     midiStream
                    PEEK                                    ; get midi stream byte
                    STW     midiCommand
                    LDW     midiStream
                    ADDI    0x01
                    STW     midiStream
                    LDI     0xF0
                    ANDW    midiCommand
                    XORI    0x90                            ; check for start note
                    BNE     playM_endnote
    
                    CALLI   midiStartNote                   ; start note
                    CALLI   midiSetVolume                   ; set note volume
                    BRA     playM_process
                    
playM_endnote       XORI    0x10                            ; check for end note
                    BNE     playM_segment
    
                    CALLI   midiEndNote                     ; end note
                    BRA     playM_process

playM_segment       XORI    0x50                            ; check for new segment
                    BNE     playM_delay
    
                    LDW     midiStream                      ; midi score
                    DEEK
                    STW     midiStream                      ; 0xD0 new midi segment address
                    BEQ     playM_exit1                     ; 0x0000 = stop
                    BRA     playM_process
    
playM_delay         LD      giga_frameCount                 ; midiDelay = (midiCommand + peek(frameCount)) & 0x00FF 
                    ADDW    midiCommand
                    ST      midiDelay
playM_exit1         POP
                    RET
%ENDS

%SUB                midiStartNote
midiStartNote       LDWI    giga_notesTable                 ; note table in ROM
                    STW     midiPtr
                    LDW     midiStream                      ; midi note
                    PEEK
                    SUBI    11
                    LSLW
                    ADDW    midiPtr
                    STW     midiPtr
                    LUP     0x00                            ; get ROM midi note low byte
                    ST      midiNote
                    LDW     midiPtr
                    LUP     0x01                            ; get ROM midi note high byte
                    ST      midiNote + 1
                    LDW     midiCommand
                    ANDI    0x03                            ; get channel
                    ADDI    0x01
                    ST      midiPtr + 1
                    LDI     0xFC
                    ST      midiPtr                         ; note address 0x01FC <-> 0x04FC
                    LDW     midiNote
                    DOKE    midiPtr                         ; set note
                    
midiSN_exit         LDW     midiStream
                    ADDI    0x01                            ; midiStream++
                    STW     midiStream
                    RET

midiSetVolume       LDI     0xFA
                    ST      midiPtr                         ; wavA address 0x01FA <-> 0x04FA, (midiPtr is still valid from midiStartNote)
                    LDW     midiStream                      ; midi volume
                    PEEK
                    POKE    midiPtr
                    BRA     midiSN_exit                     ; save a few bytes by using midiStartNote's epilogue
                    
midiEndNote         LDW     midiCommand
                    ANDI    0x03                            ; get channel
                    ADDI    0x01                
                    ST      midiPtr + 1
                    LDI     0xFC
                    ST      midiPtr                         ; channels address 0x01FC <-> 0x04FC
                    LDI     0
                    DOKE    midiPtr                         ; end note
                    RET
%ENDS

%SUB                soundAllOff
soundAllOff         LDWI    0x01FC
                    STW     sndChannel
                    LDI     0
                    DOKE    sndChannel                      ; turn off channel 0
                    INC     sndChannel + 1      
                    DOKE    sndChannel                      ; turn off channel 1
                    INC     sndChannel + 1      
                    DOKE    sndChannel                      ; turn off channel 2
                    INC     sndChannel + 1      
                    DOKE    sndChannel                      ; turn off channel 3
                    RET
%ENDS

%SUB                soundOff
soundOff            LDI     0xFC
                    ST      sndChannel
                    LDI     0
                    DOKE    sndChannel                      ; turn off channel
                    RET
%ENDS

%SUB                soundOn
soundOn             LDWI    SYS_LSRW1_48
                    STW     giga_sysFn
					LDI     0xFC
                    ST      sndChannel
                    LD      sndFrequency
                    SYS     48
                    ST      sndFrequency                    ; right shift low byte of sndFrequency by 1
                    LDW     sndFrequency                    ; format = high:8 low:07, (bit 7 of low byte = 0)
                    DOKE    sndChannel                      ; turn on channel
                    RET
%ENDS

%SUB                soundOnV
soundOnV            LDWI    SYS_LSRW1_48
                    STW     giga_sysFn
                    LDI     0xFC
                    ST      sndChannel
                    LD      sndFrequency
                    SYS     48
                    ST      sndFrequency                    ; right shift low byte of sndFrequency by 1
                    LDW     sndFrequency                    ; format = high:8 low:07, (bit 7 of low byte = 0)
                    DOKE    sndChannel                      ; turn on channel
                    LDI     0xFA
                    ST      sndChannel                      ; still pointing to the correct channel
                    LDI     63
                    SUBW    sndVolume
                    ADDI    64
                    POKE    sndChannel                      ; 0 -> 63 maps to 127 -> 64
                    INC     sndChannel
                    LD      sndWaveType
                    ANDI    3
                    POKE    sndChannel                      ; wave type in wave X
                    RET
%ENDS

%SUB                soundMod
soundMod            LDI     0xFA
                    ST      sndChannel
                    LDW     sndWaveType                     ; format = high:waveX low:waveA
                    DOKE    sndChannel                      ; set modulation
                    RET
%ENDS
