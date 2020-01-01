;
; "Monitor II on Apple-1" as found on Winston D. Gayler's tapes
; http://www.apple1notes.com/old_apple/Monitor_II_on_1.html
;
; """As a part of the material in the Gayler collection was a program
;    tape that contained a file of the Apple II monitor ported to the
;    Apple 1 that he had apparently made complete with Mini-Assembler.
;    This was not a transitional step from the Apple 1 like the Monitor
;    file by Steve and Allen but a port of the completed Apple II monitor
;    including the mini-assembler.  The file was located at E500 to EFFF
;    which makes sense for the basic Apple 1 but prevents use of Basic."""
;
; 2020-01-01 (Marcel van Kervinck)
;       Disassembed using x65dsasm
;       Retargeted for address $500.$fff and created Mon2on1.ref
;       Made compatible with ca65. Build instructions:
;               ca65 Mon2on1.s
;               ld65 -t none Mon2on1.o -o Mon2on1.bin
;               cmp Mon2on1.bin Mon2on1.ref

zp_1 = $00
zp_2 = $01
zp_3 = $20
zp_4 = $21
zp_5 = $22
zp_6 = $23
WozXAML = $24
WozXAMH = $25
WozSTL = $26
WozSTH = $27
WozL = $28
WozH = $29
WozYSAV = $2a
WozMODE = $2b
zp_15 = $2c
zp_16 = $2d
zp_17 = $2e
zp_18 = $2f
zp_19 = $30
zp_20 = $31
zp_21 = $32
zp_22 = $33
zp_23 = $34
zp_24 = $35
zp_25 = $3a
zp_26 = $3b
                ; Referenced from $0a7a (data)
                ; Referenced from $0a83 (jump)
Code_27 = $3c
zp_28 = $3d
zp_29 = $3e
zp_30 = $3f
zp_31 = $40
zp_32 = $41
zp_33 = $42
zp_34 = $43
zp_35 = $44
zp_36 = $45
zp_37 = $46
zp_38 = $47
zp_39 = $48
zp_40 = $49
zp_41 = $4a
zp_42 = $4e
zp_43 = $4f
zp_44 = $50
zp_45 = $54
zp_46 = $56
zp_47 = $a2
zp_48 = $d8
zp_49 = $ff
                ; Referenced from $059f (data)
                ; Referenced from $0626 (data)
                ; Referenced from $0634 (data)
                ; Referenced from $0d84 (data)
                ; Referenced from $0fad (data)
Ext_50 = $200
                ; Referenced from $0a8f (data)
Ext_51 = $3fe

        .localchar '@'

;-----------------------------------------------------------------------
        .org $500
;-----------------------------------------------------------------------
                ; Referenced from $0576 (branch)
Code_52: ; $0500
        sbc #$81                        ;$0500 e9 81
        lsr                             ;$0502 4a
        bne @L3                         ;$0503 d0 14
        ldy $3f ; zp_30                 ;$0505 a4 3f
        ldx $3e ; zp_29                 ;$0507 a6 3e
        bne @L1                         ;$0509 d0 01
        dey                             ;$050b 88
                ; Referenced from $0509 (branch)
@L1: ; $050c
        dex                             ;$050c ca
        txa                             ;$050d 8a
        clc                             ;$050e 18
        sbc $3a ; zp_25                 ;$050f e5 3a
        sta $3e ; zp_29                 ;$0511 85 3e
        bpl @L2                         ;$0513 10 01
        iny                             ;$0515 c8
                ; Referenced from $0513 (branch)
@L2: ; $0516
        tya                             ;$0516 98
        sbc $3b ; zp_26                 ;$0517 e5 3b
                ; Referenced from $0503 (branch)
@L3: ; $0519
        bne $0586 ; Code_57             ;$0519 d0 6b
                ; Referenced from $057a (branch)
Code_53: ; $051b
        ldy $2f ; zp_18                 ;$051b a4 2f
                ; Referenced from $0523 (branch)
@L1: ; $051d
        lda $003d,y ; zp_28             ;$051d b9 3d 00
        sta ($3a),y ; zp_25             ;$0520 91 3a
        dey                             ;$0522 88
        bpl @L1                         ;$0523 10 f8
        nop                             ;$0525 ea
        nop                             ;$0526 ea
        nop                             ;$0527 ea
        nop                             ;$0528 ea
        nop                             ;$0529 ea
        nop                             ;$052a ea
        jsr $08d0 ; Code_79             ;$052b 20 d0 08
        jsr $0953 ; Code_85             ;$052e 20 53 09
        sty $3b ; zp_26                 ;$0531 84 3b
        sta $3a ; zp_25                 ;$0533 85 3a
        jmp $0595 ; Code_60             ;$0535 4c 95 05

                ; ------------- $0538 ------------- ;

                ; Referenced from $054e (branch)
Code_54: ; $0538
        jsr $0fbe ; Code_168            ;$0538 20 be 0f
        ldy $34 ; zp_23                 ;$053b a4 34
                ; Referenced from $05a9 (branch)
Code_55: ; $053d
        jsr $0fa7 ; Code_166            ;$053d 20 a7 0f
        sty $34 ; zp_23                 ;$0540 84 34
        ldy #$17                        ;$0542 a0 17
                ; Referenced from $054a (branch)
@L1: ; $0544
        dey                             ;$0544 88
        bmi $0592 ; Code_59             ;$0545 30 4b
        cmp $0fcc,y ; Data_170          ;$0547 d9 cc 0f
        bne @L1                         ;$054a d0 f8
        cpy #$15                        ;$054c c0 15
        bne $0538 ; Code_54             ;$054e d0 e8
        lda $31 ; zp_20                 ;$0550 a5 31
        ldy #$00                        ;$0552 a0 00
        dec $34 ; zp_23                 ;$0554 c6 34
        jsr $0e00 ; Code_151            ;$0556 20 00 0e
        jmp $0595 ; Code_60             ;$0559 4c 95 05

                ; ------------- $055c ------------- ;

                ; Referenced from $057e (branch)
                ; Referenced from $0584 (branch)
                ; Referenced from $0631 (jump)
Code_56: ; $055c
        lda $3d ; zp_28                 ;$055c a5 3d
        jsr $066b ; Code_63             ;$055e 20 6b 06
        tax                             ;$0561 aa
        lda $0a00,x ; Data_95           ;$0562 bd 00 0a
        cmp $42 ; zp_33                 ;$0565 c5 42
        bne @L1                         ;$0567 d0 13
        lda $09c0,x ; Data_94           ;$0569 bd c0 09
        cmp $43 ; zp_34                 ;$056c c5 43
        bne @L1                         ;$056e d0 0c
        lda $44 ; zp_35                 ;$0570 a5 44
        ldy $2e ; zp_17                 ;$0572 a4 2e
        cpy #$9d                        ;$0574 c0 9d
        beq $0500 ; Code_52             ;$0576 f0 88
        cmp $2e ; zp_17                 ;$0578 c5 2e
        beq $051b ; Code_53             ;$057a f0 9f
                ; Referenced from $0567 (branch)
                ; Referenced from $056e (branch)
@L1: ; $057c
        dec $3d ; zp_28                 ;$057c c6 3d
        bne $055c ; Code_56             ;$057e d0 dc
        inc $44 ; zp_35                 ;$0580 e6 44
        dec $35 ; zp_24                 ;$0582 c6 35
        beq $055c ; Code_56             ;$0584 f0 d6
                ; Referenced from $0519 (branch)
Code_57: ; $0586
        ldy $34 ; zp_23                 ;$0586 a4 34
                ; Referenced from $05b1 (branch)
                ; Referenced from $05b4 (branch)
                ; Referenced from $05c5 (branch)
Code_58: ; $0588
        tya                             ;$0588 98
        tax                             ;$0589 aa
        jsr $094a ; Code_84             ;$058a 20 4a 09
        lda #$de                        ;$058d a9 de
        jsr $0ded ; Code_150            ;$058f 20 ed 0d
                ; Referenced from $0545 (branch)
Code_59: ; $0592
        jsr $0f3a ; Code_160            ;$0592 20 3a 0f
                ; Referenced from $0535 (jump)
                ; Referenced from $0559 (jump)
Code_60: ; $0595
        lda #$a1                        ;$0595 a9 a1
        sta $33 ; zp_22                 ;$0597 85 33
        jsr $0d67 ; Code_143            ;$0599 20 67 0d
        jsr $0fc7 ; Code_169            ;$059c 20 c7 0f
        lda $0200 ; Ext_50              ;$059f ad 00 02
        cmp #$a0                        ;$05a2 c9 a0
        beq @L2                         ;$05a4 f0 13
        iny                             ;$05a6 c8
        cmp #$a4                        ;$05a7 c9 a4
        beq $053d ; Code_55             ;$05a9 f0 92
        dey                             ;$05ab 88
        jsr $0fa7 ; Code_166            ;$05ac 20 a7 0f
        cmp #$93                        ;$05af c9 93
                ; Referenced from $062f (branch)
@L1: ; $05b1
        bne $0588 ; Code_58             ;$05b1 d0 d5
        txa                             ;$05b3 8a
        beq $0588 ; Code_58             ;$05b4 f0 d2
        jsr $0e78 ; Code_156            ;$05b6 20 78 0e
                ; Referenced from $05a4 (branch)
@L2: ; $05b9
        lda #$03                        ;$05b9 a9 03
        sta $3d ; zp_28                 ;$05bb 85 3d
                ; Referenced from $05d7 (branch)
@L3: ; $05bd
        jsr $0634 ; Code_61             ;$05bd 20 34 06
        asl                             ;$05c0 0a
        sbc #$be                        ;$05c1 e9 be
        cmp #$c2                        ;$05c3 c9 c2
        bcc $0588 ; Code_58             ;$05c5 90 c1
        asl                             ;$05c7 0a
        asl                             ;$05c8 0a
        ldx #$04                        ;$05c9 a2 04
                ; Referenced from $05d1 (branch)
                ; Referenced from $05d5 (branch)
@L4: ; $05cb
        asl                             ;$05cb 0a
        rol $42 ; zp_33                 ;$05cc 26 42
        rol $43 ; zp_34                 ;$05ce 26 43
        dex                             ;$05d0 ca
        bpl @L4                         ;$05d1 10 f8
        dec $3d ; zp_28                 ;$05d3 c6 3d
        beq @L4                         ;$05d5 f0 f4
        bpl @L3                         ;$05d7 10 e4
        ldx #$05                        ;$05d9 a2 05
                ; Referenced from $0610 (branch)
@L5: ; $05db
        jsr $0634 ; Code_61             ;$05db 20 34 06
        sty $34 ; zp_23                 ;$05de 84 34
        cmp $09b4,x ; Data_91           ;$05e0 dd b4 09
        bne @L6                         ;$05e3 d0 13
        jsr $0634 ; Code_61             ;$05e5 20 34 06
        cmp $09ba,x ; Data_93           ;$05e8 dd ba 09
        beq @L8                         ;$05eb f0 0d
        lda $09ba,x ; Data_93           ;$05ed bd ba 09
        beq @L7                         ;$05f0 f0 07
        cmp #$a4                        ;$05f2 c9 a4
        beq @L7                         ;$05f4 f0 03
        ldy $34 ; zp_23                 ;$05f6 a4 34
                ; Referenced from $05e3 (branch)
@L6: ; $05f8
        clc                             ;$05f8 18
                ; Referenced from $05f0 (branch)
                ; Referenced from $05f4 (branch)
@L7: ; $05f9
        dey                             ;$05f9 88
                ; Referenced from $05eb (branch)
@L8: ; $05fa
        rol $44 ; zp_35                 ;$05fa 26 44
        cpx #$03                        ;$05fc e0 03
        bne @L10                        ;$05fe d0 0d
        jsr $0fa7 ; Code_166            ;$0600 20 a7 0f
        lda $3f ; zp_30                 ;$0603 a5 3f
        beq @L9                         ;$0605 f0 01
        inx                             ;$0607 e8
                ; Referenced from $0605 (branch)
@L9: ; $0608
        stx $35 ; zp_24                 ;$0608 86 35
        ldx #$03                        ;$060a a2 03
        dey                             ;$060c 88
                ; Referenced from $05fe (branch)
@L10: ; $060d
        stx $3d ; zp_28                 ;$060d 86 3d
        dex                             ;$060f ca
        bpl @L5                         ;$0610 10 c9
        lda $44 ; zp_35                 ;$0612 a5 44
        asl                             ;$0614 0a
        asl                             ;$0615 0a
        ora $35 ; zp_24                 ;$0616 05 35
        cmp #$20                        ;$0618 c9 20
        bcs @L11                        ;$061a b0 06
        ldx $35 ; zp_24                 ;$061c a6 35
        beq @L11                        ;$061e f0 02
        ora #$80                        ;$0620 09 80
                ; Referenced from $061a (branch)
                ; Referenced from $061e (branch)
@L11: ; $0622
        sta $44 ; zp_35                 ;$0622 85 44
        sty $34 ; zp_23                 ;$0624 84 34
        lda $0200,y ; Ext_50            ;$0626 b9 00 02
        cmp #$bb                        ;$0629 c9 bb
        beq @L12                        ;$062b f0 04
        cmp #$8d                        ;$062d c9 8d
        bne @L1                         ;$062f d0 80
                ; Referenced from $062b (branch)
@L12: ; $0631
        jmp $055c ; Code_56             ;$0631 4c 5c 05

                ; ------------- $0634 ------------- ;

                ; Referenced from $05bd (subroutine)
                ; Referenced from $05db (subroutine)
                ; Referenced from $05e5 (subroutine)
                ; Referenced from $063a (branch)
Code_61: ; $0634
        lda $0200,y ; Ext_50            ;$0634 b9 00 02
        iny                             ;$0637 c8
        cmp #$a0                        ;$0638 c9 a0
        beq $0634 ; Code_61             ;$063a f0 f8
        rts                             ;$063c 60

                ; ------------- $063d ------------- ;

        .byte $ff ; $063d isc           ;$063d ff
        .byte $ff ; $063e isc           ;$063e ff
        .byte $ff ; $063f isc           ;$063f ff
        .byte $ff ; $0640 isc           ;$0640 ff
        .byte $ff ; $0641 isc           ;$0641 ff
        .byte $ff ; $0642 isc           ;$0642 ff
        .byte $ff ; $0643 isc           ;$0643 ff
        .byte $ff ; $0644 isc           ;$0644 ff
        .byte $ff ; $0645 isc           ;$0645 ff
        .byte $ff ; $0646 isc           ;$0646 ff
        .byte $ff ; $0647 isc           ;$0647 ff
        .byte $ff ; $0648 isc           ;$0648 ff
        .byte $ff ; $0649 isc           ;$0649 ff
        .byte $ff ; $064a isc           ;$064a ff
        .byte $ff ; $064b isc           ;$064b ff
        .byte $ff ; $064c isc           ;$064c ff
        .byte $ff ; $064d isc           ;$064d ff
        .byte $ff ; $064e isc           ;$064e ff
        .byte $ff ; $064f isc           ;$064f ff
        .byte $ff ; $0650 isc           ;$0650 ff
        .byte $ff ; $0651 isc           ;$0651 ff
        .byte $ff ; $0652 isc           ;$0652 ff
        .byte $ff ; $0653 isc           ;$0653 ff
        .byte $ff ; $0654 isc           ;$0654 ff
        .byte $ff ; $0655 isc           ;$0655 ff
        .byte $ff ; $0656 isc           ;$0656 ff
        .byte $ff ; $0657 isc           ;$0657 ff
        .byte $ff ; $0658 isc           ;$0658 ff
        .byte $ff ; $0659 isc           ;$0659 ff
        .byte $ff ; $065a isc           ;$065a ff
        .byte $ff ; $065b isc           ;$065b ff
        .byte $ff ; $065c isc           ;$065c ff
        .byte $ff ; $065d isc           ;$065d ff
        lda ($3a,x) ; zp_25             ;$065e a1 3a
        tay                             ;$0660 a8
        lsr                             ;$0661 4a
        bcc $066a ; Code_62 + $1        ;$0662 90 06
        lsr                             ;$0664 4a
        ora #$4c                        ;$0665 09 4c
        .byte $92 ; $0667 ???           ;$0667 92
        ora $a1                         ;$0668 05 a1
                ; Referenced from $088c (jump)
Code_62 = * - 1 ; $669
        .byte $3a ; $066a ???           ;$066a 3a
                ; Referenced from $055e (subroutine)
Code_63: ; $066b
        tay                             ;$066b a8
        lsr                             ;$066c 4a
        bcc $0675 ; Code_64             ;$066d 90 06
        lsr                             ;$066f 4a
        ora #$80                        ;$0670 09 80
        jmp $0893 ; Code_75             ;$0672 4c 93 08
                ; Referenced from $066d (branch)
Code_64: ; $0675
        jmp $089b ; Code_76             ;$0675 4c 9b 08

                ; ------------- $0678 ------------- ;

        lda #$7f                        ;$0678 a9 7f
        sta $d012 ; Ext_185             ;$067a 8d 12 d0
        ldx #$a7                        ;$067d a2 a7
        stx $d011 ; Ext_184             ;$067f 8e 11 d0
        stx $d013 ; Ext_186             ;$0682 8e 13 d0
        ldx #$8b                        ;$0685 a2 8b
        stx $d004 ; Ext_181             ;$0687 8e 04 d0
        ldx #$96                        ;$068a a2 96
        stx $d005 ; Ext_182             ;$068c 8e 05 d0
        rts                             ;$068f 60

                ; ------------- $0690 ------------- ;

                ; Referenced from $0693 (branch)
                ; Referenced from $0ded (subroutine)
Code_65: ; $0690
        bit $d012 ; Ext_185             ;$0690 2c 12 d0
        bmi $0690 ; Code_65             ;$0693 30 fb
        sta $d012 ; Ext_185             ;$0695 8d 12 d0
        rts                             ;$0698 60

                ; ------------- $0699 ------------- ;

                ; Referenced from $0d2b (jump)
Code_66: ; $0699
        cmp #$9b                        ;$0699 c9 9b
        bne @L1                         ;$069b d0 02
        lda #$98                        ;$069d a9 98
                ; Referenced from $069b (branch)
@L1: ; $069f
        cmp #$88                        ;$069f c9 88
        bne @L2                         ;$06a1 d0 02
        lda #$df                        ;$06a3 a9 df
                ; Referenced from $06a1 (branch)
@L2: ; $06a5
        rts                             ;$06a5 60

                ; ------------- $06a6 ------------- ;

        brk                             ;$06a6 00

                ; ------------- $06a7 ------------- ;

        and ($ff,x) ; zp_49             ;$06a7 21 ff
        .byte $82 ; $06a9 ???           ;$06a9 82
        brk                             ;$06aa 00

                ; ------------- $06ab ------------- ;

        brk                             ;$06ab 00

                ; ------------- $06ac ------------- ;

        eor $914d,y                     ;$06ac 59 4d 91
        .byte $92 ; $06af ???           ;$06af 92
        stx $4a ; zp_41                 ;$06b0 86 4a
        sta $9d                         ;$06b2 85 9d
        ldy $aca9                       ;$06b4 ac a9 ac
        .byte $a3 ; $06b7 lax           ;$06b7 a3
        tay                             ;$06b8 a8
        ldy $d9                         ;$06b9 a4 d9
        brk                             ;$06bb 00

                ; ------------- $06bc ------------- ;

        cld                             ;$06bc d8
        ldy $a4                         ;$06bd a4 a4
        brk                             ;$06bf 00

                ; ------------- $06c0 ------------- ;

        .byte $1c ; $06c0 ???           ;$06c0 1c
        txa                             ;$06c1 8a
        .byte $1c ; $06c2 ???           ;$06c2 1c
        .byte $23 ; $06c3 rla           ;$06c3 23
        eor $1b8b,x                     ;$06c4 5d 8b 1b
        .byte $ff ; $06c7 isc           ;$06c7 ff
        sta $1d8a,x                     ;$06c8 9d 8a 1d
        .byte $23 ; $06cb rla           ;$06cb 23
        sta $1d8b,x                     ;$06cc 9d 8b 1d
        lda ($00,x) ; zp_1              ;$06cf a1 00
        and #$19                        ;$06d1 29 19
        ldx $a869                       ;$06d3 ae 69 a8
        ora $2423,y                     ;$06d6 19 23 24
        .byte $53 ; $06d9 sre           ;$06d9 53
        .byte $1b ; $06da slo           ;$06da 1b
        .byte $23 ; $06db rla           ;$06db 23
        bit $53                         ;$06dc 24 53
        ora $ffff,y                     ;$06de 19 ff ff
        .byte $1a ; $06e1 ???           ;$06e1 1a
        .byte $5b ; $06e2 sre           ;$06e2 5b
        .byte $5b ; $06e3 sre           ;$06e3 5b
        lda $69                         ;$06e4 a5 69
        bit $24 ; WozXAML               ;$06e6 24 24
        .byte $ff ; $06e8 isc           ;$06e8 ff
        ldx $ada8                       ;$06e9 ae a8 ad
        and #$00                        ;$06ec 29 00
        .byte $7c ; $06ee ???           ;$06ee 7c

                ; ------------- $06ef ------------- ;

        brk                             ;$06ef 00

                ; ------------- $06f0 ------------- ;

        ora $9c,x                       ;$06f0 15 9c
        adc $a59c                       ;$06f2 6d 9c a5
        adc #$29                        ;$06f5 69 29
        .byte $53 ; $06f7 sre           ;$06f7 53
        sty $13                         ;$06f8 84 13
        .byte $34 ; $06fa ???           ;$06fa 34
        ora ($a5),y                     ;$06fb 11 a5
        adc #$23                        ;$06fd 69 23
        ldy #$d8                        ;$06ff a0 d8
        .byte $62 ; $0701 ???           ;$0701 62
        .byte $5a ; $0702 ???           ;$0702 5a
        pha                             ;$0703 48
        rol $62                         ;$0704 26 62
        sty $ff,x ; zp_49               ;$0706 94 ff
        .byte $54 ; $0708 ???           ;$0708 54
        .byte $44 ; $0709 ???           ;$0709 44
        iny                             ;$070a c8
        .byte $54 ; $070b ???           ;$070b 54
        pla                             ;$070c 68
        .byte $44 ; $070d ???           ;$070d 44
        inx                             ;$070e e8
        sty $00,x ; zp_1                ;$070f 94 00
        ldy $08,x                       ;$0711 b4 08
        sty $74                         ;$0713 84 74
        ldy $28,x ; WozL                ;$0715 b4 28
        ror $f474                       ;$0717 6e 74 f4
        cpy $724a                       ;$071a cc 4a 72
        .byte $f2 ; $071d ???           ;$071d f2
        ldy $ff ; zp_49                 ;$071e a4 ff
        .byte $ff ; $0720 isc           ;$0720 ff
        tax                             ;$0721 aa
        ldx #$a2                        ;$0722 a2 a2
        .byte $74 ; $0724 ???           ;$0724 74
        .byte $74 ; $0725 ???           ;$0725 74
        .byte $74 ; $0726 ???           ;$0726 74
        .byte $72 ; $0727 ???           ;$0727 72
        .byte $ff ; $0728 isc           ;$0728 ff
        pla                             ;$0729 68
        .byte $b2 ; $072a ???           ;$072a b2
        .byte $32 ; $072b ???           ;$072b 32
        .byte $b2 ; $072c ???           ;$072c b2
        brk                             ;$072d 00

                ; ------------- $072e ------------- ;

        .byte $22 ; $072e ???           ;$072e 22
        brk                             ;$072f 00

                ; ------------- $0730 ------------- ;

        .byte $1a ; $0730 ???           ;$0730 1a
        .byte $1a ; $0731 ???           ;$0731 1a
        rol $26 ; WozSTL                ;$0732 26 26
        .byte $72 ; $0734 ???           ;$0734 72
        .byte $72 ; $0735 ???           ;$0735 72
        dey                             ;$0736 88
        iny                             ;$0737 c8
        cpy $ca                         ;$0738 c4 ca
        rol $48 ; zp_39                 ;$073a 26 48
        .byte $44 ; $073c ???           ;$073c 44
        .byte $44 ; $073d ???           ;$073d 44
        ldx #$c8                        ;$073e a2 c8
        .byte $ff ; $0740 isc           ;$0740 ff
        .byte $ff ; $0741 isc           ;$0741 ff
        .byte $ff ; $0742 isc           ;$0742 ff
        jsr $b8d0                       ;$0743 20 d0 b8
        pla                             ;$0746 68
        .byte $ff ; $0747 isc           ;$0747 ff
        bit $8568                       ;$0748 2c 68 85
        and $08a2 ; Code_76 + $7        ;$074b 2d a2 08
        lda $eb10,x                     ;$074e bd 10 eb
        sta $3c,x ; Code_27             ;$0751 95 3c
        dex                             ;$0753 ca
        bne @L2 + $a9                   ;$0754 d0 f8
        lda ($3a,x) ; zp_25             ;$0756 a1 3a
        beq $079c ; Data_67 + $a        ;$0758 f0 42
        ldy $2f ; zp_18                 ;$075a a4 2f
        cmp #$20                        ;$075c c9 20
        beq @L2 + $ba                   ;$075e f0 ff
        .byte $ff ; $0760 isc           ;$0760 ff
        rts                             ;$0761 60

                ; ------------- $0762 ------------- ;

        beq $07a9 ; Data_67 + $17       ;$0762 f0 45
        cmp #$4c                        ;$0764 c9 4c
        beq L07C4                       ;$0766 f0 5c
        .byte $ff ; $0768 isc           ;$0768 ff
        jmp ($59f0)                     ;$0769 6c f0 59

                ; ------------- $076c ------------- ;

        cmp #$40                        ;$076c c9 40
        beq $07a5 ; Data_67 + $13       ;$076e f0 35
        cmp #$1f                        ;$0770 c9 1f
        eor #$14                        ;$0772 49 14
        cmp #$04                        ;$0774 c9 04
        beq @L2 + $d5                   ;$0776 f0 02
        lda ($3a),y ; zp_25             ;$0778 b1 3a
        sta $003c,y ; Code_27           ;$077a 99 3c 00
        dey                             ;$077d 88
        bpl @L2 + $d3                   ;$077e 10 f8
        jsr $ef3f                       ;$0780 20 3f ef
        jmp $003c ; Code_27             ;$0783 4c 3c 00

                ; ------------- $0786 ------------- ;

        sta $ff ; zp_49                 ;$0786 85 ff
        pla                             ;$0788 68
        pha                             ;$0789 48
        asl                             ;$078a 0a
        asl                             ;$078b 0a
        asl                             ;$078c 0a
        bmi $0792 ; Data_67             ;$078d 30 03
        jmp ($03fe) ; Ext_51            ;$078f 6c fe 03

                ; ------------- $0792 ------------- ;

Data_67: ; $0792
        plp                             ;$0792 28
        jsr $ef4c                       ;$0793 20 4c ef
        pla                             ;$0796 68
        sta $3a ; zp_25                 ;$0797 85 3a
        pla                             ;$0799 68
        sta $3b ; zp_26                 ;$079a 85 3b
        jsr $e882 ; Code_188            ;$079c 20 82 e8
        .byte $ff ; $079f isc           ;$079f ff
        .byte $ff ; $07a0 isc           ;$07a0 ff
        nop                             ;$07a1 ea
        jmp $ef65 ; Code_195            ;$07a2 4c 65 ef

                ; ------------- $07a5 ------------- ;

        clc                             ;$07a5 18
        pla                             ;$07a6 68
        sta $ff ; zp_49                 ;$07a7 85 ff
        pla                             ;$07a9 68
        sta $3a ; zp_25                 ;$07aa 85 3a
        pla                             ;$07ac 68
        sta $3b ; zp_26                 ;$07ad 85 3b
        lda $2f ; zp_18                 ;$07af a5 2f
        jsr $e956 ; Code_190            ;$07b1 20 56 e9
        sty $3b ; zp_26                 ;$07b4 84 3b
        clc                             ;$07b6 18
        bcc L07CD                       ;$07b7 90 14
        clc                             ;$07b9 18
        jsr $e954 ; Code_189            ;$07ba 20 54 e9
        tax                             ;$07bd aa
        tya                             ;$07be 98
        pha                             ;$07bf 48
        txa                             ;$07c0 8a
        pha                             ;$07c1 48
        ldy #$02                        ;$07c2 a0 02

                ; ------------- $07c4 ------------- ;

L07C4:
        clc                             ;$07c4 18
        lda ($3a),y ; zp_25             ;$07c5 b1 3a
        .byte $ff ; $07c7 isc           ;$07c7 ff
        dey                             ;$07c8 88
        lda ($3a),y ; zp_25             ;$07c9 b1 3a
        stx $3b ; zp_26                 ;$07cb 86 3b
L07CD:
        sta $3a ; zp_25                 ;$07cd 85 3a
        bcs L07C4                       ;$07cf b0 f3
        lda $2d ; zp_16                 ;$07d1 a5 2d
        pha                             ;$07d3 48
        lda $2c ; zp_15                 ;$07d4 a5 2c
        pha                             ;$07d6 48
        jsr $ed8e ; Code_192            ;$07d7 20 8e ed
        lda #$45                        ;$07da a9 45
        sta $40 ; zp_31                 ;$07dc 85 40
        lda #$ff                        ;$07de a9 ff
        .byte $ff ; $07e0 isc           ;$07e0 ff
        eor ($a2,x) ; zp_47             ;$07e1 41 a2
        .byte $fb ; $07e3 isc           ;$07e3 fb
                ; Referenced from $07fa (branch)
@L3: ; $07e4
        lda #$a0                        ;$07e4 a9 a0
        jsr $ffed ; Code_196            ;$07e6 20 ed ff
        lda $ea1a,x ; Code_191          ;$07e9 bd 1a ea
        jsr $eded ; Code_194            ;$07ec 20 ed ed
        lda #$bd                        ;$07ef a9 bd
        jsr $eded ; Code_194            ;$07f1 20 ed ed
        lda $4a,x ; zp_41               ;$07f4 b5 4a
        jsr $edda ; Code_193            ;$07f6 20 da ed
        inx                             ;$07f9 e8
        bmi @L3                         ;$07fa 30 e8
        rts                             ;$07fc 60

                ; ------------- $07fd ------------- ;

        clc                             ;$07fd 18
        ldy #$01                        ;$07fe a0 01
                ; Referenced from $0829 (subroutine)
Code_68: ; $0800
        lsr                             ;$0800 4a
        php                             ;$0801 08
        jsr $0847 ; Code_72             ;$0802 20 47 08
        plp                             ;$0805 28
        lda #$0f                        ;$0806 a9 0f
        bcc @L1                         ;$0808 90 02
        adc #$e0                        ;$080a 69 e0
                ; Referenced from $0808 (branch)
@L1: ; $080c
        sta $2e ; zp_17                 ;$080c 85 2e
        lda ($26),y ; WozSTL            ;$080e b1 26
        eor $30 ; zp_19                 ;$0810 45 30
        and $2e ; zp_17                 ;$0812 25 2e
        eor ($26),y ; WozSTL            ;$0814 51 26
        sta ($26),y ; WozSTL            ;$0816 91 26
        rts                             ;$0818 60

                ; ------------- $0819 ------------- ;

        jsr $0800 ; Code_68             ;$0819 20 00 08
        cpy $2c ; zp_15                 ;$081c c4 2c
        bcs $0831 ; Code_70 + $9        ;$081e b0 11
        iny                             ;$0820 c8
        jsr @L1 + $2                    ;$0821 20 0e 08
        bcc @L1 + $10                   ;$0824 90 f6
                ; Referenced from $082f (branch)
Code_69: ; $0826
        adc #$01                        ;$0826 69 01
                ; Referenced from $0840 (subroutine)
Code_70: ; $0828
        pha                             ;$0828 48
        jsr $0800 ; Code_68             ;$0829 20 00 08
        pla                             ;$082c 68
        cmp $2d ; zp_16                 ;$082d c5 2d
        bcc $0826 ; Code_69             ;$082f 90 f5
        rts                             ;$0831 60

                ; ------------- $0832 ------------- ;

        ldy #$2f                        ;$0832 a0 2f
        bne $0838 ; Code_71 + $2        ;$0834 d0 02
                ; Referenced from $0b46 (subroutine)
Code_71: ; $0836
        ldy #$27                        ;$0836 a0 27
        sty $2d ; zp_16                 ;$0838 84 2d
        ldy #$27                        ;$083a a0 27
                ; Referenced from $0844 (branch)
@L1: ; $083c
        lda #$00                        ;$083c a9 00
        sta $30 ; zp_19                 ;$083e 85 30
        jsr $0828 ; Code_70             ;$0840 20 28 08
        dey                             ;$0843 88
        bpl @L1                         ;$0844 10 f6
        rts                             ;$0846 60

                ; ------------- $0847 ------------- ;

                ; Referenced from $0802 (subroutine)
Code_72: ; $0847
        pha                             ;$0847 48
        lsr                             ;$0848 4a
        and #$03                        ;$0849 29 03
        ora #$04                        ;$084b 09 04
        sta $27 ; WozSTH                ;$084d 85 27
        pla                             ;$084f 68
        and #$18                        ;$0850 29 18
        bcc @L1                         ;$0852 90 02
        adc #$7f                        ;$0854 69 7f
                ; Referenced from $0852 (branch)
@L1: ; $0856
        sta $26 ; WozSTL                ;$0856 85 26
        asl                             ;$0858 0a
        asl                             ;$0859 0a
        ora $26 ; WozSTL                ;$085a 05 26
        sta $26 ; WozSTL                ;$085c 85 26
        rts                             ;$085e 60

                ; ------------- $085f ------------- ;

        lda $30 ; zp_19                 ;$085f a5 30
        clc                             ;$0861 18
        adc #$03                        ;$0862 69 03
        and #$0f                        ;$0864 29 0f
        sta $30 ; zp_19                 ;$0866 85 30
        asl                             ;$0868 0a
        asl                             ;$0869 0a
        asl                             ;$086a 0a
        asl                             ;$086b 0a
        ora $30 ; zp_19                 ;$086c 05 30
        sta $30 ; zp_19                 ;$086e 85 30
        rts                             ;$0870 60

                ; ------------- $0871 ------------- ;

        lsr                             ;$0871 4a
        php                             ;$0872 08
        jsr $0847 ; Code_72             ;$0873 20 47 08
        lda ($26),y ; WozSTL            ;$0876 b1 26
        plp                             ;$0878 28
                ; Referenced from $08a0 (subroutine)
Code_73: ; $0879
        bcc @L1                         ;$0879 90 04
        lsr                             ;$087b 4a
        lsr                             ;$087c 4a
        lsr                             ;$087d 4a
        lsr                             ;$087e 4a
                ; Referenced from $0879 (branch)
@L1: ; $087f
        and #$0f                        ;$087f 29 0f
        rts                             ;$0881 60

                ; ------------- $0882 ------------- ;

                ; Referenced from $08d0 (subroutine)
                ; Referenced from $0a9c (subroutine)
Code_74: ; $0882
        ldx $3a ; zp_25                 ;$0882 a6 3a
        ldy $3b ; zp_26                 ;$0884 a4 3b
        jsr $0d96 ; Code_146            ;$0886 20 96 0d
        jsr $0948 ; Code_83             ;$0889 20 48 09
        jmp $0669 ; Code_62             ;$088c 4c 69 06

                ; ------------- $088f ------------- ;

        nop                             ;$088f ea
        nop                             ;$0890 ea
        nop                             ;$0891 ea
        nop                             ;$0892 ea
                ; Referenced from $0672 (jump)
Code_75: ; $0893
        bcs $08a5 ; Code_77             ;$0893 b0 10
        cmp #$a2                        ;$0895 c9 a2
        beq $08a5 ; Code_77             ;$0897 f0 0c
        and #$87                        ;$0899 29 87
                ; Referenced from $0675 (jump)
Code_76: ; $089b
        lsr                             ;$089b 4a
        tax                             ;$089c aa
        lda $0962,x ; Data_88           ;$089d bd 62 09
        jsr $0879 ; Code_73             ;$08a0 20 79 08
        bne $08a9 ; Code_78             ;$08a3 d0 04
                ; Referenced from $0893 (branch)
                ; Referenced from $0897 (branch)
Code_77: ; $08a5
        ldy #$80                        ;$08a5 a0 80
        lda #$00                        ;$08a7 a9 00
                ; Referenced from $08a3 (branch)
Code_78: ; $08a9
        tax                             ;$08a9 aa
        lda $09a6,x ; Data_89           ;$08aa bd a6 09
        sta $2e ; zp_17                 ;$08ad 85 2e
        and #$03                        ;$08af 29 03
        sta $2f ; zp_18                 ;$08b1 85 2f
        tya                             ;$08b3 98
        and #$8f                        ;$08b4 29 8f
        tax                             ;$08b6 aa
        tya                             ;$08b7 98
        ldy #$03                        ;$08b8 a0 03
        cpx #$8a                        ;$08ba e0 8a
        beq @L3                         ;$08bc f0 0b
                ; Referenced from $08ca (branch)
@L1: ; $08be
        lsr                             ;$08be 4a
        bcc @L3                         ;$08bf 90 08
        lsr                             ;$08c1 4a
                ; Referenced from $08c6 (branch)
@L2: ; $08c2
        lsr                             ;$08c2 4a
        ora #$20                        ;$08c3 09 20
        dey                             ;$08c5 88
        bne @L2                         ;$08c6 d0 fa
        iny                             ;$08c8 c8
                ; Referenced from $08bc (branch)
                ; Referenced from $08bf (branch)
@L3: ; $08c9
        dey                             ;$08c9 88
        bne @L1                         ;$08ca d0 f2
        rts                             ;$08cc 60

                ; ------------- $08cd ------------- ;

        .byte $ff ; $08cd isc           ;$08cd ff
        .byte $ff ; $08ce isc           ;$08ce ff
        .byte $ff ; $08cf isc           ;$08cf ff
                ; Referenced from $052b (subroutine)
                ; Referenced from $0a43 (subroutine)
Code_79: ; $08d0
        jsr $0882 ; Code_74             ;$08d0 20 82 08
        pha                             ;$08d3 48
                ; Referenced from $08e1 (branch)
@L1: ; $08d4
        lda ($3a),y ; zp_25             ;$08d4 b1 3a
        jsr $0dda ; Code_149            ;$08d6 20 da 0d
        ldx #$01                        ;$08d9 a2 01
                ; Referenced from $08e7 (branch)
@L2: ; $08db
        jsr $094a ; Code_84             ;$08db 20 4a 09
        cpy $2f ; zp_18                 ;$08de c4 2f
        iny                             ;$08e0 c8
        bcc @L1                         ;$08e1 90 f1
        ldx #$03                        ;$08e3 a2 03
        cpy #$04                        ;$08e5 c0 04
        bcc @L2                         ;$08e7 90 f2
        pla                             ;$08e9 68
        tay                             ;$08ea a8
        lda $09c0,y ; Data_94           ;$08eb b9 c0 09
        sta $2c ; zp_15                 ;$08ee 85 2c
        lda $0a00,y ; Data_95           ;$08f0 b9 00 0a
        sta $2d ; zp_16                 ;$08f3 85 2d
                ; Referenced from $0907 (branch)
@L3: ; $08f5
        lda #$00                        ;$08f5 a9 00
        ldy #$05                        ;$08f7 a0 05
                ; Referenced from $08ff (branch)
@L4: ; $08f9
        asl $2d ; zp_16                 ;$08f9 06 2d
        rol $2c ; zp_15                 ;$08fb 26 2c
        rol                             ;$08fd 2a
        dey                             ;$08fe 88
        bne @L4                         ;$08ff d0 f8
        adc #$bf                        ;$0901 69 bf
        jsr $0ded ; Code_150            ;$0903 20 ed 0d
        dex                             ;$0906 ca
        bne @L3                         ;$0907 d0 ec
        jsr $0948 ; Code_83             ;$0909 20 48 09
        ldy $2f ; zp_18                 ;$090c a4 2f
        ldx #$06                        ;$090e a2 06
                ; Referenced from $0927 (branch)
@L5: ; $0910
L0910:
        cpx #$03                        ;$0910 e0 03
        beq $0930 ; Code_81             ;$0912 f0 1c
L0914:
        asl $2e ; zp_17                 ;$0914 06 2e
        bcc @L6                         ;$0916 90 0e
        lda $09b3,x ; Data_90           ;$0918 bd b3 09
        jsr $0ded ; Code_150            ;$091b 20 ed 0d
        lda $09b9,x ; Data_92           ;$091e bd b9 09
        beq @L6                         ;$0921 f0 03
        jsr $0ded ; Code_150            ;$0923 20 ed 0d
                ; Referenced from $0916 (branch)
                ; Referenced from $0921 (branch)
@L6: ; $0926
        dex                             ;$0926 ca
        bne L0910                       ;$0927 d0 e7
        rts                             ;$0929 60

                ; ------------- $092a ------------- ;

Data_80: ; $092a
        dey                             ;$092a 88
        bmi L0914                       ;$092b 30 e7
        jsr $0dda ; Code_149            ;$092d 20 da 0d

                ; ------------- $0930 ------------- ;

                ; Referenced from $0912 (branch)
Code_81: ; $0930
        lda $2e ; zp_17                 ;$0930 a5 2e
        cmp #$e8                        ;$0932 c9 e8
        lda ($3a),y ; zp_25             ;$0934 b1 3a
        bcc $092a ; Data_80             ;$0936 90 f2
        jsr $0956 ; Code_87             ;$0938 20 56 09
        tax                             ;$093b aa
        inx                             ;$093c e8
        bne $0940 ; Code_82             ;$093d d0 01
        iny                             ;$093f c8
                ; Referenced from $0d99 (subroutine)
Code_82: ; $0940
        tya                             ;$0940 98
        jsr $0dda ; Code_149            ;$0941 20 da 0d
        txa                             ;$0944 8a
        jmp $0dda ; Code_149            ;$0945 4c da 0d

                ; ------------- $0948 ------------- ;

                ; Referenced from $0889 (subroutine)
                ; Referenced from $0909 (subroutine)
Code_83: ; $0948
        ldx #$03                        ;$0948 a2 03
                ; Referenced from $058a (subroutine)
                ; Referenced from $08db (subroutine)
                ; Referenced from $0950 (branch)
Code_84: ; $094a
        lda #$a0                        ;$094a a9 a0
        jsr $0ded ; Code_150            ;$094c 20 ed 0d
        dex                             ;$094f ca
        bne $094a ; Code_84             ;$0950 d0 f8
        rts                             ;$0952 60

                ; ------------- $0953 ------------- ;

                ; Referenced from $052e (subroutine)
Code_85: ; $0953
        sec                             ;$0953 38
                ; Referenced from $0aba (subroutine)
Code_86: ; $0954
        lda $2f ; zp_18                 ;$0954 a5 2f
                ; Referenced from $0ab1 (subroutine)
                ; Referenced from $0b02 (subroutine)
Code_87: ; $0956
        ldy $3b ; zp_26                 ;$0956 a4 3b
        tax                             ;$0958 aa
        bpl @L1                         ;$0959 10 01
        dey                             ;$095b 88
                ; Referenced from $0959 (branch)
@L1: ; $095c
        adc $3a ; zp_25                 ;$095c 65 3a
        bcc @L2                         ;$095e 90 01
        iny                             ;$0960 c8
                ; Referenced from $095e (branch)
@L2: ; $0961
        rts                             ;$0961 60

                ; ------------- $0962 ------------- ;

                ; Referenced from $089d (data)
Data_88: ; $0962
        .byte $04 ; $0962 ???           ;$0962 04
        jsr $3054                       ;$0963 20 54 30
        ora $0480                       ;$0966 0d 80 04
        bcc $096e ; Data_88 + $c        ;$0969 90 03
        .byte $22 ; $096b ???           ;$096b 22
        .byte $54 ; $096c ???           ;$096c 54
        .byte $33 ; $096d rla           ;$096d 33
        ora $0480                       ;$096e 0d 80 04
        bcc $0977 ; Data_88 + $15       ;$0971 90 04
        jsr $3354                       ;$0973 20 54 33
        ora $0480                       ;$0976 0d 80 04
        bcc $097f ; Data_88 + $1d       ;$0979 90 04
        jsr $3b54                       ;$097b 20 54 3b
        ora $0480                       ;$097e 0d 80 04
        bcc $0983 ; Data_88 + $21       ;$0981 90 00
        .byte $22 ; $0983 ???           ;$0983 22
        .byte $44 ; $0984 ???           ;$0984 44
        .byte $33 ; $0985 rla           ;$0985 33
        ora $44c8                       ;$0986 0d c8 44
        brk                             ;$0989 00

                ; ------------- $098a ------------- ;

        ora ($22),y ; zp_5              ;$098a 11 22
        .byte $44 ; $098c ???           ;$098c 44
        .byte $33 ; $098d rla           ;$098d 33
        ora $44c8                       ;$098e 0d c8 44
        lda #$01                        ;$0991 a9 01
        .byte $22 ; $0993 ???           ;$0993 22
        .byte $44 ; $0994 ???           ;$0994 44
        .byte $33 ; $0995 rla           ;$0995 33
        ora $0480                       ;$0996 0d 80 04
        bcc $099c ; Data_88 + $3a       ;$0999 90 01
        .byte $22 ; $099b ???           ;$099b 22
        .byte $44 ; $099c ???           ;$099c 44
        .byte $33 ; $099d rla           ;$099d 33
        ora $0480                       ;$099e 0d 80 04
        bcc $09c9 ; Data_94 + $9        ;$09a1 90 26
        and ($87),y                     ;$09a3 31 87
        txs                             ;$09a5 9a
                ; Referenced from $08aa (data)
Data_89: ; $09a6
        brk                             ;$09a6 00

                ; ------------- $09a7 ------------- ;

        and ($81,x)                     ;$09a7 21 81
        .byte $82 ; $09a9 ???           ;$09a9 82
        brk                             ;$09aa 00

                ; ------------- $09ab ------------- ;

        brk                             ;$09ab 00

                ; ------------- $09ac ------------- ;

        eor $914d,y                     ;$09ac 59 4d 91
        .byte $92 ; $09af ???           ;$09af 92
        stx $4a ; zp_41                 ;$09b0 86 4a
        sta $9d                         ;$09b2 85 9d
                ; Referenced from $0918 (data)
Data_90 = * - 1 ; $9b3
                ; Referenced from $05e0 (data)
Data_91: ; $09b4
        ldy $aca9                       ;$09b4 ac a9 ac
        .byte $a3 ; $09b7 lax           ;$09b7 a3
        tay                             ;$09b8 a8
                ; Referenced from $091e (data)
Data_92: ; $09b9
        ldy $d9                         ;$09b9 a4 d9
                ; Referenced from $05e8 (data)
                ; Referenced from $05ed (data)
Data_93 = * - 1 ; $9ba
        brk                             ;$09bb 00

                ; ------------- $09bc ------------- ;

        cld                             ;$09bc d8
        ldy $a4                         ;$09bd a4 a4
        brk                             ;$09bf 00

                ; ------------- $09c0 ------------- ;

                ; Referenced from $0569 (data)
                ; Referenced from $08eb (data)
Data_94: ; $09c0
        .byte $1c ; $09c0 ???           ;$09c0 1c
        txa                             ;$09c1 8a
        .byte $1c ; $09c2 ???           ;$09c2 1c
        .byte $23 ; $09c3 rla           ;$09c3 23
        eor $1b8b,x                     ;$09c4 5d 8b 1b
        lda ($9d,x)                     ;$09c7 a1 9d
        txa                             ;$09c9 8a
        ora $9d23,x                     ;$09ca 1d 23 9d
        .byte $8b ; $09cd xaa           ;$09cd 8b
        .byte $1d,$a1,$00               ;$09ce 1d a1 00
        and #$19                        ;$09d1 29 19
        ldx $a869                       ;$09d3 ae 69 a8
        ora $2423,y                     ;$09d6 19 23 24
        .byte $53 ; $09d9 sre           ;$09d9 53
        .byte $1b ; $09da slo           ;$09da 1b
        .byte $23 ; $09db rla           ;$09db 23
        bit $53                         ;$09dc 24 53
        .byte $19,$a1,$00               ;$09de 19 a1 00
        .byte $1a ; $09e1 ???           ;$09e1 1a
        .byte $5b ; $09e2 sre           ;$09e2 5b
        .byte $5b ; $09e3 sre           ;$09e3 5b
        lda $69                         ;$09e4 a5 69
        bit $24 ; WozXAML               ;$09e6 24 24
        ldx $a8ae                       ;$09e8 ae ae a8
        .byte $ad,$29,$00               ;$09eb ad 29 00
        .byte $7c ; $09ee ???           ;$09ee 7c

                ; ------------- $09ef ------------- ;

        brk                             ;$09ef 00

                ; ------------- $09f0 ------------- ;

        ora $9c,x                       ;$09f0 15 9c
        adc $a59c                       ;$09f2 6d 9c a5
        adc #$29                        ;$09f5 69 29
        .byte $53 ; $09f7 sre           ;$09f7 53
        sty $13                         ;$09f8 84 13
        .byte $34 ; $09fa ???           ;$09fa 34
        ora ($a5),y                     ;$09fb 11 a5
        adc #$23                        ;$09fd 69 23
        ldy #$d8                        ;$09ff a0 d8
                ; Referenced from $0562 (data)
                ; Referenced from $08f0 (data)
Data_95 = * - 1 ; $a00
        .byte $62 ; $0a01 ???           ;$0a01 62
        .byte $5a ; $0a02 ???           ;$0a02 5a
        pha                             ;$0a03 48
        rol $62                         ;$0a04 26 62
        sty $88,x                       ;$0a06 94 88
        .byte $54 ; $0a08 ???           ;$0a08 54
        .byte $44 ; $0a09 ???           ;$0a09 44
        iny                             ;$0a0a c8
        .byte $54 ; $0a0b ???           ;$0a0b 54
        pla                             ;$0a0c 68
        .byte $44 ; $0a0d ???           ;$0a0d 44
        inx                             ;$0a0e e8
        sty $00,x ; zp_1                ;$0a0f 94 00
        ldy $08,x                       ;$0a11 b4 08
        sty $74                         ;$0a13 84 74
        ldy $28,x ; WozL                ;$0a15 b4 28
        ror $f474                       ;$0a17 6e 74 f4
        cpy $724a                       ;$0a1a cc 4a 72
        .byte $f2 ; $0a1d ???           ;$0a1d f2
                ; Referenced from $0ae9 (data)
Data_96: ; $0a1e
        ldy $8a                         ;$0a1e a4 8a
        brk                             ;$0a20 00

                ; ------------- $0a21 ------------- ;

        tax                             ;$0a21 aa
        ldx #$a2                        ;$0a22 a2 a2
        .byte $74 ; $0a24 ???           ;$0a24 74
        .byte $74 ; $0a25 ???           ;$0a25 74
        .byte $74 ; $0a26 ???           ;$0a26 74
        .byte $72 ; $0a27 ???           ;$0a27 72
        .byte $44 ; $0a28 ???           ;$0a28 44
        pla                             ;$0a29 68
        .byte $b2 ; $0a2a ???           ;$0a2a b2
        .byte $32 ; $0a2b ???           ;$0a2b 32
        .byte $b2 ; $0a2c ???           ;$0a2c b2
        brk                             ;$0a2d 00

                ; ------------- $0a2e ------------- ;

        .byte $22 ; $0a2e ???           ;$0a2e 22
        brk                             ;$0a2f 00

                ; ------------- $0a30 ------------- ;

        .byte $1a ; $0a30 ???           ;$0a30 1a
        .byte $1a ; $0a31 ???           ;$0a31 1a
        rol $26 ; WozSTL                ;$0a32 26 26
        .byte $72 ; $0a34 ???           ;$0a34 72
        .byte $72 ; $0a35 ???           ;$0a35 72
        dey                             ;$0a36 88
        iny                             ;$0a37 c8
        cpy $ca                         ;$0a38 c4 ca
        rol $48 ; zp_39                 ;$0a3a 26 48
        .byte $44 ; $0a3c ???           ;$0a3c 44
        .byte $44 ; $0a3d ???           ;$0a3d 44
        ldx #$c8                        ;$0a3e a2 c8
        .byte $ff ; $0a40 isc           ;$0a40 ff
        .byte $ff ; $0a41 isc           ;$0a41 ff
        .byte $ff ; $0a42 isc           ;$0a42 ff
        jsr $08d0 ; Code_79             ;$0a43 20 d0 08
        pla                             ;$0a46 68
        sta $2c ; zp_15                 ;$0a47 85 2c
        pla                             ;$0a49 68
        sta $2d ; zp_16                 ;$0a4a 85 2d
        ldx #$08                        ;$0a4c a2 08

                ; ------------- $0a4e ------------- ;

                ; Referenced from $0a54 (branch)
@L1: ; $0a4e
        lda $0b10,x ; Code_110 + $1     ;$0a4e bd 10 0b
        sta $3c,x ; Code_27             ;$0a51 95 3c
        dex                             ;$0a53 ca
        bne @L1                         ;$0a54 d0 f8
        lda ($3a,x) ; zp_25             ;$0a56 a1 3a
        beq $0a9c ; Code_98             ;$0a58 f0 42
        ldy $2f ; zp_18                 ;$0a5a a4 2f
        cmp #$20                        ;$0a5c c9 20
        beq $0ab9 ; Code_103            ;$0a5e f0 59
        cmp #$60                        ;$0a60 c9 60
        beq $0aa9 ; Code_100            ;$0a62 f0 45
        cmp #$4c                        ;$0a64 c9 4c
        beq $0ac4 ; Code_104            ;$0a66 f0 5c
        cmp #$6c                        ;$0a68 c9 6c
        beq $0ac5 ; Code_105            ;$0a6a f0 59
        cmp #$40                        ;$0a6c c9 40
        beq $0aa5 ; Code_99             ;$0a6e f0 35
        and #$1f                        ;$0a70 29 1f
        eor #$14                        ;$0a72 49 14
        cmp #$04                        ;$0a74 c9 04
        beq @L3                         ;$0a76 f0 02
                ; Referenced from $0a7e (branch)
@L2: ; $0a78
        lda ($3a),y ; zp_25             ;$0a78 b1 3a
                ; Referenced from $0a76 (branch)
@L3: ; $0a7a
        sta $003c,y ; Code_27           ;$0a7a 99 3c 00
        dey                             ;$0a7d 88
        bpl @L2                         ;$0a7e 10 f8
        jsr $0f3f ; Code_161            ;$0a80 20 3f 0f
        jmp $003c ; Code_27             ;$0a83 4c 3c 00

                ; ------------- $0a86 ------------- ;

        sta $45 ; zp_36                 ;$0a86 85 45
        pla                             ;$0a88 68
        pha                             ;$0a89 48
        asl                             ;$0a8a 0a
        asl                             ;$0a8b 0a
        asl                             ;$0a8c 0a
        bmi $0a92 ; Code_97             ;$0a8d 30 03
        jmp ($03fe) ; Ext_51            ;$0a8f 6c fe 03

                ; ------------- $0a92 ------------- ;

                ; Referenced from $0a8d (branch)
Code_97: ; $0a92
        plp                             ;$0a92 28
        jsr $0f4c ; Code_163            ;$0a93 20 4c 0f
        pla                             ;$0a96 68
        sta $3a ; zp_25                 ;$0a97 85 3a
        pla                             ;$0a99 68
        sta $3b ; zp_26                 ;$0a9a 85 3b
                ; Referenced from $0a58 (branch)
Code_98: ; $0a9c
        jsr $0882 ; Code_74             ;$0a9c 20 82 08
        jsr $0ada ; Code_107            ;$0a9f 20 da 0a
        jmp $0f65 ; Code_164            ;$0aa2 4c 65 0f

                ; ------------- $0aa5 ------------- ;

                ; Referenced from $0a6e (branch)
Code_99: ; $0aa5
        clc                             ;$0aa5 18
        pla                             ;$0aa6 68
        sta $48 ; zp_39                 ;$0aa7 85 48
                ; Referenced from $0a62 (branch)
Code_100: ; $0aa9
        pla                             ;$0aa9 68
        sta $3a ; zp_25                 ;$0aaa 85 3a
        pla                             ;$0aac 68
                ; Referenced from $0b09 (branch)
Code_101: ; $0aad
        sta $3b ; zp_26                 ;$0aad 85 3b
                ; Referenced from $0b0f (branch)
Code_102: ; $0aaf
        lda $2f ; zp_18                 ;$0aaf a5 2f
        jsr $0956 ; Code_87             ;$0ab1 20 56 09
        sty $3b ; zp_26                 ;$0ab4 84 3b
        clc                             ;$0ab6 18
        bcc $0acd ; Code_106            ;$0ab7 90 14
                ; Referenced from $0a5e (branch)
Code_103: ; $0ab9
        clc                             ;$0ab9 18
        jsr $0954 ; Code_86             ;$0aba 20 54 09
        tax                             ;$0abd aa
        tya                             ;$0abe 98
        pha                             ;$0abf 48
        txa                             ;$0ac0 8a
        pha                             ;$0ac1 48
        ldy #$02                        ;$0ac2 a0 02
                ; Referenced from $0a66 (branch)
                ; Referenced from $0acf (branch)
Code_104: ; $0ac4
        clc                             ;$0ac4 18
                ; Referenced from $0a6a (branch)
Code_105: ; $0ac5
        lda ($3a),y ; zp_25             ;$0ac5 b1 3a
        tax                             ;$0ac7 aa
        dey                             ;$0ac8 88
        lda ($3a),y ; zp_25             ;$0ac9 b1 3a
        stx $3b ; zp_26                 ;$0acb 86 3b
                ; Referenced from $0ab7 (branch)
Code_106: ; $0acd
        sta $3a ; zp_25                 ;$0acd 85 3a
        bcs $0ac4 ; Code_104            ;$0acf b0 f3
        lda $2d ; zp_16                 ;$0ad1 a5 2d
        pha                             ;$0ad3 48
        lda $2c ; zp_15                 ;$0ad4 a5 2c
        pha                             ;$0ad6 48
        jsr $0d8e ; Code_144            ;$0ad7 20 8e 0d
                ; Referenced from $0a9f (subroutine)
Code_107: ; $0ada
        lda #$45                        ;$0ada a9 45
        sta $40 ; zp_31                 ;$0adc 85 40
        lda #$00                        ;$0ade a9 00
        sta $41 ; zp_32                 ;$0ae0 85 41
        ldx #$fb                        ;$0ae2 a2 fb
                ; Referenced from $0afa (branch)
@L1: ; $0ae4
        lda #$a0                        ;$0ae4 a9 a0
        jsr $0ded ; Code_150            ;$0ae6 20 ed 0d
        lda $0a1e,x ; Data_96           ;$0ae9 bd 1e 0a
        jsr $0ded ; Code_150            ;$0aec 20 ed 0d
        lda #$bd                        ;$0aef a9 bd
        jsr $0ded ; Code_150            ;$0af1 20 ed 0d
        lda $4a,x ; zp_41               ;$0af4 b5 4a
        jsr $0dda ; Code_149            ;$0af6 20 da 0d
        inx                             ;$0af9 e8
        bmi @L1                         ;$0afa 30 e8
        rts                             ;$0afc 60

                ; ------------- $0afd ------------- ;

                ; Referenced from $0b16 (jump)
Code_108: ; $0afd
        clc                             ;$0afd 18
        ldy #$01                        ;$0afe a0 01
        lda ($3a),y ; zp_25             ;$0b00 b1 3a
        jsr $0956 ; Code_87             ;$0b02 20 56 09
        sta $3a ; zp_25                 ;$0b05 85 3a
        tya                             ;$0b07 98
        sec                             ;$0b08 38
        bcs $0aad ; Code_101            ;$0b09 b0 a2
                ; Referenced from $0b13 (jump)
Code_109: ; $0b0b
        jsr $0f4a ; Code_162            ;$0b0b 20 4a 0f
        sec                             ;$0b0e 38
                ; Referenced from $0a4e (data)
Code_110: ; $0b0f
        bcs $0aaf ; Code_102            ;$0b0f b0 9e
        nop                             ;$0b11 ea
        nop                             ;$0b12 ea
        jmp $0b0b ; Code_109            ;$0b13 4c 0b 0b
        jmp $0afd ; Code_108            ;$0b16 4c fd 0a

                ; ------------- $0b19 ------------- ;

        cmp ($d8,x) ; zp_48             ;$0b19 c1 d8
        cmp $d3d0,y ; Ext_187           ;$0b1b d9 d0 d3
        lda $c070 ; Ext_179             ;$0b1e ad 70 c0
        ldy #$00                        ;$0b21 a0 00
        nop                             ;$0b23 ea
        nop                             ;$0b24 ea
                ; Referenced from $0b2b (branch)
Code_111: ; $0b25
        lda $c064,x ; Ext_178           ;$0b25 bd 64 c0
        bpl @L1                         ;$0b28 10 04
        iny                             ;$0b2a c8
        bne $0b25 ; Code_111            ;$0b2b d0 f8
        dey                             ;$0b2d 88
                ; Referenced from $0b28 (branch)
@L1: ; $0b2e
        rts                             ;$0b2e 60

                ; ------------- $0b2f ------------- ;

Data_112: ; $0b2f
        lda #$00                        ;$0b2f a9 00
        sta $48 ; zp_39                 ;$0b31 85 48
        lda $c056 ; Ext_177             ;$0b33 ad 56 c0
        lda $c054 ; Ext_176             ;$0b36 ad 54 c0
        lda $c051 ; Ext_174             ;$0b39 ad 51 c0
        lda #$00                        ;$0b3c a9 00
        beq @L1                         ;$0b3e f0 0b
        lda $c050 ; Ext_173             ;$0b40 ad 50 c0
        lda $c053 ; Ext_175             ;$0b43 ad 53 c0
        jsr $0836 ; Code_71             ;$0b46 20 36 08
        lda #$14                        ;$0b49 a9 14

                ; ------------- $0b4b ------------- ;

                ; Referenced from $0b3e (branch)
@L1: ; $0b4b
        sta $22 ; zp_5                  ;$0b4b 85 22
        lda #$00                        ;$0b4d a9 00
        sta $20 ; zp_3                  ;$0b4f 85 20
        lda #$28                        ;$0b51 a9 28
        sta $21 ; zp_4                  ;$0b53 85 21
        lda #$18                        ;$0b55 a9 18
        sta $23 ; zp_6                  ;$0b57 85 23
        lda #$17                        ;$0b59 a9 17
        sta $25 ; WozXAMH               ;$0b5b 85 25
        jmp $0c22 ; Code_123            ;$0b5d 4c 22 0c

                ; ------------- $0b60 ------------- ;

        jsr $0ba4 ; Code_114            ;$0b60 20 a4 0b
        ldy #$10                        ;$0b63 a0 10
                ; Referenced from $0b7e (branch)
Code_113: ; $0b65
        lda $50 ; zp_44                 ;$0b65 a5 50
        lsr                             ;$0b67 4a
        bcc @L2                         ;$0b68 90 0c
        clc                             ;$0b6a 18
        ldx #$fe                        ;$0b6b a2 fe
                ; Referenced from $0b74 (branch)
@L1: ; $0b6d
        lda $54,x ; zp_45               ;$0b6d b5 54
        adc $56,x ; zp_46               ;$0b6f 75 56
        sta $54,x ; zp_45               ;$0b71 95 54
        inx                             ;$0b73 e8
        bne @L1                         ;$0b74 d0 f7
                ; Referenced from $0b68 (branch)
@L2: ; $0b76
        ldx #$03                        ;$0b76 a2 03
                ; Referenced from $0b7b (branch)
@L3: ; $0b78
        ror $50,x ; zp_44               ;$0b78 76 50
        dex                             ;$0b7a ca
        bpl @L3                         ;$0b7b 10 fb
        dey                             ;$0b7d 88
        bne $0b65 ; Code_113            ;$0b7e d0 e5
        rts                             ;$0b80 60

                ; ------------- $0b81 ------------- ;

        jsr $0ba4 ; Code_114            ;$0b81 20 a4 0b
        ldy #$10                        ;$0b84 a0 10
        asl $50 ; zp_44                 ;$0b86 06 50
        rol $51                         ;$0b88 26 51
        rol $52                         ;$0b8a 26 52
        rol $53                         ;$0b8c 26 53
        sec                             ;$0b8e 38
        lda $52                         ;$0b8f a5 52
        sbc $54 ; zp_45                 ;$0b91 e5 54
        tax                             ;$0b93 aa
        lda $53                         ;$0b94 a5 53
        sbc $55                         ;$0b96 e5 55
        bcc @L3 + $28                   ;$0b98 90 06
        stx $52                         ;$0b9a 86 52
        sta $53                         ;$0b9c 85 53
        inc $50 ; zp_44                 ;$0b9e e6 50
        dey                             ;$0ba0 88
        bne @L3 + $e                    ;$0ba1 d0 e3
        rts                             ;$0ba3 60

                ; ------------- $0ba4 ------------- ;

                ; Referenced from $0b60 (subroutine)
Code_114: ; $0ba4
        ldy #$00                        ;$0ba4 a0 00
        sty $2f ; zp_18                 ;$0ba6 84 2f
        ldx #$54                        ;$0ba8 a2 54
        jsr @L1                         ;$0baa 20 af 0b
        ldx #$50                        ;$0bad a2 50
                ; Referenced from $0baa (subroutine)
@L1: ; $0baf
        lda $01,x ; zp_2                ;$0baf b5 01
        bpl @L2                         ;$0bb1 10 0d
        sec                             ;$0bb3 38
        tya                             ;$0bb4 98
        sbc $00,x ; zp_1                ;$0bb5 f5 00
        sta $00,x ; zp_1                ;$0bb7 95 00
        tya                             ;$0bb9 98
        sbc $01,x ; zp_2                ;$0bba f5 01
        sta $01,x ; zp_2                ;$0bbc 95 01
        inc $2f ; zp_18                 ;$0bbe e6 2f
                ; Referenced from $0bb1 (branch)
@L2: ; $0bc0
        rts                             ;$0bc0 60

                ; ------------- $0bc1 ------------- ;

                ; Referenced from $0c24 (subroutine)
Code_115: ; $0bc1
        pha                             ;$0bc1 48
        lsr                             ;$0bc2 4a
        and #$03                        ;$0bc3 29 03
        ora #$04                        ;$0bc5 09 04
        sta $29 ; WozH                  ;$0bc7 85 29
        pla                             ;$0bc9 68
        and #$18                        ;$0bca 29 18
        bcc @L1                         ;$0bcc 90 02
        adc #$7f                        ;$0bce 69 7f
                ; Referenced from $0bcc (branch)
@L1: ; $0bd0
        sta $28 ; WozL                  ;$0bd0 85 28
        asl                             ;$0bd2 0a
        asl                             ;$0bd3 0a
        ora $28 ; WozL                  ;$0bd4 05 28
        sta $28 ; WozL                  ;$0bd6 85 28
        rts                             ;$0bd8 60

                ; ------------- $0bd9 ------------- ;

                ; Referenced from $0c0e (branch)
Code_116: ; $0bd9
        cmp #$87                        ;$0bd9 c9 87
        bne $0bef ; Code_116 + $16      ;$0bdb d0 12
        lda #$40                        ;$0bdd a9 40
        jsr $0ca8 ; Data_131            ;$0bdf 20 a8 0c
        ldy #$c0                        ;$0be2 a0 c0
        lda #$0c                        ;$0be4 a9 0c
        jsr $0ca8 ; Data_131            ;$0be6 20 a8 0c
        lda $c030                       ;$0be9 ad 30 c0
        dey                             ;$0bec 88
        bne $0be4 ; Code_116 + $b       ;$0bed d0 f5
        rts                             ;$0bef 60

                ; ------------- $0bf0 ------------- ;

                ; Referenced from $0bff (branch)
                ; Referenced from $0c02 (branch)
Code_117: ; $0bf0
        ldy $24 ; WozXAML               ;$0bf0 a4 24
        nop                             ;$0bf2 ea
        nop                             ;$0bf3 ea
                ; Referenced from $0c32 (branch)
Code_118: ; $0bf4
        inc $24 ; WozXAML               ;$0bf4 e6 24
        lda $24 ; WozXAML               ;$0bf6 a5 24
        cmp $21 ; zp_4                  ;$0bf8 c5 21
        bcs $0c62 ; Code_127            ;$0bfa b0 66
                ; Referenced from $0c12 (branch)
Code_119: ; $0bfc
        rts                             ;$0bfc 60

                ; ------------- $0bfd ------------- ;

                ; Referenced from $0df9 (subroutine)
Code_120: ; $0bfd
        cmp #$a0                        ;$0bfd c9 a0
        bcs $0bf0 ; Code_117            ;$0bff b0 ef
        tay                             ;$0c01 a8
        bpl $0bf0 ; Code_117            ;$0c02 10 ec
        cmp #$8d                        ;$0c04 c9 8d
        beq $0c62 ; Code_127            ;$0c06 f0 5a
        cmp #$8a                        ;$0c08 c9 8a
        beq $0c66 ; Code_128            ;$0c0a f0 5a
        cmp #$88                        ;$0c0c c9 88
        bne $0bd9 ; Code_116            ;$0c0e d0 c9
                ; Referenced from $0c34 (branch)
Code_121: ; $0c10
        dec $24 ; WozXAML               ;$0c10 c6 24
        bpl $0bfc ; Code_119            ;$0c12 10 e8
        lda $21 ; zp_4                  ;$0c14 a5 21
        sta $24 ; WozXAML               ;$0c16 85 24
        dec $24 ; WozXAML               ;$0c18 c6 24
                ; Referenced from $0c3a (branch)
Code_122: ; $0c1a
        lda $22 ; zp_5                  ;$0c1a a5 22
        cmp $25 ; WozXAMH               ;$0c1c c5 25
        bcs $0c2b ; Code_125            ;$0c1e b0 0b
        dec $25 ; WozXAMH               ;$0c20 c6 25
                ; Referenced from $0b5d (jump)
                ; Referenced from $0c56 (branch)
                ; Referenced from $0c9a (branch)
Code_123: ; $0c22
        lda $25 ; WozXAMH               ;$0c22 a5 25
                ; Referenced from $0c47 (subroutine)
                ; Referenced from $0c6c (branch)
                ; Referenced from $0c73 (subroutine)
                ; Referenced from $0c89 (subroutine)
Code_124: ; $0c24
        jsr $0bc1 ; Code_115            ;$0c24 20 c1 0b
        adc $20 ; zp_3                  ;$0c27 65 20
        sta $28 ; WozL                  ;$0c29 85 28
                ; Referenced from $0c1e (branch)
                ; Referenced from $0c40 (branch)
Code_125: ; $0c2b
        rts                             ;$0c2b 60

                ; ------------- $0c2c ------------- ;

                ; Referenced from $0d32 (subroutine)
Code_126: ; $0c2c
        eor #$c0                        ;$0c2c 49 c0
        beq @L2                         ;$0c2e f0 28
        adc #$fd                        ;$0c30 69 fd
        bcc $0bf4 ; Code_118            ;$0c32 90 c0
        beq $0c10 ; Code_121            ;$0c34 f0 da
        adc #$fd                        ;$0c36 69 fd
        bcc $0c66 ; Code_128            ;$0c38 90 2c
        beq $0c1a ; Code_122            ;$0c3a f0 de
        adc #$fd                        ;$0c3c 69 fd
        bcc $0c9c ; Code_129            ;$0c3e 90 5c
        bne $0c2b ; Code_125            ;$0c40 d0 e9
        ldy $24 ; WozXAML               ;$0c42 a4 24
        lda $25 ; WozXAMH               ;$0c44 a5 25
                ; Referenced from $0c54 (branch)
                ; Referenced from $0c60 (branch)
@L1: ; $0c46
        pha                             ;$0c46 48
        jsr $0c24 ; Code_124            ;$0c47 20 24 0c
        jsr $0c9e ; Code_130            ;$0c4a 20 9e 0c
        ldy #$00                        ;$0c4d a0 00
        pla                             ;$0c4f 68
        adc #$00                        ;$0c50 69 00
        cmp $23 ; zp_6                  ;$0c52 c5 23
        bcc @L1                         ;$0c54 90 f0
        bcs $0c22 ; Code_123            ;$0c56 b0 ca
                ; Referenced from $0c2e (branch)
@L2: ; $0c58
        lda $22 ; zp_5                  ;$0c58 a5 22
        sta $25 ; WozXAMH               ;$0c5a 85 25
        ldy #$00                        ;$0c5c a0 00
        sty $24 ; WozXAML               ;$0c5e 84 24
        beq @L1                         ;$0c60 f0 e4
                ; Referenced from $0c06 (branch)
Code_127: ; $0c62
        lda #$00                        ;$0c62 a9 00
        sta $24 ; WozXAML               ;$0c64 85 24
                ; Referenced from $0c0a (branch)
                ; Referenced from $0c38 (branch)
Code_128: ; $0c66
        inc $25 ; WozXAMH               ;$0c66 e6 25
        lda $25 ; WozXAMH               ;$0c68 a5 25
        cmp $23 ; zp_6                  ;$0c6a c5 23
        bcc $0c24 ; Code_124            ;$0c6c 90 b6
        dec $25 ; WozXAMH               ;$0c6e c6 25
        lda $22 ; zp_5                  ;$0c70 a5 22
        pha                             ;$0c72 48
        jsr $0c24 ; Code_124            ;$0c73 20 24 0c
                ; Referenced from $0c93 (branch)
@L1: ; $0c76
        lda $28 ; WozL                  ;$0c76 a5 28
        sta $2a ; WozYSAV               ;$0c78 85 2a
        lda $29 ; WozH                  ;$0c7a a5 29
        sta $2b ; WozMODE               ;$0c7c 85 2b
        ldy $21 ; zp_4                  ;$0c7e a4 21
        dey                             ;$0c80 88
        pla                             ;$0c81 68
        adc #$01                        ;$0c82 69 01
        cmp $23 ; zp_6                  ;$0c84 c5 23
        bcs @L3                         ;$0c86 b0 0d
        pha                             ;$0c88 48
        jsr $0c24 ; Code_124            ;$0c89 20 24 0c
                ; Referenced from $0c91 (branch)
@L2: ; $0c8c
        lda ($28),y ; WozL              ;$0c8c b1 28
        nop                             ;$0c8e ea
        nop                             ;$0c8f ea
        dey                             ;$0c90 88
        bpl @L2                         ;$0c91 10 f9
        bmi @L1                         ;$0c93 30 e1
                ; Referenced from $0c86 (branch)
@L3: ; $0c95
        ldy #$00                        ;$0c95 a0 00
        jsr $0c9e ; Code_130            ;$0c97 20 9e 0c
        bcs $0c22 ; Code_123            ;$0c9a b0 86
                ; Referenced from $0c3e (branch)
                ; Referenced from $0d8b (subroutine)
Code_129: ; $0c9c
        ldy $24 ; WozXAML               ;$0c9c a4 24
                ; Referenced from $0c4a (subroutine)
                ; Referenced from $0c97 (subroutine)
Code_130: ; $0c9e
        lda #$a0                        ;$0c9e a9 a0
                ; Referenced from $0ca5 (branch)
@L1: ; $0ca0
        nop                             ;$0ca0 ea
        nop                             ;$0ca1 ea
        iny                             ;$0ca2 c8
        cpy $21 ; zp_4                  ;$0ca3 c4 21
        bcc @L1                         ;$0ca5 90 f9
        rts                             ;$0ca7 60

                ; ------------- $0ca8 ------------- ;

Data_131: ; $0ca8
        sec                             ;$0ca8 38

                ; ------------- $0ca9 ------------- ;

                ; Referenced from $0cb1 (branch)
@L1: ; $0ca9
        pha                             ;$0ca9 48
                ; Referenced from $0cac (branch)
@L2: ; $0caa
        sbc #$01                        ;$0caa e9 01
        bne @L2                         ;$0cac d0 fc
        pla                             ;$0cae 68
        sbc #$01                        ;$0caf e9 01
        bne @L1                         ;$0cb1 d0 f6
        rts                             ;$0cb3 60

                ; ------------- $0cb4 ------------- ;

Data_132: ; $0cb4
        inc $42 ; zp_33                 ;$0cb4 e6 42
        bne $0cba ; Code_133            ;$0cb6 d0 02
        inc $43 ; zp_34                 ;$0cb8 e6 43

                ; ------------- $0cba ------------- ;

                ; Referenced from $0cb6 (branch)
                ; Referenced from $0dc0 (subroutine)
                ; Referenced from $0f1f (subroutine)
Code_133: ; $0cba
        lda $3c ; Code_27               ;$0cba a5 3c
        cmp $3e ; zp_29                 ;$0cbc c5 3e
        lda $3d ; zp_28                 ;$0cbe a5 3d
        sbc $3f ; zp_30                 ;$0cc0 e5 3f
        inc $3c ; Code_27               ;$0cc2 e6 3c
        bne @L1                         ;$0cc4 d0 02
        inc $3d ; zp_28                 ;$0cc6 e6 3d
                ; Referenced from $0cc4 (branch)
@L1: ; $0cc8
        rts                             ;$0cc8 60

                ; ------------- $0cc9 ------------- ;

                ; Referenced from $0cce (branch)
                ; Referenced from $0cd2 (branch)
Code_134: ; $0cc9
        ldy #$48                        ;$0cc9 a0 48
        jsr $0cdb ; Code_136            ;$0ccb 20 db 0c
        bne $0cc9 ; Code_134            ;$0cce d0 f9
        adc #$fe                        ;$0cd0 69 fe
        bcs $0cc9 ; Code_134            ;$0cd2 b0 f5
        ldy #$21                        ;$0cd4 a0 21
                ; Referenced from $0ef0 (subroutine)
Code_135: ; $0cd6
        jsr $0cdb ; Code_136            ;$0cd6 20 db 0c
        iny                             ;$0cd9 c8
        iny                             ;$0cda c8
                ; Referenced from $0ccb (subroutine)
                ; Referenced from $0cd6 (subroutine)
                ; Referenced from $0cdc (branch)
Code_136: ; $0cdb
        dey                             ;$0cdb 88
        bne $0cdb ; Code_136            ;$0cdc d0 fd
        bcc @L2                         ;$0cde 90 05
        ldy #$32                        ;$0ce0 a0 32
                ; Referenced from $0ce3 (branch)
@L1: ; $0ce2
        dey                             ;$0ce2 88
        bne @L1                         ;$0ce3 d0 fd
                ; Referenced from $0cde (branch)
@L2: ; $0ce5
        ldy $c000,x ; Ext_172           ;$0ce5 bc 00 c0
        ldy #$2c                        ;$0ce8 a0 2c
        dex                             ;$0cea ca
        rts                             ;$0ceb 60

                ; ------------- $0cec ------------- ;

                ; Referenced from $0f16 (subroutine)
                ; Referenced from $0f26 (subroutine)
Code_137: ; $0cec
        ldx #$08                        ;$0cec a2 08
                ; Referenced from $0cf7 (branch)
@L1: ; $0cee
        pha                             ;$0cee 48
        jsr $0cfa ; Code_138            ;$0cef 20 fa 0c
        pla                             ;$0cf2 68
        rol                             ;$0cf3 2a
        ldy #$3a                        ;$0cf4 a0 3a
        dex                             ;$0cf6 ca
        bne @L1                         ;$0cf7 d0 f5
        rts                             ;$0cf9 60

                ; ------------- $0cfa ------------- ;

                ; Referenced from $0cef (subroutine)
Code_138: ; $0cfa
        jsr L0CFD                       ;$0cfa 20 fd 0c
L0CFD:
        dey                             ;$0cfd 88
        lda $c081 ; Ext_180             ;$0cfe ad 81 c0
        cmp $2f ; zp_18                 ;$0d01 c5 2f
        beq L0CFD                       ;$0d03 f0 f8
        sta $2f ; zp_18                 ;$0d05 85 2f
        cpy #$80                        ;$0d07 c0 80
        rts                             ;$0d09 60

                ; ------------- $0d0a ------------- ;

        nop                             ;$0d0a ea
        nop                             ;$0d0b ea
                ; Referenced from $0d2f (subroutine)
                ; Referenced from $0d35 (subroutine)
Code_139: ; $0d0c
        ldy $24 ; WozXAML               ;$0d0c a4 24
        lda ($28),y ; WozL              ;$0d0e b1 28
        pha                             ;$0d10 48
        and #$3f                        ;$0d11 29 3f
        ora #$40                        ;$0d13 09 40
        nop                             ;$0d15 ea
        nop                             ;$0d16 ea
        pla                             ;$0d17 68
        nop                             ;$0d18 ea
        nop                             ;$0d19 ea
        nop                             ;$0d1a ea
                ; Referenced from $0d24 (branch)
@L1: ; $0d1b
        inc $4e ; zp_42                 ;$0d1b e6 4e
        bne @L2                         ;$0d1d d0 02
        inc $4f ; zp_43                 ;$0d1f e6 4f
                ; Referenced from $0d1d (branch)
@L2: ; $0d21
        lda $d011 ; Ext_184             ;$0d21 ad 11 d0
        bpl @L1                         ;$0d24 10 f5
        nop                             ;$0d26 ea
        nop                             ;$0d27 ea
        lda $d010 ; Ext_183             ;$0d28 ad 10 d0
        jmp $0699 ; Code_66             ;$0d2b 4c 99 06

                ; ------------- $0d2e ------------- ;

        nop                             ;$0d2e ea
                ; Referenced from $0d3a (branch)
Code_140: ; $0d2f
        jsr $0d0c ; Code_139            ;$0d2f 20 0c 0d
        jsr $0c2c ; Code_126            ;$0d32 20 2c 0c
                ; Referenced from $0d75 (subroutine)
Code_141: ; $0d35
        jsr $0d0c ; Code_139            ;$0d35 20 0c 0d
        cmp #$9b                        ;$0d38 c9 9b
        beq $0d2f ; Code_140            ;$0d3a f0 f3
        rts                             ;$0d3c 60

                ; ------------- $0d3d ------------- ;

                ; Referenced from $0d89 (branch)
Code_142: ; $0d3d
        lda $32 ; zp_21                 ;$0d3d a5 32
        pha                             ;$0d3f 48
        lda #$ff                        ;$0d40 a9 ff
        sta $32 ; zp_21                 ;$0d42 85 32
        lda $0200,x ; Ext_50            ;$0d44 bd 00 02
        jsr $0ded ; Code_150            ;$0d47 20 ed 0d
        pla                             ;$0d4a 68
        sta $32 ; zp_21                 ;$0d4b 85 32
        lda $0200,x ; Ext_50            ;$0d4d bd 00 02
        cmp #$df                        ;$0d50 c9 df
        beq $0d71 ; Code_143 + $a       ;$0d52 f0 1d
        cmp #$98                        ;$0d54 c9 98
        beq $0d62 ; Code_142 + $25      ;$0d56 f0 0a
        cpx #$f8                        ;$0d58 e0 f8
        bcc $0d5f ; Code_142 + $22      ;$0d5a 90 03
        jsr $0f3a ; Code_160            ;$0d5c 20 3a 0f
        inx                             ;$0d5f e8
        bne $0d75 ; Code_143 + $e       ;$0d60 d0 13
        lda #$dc                        ;$0d62 a9 dc
        jsr $0ded ; Code_150            ;$0d64 20 ed 0d
                ; Referenced from $0599 (subroutine)
                ; Referenced from $0d72 (branch)
                ; Referenced from $0f6d (subroutine)
Code_143: ; $0d67
        jsr $0d8e ; Code_144            ;$0d67 20 8e 0d
        lda $33 ; zp_22                 ;$0d6a a5 33
        jsr $0ded ; Code_150            ;$0d6c 20 ed 0d
        ldx #$01                        ;$0d6f a2 01
        txa                             ;$0d71 8a
        beq $0d67 ; Code_143            ;$0d72 f0 f3
        dex                             ;$0d74 ca
        jsr $0d35 ; Code_141            ;$0d75 20 35 0d
        cmp #$95                        ;$0d78 c9 95
        bne @L1                         ;$0d7a d0 02
        lda ($28),y ; WozL              ;$0d7c b1 28
                ; Referenced from $0d7a (branch)
@L1: ; $0d7e
        cmp #$e0                        ;$0d7e c9 e0
        bcc @L2                         ;$0d80 90 02
        and #$df                        ;$0d82 29 df
                ; Referenced from $0d80 (branch)
@L2: ; $0d84
        sta $0200,x ; Ext_50            ;$0d84 9d 00 02
        cmp #$8d                        ;$0d87 c9 8d
        bne $0d3d ; Code_142            ;$0d89 d0 b2
        jsr $0c9c ; Code_129            ;$0d8b 20 9c 0c
                ; Referenced from $0ad7 (subroutine)
                ; Referenced from $0d67 (subroutine)
                ; Referenced from $0d96 (subroutine)
Code_144: ; $0d8e
        lda #$8d                        ;$0d8e a9 8d
        bne $0ded ; Code_150            ;$0d90 d0 5b
                ; Referenced from $0db3 (subroutine)
Code_145: ; $0d92
        ldy $3d ; zp_28                 ;$0d92 a4 3d
        ldx $3c ; Code_27               ;$0d94 a6 3c
                ; Referenced from $0886 (subroutine)
Code_146: ; $0d96
        jsr $0d8e ; Code_144            ;$0d96 20 8e 0d
        jsr $0940 ; Code_82             ;$0d99 20 40 09
        ldy #$00                        ;$0d9c a0 00
        lda #$ad                        ;$0d9e a9 ad
        jmp $0ded ; Code_150            ;$0da0 4c ed 0d

                ; ------------- $0da3 ------------- ;

                ; Referenced from $0e02 (branch)
Code_147: ; $0da3
        lda $3c ; Code_27               ;$0da3 a5 3c
        ora #$07                        ;$0da5 09 07
        sta $3e ; zp_29                 ;$0da7 85 3e
        lda $3d ; zp_28                 ;$0da9 a5 3d
        sta $3f ; zp_30                 ;$0dab 85 3f
                ; Referenced from $0dc3 (branch)
L0DAD:
        lda $3c ; Code_27               ;$0dad a5 3c
        and #$07                        ;$0daf 29 07
        bne L0DB6                       ;$0db1 d0 03
L0DB3:
        jsr $0d92 ; Code_145            ;$0db3 20 92 0d
                ; Referenced from $0db1 (branch)
L0DB6:
        lda #$a0                        ;$0db6 a9 a0
        jsr $0ded ; Code_150            ;$0db8 20 ed 0d
        lda ($3c),y ; Code_27           ;$0dbb b1 3c
        jsr $0dda ; Code_149            ;$0dbd 20 da 0d
        jsr $0cba ; Code_133            ;$0dc0 20 ba 0c
        bcc L0DAD                       ;$0dc3 90 e8
        rts                             ;$0dc5 60

                ; ------------- $0dc6 ------------- ;

                ; Referenced from $0e09 (branch)
Code_148: ; $0dc6
        lsr                             ;$0dc6 4a
        bcc L0DB3 ; @L1 + $6            ;$0dc7 90 ea
        lsr                             ;$0dc9 4a
        lsr                             ;$0dca 4a
        lda $3e ; zp_29                 ;$0dcb a5 3e
        bcc $0dd1 ; Code_148 + $b       ;$0dcd 90 02
        eor #$ff                        ;$0dcf 49 ff
        adc $3c ; Code_27               ;$0dd1 65 3c
        pha                             ;$0dd3 48
        lda #$bd                        ;$0dd4 a9 bd
        jsr $0ded ; Code_150            ;$0dd6 20 ed 0d
        pla                             ;$0dd9 68
                ; Referenced from $08d6 (subroutine)
                ; Referenced from $0941 (subroutine)
                ; Referenced from $0945 (jump)
                ; Referenced from $0af6 (subroutine)
                ; Referenced from $0dbd (subroutine)
Code_149: ; $0dda
        pha                             ;$0dda 48
        lsr                             ;$0ddb 4a
        lsr                             ;$0ddc 4a
        lsr                             ;$0ddd 4a
        lsr                             ;$0dde 4a
        jsr @L1                         ;$0ddf 20 e5 0d
        pla                             ;$0de2 68
        and #$0f                        ;$0de3 29 0f
                ; Referenced from $0ddf (subroutine)
@L1: ; $0de5
        ora #$b0                        ;$0de5 09 b0
        cmp #$ba                        ;$0de7 c9 ba
        bcc $0ded ; Code_150            ;$0de9 90 02
        adc #$06                        ;$0deb 69 06
                ; Referenced from $058f (subroutine)
                ; Referenced from $0903 (subroutine)
                ; Referenced from $091b (subroutine)
                ; Referenced from $0923 (subroutine)
                ; Referenced from $094c (subroutine)
                ; Referenced from $0ae6 (subroutine)
                ; Referenced from $0aec (subroutine)
                ; Referenced from $0af1 (subroutine)
                ; Referenced from $0d6c (subroutine)
                ; Referenced from $0d90 (branch)
                ; Referenced from $0da0 (jump)
                ; Referenced from $0db8 (subroutine)
                ; Referenced from $0de9 (branch)
                ; Referenced from $0f2f (subroutine)
                ; Referenced from $0f34 (subroutine)
                ; Referenced from $0f37 (subroutine)
                ; Referenced from $0f3c (jump)
Code_150: ; $0ded
        jsr $0690 ; Code_65             ;$0ded 20 90 06
        cmp #$a0                        ;$0df0 c9 a0
        bcc @L1                         ;$0df2 90 02
        and $32 ; zp_21                 ;$0df4 25 32
                ; Referenced from $0df2 (branch)
@L1: ; $0df6
        sty $35 ; zp_24                 ;$0df6 84 35
        pha                             ;$0df8 48
        jsr $0bfd ; Code_120            ;$0df9 20 fd 0b
        pla                             ;$0dfc 68
        ldy $35 ; zp_24                 ;$0dfd a4 35
        rts                             ;$0dff 60

                ; ------------- $0e00 ------------- ;

                ; Referenced from $0556 (subroutine)
Code_151: ; $0e00
        dec $34 ; zp_23                 ;$0e00 c6 34
        beq $0da3 ; Code_147            ;$0e02 f0 9f
        dex                             ;$0e04 ca
        bne $0e1d ; Code_152            ;$0e05 d0 16
        cmp #$ba                        ;$0e07 c9 ba
        bne $0dc6 ; Code_148            ;$0e09 d0 bb
        sta $31 ; zp_20                 ;$0e0b 85 31
        lda $3e ; zp_29                 ;$0e0d a5 3e
        sta ($40),y ; zp_31             ;$0e0f 91 40
        inc $40 ; zp_31                 ;$0e11 e6 40
        bne @L1                         ;$0e13 d0 02
        inc $41 ; zp_32                 ;$0e15 e6 41
                ; Referenced from $0e13 (branch)
@L1: ; $0e17
        rts                             ;$0e17 60

                ; ------------- $0e18 ------------- ;

        ldy $34 ; zp_23                 ;$0e18 a4 34
        lda $01ff,y                     ;$0e1a b9 ff 01
                ; Referenced from $0e05 (branch)
Code_152: ; $0e1d
        sta $31 ; zp_20                 ;$0e1d 85 31
        rts                             ;$0e1f 60

                ; ------------- $0e20 ------------- ;

        ldx #$01                        ;$0e20 a2 01
        lda $3e,x ; zp_29               ;$0e22 b5 3e
        sta $42,x ; zp_33               ;$0e24 95 42
        sta $44,x ; zp_35               ;$0e26 95 44
        dex                             ;$0e28 ca
        bpl $0e22 ; Code_152 + $5       ;$0e29 10 f7
        rts                             ;$0e2b 60

                ; ------------- $0e2c ------------- ;

Data_153: ; $0e2c
        lda ($3c),y ; Code_27           ;$0e2c b1 3c
        sta ($42),y ; zp_33             ;$0e2e 91 42
        jsr $0cb4 ; Data_132            ;$0e30 20 b4 0c
        bcc $0e2c ; Data_153            ;$0e33 90 f7
        rts                             ;$0e35 60

                ; ------------- $0e36 ------------- ;

Data_154: ; $0e36
        lda ($3c),y ; Code_27           ;$0e36 b1 3c
        cmp ($42),y ; zp_33             ;$0e38 d1 42
        beq $0e58 ; Data_154 + $22      ;$0e3a f0 1c
        jsr $0d92 ; Code_145            ;$0e3c 20 92 0d
        lda ($3c),y ; Code_27           ;$0e3f b1 3c
        jsr $0dda ; Code_149            ;$0e41 20 da 0d
        lda #$a0                        ;$0e44 a9 a0
        jsr $0ded ; Code_150            ;$0e46 20 ed 0d
        lda #$a8                        ;$0e49 a9 a8
        jsr $0ded ; Code_150            ;$0e4b 20 ed 0d
        lda ($42),y ; zp_33             ;$0e4e b1 42
        jsr $0dda ; Code_149            ;$0e50 20 da 0d
        lda #$a9                        ;$0e53 a9 a9
        jsr $0ded ; Code_150            ;$0e55 20 ed 0d
        jsr $0cb4 ; Data_132            ;$0e58 20 b4 0c
        bcc $0e36 ; Data_154            ;$0e5b 90 d9
        rts                             ;$0e5d 60

                ; ------------- $0e5e ------------- ;

        jsr $0e75 ; Data_155            ;$0e5e 20 75 0e
        lda #$14                        ;$0e61 a9 14
        pha                             ;$0e63 48
        jsr $08d0 ; Code_79             ;$0e64 20 d0 08
        jsr $0953 ; Code_85             ;$0e67 20 53 09
        sta $3a ; zp_25                 ;$0e6a 85 3a
        sty $3b ; zp_26                 ;$0e6c 84 3b
        pla                             ;$0e6e 68
        sec                             ;$0e6f 38
        sbc #$01                        ;$0e70 e9 01
        bne $0e63 ; Data_154 + $2d      ;$0e72 d0 ef
        rts                             ;$0e74 60

                ; ------------- $0e75 ------------- ;

Data_155: ; $0e75
        txa                             ;$0e75 8a
        beq $0e7f ; Code_157            ;$0e76 f0 07

                ; ------------- $0e78 ------------- ;

                ; Referenced from $05b6 (subroutine)
                ; Referenced from $0e7d (branch)
Code_156: ; $0e78
        lda $3c,x ; Code_27             ;$0e78 b5 3c
        sta $3a,x ; zp_25               ;$0e7a 95 3a
        dex                             ;$0e7c ca
        bpl $0e78 ; Code_156            ;$0e7d 10 f9
                ; Referenced from $0e76 (branch)
Code_157: ; $0e7f
        rts                             ;$0e7f 60

                ; ------------- $0e80 ------------- ;

        ldy #$3f                        ;$0e80 a0 3f
        bne $0e86 ; Code_157 + $7       ;$0e82 d0 02
        ldy #$ff                        ;$0e84 a0 ff
        sty $32 ; zp_21                 ;$0e86 84 32
        rts                             ;$0e88 60

                ; ------------- $0e89 ------------- ;

Data_158: ; $0e89
        lda #$00                        ;$0e89 a9 00
        sta $3e ; zp_29                 ;$0e8b 85 3e
        ldx #$38                        ;$0e8d a2 38
        ldy #$1b                        ;$0e8f a0 1b
        bne @L1                         ;$0e91 d0 08
        lda #$00                        ;$0e93 a9 00
        sta $3e ; zp_29                 ;$0e95 85 3e
        ldx #$36                        ;$0e97 a2 36
        ldy #$f0                        ;$0e99 a0 f0

                ; ------------- $0e9b ------------- ;

                ; Referenced from $0e91 (branch)
@L1: ; $0e9b
        lda $3e ; zp_29                 ;$0e9b a5 3e
        and #$0f                        ;$0e9d 29 0f
        beq @L2                         ;$0e9f f0 06
        ora #$c0                        ;$0ea1 09 c0
        ldy #$00                        ;$0ea3 a0 00
        beq @L3                         ;$0ea5 f0 02
                ; Referenced from $0e9f (branch)
@L2: ; $0ea7
        lda #$fd                        ;$0ea7 a9 fd
                ; Referenced from $0ea5 (branch)
@L3: ; $0ea9
        sty $00,x ; zp_1                ;$0ea9 94 00
        sta $01,x ; zp_2                ;$0eab 95 01
        rts                             ;$0ead 60

                ; ------------- $0eae ------------- ;

        nop                             ;$0eae ea
        nop                             ;$0eaf ea
        jmp $e000                       ;$0eb0 4c 00 e0
        jmp $e2b3                       ;$0eb3 4c b3 e2

                ; ------------- $0eb6 ------------- ;

        jsr $0e75 ; Data_155            ;$0eb6 20 75 0e
        jsr $0f3f ; Code_161            ;$0eb9 20 3f 0f
        jmp ($003a) ; zp_25             ;$0ebc 6c 3a 00

                ; ------------- $0ebf ------------- ;

        jmp $0ad7 ; Code_106 + $a       ;$0ebf 4c d7 0a

                ; ------------- $0ec2 ------------- ;

        dec $34 ; zp_23                 ;$0ec2 c6 34
        jsr $0e75 ; Data_155            ;$0ec4 20 75 0e
        jmp $0a43 ; Data_96 + $25       ;$0ec7 4c 43 0a
        jmp $03f8                       ;$0eca 4c f8 03

                ; ------------- $0ecd ------------- ;

        lda #$40                        ;$0ecd a9 40
        jsr $0cc9 ; Code_134            ;$0ecf 20 c9 0c
        ldy #$27                        ;$0ed2 a0 27
        ldx #$00                        ;$0ed4 a2 00
        eor ($3c,x) ; Code_27           ;$0ed6 41 3c
        pha                             ;$0ed8 48
        lda ($3c,x) ; Code_27           ;$0ed9 a1 3c
        jsr @L3 + $44                   ;$0edb 20 ed 0e
        jsr $0cba ; Code_133            ;$0ede 20 ba 0c
        ldy #$1d                        ;$0ee1 a0 1d
        pla                             ;$0ee3 68
        bcc @L3 + $2b                   ;$0ee4 90 ee
        ldy #$22                        ;$0ee6 a0 22
        jsr @L3 + $44                   ;$0ee8 20 ed 0e
        beq $0f3a ; Code_160            ;$0eeb f0 4d
        ldx #$10                        ;$0eed a2 10
                ; Referenced from $0ef3 (branch)
Code_159: ; $0eef
        asl                             ;$0eef 0a
        jsr $0cd6 ; Code_135            ;$0ef0 20 d6 0c
        bne $0eef ; Code_159            ;$0ef3 d0 fa
        rts                             ;$0ef5 60

                ; ------------- $0ef6 ------------- ;

        jsr $0e00 ; Code_151            ;$0ef6 20 00 0e
        pla                             ;$0ef9 68
        pla                             ;$0efa 68
        bne $0f69 ; Code_164 + $4       ;$0efb d0 6c
        jsr $0cfa ; Code_138            ;$0efd 20 fa 0c
        lda #$16                        ;$0f00 a9 16
        jsr $0cc9 ; Code_134            ;$0f02 20 c9 0c
        sta $2e ; zp_17                 ;$0f05 85 2e
        jsr $0cfa ; Code_138            ;$0f07 20 fa 0c
        ldy #$24                        ;$0f0a a0 24
        jsr L0CFD                       ;$0f0c 20 fd 0c
        bcs $0f0a ; Code_159 + $1b      ;$0f0f b0 f9
        jsr L0CFD                       ;$0f11 20 fd 0c
        ldy #$3b                        ;$0f14 a0 3b
                ; Referenced from $0f24 (branch)
@L1: ; $0f16
        jsr $0cec ; Code_137            ;$0f16 20 ec 0c
        sta ($3c,x) ; Code_27           ;$0f19 81 3c
        eor $2e ; zp_17                 ;$0f1b 45 2e
        sta $2e ; zp_17                 ;$0f1d 85 2e
        jsr $0cba ; Code_133            ;$0f1f 20 ba 0c
        ldy #$35                        ;$0f22 a0 35
        bcc @L1                         ;$0f24 90 f0
        jsr $0cec ; Code_137            ;$0f26 20 ec 0c
        cmp $2e ; zp_17                 ;$0f29 c5 2e
        beq $0f3a ; Code_160            ;$0f2b f0 0d
        lda #$c5                        ;$0f2d a9 c5
        jsr $0ded ; Code_150            ;$0f2f 20 ed 0d
        lda #$d2                        ;$0f32 a9 d2
        jsr $0ded ; Code_150            ;$0f34 20 ed 0d
        jsr $0ded ; Code_150            ;$0f37 20 ed 0d
                ; Referenced from $0592 (subroutine)
                ; Referenced from $0f2b (branch)
                ; Referenced from $0f66 (subroutine)
Code_160: ; $0f3a
        lda #$87                        ;$0f3a a9 87
        jmp $0ded ; Code_150            ;$0f3c 4c ed 0d

                ; ------------- $0f3f ------------- ;

                ; Referenced from $0a80 (subroutine)
Code_161: ; $0f3f
        lda $48 ; zp_39                 ;$0f3f a5 48
        pha                             ;$0f41 48
        lda $45 ; zp_36                 ;$0f42 a5 45
        ldx $46 ; zp_37                 ;$0f44 a6 46
        ldy $47 ; zp_38                 ;$0f46 a4 47
        plp                             ;$0f48 28
        rts                             ;$0f49 60

                ; ------------- $0f4a ------------- ;

                ; Referenced from $0b0b (subroutine)
Code_162: ; $0f4a
        sta $45 ; zp_36                 ;$0f4a 85 45
                ; Referenced from $0a93 (subroutine)
Code_163: ; $0f4c
        stx $46 ; zp_37                 ;$0f4c 86 46
        sty $47 ; zp_38                 ;$0f4e 84 47
        php                             ;$0f50 08
        pla                             ;$0f51 68
        sta $48 ; zp_39                 ;$0f52 85 48
        tsx                             ;$0f54 ba
        stx $49 ; zp_40                 ;$0f55 86 49
        cld                             ;$0f57 d8
        rts                             ;$0f58 60

                ; ------------- $0f59 ------------- ;

        jsr $0678 ; Code_64 + $3        ;$0f59 20 78 06
        jsr $0b2f ; Data_112            ;$0f5c 20 2f 0b
        jsr $0e84 ; Code_157 + $5       ;$0f5f 20 84 0e
        jsr $0e89 ; Data_158            ;$0f62 20 89 0e
                ; Referenced from $0aa2 (jump)
                ; Referenced from $0f7b (branch)
Code_164: ; $0f65
        cld                             ;$0f65 d8
        jsr $0f3a ; Code_160            ;$0f66 20 3a 0f
        lda #$aa                        ;$0f69 a9 aa
        sta $33 ; zp_22                 ;$0f6b 85 33
        jsr $0d67 ; Code_143            ;$0f6d 20 67 0d
        jsr $0fc7 ; Code_169            ;$0f70 20 c7 0f
                ; Referenced from $0f87 (jump)
@L1: ; $0f73
        jsr $0fa7 ; Code_166            ;$0f73 20 a7 0f
        sty $34 ; zp_23                 ;$0f76 84 34
        ldy #$17                        ;$0f78 a0 17
                ; Referenced from $0f80 (branch)
@L2: ; $0f7a
        dey                             ;$0f7a 88
        bmi $0f65 ; Code_164            ;$0f7b 30 e8
        cmp $0fcc,y ; Data_170          ;$0f7d d9 cc 0f
        bne @L2                         ;$0f80 d0 f8
        jsr $0fbe ; Code_168            ;$0f82 20 be 0f
        ldy $34 ; zp_23                 ;$0f85 a4 34
        jmp @L1                         ;$0f87 4c 73 0f

                ; ------------- $0f8a ------------- ;

                ; Referenced from $0fb5 (branch)
                ; Referenced from $0fbb (branch)
Code_165: ; $0f8a
        ldx #$03                        ;$0f8a a2 03
        asl                             ;$0f8c 0a
        asl                             ;$0f8d 0a
        asl                             ;$0f8e 0a
        asl                             ;$0f8f 0a
                ; Referenced from $0f96 (branch)
@L1: ; $0f90
        asl                             ;$0f90 0a
        rol $3e ; zp_29                 ;$0f91 26 3e
        rol $3f ; zp_30                 ;$0f93 26 3f
        dex                             ;$0f95 ca
        bpl @L1                         ;$0f96 10 f8
                ; Referenced from $0fa3 (branch)
@L2: ; $0f98
        lda $31 ; zp_20                 ;$0f98 a5 31
        bne @L3                         ;$0f9a d0 06
        lda $3f,x ; zp_30               ;$0f9c b5 3f
        sta $3d,x ; zp_28               ;$0f9e 95 3d
        sta $41,x ; zp_32               ;$0fa0 95 41
                ; Referenced from $0f9a (branch)
@L3: ; $0fa2
        inx                             ;$0fa2 e8
        beq @L2                         ;$0fa3 f0 f3
        bne $0fad ; Code_167            ;$0fa5 d0 06
                ; Referenced from $053d (subroutine)
                ; Referenced from $05ac (subroutine)
                ; Referenced from $0600 (subroutine)
                ; Referenced from $0f73 (subroutine)
Code_166: ; $0fa7
        ldx #$00                        ;$0fa7 a2 00
        stx $3e ; zp_29                 ;$0fa9 86 3e
        stx $3f ; zp_30                 ;$0fab 86 3f
                ; Referenced from $0fa5 (branch)
Code_167: ; $0fad
        lda $0200,y ; Ext_50            ;$0fad b9 00 02
        iny                             ;$0fb0 c8
        eor #$b0                        ;$0fb1 49 b0
        cmp #$0a                        ;$0fb3 c9 0a
        bcc $0f8a ; Code_165            ;$0fb5 90 d3
        adc #$88                        ;$0fb7 69 88
        cmp #$fa                        ;$0fb9 c9 fa
        bcs $0f8a ; Code_165            ;$0fbb b0 cd
        rts                             ;$0fbd 60

                ; ------------- $0fbe ------------- ;

                ; Referenced from $0538 (subroutine)
                ; Referenced from $0f82 (subroutine)
Code_168: ; $0fbe
        lda #$0e                        ;$0fbe a9 0e
        pha                             ;$0fc0 48
        lda $0fe3,y ; Data_171          ;$0fc1 b9 e3 0f
        pha                             ;$0fc4 48
        lda $31 ; zp_20                 ;$0fc5 a5 31
                ; Referenced from $059c (subroutine)
                ; Referenced from $0f70 (subroutine)
Code_169: ; $0fc7
        ldy #$00                        ;$0fc7 a0 00
        sty $31 ; zp_20                 ;$0fc9 84 31
        rts                             ;$0fcb 60

                ; ------------- $0fcc ------------- ;

                ; Referenced from $0547 (data)
                ; Referenced from $0f7d (data)
Data_170: ; $0fcc
        ldy $beb2,x                     ;$0fcc bc b2 be
        sbc $c4ef                       ;$0fcf ed ef c4
        cpx $bba9                       ;$0fd2 ec a9 bb
        ldx $a4                         ;$0fd5 a6 a4
        asl $95                         ;$0fd7 06 95
        .byte $07 ; $0fd9 slo           ;$0fd9 07
        .byte $02 ; $0fda ???           ;$0fda 02
        ora $f0                         ;$0fdb 05 f0
        brk                             ;$0fdd 00

                ; ------------- $0fde ------------- ;

        .byte $eb ; $0fde sbi           ;$0fde eb
        .byte $93 ; $0fdf ahx           ;$0fdf 93
        .byte $a7 ; $0fe0 lax           ;$0fe0 a7
        dec $99                         ;$0fe1 c6 99
                ; Referenced from $0fc1 (data)
Data_171: ; $0fe3
        .byte $b2 ; $0fe3 ???           ;$0fe3 b2
        cmp #$be                        ;$0fe4 c9 be
        cmp ($35,x) ; zp_24             ;$0fe6 c1 35
        sty $96c3                       ;$0fe8 8c c3 96
        .byte $af ; $0feb lax           ;$0feb af
        .byte $17 ; $0fec slo           ;$0fec 17
        .byte $17 ; $0fed slo           ;$0fed 17
        .byte $2b ; $0fee aac           ;$0fee 2b
        .byte $1f ; $0fef slo           ;$0fef 1f
        .byte $83 ; $0ff0 sax           ;$0ff0 83
        .byte $7f ; $0ff1 rra           ;$0ff1 7f
        eor $b5cc,x                     ;$0ff2 5d cc b5
        .byte $fc ; $0ff5 ???           ;$0ff5 fc
        .byte $17 ; $0ff6 slo           ;$0ff6 17
        .byte $17 ; $0ff7 slo           ;$0ff7 17
        sbc $03,x                       ;$0ff8 f5 03
        .byte $fb ; $0ffa isc           ;$0ffa fb
        .byte $03 ; $0ffb slo           ;$0ffb 03
        eor $86ff,y                     ;$0ffc 59 ff 86
        .byte $fa ; $0fff ???           ;$0fff fa
