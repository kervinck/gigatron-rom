;  The WOZ Monitor for the Apple 1
;  Written by Steve Wozniak in 1976

; Adapted for Gigatron TTL microcomputer

; wozmon.gt1x: wozmon.s
;        ca65 wozmon.s -o wozmon.o -l wozmon.lst
;        ld65 -t none -o wozmon.gt1x wozmon.o

; Page 0 Variables

buttonState     = $11           ; [Gigatron] Edge-triggered resettable input bits

                                ; [Gigatron] ZP addresses moved from $2X to $3X
XAML            = $34           ; Last "opened" location Low
XAMH            = $35           ; Last "opened" location High
STL             = $36           ; Store address Low
STH             = $37           ; Store address High
L               = $38           ; Hex value parsing Low
H               = $39           ; Hex value parsing High
YSAV            = $3A           ; Used to see if hex value is given
MODE            = $3B           ; $00=XAM, $7F=STOR, $AE=BLOCK XAM


; Other Variables

IN              = $200          ; Input buffer to $027F

                ; Gigatron vCPU startup
                .BYTE >START,<START,ENDSTART-START
                .ORG $200
START:          .BYTE $1A,$21,$E6,$F8,$35,$53,$0B,$21,$0E,$F3,$17,$90,$05,$1A,$21,$82
                .BYTE $F8,$5E,$21,$11,$00,$01,$2B,$30,$59,$78,$8C,$7F,$F3,$30,$93,$30
                .BYTE $93,$30,$E3,$01,$8C,$7F,$35,$72,$18,$11,$55,$03,$2B,$32,$11,$00
                .BYTE $03,$2B,$1A,$11,$80,$02,$FF
ENDSTART:

                ; Munching square intro
                .BYTE >MUNCH,<MUNCH,ENDMUNCH-MUNCH
                .ORG $280
MUNCH:          .BYTE $18,$69,$17,$85,$40,$A9,$00,$85,$41,$A9,$10,$85,$42,$A9,$07,$85
                .BYTE $43,$45,$41,$A8,$A5,$40,$91,$42,$E6,$43,$A5,$43,$C9,$7F,$D0,$F1
                .BYTE $A5,$0F,$C9,$03,$F0,$09,$E6,$41,$10,$E3,$A5,$40,$4C,$80,$02,$A9
                .BYTE $8D,$00,$4C,$00,$FF
ENDMUNCH:

                ; Main loop and I/O mockup
                .BYTE >MAIN,<MAIN,ENDMAIN-MAIN
                .ORG $300
MAIN:           .BYTE $2B,$1A,$11,$0C,$0B,$2B,$22,$1A,$3A,$B4,$E6,$75,$35,$72,$4C,$1A
                .BYTE $0F,$8C,$FF,$35,$72,$0D,$1A,$0E,$82,$30,$35,$72,$1F,$59,$A0,$90
                .BYTE $21,$59,$C0,$CF,$32,$2B,$30,$1A,$0F,$5E,$3A,$82,$80,$35,$72,$14
                .BYTE $59,$A0,$CF,$32,$2B,$30,$1A,$3A,$E6,$60,$35,$50,$3F,$E3,$40,$5E
                .BYTE $3A,$1A,$3A,$8C,$0A,$35,$72,$4A,$59,$0D,$5E,$3A,$90,$50,$5E,$3A
                .BYTE $CF,$32,$63,$90,$00,$EC,$FE,$11,$E1,$04,$2B,$22,$11,$00,$08,$2B
                .BYTE $24,$EE,$FE,$8C,$8D,$35,$3F,$6F,$1A,$30,$E6,$9B,$35,$56,$6F,$59
                .BYTE $00,$35,$72,$AB,$11,$00,$01,$5E,$30,$AD,$5E,$31,$21,$30,$2B,$28
                .BYTE $5E,$26,$B4,$CB,$93,$28,$1A,$28,$8C,$A0,$35,$72,$80,$11,$EE,$01
                .BYTE $2B,$3C,$21,$3C,$AD,$E6,$77,$35,$53,$9C,$E3,$7F,$90,$9E,$E3,$07
                .BYTE $F0,$3C,$21,$3C,$E6,$02,$2B,$3C,$8C,$FE,$35,$72,$90,$EE,$FE,$E6
                .BYTE $A0,$35,$50,$F4,$E6,$32,$35,$53,$C0,$E3,$32,$2B,$3C,$11,$00,$07
                .BYTE $90,$C5,$2B,$3C,$11,$00,$08,$2B,$3E,$21,$3C,$E9,$E9,$99,$3C,$99
                .BYTE $3E,$2B,$3E,$21,$30,$2B,$28,$EC,$FE,$E3,$06,$2B,$30,$59,$05,$2B
                .BYTE $3C,$21,$3E,$7F,$00,$5E,$26,$B4,$CB,$93,$3E,$93,$28,$21,$3C,$E6
                .BYTE $01,$35,$4D,$DD,$EE,$FE,$FF
ENDMAIN:

                ; Gigatron GT1 file segment header for WozMon code
                .BYTE >RESET,<RESET,(END-RESET)&255
               .ORG $FF00
RESET:          CLD             ; Clear decimal arithmetic mode.
                CLI
                LDY #$7F        ; Mask for DSP data direction register.
        .BYTE   $2C,$12,$D0     ; [Gigatron] BIT DSP (was: STY DSP)
                LDA #$A7        ; KBD and DSP control register mask.
        .BYTE   $2C,$11,$D0     ; [Gigatron] BIT KBDCR (was: STA KBDCR)
        .BYTE   $2C,$13,$D0     ; [Gigatron] BIT DSPCR (was: STA DSPCR)
NOTCR:          CMP #$DF        ; "<-"?
                BEQ BACKSPACE   ; Yes.
                CMP #$9B        ; ESC?
                BEQ ESCAPE      ; Yes.
                INY             ; Advance text index.
                BPL NEXTCHAR    ; Auto ESC if > 127.
ESCAPE:         LDA #$DC        ; "\".
                JSR ECHO        ; Output it.
GETLINE:        LDA #$8D        ; CR.
                JSR ECHO        ; Output it.
                LDY #$01        ; Initiallize text index.
BACKSPACE:      DEY             ; Back up text index.
                BMI GETLINE     ; Beyond start of line, reinitialize.
NEXTCHAR:       LDA #$00        ; [Gigatron] A=0 to read keyboard.
                BRK             ; [Gigatron] Transfer to vCPU for input.
                ORA #$80        ; [Gigatron] B7 should be '1'.
        .BYTE   $2C,$10,$D0     ; [Gigatron] BIT KBD (was: LDA KBD)
                STA IN,Y        ; Add to text buffer.
                JSR ECHO        ; Display character.
                CMP #$8D        ; CR?
                BNE NOTCR       ; No.
                LDY #$FF        ; Reset text index.
                LDA #$00        ; For XAM mode.
                TAX             ; 0->X.
SETSTOR:        ASL             ; Leaves $7B if setting STOR mode.
SETMODE:        STA MODE        ; $00 = XAM, $7B= STOR,$AE= BLOK XAM
BLSKIP:         INY             ; Advance text index.
NEXTITEM:       LDA IN,Y        ; Get character.
                CMP #$8D        ; CR?
                BEQ GETLINE     ; Yes, done this line.
                CMP #$AE        ; "."?
                BCC BLSKIP      ; Skip delimiter.
                BEQ SETMODE     ; Yes, BLOCK XAM mode.
                CMP #$BA        ; ":"?
                BEQ SETSTOR     ; Yes, set STOR mode.
                CMP #$D2        ; "R"?
                BEQ RUN         ; Yes, run user program.
                STX L           ; $00->L.
                STX H           ;  and H.
                STY YSAV        ; Save Y for comparison.
NEXTHEX:        LDA IN,Y        ; Get character for hex test.
                EOR #$B0        ; Map digits to $0-9.
                CMP #$0A        ; Digit?
                BCC DIG         ; Yes.
                ADC #$88        ; Map letter "A"-"F" to $FA-FF.
                CMP #$FA        ; Hex letter?
                BCC NOTHEX      ; No, character not hex.
DIG:            ASL
                ASL             ; Hex digit to MSD of A.
                ASL
                ASL
                LDX #$04        ; Shift count.
HEXSHIFT:       ASL             ; Hex digit left, MSB to carry.
                ROL L           ; Rotate into LSD.
                ROL H           ; Rotate into MSD's.
                DEX             ; Done 4 shifts?
                BNE HEXSHIFT    ; No, loop.
                INY             ; Advance text index.
                BNE NEXTHEX     ; Always taken. Check next character for hex.
NOTHEX:         CPY YSAV        ; Check if L, H empty (no hex digits).
                BEQ ESCAPE      ; Yes, generate  ESC sequence.
                BIT MODE        ; Test MODE byte.
                BVC NOTSTOR     ; B6 = 0 for STOR, 1 for XAM and BLOCK XAM
                LDA L           ; LSD's of hex data.
                STA (STL,X)     ; Store at current 'store index'.
                INC STL         ; Increment store index.
                BNE NEXTITEM    ; Get next item. (no carry).
                INC STH         ; Add carry to 'store index' high order.
TONEXTITEM:     JMP NEXTITEM    ; Get next command item.
RUN:            JMP (XAML)      ; Run at current XAM index.
NOTSTOR:        BMI XAMNEXT     ; B7 = 0 for XAM, 1 for BLOCK XAM.
                LDX #$02        ; Byte count.
SETADR:         LDA L-1,X       ; Copy hex data to
                STA STL-1,X     ;   'store index'.
                STA XAML-1,X    ; And to 'XAM index'.
                DEX             ; Next of 2 bytes.
                BNE SETADR      ; Loop unless X = 0.
NXTPRNT:        BNE PRDATA      ; NE means no address to print.
                LDA #$8D        ; CR.
                JSR ECHO        ; Output it.
                LDA XAMH        ; 'Examine index' high-order byte.
                JSR PRBYTE      ; Output it in hex format.
                LDA XAML        ; Low-order 'examine index' byte.
                JSR PRBYTE      ; Output it in hex format.
                LDA #$BA        ; ":".
                JSR ECHO        ; Output it.
PRDATA:         LDA #$04        ; [Gigatron] 4 pixels wide blank.
                JSR BLANK       ; [Gigatron] Output it.
                LDA (XAML,X)    ; Get data byte at 'examine index'.
                JSR PRBYTE      ; Output it in hex format.
XAMNEXT:        STX MODE        ; 0->MODE (XAM mode).
                LDA XAML
                CMP L           ; Compare 'examine index' to hex data.
                LDA XAMH
                SBC H
                BCS TONEXTITEM  ; Not less, so no more data to output.
                INC XAML
                BNE MOD8CHK     ; Increment 'examine index'.
                INC XAMH
MOD8CHK:        LDA XAML        ; Check low-order 'examine index' byte
                AND #$07        ;  For MOD 8= 0
                BPL NXTPRNT     ; Always taken.
PRBYTE:         PHA             ; Save A for LSD.
                LSR
                LSR
                LSR             ; MSD to LSD position.
                LSR
                JSR PRHEX       ; Output hex digit.
                PLA             ; Restore A.
PRHEX:          AND #$0F        ; Mask LSD for hex print.
                ORA #$B0        ; Add "0".
                CMP #$BA        ; Digit?
                BCC ECHO        ; Yes, output it.
                ADC #$06        ; Add offset for letter.
ECHO:           PHA             ; [Gigatron]
                BRK             ; [Gigatron] Transfer to vCPU for display.
                PLA             ; [Gigatron]
                RTS             ; [Gigatron] Return.
BLANK:          CLC             ; [Gigatron] Advance fewer pixels so BLOCK
                ADC $30         ; [Gigatron]   XAM doesn't wrap around
                STA $30         ; [Gigatron]   the 160 pixel wide screen.
                RTS             ; [Gigatron]

        .BYTE   $00             ;(unused)
        .BYTE   $00,$00         ;(NMI)
        .BYTE   $00,$FF         ;(RESET)
        .BYTE   $00,$00         ;(IRQ)

END:            ; Gigatron end of GT1 file footer
        .BYTE   0,>START,<START
