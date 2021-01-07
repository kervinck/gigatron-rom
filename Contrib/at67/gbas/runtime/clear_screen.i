; do *NOT* use register4 to register7 during time slicing
xreset              EQU     register0
xcount              EQU     register1
ycount              EQU     register2
treset              EQU     register3
breset              EQU     register8
top                 EQU     register9
bot                 EQU     register10
vramAddr            EQU     register11
evenAddr            EQU     register12
clsAddress          EQU     register13
varAddress          EQU     register13
clrAddress          EQU     register10
clrLines            EQU     register11
clrWidth            EQU     register12
clrStart            EQU     register13
clrEnd              EQU     register14
clrRem              EQU     register15


%SUB                resetVars
resetVars           LDI     0
                    DOKE    varAddress
                    INC     varAddress
                    INC     varAddress
                    LD      varAddress
                    XORI    giga_One                            ; end of user vars
                    BNE     resetVars
                    RET
%ENDS

%SUB                resetVideoFlags
resetVideoFlags     LDI     giga_CursorX                        ; cursor x start
                    STW     cursorXY
                    LDWI    ON_BOTTOM_ROW_MSK
                    ANDW    miscFlags
                    STW     miscFlags                           ; reset on bottom row flag
                    RET
%ENDS

%SUB                resetVideoTable
                    ; resets video table pointers
resetVideoTable     PUSH
                    LDI     8
                    STW     vramAddr
                    LDWI    giga_videoTable
                    STW     evenAddr

%if TIME_SLICING
resetVT_loop        CALL    realTimeStubAddr
                    LDW     vramAddr
%else
resetVT_loop        LDW     vramAddr
%endif
                    DOKE    evenAddr
                    INC     evenAddr
                    INC     evenAddr
                    INC     vramAddr
                    LD      vramAddr
                    SUBI    giga_yres + 8
                    BLT     resetVT_loop
                    
                    LDWI    resetVideoFlags
                    CALL    giga_vAC                    
                    POP
                    RET
%ENDS   
    
%SUB                initClearFuncs
initClearFuncs      PUSH
                    LDWI    resetVideoFlags
                    CALL    giga_vAC
            
                    LD      fgbgColour
                    ST      giga_sysArg0
                    ST      giga_sysArg0 + 1
                    ST      giga_sysArg2
                    ST      giga_sysArg2 + 1                    ; 4 pixels of bg colour
    
                    LDWI    SYS_Draw4_30                        ; setup 4 pixel SYS routine
                    STW     giga_sysFn
                    POP
                    RET
%ENDS   

%SUB                clearScreen
                    ; clears the viewable screen, (unrolled 4 times with a SYS call doing 4 pixels, so 16 pixels per loop)
clearScreen         PUSH
                    LDWI    initClearFuncs
                    CALL    giga_vAC
                    
                    LDW     clsAddress
                    STW     giga_sysArg4
                    LDWI    (giga_yres - 1) * 256
                    ADDW    clsAddress
                    STW     clsAddress                          ; end address

%if TIME_SLICING
clearS_loop         CALL    realTimeStubAddr
                    LD      giga_sysArg4
%else
clearS_loop         LD      giga_sysArg4
%endif
                    SYS     30
                    ADDI    0x04
                    ST      giga_sysArg4
                    SYS     30
                    ADDI    0x04
                    ST      giga_sysArg4
                    SYS     30
                    ADDI    0x04
                    ST      giga_sysArg4
                    SYS     30
                    ADDI    0x04
                    ST      giga_sysArg4
                    SUBI    giga_xres
                    BLT     clearS_loop
    
                    LDI     0
                    ST      giga_sysArg4
                    INC     giga_sysArg4 + 1                    ; next top line
                    LDW     giga_sysArg4
                    SUBW    clsAddress
                    BLE     clearS_loop
                    POP
                    RET
%ENDS   

%SUB                clearRect
                    ; clears a rectangle on the viewable screen
clearRect           PUSH
                    LDWI    initClearFuncs
                    CALL    giga_vAC
                    LDW     clrAddress
                    STW     giga_sysArg4
                    LD      giga_sysArg4 + 1
                    ADDW    clrLines
                    STW     clrLines
                    LD      giga_sysArg4                    ; clr start
                    STW     clrStart
                    ADDW    clrWidth
                    STW     clrEnd
                    
clearR_loop         LDWI    clearLine
                    CALL    giga_vAC
                    INC     giga_sysArg4 + 1                ; next line
                    LD      giga_sysArg4 + 1
                    SUBW    clrLines
                    BLT     clearR_loop 
                    POP
                    RET
%ENDS   

%SUB                clearLine
clearLine           PUSH
                    LD      clrStart
                    ST      giga_sysArg4
                    LD      clrWidth
                    ANDI    0xFC
                    BEQ     clearL_remloop
                    STW     clrRem

clearL_modloop      SYS     30
%if TIME_SLICING
                    CALL    realTimeStubAddr
%endif
                    LD      giga_sysArg4
                    ADDI    4
                    ST      giga_sysArg4
                    SUBW    clrEnd
                    ADDW    clrRem
                    BLT     clearL_modloop                  ; all 4 pixel chunks
                    BEQ     clearL_exit
                    LD      giga_sysArg4
                    SUBI    4
                    ST      giga_sysArg4
                    
clearL_remloop      LD      fgbgColour
                    POKE    giga_sysArg4
                    INC     giga_sysArg4
                    LD      giga_sysArg4
                    SUBW    clrEnd
                    BLT     clearL_remloop                  ; remaining pixels
                    
clearL_exit         POP
                    RET
%ENDS

%SUB                clearVertBlinds
                    ; clears the viewable screen using a vertical blinds effect
clearVertBlinds     PUSH
                    LDWI    initClearFuncs
                    CALL    giga_vAC

                    LDWI    giga_vram
                    STW     giga_sysArg4
                    LD      giga_sysArg4 + 1
                    ST      top

%if TIME_SLICING
clearVB_loop        CALL    realTimeStubAddr
                    LD      top
%else
clearVB_loop        LD      top
%endif
                    ST      giga_sysArg4 + 1                    ; top line
                    SYS     30
    
                    LDWI    giga_yres - 1 + 16
                    SUBW    top
                    ST      giga_sysArg4 + 1                    ; bottom line
                    SYS     30
    
                    LD      giga_sysArg4
                    ADDI    0x04
                    ST      giga_sysArg4
                    SUBI    giga_xres
                    BLT     clearVB_loop
    
                    LDI     0
                    ST      giga_sysArg4
                    INC     top                                 ; next top line
                    LD      top
                    SUBI    giga_yres / 2 + 8
                    BLT     clearVB_loop
                    POP
                    RET
%ENDS
