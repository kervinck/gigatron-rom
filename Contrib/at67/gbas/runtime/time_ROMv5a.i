timeByte            EQU     register0
timeDigit           EQU     register1
timeStrAddr         EQU     register2
timeArrayExt        EQU     register3                           ; reg0-3 are used outside of the interrupt
timeArrayInt        EQU     register4                           ; reg4 is used within the interrupt


%SUB                tickTime
tickTime            LDW     timerTick
                    ADDI    1
                    STW     timerTick                           ; 1/60 user timer, (max time = 546.116 seconds)
                    INC     timerPrev                           ; 1/60 internal counter
                    LD      timerPrev
                    XORI    60
                    BNE     tickT_exit
                    LDI     0
                    ST      timerPrev
                    PUSH
                    CALLI   handleTime                          ; handle time every second
                    POP

tickT_exit          RET                    
%ENDS

%SUB                handleTime
handleTime          LDWI    _timeArray_
                    STW     timeArrayInt
                    PEEK
                    ADDI    1
                    POKE    timeArrayInt                        ; seconds
                    XORI    60
                    BNE     handleT_exit
                    LDI     0
                    POKE    timeArrayInt                        ; reset seconds
                    
                    INC     timeArrayInt
                    LDW     timeArrayInt
                    PEEK
                    ADDI    1
                    POKE    timeArrayInt                        ; minutes
                    XORI    60
                    BNE     handleT_exit
                    LDI     0
                    POKE    timeArrayInt                        ; reset minutes
                    
                    INC     timeArrayInt
                    LDW     timeArrayInt
                    PEEK
                    ADDI    1
                    POKE    timeArrayInt                        ; hours
handleT_mode        XORI    24                                  ; [handleT_mode + 1] = 12 hour/24 hour
                    BNE     handleT_exit
handleT_epoch       LDI     0                                   ; [handleT_epoch + 1] = start hour
                    POKE    timeArrayInt                        ; reset hours

handleT_exit        RET                    
%ENDS

%SUB                timeDigits
timeDigits          STW     timeByte

timeD_index         SUBI    10
                    BLT     timeD_cont
                    STW     timeByte
                    INC     timeDigit                           ; calculate 10's digit
                    BRA     timeD_index
    
timeD_cont          LD      timeDigit
                    ORI     0x30
                    POKE    timeStrAddr                         ; store 10's digit
                    INC     timeStrAddr
                    LD      timeByte
                    ORI     0x30
                    POKE    timeStrAddr                         ; store 1's digit
                    INC     timeStrAddr                         ; skip colon
                    INC     timeStrAddr                         ; next 10's digit
                    LDI     0x30
                    ST      timeDigit                           ; reset 10's digit
                    RET
%ENDS

%SUB                timeString
                    ; create a time string
timeString          PUSH
                    LDI     0
                    STW     timeDigit
                    LDWI    _timeString_ + 1
                    STW     timeStrAddr                         ; skip length byte

                    LDWI    _timeArray_
                    STW     timeArrayExt
                    ADDI    2
                    PEEK                                        ; hours
                    CALLI   timeDigits

                    LDW     timeArrayExt
                    ADDI    1
                    PEEK                                        ; minutes
                    CALLI   timeDigits

                    LDW     timeArrayExt
                    PEEK                                        ; seconds
                    CALLI   timeDigits
                    POP
                    RET
%ENDS
