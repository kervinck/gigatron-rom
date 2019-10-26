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
%ENDS
     
%SUB            clearRegion     
                ; clears a region within the viewable screen
clearRegion     PUSH
                LDWI    resetVideoTable
                CALL    giga_vAC
                POP

                LDWI    SYS_Draw4_30    ; setup 4 pixel SYS routine
                STW     giga_sysFn

clearR_loop     LDW     top
                STW     giga_sysArg4    ; top line
                SYS     0xFF            ; SYS_Draw4_30, 270 - 30/2 = 0xFF

                LDW     bot
                STW     giga_sysArg4    ; bottom line
                SYS     0xFF            ; SYS_Draw4_30, 270 - 30/2 = 0xFF

                LD      top             ; 4 horizontal pixels
                ADDI    0x04
                ST      top
                LD      bot             ; 4 horizontal pixels
                ADDI    0x04
                ST      bot
                LoopCounter xcount clearR_loop

                INC     top + 1         ; next top line
                LD      bot + 1         ; next bottom line
                SUBI    0x01
                ST      bot + 1

                LD      treset         ; reset low bytes of treset, breset and xcount
                ST      top
                LD      breset
                ST      bot
                LD      xreset
                ST      xcount
                LoopCounter ycount clearR_loop
                RET
%ENDS