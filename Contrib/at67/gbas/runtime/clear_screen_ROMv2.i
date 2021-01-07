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
clsLines            EQU     register14
varAddress          EQU     register13
clrAddress          EQU     register13
clrLines            EQU     register14
clrWidth            EQU     register15
    

%SUB                resetVars
resetVars           LDI     0
                    DOKE    varAddress
                    INC     varAddress
                    INC     varAddress
                    LD      varAddress
                    XORI    giga_One                        ; end of user vars
                    BNE     resetVars
                    RET
%ENDS

%SUB                resetVideoFlags
resetVideoFlags     LDI     giga_CursorX                    ; cursor x start
                    STW     cursorXY
                    LDWI    ON_BOTTOM_ROW_MSK
                    ANDW    miscFlags
                    STW     miscFlags                       ; reset on bottom row flag
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
                    LDWI    SYS_SetMemory_v2_54             ; setup fill memory SYS routine
                    STW     giga_sysFn
                    LD      fgbgColour
                    ST      giga_sysArg1                    ; fill value
                    POP
                    RET
%ENDS   

%SUB                clearScreen
                    ; clears the viewable screen
clearScreen         PUSH
                    LDWI    initClearFuncs
                    CALL    giga_vAC
                    LD      clsAddress + 1
                    ST      giga_sysArg3
                    LDI     120
                    
clearCS_loopy       ST      clsLines
                    LDI     giga_xres
                    ST      giga_sysArg0
                    LD      clsAddress
                    ST      giga_sysArg2
                    SYS     54                              ; fill memory
                    INC     giga_sysArg3                    ; next line
                    LD      clsLines
                    SUBI    1
                    BNE     clearCS_loopy
%if TIME_SLICING
                    CALL    realTimeStubAddr
%endif
                    POP
                    RET
%ENDS   

%SUB                clearRect
                    ; clears a rectangle on the viewable screen
clearRect           PUSH
                    LDWI    initClearFuncs
                    CALL    giga_vAC
                    LD      clrAddress + 1
                    ST      giga_sysArg3
                    LD      clrLines
                    
clearR_loop         ST      clrLines
                    LD      clrWidth
                    ST      giga_sysArg0
                    LD      clrAddress
                    ST      giga_sysArg2
                    SYS     54                              ; fill memory
                    INC     giga_sysArg3                    ; next line
                    LD      clrLines
                    SUBI    1
                    BNE     clearR_loop
%if TIME_SLICING
                    CALL    realTimeStubAddr
%endif
                    POP
                    RET
%ENDS
    
%SUB                clearVertBlinds
                    ; clears the viewable screen using a vertical blinds effect
clearVertBlinds     PUSH
                    LDWI    initClearFuncs
                    CALL    giga_vAC
                    LDI     giga_vram >> 8
                    STW     top  
                    
clearVB_loopy       LDI     giga_xres
                    ST      giga_sysArg0
                    LDI     0
                    ST      giga_sysArg2                    ; low start address
                    LD      top
                    ST      giga_sysArg3                    ; top line
                    SYS     54                              ; fill memory
    
                    LDI     giga_xres
                    ST      giga_sysArg0
                    LDI     0
                    ST      giga_sysArg2                    ; low start address
                    LDWI    giga_yres - 1 + 16
                    SUBW    top
                    ST      giga_sysArg3                    ; bottom line
                    SYS     54                              ; fill memory
                    INC     top                             ; next top line
%if TIME_SLICING
                    CALL    realTimeStubAddr
%endif
                    LD      top
                    SUBI    giga_yres / 2 + 8
                    BLT     clearVB_loopy
                    POP
                    RET
%ENDS
