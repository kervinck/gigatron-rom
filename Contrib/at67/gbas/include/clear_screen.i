xreset          EQU     register0
xcount          EQU     register1
ycount          EQU     register2
treset          EQU     register3
breset          EQU     register4
top             EQU     register5
bot             EQU     register6
vramptr         EQU     register7
evenaddr        EQU     register8


%SUB            resetVideoTable
                ; resets video table pointers
resetVideoTable LDWI    0x0008
                STW     vramptr
                LDWI    giga_videoTable
                STW     evenaddr

resetVT_loop    LDW     vramptr
                DOKE    evenaddr
                INC     evenaddr
                INC     evenaddr

                INC     vramptr
                LD      vramptr
                SUBI    giga_yres+8
                BLT     resetVT_loop
                RET
                
initClearFuncs  PUSH
                LDWI    resetVideoTable
                CALL    giga_vAC

                LDWI    0x0002                              ; starting cursor position
                STW     cursorXY
                LDWI    0x0001                              ; reset flags
                STW     miscFlags
        
                LD      fgbgColour
                ST      giga_sysArg0
                ST      giga_sysArg0 + 1
                ST      giga_sysArg2
                ST      giga_sysArg2 + 1                    ; 4 pixels of colour

                LDWI    SYS_Draw4_30                        ; setup 4 pixel SYS routine
                STW     giga_sysFn
                POP
                RET
%ENDS

%SUB            clearScreen
                ; clears the viewable screen
clearScreen     PUSH
                LDWI    initClearFuncs
                CALL    giga_vAC
                
                LDI     giga_yres / 2
                ST      ycount
                LDI     giga_xres / 4
                ST      xcount
                ST      xreset

                LDWI    giga_vram                           ; top line
                STW     treset
                STW     top
                LDWI    (giga_yres - 1)*256 + giga_vram     ; bottom line
                STW     breset
                STW     bot

clearS_loop     LDW     top
                STW     giga_sysArg4                        ; top line
                SYS     0xFF                                ; SYS_Draw4_30, 270 - 30/2 = 0xFF

                LDW     bot
                STW     giga_sysArg4                        ; bottom line
                SYS     0xFF                                ; SYS_Draw4_30, 270 - 30/2 = 0xFF

                LD      top                                 ; 4 horizontal pixels
                ADDI    0x04
                ST      top
                LD      bot                                 ; 4 horizontal pixels
                ADDI    0x04
                ST      bot
                LoopCounter xcount clearS_loop

                INC     top + 1                             ; next top line
                LD      bot + 1                             ; next bottom line
                SUBI    0x01
                ST      bot + 1

                LD      treset                              ; reset low bytes of treset, breset and xcount
                ST      top
                LD      breset
                ST      bot
                LD      xreset
                ST      xcount
                LoopCounter ycount clearS_loop
                POP
                RET
%ENDS
     
%SUB            clearRegion
                ; clears a region within the viewable screen
clearRegion     PUSH
                LDWI    initClearFuncs
                CALL    giga_vAC

clearR_loop     LDW     top
                STW     giga_sysArg4                        ; top line
                SYS     0xFF                                ; SYS_Draw4_30, 270 - 30/2 = 0xFF

                LDW     bot
                STW     giga_sysArg4                        ; bottom line
                SYS     0xFF                                ; SYS_Draw4_30, 270 - 30/2 = 0xFF

                LD      top                                 ; 4 horizontal pixels
                ADDI    0x04
                ST      top
                LD      bot                                 ; 4 horizontal pixels
                ADDI    0x04
                ST      bot
                LoopCounter xcount clearR_loop

                INC     top + 1                             ; next top line
                LD      bot + 1                             ; next bottom line
                SUBI    0x01
                ST      bot + 1

                LD      treset                              ; reset low bytes of treset, breset and xcount
                ST      top
                LD      breset
                ST      bot
                LD      xreset
                ST      xcount
                LoopCounter ycount clearR_loop
                POP
                RET
%ENDS