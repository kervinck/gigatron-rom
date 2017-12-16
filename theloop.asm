              address
              |    encoding
              |    |     instruction
              |    |     |    operands
              |    |     |    |
              V    V     V    V
              0000 0000  ld   $00         ;LEDs |OOOO|
              0001 1880  ld   $80,out
              0002 18c0  ld   $c0,out
              0003 0001  ld   $01         ;RAM test and count
.countMem0:   0004 d601  st   [$01],y
              0005 00ff  ld   $ff
              0006 6900  xora [y,$00]
              0007 ca00  st   [y,$00]
              0008 c200  st   [$00]
              0009 6900  xora [y,$00]
              000a ec0a  bne  $000a
              000b 00ff  ld   $ff
              000c 6900  xora [y,$00]
              000d ca00  st   [y,$00]
              000e 6100  xora [$00]
              000f f013  beq  .countMem1
              0010 0101  ld   [$01]
              0011 fc04  bra  .countMem0
              0012 8200  adda ac
.countMem1:   0013 0001  ld   $01         ;LEDs |*OOO|
              0014 1880  ld   $80,out
              0015 18c0  ld   $c0,out
              0016 0000  ld   $00         ;Collect entropy and debounce
              0017 d21e  st   [$1e],x
              0018 d61f  st   [$1f],y
.initEnt0:    0019 0110  ld   [$10]
              001a f41d  bge  .initEnt1
              001b 8d00  adda [y,x]
              001c 60bf  xora $bf
.initEnt1:    001d c210  st   [$10]
              001e 0111  ld   [$11]
              001f f422  bge  .initEnt2
              0020 8110  adda [$10]
              0021 60c1  xora $c1
.initEnt2:    0022 c211  st   [$11]
              0023 011e  ld   [$1e]
              0024 8001  adda $01
              0025 ec19  bne  .initEnt0
              0026 d21e  st   [$1e],x
              0027 011f  ld   [$1f]
              0028 8001  adda $01
              0029 ec19  bne  .initEnt0
              002a d61f  st   [$1f],y
              002b 0003  ld   $03         ;LEDs |**OO|
              002c 1880  ld   $80,out
              002d 18c0  ld   $c0,out
              002e 0104  ld   [$04]       ;Cold or warm boot?
              002f 8105  adda [$05]
              0030 805a  adda $5a
              0031 ec35  bne  cold
              0032 0000  ld   $00
warm:         0033 0104  ld   [$04]
              0034 8001  adda $01
cold:         0035 c204  st   [$04]
              0036 60ff  xora $ff
              0037 a059  suba $59
              0038 c205  st   [$05]
              0039 00ff  ld   $ff         ;Setup system timer
              003a c20d  st   [$0d]
              003b 1401  ld   $01,y       ;Setup video scan table
              003c 1000  ld   $00,x
              003d 0008  ld   $08
              003e de00  st   [y,x++]
.initVideo:   003f dc00  st   $00,[y,x++]
              0040 8001  adda $01
              0041 f43f  bge  .initVideo
              0042 de00  st   [y,x++]
              0043 00f2  ld   $f2
              0044 c20c  st   [$0c]
              0045 1402  ld   $02,y       ;Setup shift2 table
              0046 0000  ld   $00
              0047 c202  st   [$02]
.loop:        0048 c21e  st   [$1e]
              0049 8200  adda ac
              004a 9200  adda ac,x
              004b 011e  ld   [$1e]
              004c ce00  st   [y,x]
              004d 8001  adda $01
              004e 6040  xora $40
              004f ec48  bne  .loop
              0050 6040  xora $40
              0051 0078  ld   $78         ;Setup LED sequencer
              0052 c213  st   [$13]
              0053 0000  ld   $00
              0054 c214  st   [$14]
              0055 000a  ld   $0a
              0056 c215  st   [$15]
              0057 1401  ld   $01,y       ;Setup channel 1
              0058 10fa  ld   $fa,x
              0059 dc38  st   $38,[y,x++]
              005a cc06  st   $06,[y,x]
              005b 1402  ld   $02,y       ;Setup channel 2
              005c 10fa  ld   $fa,x
              005d dc70  st   $70,[y,x++]
              005e cc0c  st   $0c,[y,x]
              005f 1403  ld   $03,y       ;Setup channel 3
              0060 10fa  ld   $fa,x
              0061 dc10  st   $10,[y,x++]
              0062 cc10  st   $10,[y,x]
              0063 1404  ld   $04,y       ;Setup channel 4
              0064 10fa  ld   $fa,x
              0065 dc20  st   $20,[y,x++]
              0066 cc13  st   $13,[y,x]
              0067 0000  ld   $00         ;Setup sound timer
              0068 c212  st   [$12]
              0069 00ff  ld   $ff         ;Setup serial input
              006a c216  st   [$16]
              006b c217  st   [$17]
              006c 0007  ld   $07         ;LEDs |***O|
              006d 1880  ld   $80,out
              006e 18c0  ld   $c0,out
              006f 0075  ld   $75         ;Bootstrap vCPU
              0070 c20e  st   [$0e]
              0071 0000  ld   $00
              0072 1406  ld   $06,y
              0073 e000  jmp  y,$00
              0074 c20f  st   [$0f]
.retn:        0075 000f  ld   $0f         ;LEDs |****|
              0076 1880  ld   $80,out
              0077 18c0  ld   $c0,out
              0078 c207  st   [$07]
              0079 c206  st   [$06]
              007a 1401  ld   $01,y       ;Enter video loop
              007b e000  jmp  y,$00
              007c 00c0  ld   $c0
              007d 0200  nop
              007e 0200  nop
              007f 0200  nop
              * 131 times
videoLoop:    0100 c209  st   [$09]       ;Start vertical blank interval
              0101 0080  ld   $80
              0102 c20a  st   [$0a]
              0103 c000  st   $00,[$00]
              0104 0001  ld   $01
              0105 c280  st   [$80]
              0106 010d  ld   [$0d]
              0107 8001  adda $01
              0108 c20d  st   [$0d]
              0109 c003  st   $03,[$03]
              010a 0011  ld   $11         ;Run vCPU for 126 cycles
              010b c20e  st   [$0e]
              010c 0001  ld   $01
              010d c20f  st   [$0f]
              010e 1404  ld   $04,y
              010f e000  jmp  y,$00       ;ENTER
              0110 0029  ld   $29
              0111 0113  ld   [$13]
              0112 ec38  bne  .leds4
              0113 0017  ld   $17
              0114 8114  adda [$14]
              0115 fe00  bra  ac
              0116 fc2f  bra  .leds1
.leds0:       0117 000f  ld   $0f         ;LEDs |****|
              0118 0007  ld   $07         ;LEDs |***O|
              0119 0003  ld   $03         ;LEDs |**OO|
              011a 0001  ld   $01         ;LEDs |*OOO|
              011b 0002  ld   $02         ;LEDs |O*OO|
              011c 0004  ld   $04         ;LEDs |OO*O|
              011d 0008  ld   $08         ;LEDs |OOO*|
              011e 0004  ld   $04         ;LEDs |OO*O|
              011f 0002  ld   $02         ;LEDs |O*OO|
              0120 0001  ld   $01         ;LEDs |*OOO|
              0121 0003  ld   $03         ;LEDs |**OO|
              0122 0007  ld   $07         ;LEDs |***O|
              0123 000f  ld   $0f         ;LEDs |****|
              0124 000e  ld   $0e         ;LEDs |O***|
              0125 000c  ld   $0c         ;LEDs |OO**|
              0126 0008  ld   $08         ;LEDs |OOO*|
              0127 0004  ld   $04         ;LEDs |OO*O|
              0128 0002  ld   $02         ;LEDs |O*OO|
              0129 0001  ld   $01         ;LEDs |*OOO|
              012a 0002  ld   $02         ;LEDs |O*OO|
              012b 0004  ld   $04         ;LEDs |OO*O|
              012c 0008  ld   $08         ;LEDs |OOO*|
              012d 000c  ld   $0c         ;LEDs |OO**|
              012e 008e  ld   $8e         ;LEDs |O***|
.leds1:       012f c207  st   [$07]
              0130 e833  blt  .leds2
              0131 fc34  bra  .leds3
              0132 0114  ld   [$14]
.leds2:       0133 00ff  ld   $ff
.leds3:       0134 8001  adda $01
              0135 c214  st   [$14]
              0136 fc3e  bra  .leds5
              0137 0115  ld   [$15]
.leds4:       0138 0003  ld   $03         ;Wait 10 cycles
              0139 ec39  bne  $0139
              013a a001  suba $01
              013b 0200  nop
              013c 0113  ld   [$13]
              013d a001  suba $01
.leds5:       013e c213  st   [$13]
              013f 0107  ld   [$07]
              0140 200f  anda $0f
              0141 c207  st   [$07]
              0142 0112  ld   [$12]
              0143 ec46  bne  .snd0
              0144 fc47  bra  .snd1
              0145 0000  ld   $00
.snd0:        0146 00f0  ld   $f0
.snd1:        0147 4107  ora  [$07]
              0148 c207  st   [$07]
              0149 0112  ld   [$12]
              014a f04d  beq  .snd2
              014b fc4e  bra  .snd3
              014c a001  suba $01
.snd2:        014d 0000  ld   $00
.snd3:        014e c212  st   [$12]
              014f 0027  ld   $27
              0150 c208  st   [$08]
              0151 1909  ld   [$09],out   ;New scanline
sound1:       0152 0102  ld   [$02]       ;Advance to next sound channel
              0153 2003  anda $03
              0154 8001  adda $01
              0155 190a  ld   [$0a],out   ;Start horizontal pulse
              0156 d602  st   [$02],y
              0157 007f  ld   $7f         ;Update sound channel
              0158 29fe  anda [y,$fe]
              0159 89fa  adda [y,$fa]
              015a cafe  st   [y,$fe]
              015b 3080  anda $80,x
              015c 0500  ld   [x]
              015d 89ff  adda [y,$ff]
              015e 89fb  adda [y,$fb]
              015f caff  st   [y,$ff]
              0160 0200  nop
              0161 0200  nop
              0162 30fc  anda $fc,x
              0163 1402  ld   $02,y
              0164 0d00  ld   [y,x]
              0165 8103  adda [$03]
              0166 c203  st   [$03]
              0167 0200  nop
              0168 0200  nop
              0169 0200  nop
              016a 0200  nop
              016b 0106  ld   [$06]
              016c 0200  nop
              016d 1909  ld   [$09],out   ;End horizontal pulse
              016e 0108  ld   [$08]
              016f f0a0  beq  vBlankLast0
              0170 a001  suba $01
              0171 c208  st   [$08]
              0172 a01a  suba $1a
              0173 ec78  bne  vSync0
              0174 a008  suba $08
              0175 00c0  ld   $c0
              0176 fc7d  bra  vSync2
              0177 c209  st   [$09]
vSync0:       0178 ec7c  bne  vSync1
              0179 0040  ld   $40
              017a fc7e  bra  vSync3
              017b c209  st   [$09]
vSync1:       017c 0109  ld   [$09]
vSync2:       017d 0200  nop
vSync3:       017e 6040  xora $40
              017f c20a  st   [$0a]
              0180 0108  ld   [$08]       ;Capture serial input
              0181 6019  xora $19
              0182 ec85  bne  .ser0
              0183 fc86  bra  .ser1
              0184 c316  st   in,[$16]
.ser0:        0185 0200  nop
.ser1:        0186 0108  ld   [$08]
              0187 2003  anda $03
              0188 ec97  bne  vBlankNormal
              0189 0103  ld   [$03]
vBlankSample: 018a 400f  ora  $0f         ;New sound sample is ready
              018b 2107  anda [$07]
              018c c206  st   [$06]
              018d c003  st   $03,[$03]   ;Reset for next sample
              018e 0095  ld   $95         ;Run vCPU for 144 cycles
              018f c20e  st   [$0e]
              0190 0001  ld   $01
              0191 c20f  st   [$0f]
              0192 1404  ld   $04,y
              0193 e000  jmp  y,$00       ;ENTER
              0194 0032  ld   $32
              0195 fc52  bra  sound1
              0196 1909  ld   [$09],out   ;New scanline
vBlankNormal: 0197 009e  ld   $9e         ;Run vCPU for 148 cycles
              0198 c20e  st   [$0e]
              0199 0001  ld   $01
              019a c20f  st   [$0f]
              019b 1404  ld   $04,y
              019c e000  jmp  y,$00       ;ENTER
              019d 0034  ld   $34
              019e fc52  bra  sound1
              019f 1909  ld   [$09],out   ;New scanline
vBlankLast0:  01a0 0000  ld   $00
              01a1 c209  st   [$09]
              01a2 c20b  st   [$0b]
vBlankLast1:  01a3 0116  ld   [$16]
              01a4 f0a7  beq  .multi0
              01a5 fca8  bra  .multi1
              01a6 c217  st   [$17]
.multi0:      01a7 0040  ld   $40
.multi1:      01a8 4117  ora  [$17]
              01a9 c217  st   [$17]
              01aa 0117  ld   [$17]
              01ab 60df  xora $df
              01ac f0af  beq  .sel0
              01ad fcb0  bra  .sel1
              01ae 0000  ld   $00
.sel0:        01af 002c  ld   $2c
.sel1:        01b0 610c  xora [$0c]
              01b1 c20c  st   [$0c]
              01b2 0200  nop              ;Run vCPU for 151 cycles
              01b3 00ba  ld   $ba
              01b4 c20e  st   [$0e]
              01b5 0001  ld   $01
              01b6 c20f  st   [$0f]
              01b7 1404  ld   $04,y
              01b8 e000  jmp  y,$00       ;ENTER
              01b9 0035  ld   $35
              01ba 0102  ld   [$02]
              01bb 2003  anda $03         ;New scanline
              01bc 8001  adda $01
              01bd 1402  ld   $02,y
              01be e0b0  jmp  y,$b0
              01bf 1880  ld   $80,out
              01c0 0200  nop
              01c1 0200  nop
              01c2 0200  nop
              * 64 times
visiblePage:
videoA:       0200 00c8  ld   $c8
              0201 c20b  st   [$0b]
              0202 1401  ld   $01,y
              0203 1108  ld   [$08],x
              0204 0d00  ld   [y,x]
              0205 de00  st   [y,x++]
              0206 c20a  st   [$0a]
              0207 0d00  ld   [y,x]
              0208 9109  adda [$09],x
              0209 150a  ld   [$0a],y
              020a 00c0  ld   $c0
pixels:       020b 5d00  ora  [y,x++],out ;Pixel burst
              020c 5d00  ora  [y,x++],out
              020d 5d00  ora  [y,x++],out
              * 160 times
              02ab 18c0  ld   $c0,out     ;New scanline
              02ac 0102  ld   [$02]       ;Advance to next sound channel
soundF:       02ad 2003  anda $03
              02ae 8001  adda $01
              02af 1880  ld   $80,out     ;Start horizontal pulse
sound2:       02b0 d602  st   [$02],y
              02b1 007f  ld   $7f
              02b2 29fe  anda [y,$fe]
              02b3 89fa  adda [y,$fa]
              02b4 cafe  st   [y,$fe]
              02b5 3080  anda $80,x
              02b6 0500  ld   [x]
              02b7 89ff  adda [y,$ff]
              02b8 89fb  adda [y,$fb]
              02b9 caff  st   [y,$ff]
              02ba 0200  nop
              02bb 0200  nop
              02bc 30fc  anda $fc,x
              02bd 1402  ld   $02,y
              02be 0d00  ld   [y,x]
              02bf 8103  adda [$03]
              02c0 c203  st   [$03]
              02c1 0200  nop
              02c2 0200  nop
              02c3 0200  nop
              02c4 0200  nop
              02c5 0106  ld   [$06]
              02c6 fd0b  bra  [$0b]
              02c7 18c0  ld   $c0,out     ;End horizontal pulse
videoB:       02c8 00d3  ld   $d3
              02c9 c20b  st   [$0b]
              02ca 1401  ld   $01,y
              02cb 0108  ld   [$08]
              02cc 9001  adda $01,x
              02cd 0109  ld   [$09]
              02ce 8d00  adda [y,x]
              02cf d209  st   [$09],x
              02d0 150a  ld   [$0a],y
              02d1 fc0b  bra  pixels
              02d2 00c0  ld   $c0
videoC:       02d3 0103  ld   [$03]       ;New sound sample is ready
              02d4 400f  ora  $0f
              02d5 2107  anda [$07]
              02d6 c206  st   [$06]
              02d7 c003  st   $03,[$03]   ;Reset for next sample
              02d8 010c  ld   [$0c]       ;Mode for scanline 4
              02d9 c20b  st   [$0b]
              02da 1109  ld   [$09],x
              02db 150a  ld   [$0a],y
              02dc fc0b  bra  pixels
              02dd 00c0  ld   $c0
videoD:       02de 1109  ld   [$09],x
              02df 0108  ld   [$08]
              02e0 a0ee  suba $ee
              02e1 f0e9  beq  last
              02e2 150a  ld   [$0a],y
              02e3 80f0  adda $f0
              02e4 c208  st   [$08]
              02e5 0000  ld   $00
              02e6 c20b  st   [$0b]
              02e7 fc0b  bra  pixels
              02e8 00c0  ld   $c0
last:         02e9 0200  nop
              02ea 0200  nop
              02eb 00ef  ld   $ef
              02ec c20b  st   [$0b]
              02ed fc0b  bra  pixels
              02ee 00c0  ld   $c0
videoE:       02ef 1401  ld   $01,y
              02f0 e000  jmp  y,$00
              02f1 00c0  ld   $c0
videoF:       02f2 0108  ld   [$08]
              02f3 a0ee  suba $ee
              02f4 ecf8  bne  notlast
              02f5 80f0  adda $f0
              02f6 fcfa  bra  .join
              02f7 00ef  ld   $ef
notlast:      02f8 c208  st   [$08]
              02f9 0000  ld   $00
.join:        02fa c20b  st   [$0b]
              02fb 0200  nop              ;Run vCPU for 163 cycles
              02fc 0003  ld   $03
              02fd c20e  st   [$0e]
              02fe 0003  ld   $03
              02ff c20f  st   [$0f]
              0300 1404  ld   $04,y
              0301 e000  jmp  y,$00       ;ENTER
              0302 003b  ld   $3b
              0303 1402  ld   $02,y
              0304 e0ad  jmp  y,$ad       ;New scanline
              0305 0102  ld   [$02]
              0306 0200  nop
              0307 0200  nop
              0308 0200  nop
              * 250 times
ENTER:        0400 fc06  bra  .next2      ;vCPU interpreter
              0401 1519  ld   [$19],y
next14:       0402 c21a  st   [$1a]
              0403 00f8  ld   $f8
NEXT:         0404 811e  adda [$1e]       ;Track elapsed ticks
              0405 e80e  blt  RETURN      ;Escape near time out
.next2:       0406 c21e  st   [$1e]
              0407 0118  ld   [$18]       ;Advance vPC
              0408 8002  adda $02
              0409 d218  st   [$18],x
              040a 0d00  ld   [y,x]       ;Fetch opcode
              040b de00  st   [y,x++]
              040c fe00  bra  ac          ;Dispatch
              040d 0d00  ld   [y,x]       ;Prefetch operand
RETURN:       040e 800e  adda $0e
              040f e40f  bgt  $040f       ;Resync
              0410 a001  suba $01
              0411 150f  ld   [$0f],y
              0412 e10e  jmp  y,[$0e]     ;Return to caller
              0413 0200  nop
LDI:          0414 c21a  st   [$1a]
              0415 0000  ld   $00
              0416 c21b  st   [$1b]
              0417 00f8  ld   $f8
              0418 fc04  bra  NEXT
              0419 0200  nop
LDWI:         041a c21a  st   [$1a]
              041b de00  st   [y,x++]
              041c 0d00  ld   [y,x]
              041d c21b  st   [$1b]
              041e 0118  ld   [$18]
              041f 8001  adda $01
              0420 c218  st   [$18]
              0421 00f6  ld   $f6
              0422 fc04  bra  NEXT
LD:           0423 1200  ld   ac,x
              0424 0500  ld   [x]
              0425 c21a  st   [$1a]
              0426 0000  ld   $00
              0427 c21b  st   [$1b]
              0428 00f7  ld   $f7
              0429 fc04  bra  NEXT
LDW:          042a 1200  ld   ac,x
              042b 8001  adda $01
              042c c21f  st   [$1f]
              042d 0500  ld   [x]
              042e c21a  st   [$1a]
              042f 111f  ld   [$1f],x
              0430 0500  ld   [x]
              0431 c21b  st   [$1b]
              0432 fc04  bra  NEXT
              0433 00f6  ld   $f6
STW:          0434 1200  ld   ac,x
              0435 8001  adda $01
              0436 c21f  st   [$1f]
              0437 011a  ld   [$1a]
              0438 c600  st   [x]
              0439 111f  ld   [$1f],x
              043a 011b  ld   [$1b]
              043b c600  st   [x]
              043c fc04  bra  NEXT
              043d 00f6  ld   $f6
SIGNW:        043e 0118  ld   [$18]
              043f a001  suba $01
              0440 c218  st   [$18]
              0441 011b  ld   [$1b]
              0442 ec4d  bne  .testw3
              0443 e850  blt  .testw4
              0444 c21b  st   [$1b]
              0445 011a  ld   [$1a]
              0446 ec49  bne  .testw1
              0447 fc4a  bra  .testw2
.testw0:      0448 0000  ld   $00
.testw1:      0449 0001  ld   $01
.testw2:      044a c21a  st   [$1a]
              044b fc04  bra  NEXT
              044c 00f4  ld   $f4
.testw3:      044d 0000  ld   $00
              044e fc48  bra  .testw0
              044f c21b  st   [$1b]
.testw4:      0450 00ff  ld   $ff
              0451 c21b  st   [$1b]
              0452 fc4a  bra  .testw2
              0453 0200  nop
BEQ:          0454 011a  ld   [$1a]
              0455 ec59  bne  br1
              0456 0d00  ld   [y,x]
br0:          0457 c218  st   [$18]
              0458 fc04  bra  NEXT
br1:          0459 00f8  ld   $f8
              045a fc04  bra  NEXT
ST:           045b 1200  ld   ac,x
              045c 011a  ld   [$1a]
              045d fc02  bra  next14
              045e c61a  st   [x]
BNE:          045f 011a  ld   [$1a]
              0460 ec57  bne  br0
              0461 0d00  ld   [y,x]
              0462 00f8  ld   $f8
              0463 fc04  bra  NEXT
AND:          0464 1200  ld   ac,x
              0465 011a  ld   [$1a]
              0466 fc02  bra  next14
              0467 2500  anda [x]
ANDI:         0468 211a  anda [$1a]
              0469 c21a  st   [$1a]
              046a 0000  ld   $00
              046b c21b  st   [$1b]
              046c fc04  bra  NEXT
              046d 00f8  ld   $f8
ORI:          046e 411a  ora  [$1a]
              046f c21a  st   [$1a]
              0470 fc04  bra  NEXT
              0471 00f9  ld   $f9
XORI:         0472 611a  xora [$1a]
              0473 c21a  st   [$1a]
              0474 fc04  bra  NEXT
              0475 00f9  ld   $f9
BGT:          0476 011a  ld   [$1a]
              0477 e457  bgt  br0
              0478 0d00  ld   [y,x]
              0479 00f8  ld   $f8
              047a fc04  bra  NEXT
OR:           047b 1200  ld   ac,x
              047c 011a  ld   [$1a]
              047d fc02  bra  next14
              047e 4500  ora  [x]
BLT:          047f 011a  ld   [$1a]
              0480 e857  blt  br0
              0481 0d00  ld   [y,x]
              0482 00f8  ld   $f8
              0483 fc04  bra  NEXT
XOR:          0484 1200  ld   ac,x
              0485 011a  ld   [$1a]
              0486 fc02  bra  next14
              0487 6500  xora [x]
BRA:          0488 c218  st   [$18]
              0489 00f9  ld   $f9
              048a fc04  bra  NEXT
              048b 0200  nop
BGE:          048c 011a  ld   [$1a]
              048d f457  bge  br0
              048e 0d00  ld   [y,x]
              048f 00f8  ld   $f8
              0490 fc04  bra  NEXT
              0491 0200  nop
BLE:          0492 011a  ld   [$1a]
              0493 f857  ble  br0
              0494 0d00  ld   [y,x]
              0495 00f8  ld   $f8
              0496 fc04  bra  NEXT
              0497 0200  nop
ADDW:         0498 1200  ld   ac,x
              0499 8001  adda $01
              049a c21f  st   [$1f]
              049b 011a  ld   [$1a]
              049c 8500  adda [x]
              049d c21a  st   [$1a]
              049e e8a2  blt  .addw0
              049f a500  suba [x]
              04a0 fca4  bra  .addw1
              04a1 4500  ora  [x]
.addw0:       04a2 2500  anda [x]
              04a3 0200  nop
.addw1:       04a4 3080  anda $80,x
              04a5 0500  ld   [x]
              04a6 811b  adda [$1b]
              04a7 111f  ld   [$1f],x
              04a8 8500  adda [x]
              04a9 c21b  st   [$1b]
              04aa fc04  bra  NEXT
              04ab 00f2  ld   $f2
SUBW:         04ac 1200  ld   ac,x
              04ad 8001  adda $01
              04ae c21f  st   [$1f]
              04af 011a  ld   [$1a]
              04b0 e8b5  blt  .subw0
              04b1 a500  suba [x]
              04b2 c21a  st   [$1a]
              04b3 fcb8  bra  .subw1
              04b4 4500  ora  [x]
.subw0:       04b5 c21a  st   [$1a]
              04b6 2500  anda [x]
              04b7 0200  nop
.subw1:       04b8 3080  anda $80,x
              04b9 011b  ld   [$1b]
              04ba a500  suba [x]
              04bb 111f  ld   [$1f],x
              04bc a500  suba [x]
              04bd c21b  st   [$1b]
              04be 0200  nop
              04bf fc04  bra  NEXT
              04c0 00f2  ld   $f2
PEEK:         04c1 0118  ld   [$18]
              04c2 a001  suba $01
              04c3 c218  st   [$18]
              04c4 111a  ld   [$1a],x
              04c5 151b  ld   [$1b],y
              04c6 0d00  ld   [y,x]
              04c7 c21a  st   [$1a]
              04c8 0000  ld   $00
              04c9 c21b  st   [$1b]
              04ca 1519  ld   [$19],y
              04cb fc04  bra  NEXT
              04cc 00f5  ld   $f5
POKE:         04cd c21f  st   [$1f]
              04ce 9001  adda $01,x
              04cf 0500  ld   [x]
              04d0 1600  ld   ac,y
              04d1 111f  ld   [$1f],x
              04d2 0500  ld   [x]
              04d3 1200  ld   ac,x
              04d4 011a  ld   [$1a]
              04d5 ce00  st   [y,x]
              04d6 1519  ld   [$19],y
              04d7 fc04  bra  NEXT
              04d8 00f5  ld   $f5
LOOKUP:       04d9 c21f  st   [$1f]
              04da 9001  adda $01,x
              04db 0500  ld   [x]
              04dc 1600  ld   ac,y
              04dd 111f  ld   [$1f],x
              04de e0fb  jmp  y,$fb       ;Trampoline offset
              04df 0500  ld   [x]
.lookup0:     04e0 c21a  st   [$1a]
              04e1 0000  ld   $00
              04e2 c21b  st   [$1b]
              04e3 fc04  bra  NEXT
              04e4 00f2  ld   $f2
CALL:         04e5 0118  ld   [$18]
              04e6 8001  adda $01         ;CALL has no operand, advances PC by 1
              04e7 c21c  st   [$1c]
              04e8 011a  ld   [$1a]
              04e9 a002  suba $02         ;vAC is actual address, NEXT adds 2
              04ea c218  st   [$18]
              04eb 0119  ld   [$19]
              04ec c21d  st   [$1d]
              04ed 011b  ld   [$1b]
              04ee d619  st   [$19],y
              04ef fc04  bra  NEXT
              04f0 00f5  ld   $f5
SYS:          04f1 0200  nop
              04f2 0200  nop
              04f3 0200  nop
              * 15 times
font:         0500 007c  ld   $7c         ;Char ' '
              0501 0082  ld   $82
              0502 0082  ld   $82
              0503 0092  ld   $92
              0504 005c  ld   $5c
              0505 0000  ld   $00         ;Char '!'
              0506 0022  ld   $22
              0507 00be  ld   $be
              0508 0002  ld   $02
              0509 0000  ld   $00
              050a 0018  ld   $18         ;Char '"'
              050b 0025  ld   $25
              050c 0025  ld   $25
              050d 0025  ld   $25
              050e 001e  ld   $1e
              050f 0004  ld   $04         ;Char '#'
              0510 002a  ld   $2a
              0511 002a  ld   $2a
              0512 002a  ld   $2a
              0513 001e  ld   $1e
              0514 0020  ld   $20         ;Char '$'
              0515 00fc  ld   $fc
              0516 0022  ld   $22
              0517 0002  ld   $02
              0518 0004  ld   $04
              0519 003e  ld   $3e         ;Char '%'
              051a 0010  ld   $10
              051b 0020  ld   $20
              051c 0020  ld   $20
              051d 0010  ld   $10
              051e 001c  ld   $1c         ;Char '&'
              051f 0022  ld   $22
              0520 0022  ld   $22
              0521 0022  ld   $22
              0522 001c  ld   $1c
              0523 003e  ld   $3e         ;Char "'"
              0524 0010  ld   $10
              0525 0020  ld   $20
              0526 0020  ld   $20
              0527 001e  ld   $1e
              0528 0000  ld   $00         ;Char '('
              0529 0000  ld   $00
              052a 0000  ld   $00
              * 5 times
              052d 0080  ld   $80         ;Char ')'
              052e 0080  ld   $80
              052f 00fe  ld   $fe
              0530 0080  ld   $80
              0531 0080  ld   $80
              0532 0080  ld   $80         ;Char '*'
              0533 0080  ld   $80
              0534 00fe  ld   $fe
              0535 0080  ld   $80
              0536 0080  ld   $80
              0537 00fe  ld   $fe         ;Char '+'
              0538 0002  ld   $02
              0539 0002  ld   $02
              053a 0002  ld   $02
              053b 0002  ld   $02
              053c 0000  ld   $00         ;Char ','
              053d 0000  ld   $00
              053e 0000  ld   $00
              * 5 times
              0541 003e  ld   $3e         ;Char '-'
              0542 0020  ld   $20
              0543 001c  ld   $1c
              0544 0020  ld   $20
              0545 001e  ld   $1e
              0546 0000  ld   $00         ;Char '.'
              0547 0022  ld   $22
              0548 00be  ld   $be
              0549 0002  ld   $02
              054a 0000  ld   $00
              054b 001c  ld   $1c         ;Char '/'
              054c 0022  ld   $22
              054d 0022  ld   $22
              054e 0022  ld   $22
              054f 0014  ld   $14
              0550 003e  ld   $3e         ;Char '0'
              0551 0010  ld   $10
              0552 0020  ld   $20
              0553 0020  ld   $20
              0554 0010  ld   $10
              0555 001c  ld   $1c         ;Char '1'
              0556 0022  ld   $22
              0557 0022  ld   $22
              0558 0022  ld   $22
              0559 001c  ld   $1c
              055a 001c  ld   $1c         ;Char '2'
              055b 0022  ld   $22
              055c 0022  ld   $22
              055d 0022  ld   $22
              055e 0014  ld   $14
              055f 001c  ld   $1c         ;Char '3'
              0560 0022  ld   $22
              0561 0022  ld   $22
              0562 0022  ld   $22
              0563 001c  ld   $1c
              0564 003e  ld   $3e         ;Char '4'
              0565 0020  ld   $20
              0566 001c  ld   $1c
              0567 0020  ld   $20
              0568 001e  ld   $1e
              0569 003f  ld   $3f         ;Char '5'
              056a 0024  ld   $24
              056b 0024  ld   $24
              056c 0024  ld   $24
              056d 0018  ld   $18
              056e 003c  ld   $3c         ;Char '6'
              056f 0002  ld   $02
              0570 0002  ld   $02
              0571 0004  ld   $04
              0572 003e  ld   $3e
              0573 0020  ld   $20         ;Char '7'
              0574 00fc  ld   $fc
              0575 0022  ld   $22
              0576 0002  ld   $02
              0577 0004  ld   $04
              0578 001c  ld   $1c         ;Char '8'
              0579 002a  ld   $2a
              057a 002a  ld   $2a
              057b 002a  ld   $2a
              057c 0018  ld   $18
              057d 003e  ld   $3e         ;Char '9'
              057e 0010  ld   $10
              057f 0020  ld   $20
              0580 0020  ld   $20
              0581 0010  ld   $10
              0582 0000  ld   $00         ;Char ':'
              0583 0000  ld   $00
              0584 0000  ld   $00
              * 5 times
              0587 00fe  ld   $fe         ;Char ';'
              0588 0090  ld   $90
              0589 0098  ld   $98
              058a 0094  ld   $94
              058b 0062  ld   $62
              058c 007c  ld   $7c         ;Char '<'
              058d 0082  ld   $82
              058e 0082  ld   $82
              058f 0082  ld   $82
              0590 007c  ld   $7c
              0591 00fe  ld   $fe         ;Char '='
              0592 0040  ld   $40
              0593 0020  ld   $20
              0594 0040  ld   $40
              0595 00fe  ld   $fe
              0596 0038  ld   $38         ;Char '>'
              0597 0004  ld   $04
              0598 0002  ld   $02
              0599 0004  ld   $04
              059a 0038  ld   $38
              059b 007c  ld   $7c         ;Char '?'
              059c 008a  ld   $8a
              059d 0092  ld   $92
              059e 00a2  ld   $a2
              059f 007c  ld   $7c
              05a0 0200  nop
              05a1 0200  nop
              05a2 0200  nop
              * 91 times
              05fb fe00  bra  ac          ;Trampoline for page $05 lookups
              05fc fcfd  bra  $05fd
              05fd 1404  ld   $04,y
              05fe e0e0  jmp  y,$e0
              05ff 1519  ld   [$19],y
initVcpu:     0600 1000  ld   $00,x
              0601 1404  ld   $04,y
              0602 dc14  st   $14,[y,x++] ;0400 LDI
              0603 dc78  st   $78,[y,x++]
              0604 dc5b  st   $5b,[y,x++] ;0402 ST
              0605 dc12  st   $12,[y,x++]
              0606 dc23  st   $23,[y,x++] ;0404 LD
              0607 dc12  st   $12,[y,x++]
              0608 dc3e  st   $3e,[y,x++] ;0406 SIGNW
              0609 dc5f  st   $5f,[y,x++] ;0407 BNE
              060a dc02  st   $02,[y,x++]
              060b dc14  st   $14,[y,x++] ;0409 LDI
              060c dc3e  st   $3e,[y,x++]
              060d dc34  st   $34,[y,x++] ;040b STW
              060e dc9f  st   $9f,[y,x++] ;040d 'P'
              060f dc1a  st   $1a,[y,x++] ;040d LDWI
              0610 dc00  st   $00,[y,x++]
              0611 dc05  st   $05,[y,x++]
              0612 dc34  st   $34,[y,x++] ;0410 STW
              0613 dc8b  st   $8b,[y,x++] ;0412 'F'
              0614 dc1a  st   $1a,[y,x++] ;0412 LDWI
              0615 dc00  st   $00,[y,x++]
              0616 dc08  st   $08,[y,x++]
              0617 dc34  st   $34,[y,x++] ;0415 STW
              0618 dca5  st   $a5,[y,x++] ;0417 'S'
              0619 dc1a  st   $1a,[y,x++] ;0417 LDWI
              061a dc00  st   $00,[y,x++]
              061b dc10  st   $10,[y,x++]
              061c dc34  st   $34,[y,x++] ;041a STW
              061d dca9  st   $a9,[y,x++] ;041c 'U'
              061e dcd9  st   $d9,[y,x++] ;041c LOOKUP
              061f dc8b  st   $8b,[y,x++] ;041e 'F'
              0620 dc34  st   $34,[y,x++] ;041e STW
              0621 dc81  st   $81,[y,x++] ;0420 'A'
              0622 dc2a  st   $2a,[y,x++] ;0420 LDW
              0623 dca5  st   $a5,[y,x++] ;0422 'S'
              0624 dc34  st   $34,[y,x++] ;0422 STW
              0625 dca7  st   $a7,[y,x++] ;0424 'T'
              0626 dc2a  st   $2a,[y,x++] ;0424 LDW
              0627 dc81  st   $81,[y,x++] ;0426 'A'
              0628 dc68  st   $68,[y,x++] ;0426 ANDI
              0629 dc80  st   $80,[y,x++]
              062a dc3e  st   $3e,[y,x++] ;0428 SIGNW
              062b dc5f  st   $5f,[y,x++] ;0429 BNE
              062c dc2f  st   $2f,[y,x++]
              062d dc14  st   $14,[y,x++] ;042b LDI
              062e dc00  st   $00,[y,x++]
              062f dccd  st   $cd,[y,x++] ;042d POKE
              0630 dca7  st   $a7,[y,x++] ;042f 'T'
              0631 dc88  st   $88,[y,x++] ;042f BRA
              0632 dc33  st   $33,[y,x++]
              0633 dc2a  st   $2a,[y,x++] ;0431 LDW
              0634 dc9f  st   $9f,[y,x++] ;0433 'P'
              0635 dccd  st   $cd,[y,x++] ;0433 POKE
              0636 dca7  st   $a7,[y,x++] ;0435 'T'
              0637 dc2a  st   $2a,[y,x++] ;0435 LDW
              0638 dc81  st   $81,[y,x++] ;0437 'A'
              0639 dc98  st   $98,[y,x++] ;0437 ADDW
              063a dc81  st   $81,[y,x++] ;0439 'A'
              063b dc34  st   $34,[y,x++] ;0439 STW
              063c dc81  st   $81,[y,x++] ;043b 'A'
              063d dc1a  st   $1a,[y,x++] ;043b LDWI
              063e dc00  st   $00,[y,x++]
              063f dc01  st   $01,[y,x++]
              0640 dc98  st   $98,[y,x++] ;043e ADDW
              0641 dca7  st   $a7,[y,x++] ;0440 'T'
              0642 dc34  st   $34,[y,x++] ;0440 STW
              0643 dca7  st   $a7,[y,x++] ;0442 'T'
              0644 dcac  st   $ac,[y,x++] ;0442 SUBW
              0645 dca9  st   $a9,[y,x++] ;0444 'U'
              0646 dc3e  st   $3e,[y,x++] ;0444 SIGNW
              0647 dc7f  st   $7f,[y,x++] ;0445 BLT
              0648 dc22  st   $22,[y,x++]
              0649 dc14  st   $14,[y,x++] ;0447 LDI
              064a dc01  st   $01,[y,x++]
              064b dc98  st   $98,[y,x++] ;0449 ADDW
              064c dc8b  st   $8b,[y,x++] ;044b 'F'
              064d dc34  st   $34,[y,x++] ;044b STW
              064e dc8b  st   $8b,[y,x++] ;044d 'F'
              064f dc14  st   $14,[y,x++] ;044d LDI
              0650 dc01  st   $01,[y,x++]
              0651 dc98  st   $98,[y,x++] ;044f ADDW
              0652 dca5  st   $a5,[y,x++] ;0451 'S'
              0653 dc34  st   $34,[y,x++] ;0451 STW
              0654 dca5  st   $a5,[y,x++] ;0453 'S'
              0655 dc68  st   $68,[y,x++] ;0453 ANDI
              0656 dcff  st   $ff,[y,x++]
              0657 dc72  st   $72,[y,x++] ;0455 XORI
              0658 dca0  st   $a0,[y,x++]
              0659 dc3e  st   $3e,[y,x++] ;0457 SIGNW
              065a dc5f  st   $5f,[y,x++] ;0458 BNE
              065b dc1a  st   $1a,[y,x++]
              065c dc2a  st   $2a,[y,x++] ;045a LDW
              065d dc9f  st   $9f,[y,x++] ;045c 'P'
              065e dc72  st   $72,[y,x++] ;045c XORI
              065f dc08  st   $08,[y,x++]
              0660 dc3e  st   $3e,[y,x++] ;045e SIGNW
              0661 dc54  st   $54,[y,x++] ;045f BEQ
              0662 dc68  st   $68,[y,x++]
              0663 dc1a  st   $1a,[y,x++] ;0461 LDWI
              0664 dcf7  st   $f7,[y,x++]
              0665 dcff  st   $ff,[y,x++]
              0666 dc98  st   $98,[y,x++] ;0464 ADDW
              0667 dc9f  st   $9f,[y,x++] ;0466 'P'
              0668 dc34  st   $34,[y,x++] ;0466 STW
              0669 dc9f  st   $9f,[y,x++] ;0468 'P'
              066a dc88  st   $88,[y,x++] ;0468 BRA
              066b dc0b  st   $0b,[y,x++]
              066c dc1a  st   $1a,[y,x++] ;046a LDWI
              066d dc11  st   $11,[y,x++]
              066e dc01  st   $01,[y,x++]
              066f dc34  st   $34,[y,x++] ;046d STW
              0670 dca5  st   $a5,[y,x++] ;046f 'S'
              0671 dc14  st   $14,[y,x++] ;046f LDI
              0672 dc00  st   $00,[y,x++]
              0673 dc34  st   $34,[y,x++] ;0471 STW
              0674 dca1  st   $a1,[y,x++] ;0473 'Q'
              0675 dc34  st   $34,[y,x++] ;0473 STW
              0676 dca3  st   $a3,[y,x++] ;0475 'R'
              0677 dc34  st   $34,[y,x++] ;0475 STW
              0678 dc93  st   $93,[y,x++] ;0477 'J'
              0679 dc34  st   $34,[y,x++] ;0477 STW
              067a dc89  st   $89,[y,x++] ;0479 'E'
              067b dc34  st   $34,[y,x++] ;0479 STW
              067c dcb3  st   $b3,[y,x++] ;047b 'Z'
              067d dc34  st   $34,[y,x++] ;047b STW
              067e dc8d  st   $8d,[y,x++] ;047d 'G'
              067f dc1a  st   $1a,[y,x++] ;047d LDWI
              0680 dc00  st   $00,[y,x++]
              0681 dc18  st   $18,[y,x++]
              0682 dc34  st   $34,[y,x++] ;0480 STW
              0683 dcb1  st   $b1,[y,x++] ;0482 'Y'
              0684 dc34  st   $34,[y,x++] ;0482 STW
              0685 dc83  st   $83,[y,x++] ;0484 'B'
              0686 dc14  st   $14,[y,x++] ;0484 LDI
              0687 dc5a  st   $5a,[y,x++]
              0688 dc34  st   $34,[y,x++] ;0486 STW
              0689 dc8f  st   $8f,[y,x++] ;0488 'H'
              068a dc1a  st   $1a,[y,x++] ;0488 LDWI
              068b dc00  st   $00,[y,x++]
              068c dc05  st   $05,[y,x++]
              068d dce5  st   $e5,[y,x++] ;048b CALL
              068e 1000  ld   $00,x
              068f 1405  ld   $05,y
              0690 dc2a  st   $2a,[y,x++] ;0500 LDW
              0691 dca5  st   $a5,[y,x++] ;0502 'S'
              0692 dcc1  st   $c1,[y,x++] ;0502 PEEK
              0693 dc34  st   $34,[y,x++] ;0503 STW
              0694 dcaf  st   $af,[y,x++] ;0505 'X'
              0695 dc72  st   $72,[y,x++] ;0505 XORI
              0696 dc78  st   $78,[y,x++]
              0697 dc3e  st   $3e,[y,x++] ;0507 SIGNW
              0698 dc5f  st   $5f,[y,x++] ;0508 BNE
              0699 dc0c  st   $0c,[y,x++]
              069a dc14  st   $14,[y,x++] ;050a LDI
              069b dc0a  st   $0a,[y,x++]
              069c dc34  st   $34,[y,x++] ;050c STW
              069d dc89  st   $89,[y,x++] ;050e 'E'
              069e dc2a  st   $2a,[y,x++] ;050e LDW
              069f dca5  st   $a5,[y,x++] ;0510 'S'
              06a0 dcc1  st   $c1,[y,x++] ;0510 PEEK
              06a1 dc98  st   $98,[y,x++] ;0511 ADDW
              06a2 dca3  st   $a3,[y,x++] ;0513 'R'
              06a3 dc98  st   $98,[y,x++] ;0513 ADDW
              06a4 dca1  st   $a1,[y,x++] ;0515 'Q'
              06a5 dc34  st   $34,[y,x++] ;0515 STW
              06a6 dca3  st   $a3,[y,x++] ;0517 'R'
              06a7 dc98  st   $98,[y,x++] ;0517 ADDW
              06a8 dca1  st   $a1,[y,x++] ;0519 'Q'
              06a9 dc3e  st   $3e,[y,x++] ;0519 SIGNW
              06aa dc8c  st   $8c,[y,x++] ;051a BGE
              06ab dc1c  st   $1c,[y,x++]
              06ac dc72  st   $72,[y,x++] ;051c XORI
              06ad dc2b  st   $2b,[y,x++]
              06ae dc98  st   $98,[y,x++] ;051e ADDW
              06af dca1  st   $a1,[y,x++] ;0520 'Q'
              06b0 dc98  st   $98,[y,x++] ;0520 ADDW
              06b1 dca1  st   $a1,[y,x++] ;0522 'Q'
              06b2 dc34  st   $34,[y,x++] ;0522 STW
              06b3 dca1  st   $a1,[y,x++] ;0524 'Q'
              06b4 dc1a  st   $1a,[y,x++] ;0524 LDWI
              06b5 dcc4  st   $c4,[y,x++]
              06b6 dcff  st   $ff,[y,x++]
              06b7 dc98  st   $98,[y,x++] ;0527 ADDW
              06b8 dc8f  st   $8f,[y,x++] ;0529 'H'
              06b9 dc3e  st   $3e,[y,x++] ;0529 SIGNW
              06ba dc92  st   $92,[y,x++] ;052a BLE
              06bb dc38  st   $38,[y,x++]
              06bc dc2a  st   $2a,[y,x++] ;052c LDW
              06bd dca3  st   $a3,[y,x++] ;052e 'R'
              06be dc68  st   $68,[y,x++] ;052e ANDI
              06bf dc10  st   $10,[y,x++]
              06c0 dc3e  st   $3e,[y,x++] ;0530 SIGNW
              06c1 dc54  st   $54,[y,x++] ;0531 BEQ
              06c2 dc38  st   $38,[y,x++]
              06c3 dc1a  st   $1a,[y,x++] ;0533 LDWI
              06c4 dcff  st   $ff,[y,x++]
              06c5 dcff  st   $ff,[y,x++]
              06c6 dc98  st   $98,[y,x++] ;0536 ADDW
              06c7 dc8f  st   $8f,[y,x++] ;0538 'H'
              06c8 dc34  st   $34,[y,x++] ;0538 STW
              06c9 dc8f  st   $8f,[y,x++] ;053a 'H'
              06ca dc1a  st   $1a,[y,x++] ;053a LDWI
              06cb dca6  st   $a6,[y,x++]
              06cc dcff  st   $ff,[y,x++]
              06cd dc98  st   $98,[y,x++] ;053d ADDW
              06ce dc8f  st   $8f,[y,x++] ;053f 'H'
              06cf dc3e  st   $3e,[y,x++] ;053f SIGNW
              06d0 dc8c  st   $8c,[y,x++] ;0540 BGE
              06d1 dc4d  st   $4d,[y,x++]
              06d2 dc2a  st   $2a,[y,x++] ;0542 LDW
              06d3 dca3  st   $a3,[y,x++] ;0544 'R'
              06d4 dc68  st   $68,[y,x++] ;0544 ANDI
              06d5 dc80  st   $80,[y,x++]
              06d6 dc3e  st   $3e,[y,x++] ;0546 SIGNW
              06d7 dc54  st   $54,[y,x++] ;0547 BEQ
              06d8 dc4d  st   $4d,[y,x++]
              06d9 dc14  st   $14,[y,x++] ;0549 LDI
              06da dc01  st   $01,[y,x++]
              06db dc98  st   $98,[y,x++] ;054b ADDW
              06dc dc8f  st   $8f,[y,x++] ;054d 'H'
              06dd dc34  st   $34,[y,x++] ;054d STW
              06de dc8f  st   $8f,[y,x++] ;054f 'H'
              06df dc14  st   $14,[y,x++] ;054f LDI
              06e0 dc08  st   $08,[y,x++]
              06e1 dc98  st   $98,[y,x++] ;0551 ADDW
              06e2 dc8f  st   $8f,[y,x++] ;0553 'H'
              06e3 dc5b  st   $5b,[y,x++] ;0553 ST
              06e4 dc94  st   $94,[y,x++] ;0555 >'J'
              06e5 dc14  st   $14,[y,x++] ;0555 LDI
              06e6 dca0  st   $a0,[y,x++]
              06e7 dc98  st   $98,[y,x++] ;0557 ADDW
              06e8 dcaf  st   $af,[y,x++] ;0559 'X'
              06e9 dc68  st   $68,[y,x++] ;0559 ANDI
              06ea dcff  st   $ff,[y,x++]
              06eb dc34  st   $34,[y,x++] ;055b STW
              06ec dca7  st   $a7,[y,x++] ;055d 'T'
              06ed dc1a  st   $1a,[y,x++] ;055d LDWI
              06ee dc00  st   $00,[y,x++]
              06ef dc10  st   $10,[y,x++]
              06f0 dc98  st   $98,[y,x++] ;0560 ADDW
              06f1 dca7  st   $a7,[y,x++] ;0562 'T'
              06f2 dc34  st   $34,[y,x++] ;0562 STW
              06f3 dcab  st   $ab,[y,x++] ;0564 'V'
              06f4 dccd  st   $cd,[y,x++] ;0564 POKE
              06f5 dcab  st   $ab,[y,x++] ;0566 'V'
              06f6 dc1a  st   $1a,[y,x++] ;0566 LDWI
              06f7 dc00  st   $00,[y,x++]
              06f8 dc01  st   $01,[y,x++]
              06f9 dc98  st   $98,[y,x++] ;0569 ADDW
              06fa dcab  st   $ab,[y,x++] ;056b 'V'
              06fb dc34  st   $34,[y,x++] ;056b STW
              06fc dcab  st   $ab,[y,x++] ;056d 'V'
              06fd dc3e  st   $3e,[y,x++] ;056d SIGNW
              06fe dc76  st   $76,[y,x++] ;056e BGT
              06ff dc62  st   $62,[y,x++]
              0700 dc2a  st   $2a,[y,x++] ;0570 LDW
              0701 dca3  st   $a3,[y,x++] ;0572 'R'
              0702 dc68  st   $68,[y,x++] ;0572 ANDI
              0703 dc7f  st   $7f,[y,x++]
              0704 dc34  st   $34,[y,x++] ;0574 STW
              0705 dc81  st   $81,[y,x++] ;0576 'A'
              0706 dc1a  st   $1a,[y,x++] ;0576 LDWI
              0707 dc91  st   $91,[y,x++]
              0708 dcff  st   $ff,[y,x++]
              0709 dc98  st   $98,[y,x++] ;0579 ADDW
              070a dc81  st   $81,[y,x++] ;057b 'A'
              070b dc3e  st   $3e,[y,x++] ;057b SIGNW
              070c dc92  st   $92,[y,x++] ;057c BLE
              070d dc80  st   $80,[y,x++]
              070e dc14  st   $14,[y,x++] ;057e LDI
              070f dc77  st   $77,[y,x++]
              0710 dc34  st   $34,[y,x++] ;0580 STW
              0711 dc81  st   $81,[y,x++] ;0582 'A'
              0712 dc14  st   $14,[y,x++] ;0582 LDI
              0713 dc10  st   $10,[y,x++]
              0714 dc98  st   $98,[y,x++] ;0584 ADDW
              0715 dc81  st   $81,[y,x++] ;0586 'A'
              0716 dc5b  st   $5b,[y,x++] ;0586 ST
              0717 dc82  st   $82,[y,x++] ;0588 >'A'
              0718 dc2a  st   $2a,[y,x++] ;0588 LDW
              0719 dca7  st   $a7,[y,x++] ;058a 'T'
              071a dc5b  st   $5b,[y,x++] ;058a ST
              071b dc81  st   $81,[y,x++] ;058c <'A'
              071c dc14  st   $14,[y,x++] ;058c LDI
              071d dc02  st   $02,[y,x++]
              071e dccd  st   $cd,[y,x++] ;058e POKE
              071f dc81  st   $81,[y,x++] ;0590 'A'
              0720 dc2a  st   $2a,[y,x++] ;0590 LDW
              0721 dc93  st   $93,[y,x++] ;0592 'J'
              0722 dc98  st   $98,[y,x++] ;0592 ADDW
              0723 dca7  st   $a7,[y,x++] ;0594 'T'
              0724 dc34  st   $34,[y,x++] ;0594 STW
              0725 dcab  st   $ab,[y,x++] ;0596 'V'
              0726 dc14  st   $14,[y,x++] ;0596 LDI
              0727 dc3f  st   $3f,[y,x++]
              0728 dccd  st   $cd,[y,x++] ;0598 POKE
              0729 dcab  st   $ab,[y,x++] ;059a 'V'
              072a dc1a  st   $1a,[y,x++] ;059a LDWI
              072b dc00  st   $00,[y,x++]
              072c dc01  st   $01,[y,x++]
              072d dc98  st   $98,[y,x++] ;059d ADDW
              072e dcab  st   $ab,[y,x++] ;059f 'V'
              072f dc34  st   $34,[y,x++] ;059f STW
              0730 dcab  st   $ab,[y,x++] ;05a1 'V'
              0731 dc2a  st   $2a,[y,x++] ;05a1 LDW
              0732 dcaf  st   $af,[y,x++] ;05a3 'X'
              0733 dc68  st   $68,[y,x++] ;05a3 ANDI
              0734 dc08  st   $08,[y,x++]
              0735 dc98  st   $98,[y,x++] ;05a5 ADDW
              0736 dc8f  st   $8f,[y,x++] ;05a7 'H'
              0737 dc34  st   $34,[y,x++] ;05a7 STW
              0738 dc85  st   $85,[y,x++] ;05a9 'C'
              0739 dc14  st   $14,[y,x++] ;05a9 LDI
              073a dc01  st   $01,[y,x++]
              073b dc98  st   $98,[y,x++] ;05ab ADDW
              073c dc85  st   $85,[y,x++] ;05ad 'C'
              073d dc34  st   $34,[y,x++] ;05ad STW
              073e dc85  st   $85,[y,x++] ;05af 'C'
              073f dc68  st   $68,[y,x++] ;05af ANDI
              0740 dc08  st   $08,[y,x++]
              0741 dc3e  st   $3e,[y,x++] ;05b1 SIGNW
              0742 dc5f  st   $5f,[y,x++] ;05b2 BNE
              0743 dcb6  st   $b6,[y,x++]
              0744 dc14  st   $14,[y,x++] ;05b4 LDI
              0745 dc2a  st   $2a,[y,x++]
              0746 dc88  st   $88,[y,x++] ;05b6 BRA
              0747 dcb8  st   $b8,[y,x++]
              0748 dc14  st   $14,[y,x++] ;05b8 LDI
              0749 dc20  st   $20,[y,x++]
              074a dccd  st   $cd,[y,x++] ;05ba POKE
              074b dcab  st   $ab,[y,x++] ;05bc 'V'
              074c dc1a  st   $1a,[y,x++] ;05bc LDWI
              074d dc00  st   $00,[y,x++]
              074e dc01  st   $01,[y,x++]
              074f dc98  st   $98,[y,x++] ;05bf ADDW
              0750 dcab  st   $ab,[y,x++] ;05c1 'V'
              0751 dc34  st   $34,[y,x++] ;05c1 STW
              0752 dcab  st   $ab,[y,x++] ;05c3 'V'
              0753 dc3e  st   $3e,[y,x++] ;05c3 SIGNW
              0754 dc76  st   $76,[y,x++] ;05c4 BGT
              0755 dca7  st   $a7,[y,x++]
              0756 dc1a  st   $1a,[y,x++] ;05c6 LDWI
              0757 dc00  st   $00,[y,x++]
              0758 dc06  st   $06,[y,x++]
              0759 dce5  st   $e5,[y,x++] ;05c9 CALL
              075a dc88  st   $88,[y,x++] ;05ca BRA
              075b dcfe  st   $fe,[y,x++]
              075c 1000  ld   $00,x
              075d 1406  ld   $06,y
              075e dc23  st   $23,[y,x++] ;0600 LD
              075f dc08  st   $08,[y,x++]
              0760 dc3e  st   $3e,[y,x++] ;0602 SIGNW
              0761 dc5f  st   $5f,[y,x++] ;0603 BNE
              0762 dcfe  st   $fe,[y,x++]
              0763 dc14  st   $14,[y,x++] ;0605 LDI
              0764 dc01  st   $01,[y,x++]
              0765 dc98  st   $98,[y,x++] ;0607 ADDW
              0766 dcaf  st   $af,[y,x++] ;0609 'X'
              0767 dccd  st   $cd,[y,x++] ;0609 POKE
              0768 dca5  st   $a5,[y,x++] ;060b 'S'
              0769 dc2a  st   $2a,[y,x++] ;060b LDW
              076a dc8d  st   $8d,[y,x++] ;060d 'G'
              076b dccd  st   $cd,[y,x++] ;060d POKE
              076c dc83  st   $83,[y,x++] ;060f 'B'
              076d dc2a  st   $2a,[y,x++] ;060f LDW
              076e dc89  st   $89,[y,x++] ;0611 'E'
              076f dc98  st   $98,[y,x++] ;0611 ADDW
              0770 dcb3  st   $b3,[y,x++] ;0613 'Z'
              0771 dc34  st   $34,[y,x++] ;0613 STW
              0772 dcb3  st   $b3,[y,x++] ;0615 'Z'
              0773 dc2a  st   $2a,[y,x++] ;0615 LDW
              0774 dcb1  st   $b1,[y,x++] ;0617 'Y'
              0775 dc98  st   $98,[y,x++] ;0617 ADDW
              0776 dcb3  st   $b3,[y,x++] ;0619 'Z'
              0777 dc34  st   $34,[y,x++] ;0619 STW
              0778 dcb1  st   $b1,[y,x++] ;061b 'Y'
              0779 dc14  st   $14,[y,x++] ;061b LDI
              077a dc32  st   $32,[y,x++]
              077b dc98  st   $98,[y,x++] ;061d ADDW
              077c dcaf  st   $af,[y,x++] ;061f 'X'
              077d dc34  st   $34,[y,x++] ;061f STW
              077e dc83  st   $83,[y,x++] ;0621 'B'
              077f dc23  st   $23,[y,x++] ;0621 LD
              0780 dcb2  st   $b2,[y,x++] ;0623 >'Y'
              0781 dc5b  st   $5b,[y,x++] ;0623 ST
              0782 dc84  st   $84,[y,x++] ;0625 >'B'
              0783 dc2a  st   $2a,[y,x++] ;0625 LDW
              0784 dc83  st   $83,[y,x++] ;0627 'B'
              0785 dcc1  st   $c1,[y,x++] ;0627 PEEK
              0786 dc34  st   $34,[y,x++] ;0628 STW
              0787 dc8d  st   $8d,[y,x++] ;062a 'G'
              0788 dc14  st   $14,[y,x++] ;062a LDI
              0789 dc02  st   $02,[y,x++]
              078a dcac  st   $ac,[y,x++] ;062c SUBW
              078b dc8d  st   $8d,[y,x++] ;062e 'G'
              078c dc3e  st   $3e,[y,x++] ;062e SIGNW
              078d dc8c  st   $8c,[y,x++] ;062f BGE
              078e dc4e  st   $4e,[y,x++]
              078f dc14  st   $14,[y,x++] ;0631 LDI
              0790 dc00  st   $00,[y,x++]
              0791 dcac  st   $ac,[y,x++] ;0633 SUBW
              0792 dcb3  st   $b3,[y,x++] ;0635 'Z'
              0793 dc34  st   $34,[y,x++] ;0635 STW
              0794 dcb3  st   $b3,[y,x++] ;0637 'Z'
              0795 dc2a  st   $2a,[y,x++] ;0637 LDW
              0796 dcb1  st   $b1,[y,x++] ;0639 'Y'
              0797 dc98  st   $98,[y,x++] ;0639 ADDW
              0798 dcb3  st   $b3,[y,x++] ;063b 'Z'
              0799 dc34  st   $34,[y,x++] ;063b STW
              079a dcb1  st   $b1,[y,x++] ;063d 'Y'
              079b dc14  st   $14,[y,x++] ;063d LDI
              079c dc0a  st   $0a,[y,x++]
              079d dc5b  st   $5b,[y,x++] ;063f ST
              079e dc12  st   $12,[y,x++]
              079f dc14  st   $14,[y,x++] ;0641 LDI
              07a0 dc32  st   $32,[y,x++]
              07a1 dc98  st   $98,[y,x++] ;0643 ADDW
              07a2 dcaf  st   $af,[y,x++] ;0645 'X'
              07a3 dc34  st   $34,[y,x++] ;0645 STW
              07a4 dc83  st   $83,[y,x++] ;0647 'B'
              07a5 dc23  st   $23,[y,x++] ;0647 LD
              07a6 dcb2  st   $b2,[y,x++] ;0649 >'Y'
              07a7 dc5b  st   $5b,[y,x++] ;0649 ST
              07a8 dc84  st   $84,[y,x++] ;064b >'B'
              07a9 dc2a  st   $2a,[y,x++] ;064b LDW
              07aa dc83  st   $83,[y,x++] ;064d 'B'
              07ab dcc1  st   $c1,[y,x++] ;064d PEEK
              07ac dc34  st   $34,[y,x++] ;064e STW
              07ad dc8d  st   $8d,[y,x++] ;0650 'G'
              07ae dc14  st   $14,[y,x++] ;0650 LDI
              07af dc3f  st   $3f,[y,x++]
              07b0 dccd  st   $cd,[y,x++] ;0652 POKE
              07b1 dc83  st   $83,[y,x++] ;0654 'B'
              07b2 dc2a  st   $2a,[y,x++] ;0654 LDW
              07b3 dc1c  st   $1c,[y,x++] ;0655 vRT
              07b4 dce5  st   $e5,[y,x++] ;0656 CALL
              07b5 00fe  ld   $fe
              07b6 c218  st   [$18]
              07b7 0004  ld   $04
              07b8 c219  st   [$19]
              07b9 150f  ld   [$0f],y
              07ba e10e  jmp  y,[$0e]
              07bb 0200  nop
              07bc
