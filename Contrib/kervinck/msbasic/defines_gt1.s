; configuration
CONFIG_2A := 1

CONFIG_CBM_ALL := 1

CONFIG_DATAFLG := 1
CONFIG_EASTER_EGG := 1
;CONFIG_FILE := 1; support PRINT#, INPUT#, GET#, CMD
CONFIG_NO_CR := 1; terminal doesn't need explicit CRs on line ends
CONFIG_NO_LINE_EDITING := 1; support for "@", "_", BEL etc.
CONFIG_NO_READ_Y_IS_ZERO_HACK := 1
CONFIG_PEEK_SAVE_LINNUM := 1
CONFIG_SCRTCH_ORDER := 2


; zero page
ZP_START1 = $00+$38     ; 10 bytes
ZP_START2 = $0D+$38     ; 6 bytes
ZP_START3 = $03+$38     ; 11 bytes
ZP_START4 = $13+$38     ; 94 bytes incl. gap at $80 (until $a8)

; extra/override ZP variables
CURDVC			:= $000E+$38
TISTR			:= $00A9 ; 3 bytes (was $008D)
Z96			:= $00AC ; 2 bytes (was $0096)
POSX			:= $00AE ; 1 byte  (was $00C6)
TXPSV			:= LASTOP
USR				:= GORESTART ; XXX

STACK			:= $0000
			; Stack size: 81 bytes ($AF.$FF)

STACK2			:= $7100
			; Floating point buffer

; inputbuffer
INPUTBUFFER     := $0200

; constants
SPACE_FOR_GOSUB := $3E          ; XXX ???
STACK_TOP		:= $FF  ; (was $FA)
WIDTH			:= 26   ; (was 40)

WIDTH2			:= 30   ; XXX ???

RAMSTART2		:= $7200

; magic memory locations
ENTROPY = $06

; monitor functions
OPEN	:= $FFC0
CLOSE	:= $FFC3
CHKIN	:= $FFC6
CHKOUT	:= $FFC9
CLRCH	:= $FFCC
CHRIN	:= $FFCF
CHROUT	:= $2700        ; Gigatron
LOAD	:= $FFD5
SAVE	:= $FFD8
VERIFY	:= $FFDB
SYS		:= $FFDE
ISCNTC	:= $2900
GETIN	:= $2800
CLALL	:= $2A00
LE7F3	:= $E7F3; for CBM1
MONCOUT	:= CHROUT
MONRDKEY := GETIN
