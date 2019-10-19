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
SCRATCH = $19                           ; Use vACH as scratch location
ZP_START1 = $38                         ; 10 bytes
ZP_START2 = ZP_START1+10                ; 6 bytes
ZP_START3 = ZP_START1+3                 ; 11 bytes
ZP_START4 = ZP_START1+16                ; 94 bytes incl. gap at $80

; extra/override ZP variables
POSX                    := $30          ; X position for POS() and TAB()
CURDVC                  := SCRATCH      ; Current device
Z96                     := SCRATCH      ; System file status variable ST
STACK_BOT               := <(GENERIC_CHRGET_END-GENERIC_CHRGET + CHRGET) ; For CHKMEM
TXPSV                   := LASTOP       ; 2 bytes text pointer
USR                     := GORESTART    ; Trampoline for USR()

STACK                   := $0000        ; v6502 has stack in zero page
STACK2                  := $7100        ; Floating point buffer (13+3 bytes)

; inputbuffer
INPUTBUFFER             := $2400        ; Will use INPUTBUFFER-5 and up!
CONFIG_INPUTBUFFER_0200 := 1

; constants
NUMLEV                  := 23           ; Max internal stack usage (words)
;SPACE_FOR_GOSUB         := STACK_BOT + 2*NUMLEV
SPACE_FOR_GOSUB         := STACK_BOT + 2*5
; Original
;       SPACE_FOR_GOSUB $3E = 2*NUMLEV + 3*ADDPRC + 13 = 62
; With:
;       Sizeof FBUFFR = 13 (normally at bottom of page 1)
;       ADDPRC = 1 "FOR ADDITIONAL PRECISION" (extra bytes)
; Both aren't there in the Gigatron...
; Instead, Gigatron has zero page usage until ~$00A8 and stack above

STACK_TOP               := $FF          ; Was $FA because INPUTBUFFER-5
WIDTH                   := 40           ; Value put in Z17, but never used
WIDTH2                  := 30           ; Value put in Z18, but never used
Z17                     := SCRATCH
Z18                     := SCRATCH

RAMSTART2               := $7200

; magic memory locations
ENTROPY = $06

; monitor functions
CHROUT  := $2900
LOAD    := SYNERR
SAVE    := SYNERR
VERIFY  := SYNERR
ISCNTC  := $2B00
GETIN   := $2A00
CLALL   := $2C00
TISTR   := $2D00        ; 60 Hz 24-hour clock (3+1 bytes, big-endian)
MONCOUT := CHROUT
MONRDKEY := GETIN

.segment "CODE"

; Update 3-byte timer with latest frameCount before exporting to BASIC
GETTIM:
        lda     $0e     ; frameCount
        tay
        sec
        sbc     TISTR+3
        sty     TISTR+3
        clc
        adc     TISTR+2
        sta     TISTR+2
        lda     #0
        adc     TISTR+1
        sta     TISTR+1
        lda     #0
        adc     TISTR+0
        sta     TISTR+0
        sec             ; for FLOAT3
        jmp     GETTIM1
