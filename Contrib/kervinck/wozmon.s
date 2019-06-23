;  The WOZ Monitor for the Apple 1
;  Written by Steve Wozniak in 1976

; Adapted for Gigatron TTL microcomputer

; XXX Map Delete to Rubout $5f

; wozmon.gt1x: wozmon.s
;        ca65 wozmon.s -o wozmon.o -l wozmon.lst
;        ld65 -t none -o wozmon.gt1x wozmon.o

; Page 0 Variables

buttonState     = $11           ; [Gigatron] Edge-triggered resettable input bits

XAML            = $36           ; Last "opened" location Low
XAMH            = $37           ; Last "opened" location High
STL             = $38           ; Store address Low
STH             = $39           ; Store address High
L               = $3A           ; Hex value parsing Low
H               = $3B           ; Hex value parsing High
YSAV            = $3C           ; Used to see if hex value is given
MODE            = $3D           ; $00=XAM, $7F=STOR, $AE=BLOCK XAM


; Other Variables

IN              = $0200         ;  Input buffer to $027F

                ; Gigatron vCPU startup and PrintChar/Newline loop (Apple1dsp.gcl)
START           = $0200
               .BYTE $02,$00,$18,$1A,$21,$E6,$28,$35,$4D,$0B,$21,$0E,$F3,$17,$90,$05
               .BYTE $11,$00,$06,$2B,$1A,$59,$8A,$2B,$30,$90,$4E,$02,$50,$A9,$EC,$FE
               .BYTE $11,$E1,$04,$2B,$22,$11,$00,$08,$2B,$24,$EE,$FE,$8C,$8A,$35,$3F
               .BYTE $6A,$1A,$30,$E6,$9B,$35,$56,$6A,$59,$00,$35,$72,$A9,$21,$24,$5E
               .BYTE $30,$99,$30,$35,$53,$78,$21,$24,$2B,$30,$2B,$28,$5E,$26,$B4,$CB
               .BYTE $93,$28,$1A,$28,$8C,$A0,$35,$72,$7E,$11,$EE,$01,$2B,$32,$21,$32
               .BYTE $AD,$E6,$78,$35,$53,$9A,$8C,$80,$90,$9C,$8C,$08,$F0,$32,$21,$32
               .BYTE $E6,$02,$2B,$32,$8C,$FE,$35,$72,$8E,$EE,$FE,$E6,$A0,$35,$50,$EE
               .BYTE $E6,$32,$35,$53,$BE,$E3,$32,$2B,$32,$11,$00,$07,$90,$C3,$2B,$32
               .BYTE $11,$00,$08,$2B,$34,$21,$32,$E9,$E9,$99,$32,$99,$34,$2B,$34,$21
               .BYTE $30,$2B,$28,$E3,$06,$2B,$30,$59,$05,$2B,$32,$21,$34,$7F,$00,$5E
               .BYTE $26,$B4,$CB,$93,$34,$93,$28,$21,$32,$E6,$01,$35,$4D,$D9,$11,$0C
               .BYTE $0B,$2B,$22,$B4,$E6,$90,$4E

                ; Gigatron GT1 file segment header for WozMon code
                .BYTE >RESET,<RESET,END-RESET

               .org $0600       ; [Gigatron] Original $ff00 appears in screen memory

RESET:          CLD             ; Clear decimal arithmetic mode.
                CLI
                LDX #0          ; [Gigatron]
                BEQ ESCAPE      ; [Gigatron] Always taken.
GETKBD:         LDA buttonState ; [Gigatron] Key ready?
                BMI GETKBD      ; [Gigatron] Loop until ready.
                DEX             ; [Gigatron] 255->X.
                STX buttonState ; [Gigatron] Mark all bits as read.
                INX             ; [Gigatron] 0->X.
                RTS             ; [Gigatron]
NOTCR:          CMP #$FF        ; [Gigatron] "<-"? Keyboard sends 127 (not "_").
                BEQ BACKSPACE   ; Yes.
                CMP #$9B        ; ESC?
                BEQ ESCAPE      ; Yes.
                INY             ; Advance text index.
                BPL NEXTCHAR    ; Auto ESC if > 127.
ESCAPE:         LDA #$DC        ; "\".
                JSR ECHO        ; Output it.
GETLINE:        LDA #$8A        ; [Gigatron] Keyboard sends LF (not CR).
                JSR ECHO        ; Output it.
                LDY #$01        ; Initiallize text index.
BACKSPACE:      DEY             ; Back up text index.
                BMI GETLINE     ; Beyond start of line, reinitialize.
NEXTCHAR:       JSR GETKBD      ; [Gigatron] Load character.
                ORA #$80        ; [Gigatron] B7 should be '1'.
                NOP             ; [Gigatron]
                NOP             ; [Gigatron]
                NOP             ; [Gigatron]
                STA IN,Y        ; Add to text buffer.
                JSR ECHO        ; Display character.
                CMP #$8A        ; [Gigatron] LF?
                BNE NOTCR       ; No.
                LDY #$FF        ; Reset text index.
                LDA #$00        ; For XAM mode.
                TAX             ; 0->X.
SETSTOR:        ASL             ; Leaves $7B if setting STOR mode.
SETMODE:        STA MODE        ; $00 = XAM, $7B= STOR,$AE= BLOK XAM
BLSKIP:         INY             ; Advance text index.
NEXTITEM:       LDA IN,Y        ; Get character.
                CMP #$8A        ; [Gigatron] LF?
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
                LDA #$8A        ; [Gigatron] LF.
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

END:            ; Gigatron end of GT1 file footer
                .BYTE 0,>START,<START
