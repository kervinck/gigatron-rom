;  The WOZ Monitor for the Apple 1
;  Written by Steve Wozniak in 1976

; Adapted for Gigatron TTL microcomputer

; wozmon.gt1x: wozmon.s
;        ca65 wozmon.s -o wozmon.o -l wozmon.lst
;        ld65 -t none -o wozmon.gt1x wozmon.o

; Page 0 Variables

buttonState     = $11           ; [Gigatron] Edge-triggered resettable input bits

XAML            = $44           ; Last "opened" location Low
XAMH            = $45           ; Last "opened" location High
STL             = $46           ; Store address Low
STH             = $47           ; Store address High
L               = $48           ; Hex value parsing Low
H               = $49           ; Hex value parsing High
YSAV            = $4A           ; Used to see if hex value is given
MODE            = $4B           ; $00=XAM, $7F=STOR, $AE=BLOCK XAM


; Other Variables

IN              = $0200         ;  Input buffer to $027F

                ; Gigatron vCPU startup and PrintChar/Newline loop (Apple1dsp.gcl)
START           = $0200

                ; Setup
                .BYTE $02,$00,$1F
                .BYTE $1A,$21,$E6,$28,$35,$4D,$0B,$21,$0E,$F3,$17,$90,$05,$11,$00,$03
                .BYTE $2B,$30,$59,$8D,$2B,$32,$CF,$30,$11,$00,$06,$2B,$1A,$90,$7E

                ; Main loop
                .BYTE $02,$80,$45
                .BYTE $11,$0C,$0B,$2B,$22,$21,$34,$B4,$E6,$75,$35,$72,$BC,$59,$C0,$CF
                .BYTE $30,$2B,$32,$1A,$11,$2B,$34,$82,$80,$35,$72,$91,$59,$A0,$CF,$30
                .BYTE $2B,$32,$59,$FF,$5E,$11,$21,$34,$E6,$60,$35,$50,$AF,$E3,$40,$2B
                .BYTE $34,$21,$34,$8C,$0A,$35,$72,$BA,$59,$0D,$2B,$34,$90,$C0,$2B,$34
                .BYTE $CF,$30,$63,$90,$7E

                ; PrintChar
                .BYTE $03,$00,$a5
                .BYTE $EC,$FE,$11,$E1,$04,$2B,$22,$11,$00,$08,$2B,$24,$EE,$FE,$8C,$8D
                .BYTE $35,$3F,$1A,$1A,$32,$E6,$9B,$35,$56,$1A,$59,$00,$35,$72,$59,$21
                .BYTE $24,$5E,$32,$99,$32,$35,$53,$28,$21,$24,$2B,$32,$2B,$28,$5E,$26
                .BYTE $B4,$CB,$93,$28,$1A,$28,$8C,$A0,$35,$72,$2E,$11,$EE,$01,$2B,$36
                .BYTE $21,$36,$AD,$E6,$78,$35,$53,$4A,$8C,$80,$90,$4C,$8C,$08,$F0,$36
                .BYTE $21,$36,$E6,$02,$2B,$36,$8C,$FE,$35,$72,$3E,$EE,$FE,$E6,$A0,$35
                .BYTE $50,$A2,$E6,$32,$35,$53,$6E,$E3,$32,$2B,$36,$11,$00,$07,$90,$73
                .BYTE $2B,$36,$11,$00,$08,$2B,$38,$21,$36,$E9,$E9,$99,$36,$99,$38,$2B
                .BYTE $38,$21,$32,$2B,$28,$EC,$FE,$E3,$06,$2B,$32,$59,$05,$2B,$36,$21
                .BYTE $38,$7F,$00,$5E,$26,$B4,$CB,$93,$38,$93,$28,$21,$36,$E6,$01,$35
                .BYTE $4D,$8B,$EE,$FE,$FF

                ; Gigatron GT1 file segment header for WozMon code
                .BYTE >RESET,<RESET,(END-RESET)&255

               .org $0600       ; [Gigatron] Original $ff00 appears in screen memory
RESET:          CLD             ; Clear decimal arithmetic mode.
                CLI
                LDY #$7F        ; Mask for DSP data direction register.
                NOP             ; [Gigatron]
                NOP             ; [Gigatron]
                NOP             ; [Gigatron]
                LDA #$A7        ; KBD and DSP control register mask.
                NOP             ; [Gigatron]
                NOP             ; [Gigatron]
                NOP             ; [Gigatron]
                NOP             ; [Gigatron]
                NOP             ; [Gigatron]
                NOP             ; [Gigatron]
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
                NOP             ; [Gigatron]
                NOP             ; [Gigatron]
                NOP             ; [Gigatron]
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
                ADC $32         ; [Gigatron]   XAM doesn't wrap around
                STA $32         ; [Gigatron]   the 160 pixel wide screen.
                RTS             ; [Gigatron]

                .BYTE $00       ; (unused)
                .BYTE $00,$00   ; (NMI)
                .BYTE $00,$06   ; (RESET)
                .BYTE $00,$00   ; (IRQ)

END:            ; Gigatron end of GT1 file footer
                .BYTE 0,>START,<START
