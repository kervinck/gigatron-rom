; configuration
CONFIG_2A = 1                   ; Base on CBMBASIC2
CONFIG_CBM_ALL = 1

CONFIG_DATAFLG = 1
CONFIG_EASTER_EGG = 0           ; WAIT-6502 Easter egg with CHROUT
CONFIG_NO_CR = 1                ; Terminal has automatic line wrap
CONFIG_NO_LINE_EDITING = 1      ; Terminal doesn't have "@", "_", BEL etc.
CONFIG_NO_READ_Y_IS_ZERO_HACK = 1
CONFIG_PEEK_SAVE_LINNUM = 1
CONFIG_SCRTCH_ORDER = 2
CONFIG_CHRGET_NOT_IN_ZP = 1

; zero page
SCRATCH         := $19          ; Use vACH as scratch location
ZP_START1       := $38          ; 10 bytes
ZP_START2 := ZP_START1+10       ; 6 bytes
ZP_START3 := ZP_START1+3        ; 11 bytes
ZP_START4 := ZP_START1+16       ; Many bytes incl. gap at $80

; extra/override ZP variables
POSX            := $30          ; X position for POS() and TAB()
CURDVC          := SCRATCH      ; Current device
Z17             := SCRATCH      ; WIDTH
Z18             := SCRATCH      ; WIDTH2
Z96             := SCRATCH      ; System file status variable ST
TXPSV           := LASTOP       ; Text pointer (2 bytges)
USR             := GORESTART    ; Trampoline for USR()

STACK           := $0000        ; v6502 has its stack in page zero
CHRGET          := GENERIC_CHRGET ; Keep out of zero page

TISTR           := $0200        ; 60 Hz 24hr clock TI$ (3+1 bytes)
STACK2          := $0204+1      ; Floating point buffer (13+3 bytes)
                                ; String conversion uses offset -1 as well!

; inputbuffer
INPUTBUFFER     := $2405        ; Will use INPUTBUFFER-5 and up!
CONFIG_INPUTBUFFER_0200 = 1

; constants
NUMLEV          = 5             ; (Originally 23) Max internal stack levels
SPACE_FOR_GOSUB = STACK_BOT + 2*NUMLEV
STACK_TOP       := $FF          ; Was $FA because INPUTBUFFER-5
WIDTH           = 40            ; Value put in Z17, but never used
WIDTH2          = 30            ; Value put in Z18, but never used
RAMSTART2       := $7100        ; User space

; magic memory locations
ENTROPY         := $06

; monitor functions
CHROUT          := $2A00        ; Send char or newline to video terminal
GETIN           := $2B00        ; Get key stroke, update TI$
ISCNTC          := $2C00        ; Check for Ctrl-C, update TI$
TICK            := $2D00        ; Update TISTR
CLALL           := TICK         ; Not implemented, do nothing
LINEEDIT        := $2E00        ; Delete character or line if needed
MONCOUT         := CHROUT
MONRDKEY        := GETIN
LOAD            := $3100        ; Not implemented, give help message
SAVE            := $2F00        ; Send program list to BabelFish
VERIFY          := SYNERR       ; Not implemented, give error

; patches

.segment "CODE"

; update 3-byte timer with latest frameCount before exporting to BASIC
GETTIM:
                jsr     TICK
                sec             ; for FLOAT3
                jmp     GETTIM1

; convert POSX pixel position (A) to character position (Y), exit with C=1
CONVPOSX:
                ldy     #27
                clc
POS1:           dey
                adc     #6
                bcc     POS1
                rts

MEMSIZE         = $01
V6502_Y         = $2B

