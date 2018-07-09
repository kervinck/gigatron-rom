xreset          EQU     register0
xcount          EQU     register1
ycount          EQU     register2
treset          EQU     register3
breset          EQU     register4
top             EQU     register5
bot             EQU     register6


                ; clears a region within the viewable screen
clearRegion     LDWI    SYS_Draw4_30    ; setup 4 pixel SYS routine
                STW     giga_sysFn

clearS_loop     LDW     top
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
                LoopCounter xcount clearS_loop

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
                LoopCounter ycount clearS_loop
                RET
