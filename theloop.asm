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
.countMem1:   0013 00ff  ld   $ff         ;Debounce reset button
.debounce:    0014 c200  st   [$00]
              0015 ec15  bne  $0015
              0016 a001  suba $01
              0017 0100  ld   [$00]
              0018 ec14  bne  .debounce
              0019 a001  suba $01
              001a 0001  ld   $01         ;LEDs |*OOO|
              001b 1880  ld   $80,out
              001c 18c0  ld   $c0,out
              001d 0000  ld   $00         ;Collect entropy from RAM
              001e d21e  st   [$1e],x
              001f d61f  st   [$1f],y
.initEnt0:    0020 0110  ld   [$10]
              0021 f424  bge  .initEnt1
              0022 8d00  adda [y,x]
              0023 60bf  xora $bf
.initEnt1:    0024 c210  st   [$10]
              0025 0111  ld   [$11]
              0026 f429  bge  .initEnt2
              0027 8110  adda [$10]
              0028 60c1  xora $c1
.initEnt2:    0029 c211  st   [$11]
              002a 011e  ld   [$1e]
              002b 8001  adda $01
              002c ec20  bne  .initEnt0
              002d d21e  st   [$1e],x
              002e 011f  ld   [$1f]
              002f 8001  adda $01
              0030 ec20  bne  .initEnt0
              0031 d61f  st   [$1f],y
              0032 0003  ld   $03         ;LEDs |**OO|
              0033 1880  ld   $80,out
              0034 18c0  ld   $c0,out
              0035 0104  ld   [$04]       ;Cold or warm boot?
              0036 8105  adda [$05]
              0037 805a  adda $5a
              0038 ec3c  bne  cold
              0039 0000  ld   $00
warm:         003a 0104  ld   [$04]
              003b 8001  adda $01
cold:         003c c204  st   [$04]
              003d 60ff  xora $ff
              003e a059  suba $59
              003f c205  st   [$05]
              0040 00ff  ld   $ff         ;Setup system timer
              0041 c20d  st   [$0d]
              0042 1401  ld   $01,y       ;Setup video scan table
              0043 1000  ld   $00,x
              0044 0008  ld   $08
              0045 de00  st   [y,x++]
.initVideo:   0046 dc00  st   $00,[y,x++]
              0047 8001  adda $01
              0048 f446  bge  .initVideo
              0049 de00  st   [y,x++]
              004a 00f2  ld   $f2
              004b c20c  st   [$0c]
              004c 1402  ld   $02,y       ;Setup shift2 table
              004d 0000  ld   $00
              004e c202  st   [$02]
.loop:        004f c21e  st   [$1e]
              0050 8200  adda ac
              0051 9200  adda ac,x
              0052 011e  ld   [$1e]
              0053 ce00  st   [y,x]
              0054 8001  adda $01
              0055 6040  xora $40
              0056 ec4f  bne  .loop
              0057 6040  xora $40
              0058 0078  ld   $78         ;Setup LED sequencer
              0059 c213  st   [$13]
              005a 0000  ld   $00
              005b c214  st   [$14]
              005c 000a  ld   $0a
              005d c215  st   [$15]
              005e 1401  ld   $01,y       ;Setup channel 1
              005f 10fa  ld   $fa,x
              0060 dc38  st   $38,[y,x++]
              0061 cc06  st   $06,[y,x]
              0062 1402  ld   $02,y       ;Setup channel 2
              0063 10fa  ld   $fa,x
              0064 dc70  st   $70,[y,x++]
              0065 cc0c  st   $0c,[y,x]
              0066 1403  ld   $03,y       ;Setup channel 3
              0067 10fa  ld   $fa,x
              0068 dc10  st   $10,[y,x++]
              0069 cc10  st   $10,[y,x]
              006a 1404  ld   $04,y       ;Setup channel 4
              006b 10fa  ld   $fa,x
              006c dc20  st   $20,[y,x++]
              006d cc13  st   $13,[y,x]
              006e 0000  ld   $00         ;Setup sound timer
              006f c212  st   [$12]
              0070 00ff  ld   $ff         ;Setup serial input
              0071 c216  st   [$16]
              0072 c217  st   [$17]
              0073 0007  ld   $07         ;LEDs |***O|
              0074 1880  ld   $80,out
              0075 18c0  ld   $c0,out
              0076 007c  ld   $7c         ;Bootstrap vCPU
              0077 c20e  st   [$0e]
              0078 0000  ld   $00
              0079 1405  ld   $05,y
              007a e000  jmp  y,$00
              007b c20f  st   [$0f]
.retn:        007c 000f  ld   $0f         ;LEDs |****|
              007d 1880  ld   $80,out
              007e 18c0  ld   $c0,out
              007f c207  st   [$07]
              0080 c206  st   [$06]
              0081 1402  ld   $02,y       ;Enter video loop
              0082 e006  jmp  y,$06
              0083 00c0  ld   $c0
              0084 0200  nop
              0085 0200  nop
              0086 0200  nop
              * 124 times
videoA:       0100 00c8  ld   $c8
              0101 c20b  st   [$0b]
              0102 1401  ld   $01,y
              0103 1108  ld   [$08],x
              0104 0d00  ld   [y,x]
              0105 de00  st   [y,x++]
              0106 c20a  st   [$0a]
              0107 0d00  ld   [y,x]
              0108 9109  adda [$09],x
              0109 150a  ld   [$0a],y
              010a 00c0  ld   $c0
pixels:       010b 5d00  ora  [y,x++],out ;Pixel burst
              010c 5d00  ora  [y,x++],out
              010d 5d00  ora  [y,x++],out
              * 160 times
              01ab 18c0  ld   $c0,out     ;<New scanline start>
              01ac 0102  ld   [$02]       ;Advance to next sound channel
soundF:       01ad 2003  anda $03
              01ae 8001  adda $01
              01af 1880  ld   $80,out     ;Start horizontal pulse
sound2:       01b0 d602  st   [$02],y
              01b1 007f  ld   $7f
              01b2 29fe  anda [y,$fe]
              01b3 89fa  adda [y,$fa]
              01b4 cafe  st   [y,$fe]
              01b5 3080  anda $80,x
              01b6 0500  ld   [x]
              01b7 89ff  adda [y,$ff]
              01b8 89fb  adda [y,$fb]
              01b9 caff  st   [y,$ff]
              01ba 0200  nop
              01bb 0200  nop
              01bc 30fc  anda $fc,x
              01bd 1402  ld   $02,y
              01be 0d00  ld   [y,x]
              01bf 8103  adda [$03]
              01c0 c203  st   [$03]
              01c1 0200  nop
              01c2 0200  nop
              01c3 0200  nop
              01c4 0200  nop
              01c5 0106  ld   [$06]
              01c6 fd0b  bra  [$0b]
              01c7 18c0  ld   $c0,out     ;End horizontal pulse
videoB:       01c8 00d3  ld   $d3
              01c9 c20b  st   [$0b]
              01ca 1401  ld   $01,y
              01cb 0108  ld   [$08]
              01cc 9001  adda $01,x
              01cd 0109  ld   [$09]
              01ce 8d00  adda [y,x]
              01cf d209  st   [$09],x
              01d0 150a  ld   [$0a],y
              01d1 fc0b  bra  pixels
              01d2 00c0  ld   $c0
videoC:       01d3 0103  ld   [$03]       ;New sound sample is ready
              01d4 400f  ora  $0f
              01d5 2107  anda [$07]
              01d6 c206  st   [$06]
              01d7 c003  st   $03,[$03]   ;Reset for next sample
              01d8 010c  ld   [$0c]       ;Mode for scanline 4
              01d9 c20b  st   [$0b]
              01da 1109  ld   [$09],x
              01db 150a  ld   [$0a],y
              01dc fc0b  bra  pixels
              01dd 00c0  ld   $c0
videoD:       01de 1109  ld   [$09],x
              01df 0108  ld   [$08]
              01e0 a0ee  suba $ee
              01e1 f0e9  beq  last
              01e2 150a  ld   [$0a],y
              01e3 80f0  adda $f0
              01e4 c208  st   [$08]
              01e5 0000  ld   $00
              01e6 c20b  st   [$0b]
              01e7 fc0b  bra  pixels
              01e8 00c0  ld   $c0
last:         01e9 0200  nop
              01ea 0200  nop
              01eb 00ef  ld   $ef
              01ec c20b  st   [$0b]
              01ed fc0b  bra  pixels
              01ee 00c0  ld   $c0
videoE:       01ef 1402  ld   $02,y
              01f0 e006  jmp  y,$06
              01f1 00c0  ld   $c0
videoF:       01f2 0108  ld   [$08]
              01f3 a0ee  suba $ee
              01f4 ecf8  bne  notlast
              01f5 80f0  adda $f0
              01f6 fcfa  bra  .join
              01f7 00ef  ld   $ef
notlast:      01f8 c208  st   [$08]
              01f9 0000  ld   $00
.join:        01fa c20b  st   [$0b]
              01fb 0200  nop              ;Run vCPU for 163 cycles
              01fc 0003  ld   $03
              01fd c20e  st   [$0e]
              01fe 0002  ld   $02
              01ff c20f  st   [$0f]
              0200 1403  ld   $03,y
              0201 e000  jmp  y,$00
              0202 003b  ld   $3b
              0203 1401  ld   $01,y
              0204 e0ad  jmp  y,$ad       ;<New scanline start>
              0205 0102  ld   [$02]
vBlank:       0206 c209  st   [$09]       ;Start vertical blank interval
              0207 0080  ld   $80
              0208 c20a  st   [$0a]
              0209 c000  st   $00,[$00]
              020a 0001  ld   $01
              020b c280  st   [$80]
              020c 010d  ld   [$0d]
              020d 8001  adda $01
              020e c20d  st   [$0d]
              020f c003  st   $03,[$03]
              0210 0017  ld   $17         ;Run vCPU for 126 cycles
              0211 c20e  st   [$0e]
              0212 0002  ld   $02
              0213 c20f  st   [$0f]
              0214 1403  ld   $03,y
              0215 e000  jmp  y,$00
              0216 0029  ld   $29
              0217 0113  ld   [$13]
              0218 ec3e  bne  .leds4
              0219 001d  ld   $1d
              021a 8114  adda [$14]
              021b fe00  bra  ac
              021c fc35  bra  .leds1
.leds0:       021d 000f  ld   $0f         ;LEDs |****|
              021e 0007  ld   $07         ;LEDs |***O|
              021f 0003  ld   $03         ;LEDs |**OO|
              0220 0001  ld   $01         ;LEDs |*OOO|
              0221 0002  ld   $02         ;LEDs |O*OO|
              0222 0004  ld   $04         ;LEDs |OO*O|
              0223 0008  ld   $08         ;LEDs |OOO*|
              0224 0004  ld   $04         ;LEDs |OO*O|
              0225 0002  ld   $02         ;LEDs |O*OO|
              0226 0001  ld   $01         ;LEDs |*OOO|
              0227 0003  ld   $03         ;LEDs |**OO|
              0228 0007  ld   $07         ;LEDs |***O|
              0229 000f  ld   $0f         ;LEDs |****|
              022a 000e  ld   $0e         ;LEDs |O***|
              022b 000c  ld   $0c         ;LEDs |OO**|
              022c 0008  ld   $08         ;LEDs |OOO*|
              022d 0004  ld   $04         ;LEDs |OO*O|
              022e 0002  ld   $02         ;LEDs |O*OO|
              022f 0001  ld   $01         ;LEDs |*OOO|
              0230 0002  ld   $02         ;LEDs |O*OO|
              0231 0004  ld   $04         ;LEDs |OO*O|
              0232 0008  ld   $08         ;LEDs |OOO*|
              0233 000c  ld   $0c         ;LEDs |OO**|
              0234 008e  ld   $8e         ;LEDs |O***|
.leds1:       0235 c207  st   [$07]
              0236 e839  blt  .leds2
              0237 fc3a  bra  .leds3
              0238 0114  ld   [$14]
.leds2:       0239 00ff  ld   $ff
.leds3:       023a 8001  adda $01
              023b c214  st   [$14]
              023c fc44  bra  .leds5
              023d 0115  ld   [$15]
.leds4:       023e 0003  ld   $03         ;Wait 10 cycles
              023f ec3f  bne  $023f
              0240 a001  suba $01
              0241 0200  nop
              0242 0113  ld   [$13]
              0243 a001  suba $01
.leds5:       0244 c213  st   [$13]
              0245 0107  ld   [$07]
              0246 200f  anda $0f
              0247 c207  st   [$07]
              0248 0112  ld   [$12]
              0249 ec4c  bne  .snd0
              024a fc4d  bra  .snd1
              024b 0000  ld   $00
.snd0:        024c 00f0  ld   $f0
.snd1:        024d 4107  ora  [$07]
              024e c207  st   [$07]
              024f 0112  ld   [$12]
              0250 f053  beq  .snd2
              0251 fc54  bra  .snd3
              0252 a001  suba $01
.snd2:        0253 0000  ld   $00
.snd3:        0254 c212  st   [$12]
              0255 0027  ld   $27
              0256 c208  st   [$08]
              0257 1909  ld   [$09],out   ;<New scanline start>
sound1:       0258 0102  ld   [$02]       ;Advance to next sound channel
              0259 2003  anda $03
              025a 8001  adda $01
              025b 190a  ld   [$0a],out   ;Start horizontal pulse
              025c d602  st   [$02],y
              025d 007f  ld   $7f         ;Update sound channel
              025e 29fe  anda [y,$fe]
              025f 89fa  adda [y,$fa]
              0260 cafe  st   [y,$fe]
              0261 3080  anda $80,x
              0262 0500  ld   [x]
              0263 89ff  adda [y,$ff]
              0264 89fb  adda [y,$fb]
              0265 caff  st   [y,$ff]
              0266 0200  nop
              0267 0200  nop
              0268 30fc  anda $fc,x
              0269 1402  ld   $02,y
              026a 0d00  ld   [y,x]
              026b 8103  adda [$03]
              026c c203  st   [$03]
              026d 0200  nop
              026e 0200  nop
              026f 0200  nop
              0270 0200  nop
              0271 0106  ld   [$06]
              0272 0200  nop
              0273 1909  ld   [$09],out   ;End horizontal pulse
              0274 0108  ld   [$08]
              0275 f0a6  beq  vBlankLast0
              0276 a001  suba $01
              0277 c208  st   [$08]
              0278 a01a  suba $1a
              0279 ec7e  bne  vSync0
              027a a008  suba $08
              027b 00c0  ld   $c0
              027c fc83  bra  vSync2
              027d c209  st   [$09]
vSync0:       027e ec82  bne  vSync1
              027f 0040  ld   $40
              0280 fc84  bra  vSync3
              0281 c209  st   [$09]
vSync1:       0282 0109  ld   [$09]
vSync2:       0283 0200  nop
vSync3:       0284 6040  xora $40
              0285 c20a  st   [$0a]
              0286 0108  ld   [$08]       ;Capture serial input
              0287 6019  xora $19
              0288 ec8b  bne  .ser0
              0289 fc8c  bra  .ser1
              028a c316  st   in,[$16]
.ser0:        028b 0200  nop
.ser1:        028c 0108  ld   [$08]
              028d 2003  anda $03
              028e ec9d  bne  vBlankNormal
              028f 0103  ld   [$03]
vBlankSample: 0290 400f  ora  $0f         ;New sound sample is ready
              0291 2107  anda [$07]
              0292 c206  st   [$06]
              0293 c003  st   $03,[$03]   ;Reset for next sample
              0294 009b  ld   $9b         ;Run vCPU for 144 cycles
              0295 c20e  st   [$0e]
              0296 0002  ld   $02
              0297 c20f  st   [$0f]
              0298 1403  ld   $03,y
              0299 e000  jmp  y,$00
              029a 0032  ld   $32
              029b fc58  bra  sound1
              029c 1909  ld   [$09],out   ;<New scanline start>
vBlankNormal: 029d 00a4  ld   $a4         ;Run vCPU for 148 cycles
              029e c20e  st   [$0e]
              029f 0002  ld   $02
              02a0 c20f  st   [$0f]
              02a1 1403  ld   $03,y
              02a2 e000  jmp  y,$00
              02a3 0034  ld   $34
              02a4 fc58  bra  sound1
              02a5 1909  ld   [$09],out   ;<New scanline start>
vBlankLast0:  02a6 0000  ld   $00
              02a7 c209  st   [$09]
              02a8 c20b  st   [$0b]
vBlankLast1:  02a9 0116  ld   [$16]
              02aa f0ad  beq  .multi0
              02ab fcae  bra  .multi1
              02ac c217  st   [$17]
.multi0:      02ad 0040  ld   $40
.multi1:      02ae 4117  ora  [$17]
              02af c217  st   [$17]
              02b0 0117  ld   [$17]
              02b1 60df  xora $df
              02b2 f0b5  beq  .sel0
              02b3 fcb6  bra  .sel1
              02b4 0000  ld   $00
.sel0:        02b5 002c  ld   $2c
.sel1:        02b6 610c  xora [$0c]
              02b7 c20c  st   [$0c]
              02b8 0200  nop              ;Run vCPU for 151 cycles
              02b9 00c0  ld   $c0
              02ba c20e  st   [$0e]
              02bb 0002  ld   $02
              02bc c20f  st   [$0f]
              02bd 1403  ld   $03,y
              02be e000  jmp  y,$00
              02bf 0035  ld   $35
              02c0 0102  ld   [$02]
              02c1 2003  anda $03         ;<New scanline start>
              02c2 8001  adda $01
              02c3 1401  ld   $01,y
              02c4 e0b0  jmp  y,$b0
              02c5 1880  ld   $80,out
              02c6 0200  nop
              02c7 0200  nop
              02c8 0200  nop
              * 58 times
ENTER:        0300 fc06  bra  .next2      ;vCPU interpreter
              0301 1519  ld   [$19],y
next14:       0302 c21a  st   [$1a]
              0303 00f8  ld   $f8
NEXT:         0304 811e  adda [$1e]       ;Track elapsed ticks
              0305 e80e  blt  RETURN      ;Escape near time out
.next2:       0306 c21e  st   [$1e]
              0307 0118  ld   [$18]       ;Advance vPC
              0308 8002  adda $02
              0309 d218  st   [$18],x
              030a 0d00  ld   [y,x]       ;Fetch opcode
              030b de00  st   [y,x++]
              030c fe00  bra  ac          ;Dispatch
              030d 0d00  ld   [y,x]       ;Prefetch operand
RETURN:       030e 800e  adda $0e
              030f e40f  bgt  $030f       ;Resync
              0310 a001  suba $01
              0311 150f  ld   [$0f],y
              0312 e10e  jmp  y,[$0e]     ;Return to caller
              0313 0200  nop
LDI:          0314 c21a  st   [$1a]
              0315 0000  ld   $00
              0316 c21b  st   [$1b]
              0317 00f8  ld   $f8
              0318 fc04  bra  NEXT
              0319 0200  nop
LDWI:         031a c21a  st   [$1a]
              031b de00  st   [y,x++]
              031c 0d00  ld   [y,x]
              031d c21b  st   [$1b]
              031e 0118  ld   [$18]
              031f 8001  adda $01
              0320 c218  st   [$18]
              0321 00f6  ld   $f6
              0322 fc04  bra  NEXT
LD:           0323 1200  ld   ac,x
              0324 0500  ld   [x]
              0325 c21a  st   [$1a]
              0326 0000  ld   $00
              0327 c21b  st   [$1b]
              0328 00f7  ld   $f7
              0329 fc04  bra  NEXT
LDW:          032a 1200  ld   ac,x
              032b 8001  adda $01
              032c c21f  st   [$1f]
              032d 0500  ld   [x]
              032e c21a  st   [$1a]
              032f 111f  ld   [$1f],x
              0330 0500  ld   [x]
              0331 c21b  st   [$1b]
              0332 fc04  bra  NEXT
              0333 00f6  ld   $f6
STW:          0334 1200  ld   ac,x
              0335 8001  adda $01
              0336 c21f  st   [$1f]
              0337 011a  ld   [$1a]
              0338 c600  st   [x]
              0339 111f  ld   [$1f],x
              033a 011b  ld   [$1b]
              033b c600  st   [x]
              033c fc04  bra  NEXT
              033d 00f6  ld   $f6
SIGNW:        033e 0118  ld   [$18]
              033f a001  suba $01
              0340 c218  st   [$18]
              0341 011b  ld   [$1b]
              0342 ec4d  bne  .testw3
              0343 e850  blt  .testw4
              0344 c21b  st   [$1b]
              0345 011a  ld   [$1a]
              0346 ec49  bne  .testw1
              0347 fc4a  bra  .testw2
.testw0:      0348 0000  ld   $00
.testw1:      0349 0001  ld   $01
.testw2:      034a c21a  st   [$1a]
              034b fc04  bra  NEXT
              034c 00f4  ld   $f4
.testw3:      034d 0000  ld   $00
              034e fc48  bra  .testw0
              034f c21b  st   [$1b]
.testw4:      0350 00ff  ld   $ff
              0351 c21b  st   [$1b]
              0352 fc4a  bra  .testw2
              0353 0200  nop
BEQ:          0354 011a  ld   [$1a]
              0355 ec59  bne  br1
              0356 0d00  ld   [y,x]
br0:          0357 c218  st   [$18]
              0358 fc04  bra  NEXT
br1:          0359 00f8  ld   $f8
              035a fc04  bra  NEXT
ST:           035b 1200  ld   ac,x
              035c 011a  ld   [$1a]
              035d fc02  bra  next14
              035e c61a  st   [x]
BNE:          035f 011a  ld   [$1a]
              0360 ec57  bne  br0
              0361 0d00  ld   [y,x]
              0362 00f8  ld   $f8
              0363 fc04  bra  NEXT
AND:          0364 1200  ld   ac,x
              0365 011a  ld   [$1a]
              0366 fc02  bra  next14
              0367 2500  anda [x]
ANDI:         0368 211a  anda [$1a]
              0369 c21a  st   [$1a]
              036a 0000  ld   $00
              036b c21b  st   [$1b]
              036c fc04  bra  NEXT
              036d 00f8  ld   $f8
ORI:          036e 411a  ora  [$1a]
              036f c21a  st   [$1a]
              0370 fc04  bra  NEXT
              0371 00f9  ld   $f9
XORI:         0372 611a  xora [$1a]
              0373 c21a  st   [$1a]
              0374 fc04  bra  NEXT
              0375 00f9  ld   $f9
BGT:          0376 011a  ld   [$1a]
              0377 e457  bgt  br0
              0378 0d00  ld   [y,x]
              0379 00f8  ld   $f8
              037a fc04  bra  NEXT
OR:           037b 1200  ld   ac,x
              037c 011a  ld   [$1a]
              037d fc02  bra  next14
              037e 4500  ora  [x]
BLT:          037f 011a  ld   [$1a]
              0380 e857  blt  br0
              0381 0d00  ld   [y,x]
              0382 00f8  ld   $f8
              0383 fc04  bra  NEXT
XOR:          0384 1200  ld   ac,x
              0385 011a  ld   [$1a]
              0386 fc02  bra  next14
              0387 6500  xora [x]
BRA:          0388 c218  st   [$18]
              0389 00f9  ld   $f9
              038a fc04  bra  NEXT
              038b 0200  nop
BGE:          038c 011a  ld   [$1a]
              038d f457  bge  br0
              038e 0d00  ld   [y,x]
              038f 00f8  ld   $f8
              0390 fc04  bra  NEXT
              0391 0200  nop
BLE:          0392 011a  ld   [$1a]
              0393 f857  ble  br0
              0394 0d00  ld   [y,x]
              0395 00f8  ld   $f8
              0396 fc04  bra  NEXT
              0397 0200  nop
ADDW:         0398 1200  ld   ac,x
              0399 8001  adda $01
              039a c21f  st   [$1f]
              039b 011a  ld   [$1a]
              039c 8500  adda [x]
              039d c21a  st   [$1a]
              039e e8a2  blt  .addw0
              039f a500  suba [x]
              03a0 fca4  bra  .addw1
              03a1 4500  ora  [x]
.addw0:       03a2 2500  anda [x]
              03a3 0200  nop
.addw1:       03a4 3080  anda $80,x
              03a5 0500  ld   [x]
              03a6 811b  adda [$1b]
              03a7 111f  ld   [$1f],x
              03a8 8500  adda [x]
              03a9 c21b  st   [$1b]
              03aa fc04  bra  NEXT
              03ab 00f2  ld   $f2
SUBW:         03ac 1200  ld   ac,x
              03ad 8001  adda $01
              03ae c21f  st   [$1f]
              03af 011a  ld   [$1a]
              03b0 e8b5  blt  .subw0
              03b1 a500  suba [x]
              03b2 c21a  st   [$1a]
              03b3 fcb8  bra  .subw1
              03b4 4500  ora  [x]
.subw0:       03b5 c21a  st   [$1a]
              03b6 2500  anda [x]
              03b7 0200  nop
.subw1:       03b8 3080  anda $80,x
              03b9 011b  ld   [$1b]
              03ba a500  suba [x]
              03bb 111f  ld   [$1f],x
              03bc a500  suba [x]
              03bd c21b  st   [$1b]
              03be 0200  nop
              03bf fc04  bra  NEXT
              03c0 00f2  ld   $f2
PEEK:         03c1 0118  ld   [$18]
              03c2 a001  suba $01
              03c3 c218  st   [$18]
              03c4 111a  ld   [$1a],x
              03c5 151b  ld   [$1b],y
              03c6 0d00  ld   [y,x]
              03c7 c21a  st   [$1a]
              03c8 0000  ld   $00
              03c9 c21b  st   [$1b]
              03ca 1519  ld   [$19],y
              03cb fc04  bra  NEXT
              03cc 00f5  ld   $f5
POKE:         03cd c21f  st   [$1f]
              03ce 9001  adda $01,x
              03cf 0500  ld   [x]
              03d0 1600  ld   ac,y
              03d1 111f  ld   [$1f],x
              03d2 0500  ld   [x]
              03d3 1200  ld   ac,x
              03d4 011a  ld   [$1a]
              03d5 ce00  st   [y,x]
              03d6 1519  ld   [$19],y
              03d7 fc04  bra  NEXT
              03d8 00f5  ld   $f5
LOOKUP:       03d9 c21f  st   [$1f]
              03da 9001  adda $01,x
              03db 0500  ld   [x]
              03dc 1600  ld   ac,y
              03dd 111f  ld   [$1f],x
              03de e0fb  jmp  y,$fb       ;Trampoline offset
              03df 0500  ld   [x]
.lookup0:     03e0 c21a  st   [$1a]
              03e1 0000  ld   $00
              03e2 c21b  st   [$1b]
              03e3 fc04  bra  NEXT
              03e4 00f2  ld   $f2
CALL:         03e5 0118  ld   [$18]
              03e6 8001  adda $01         ;CALL has no operand, advances PC by 1
              03e7 c21c  st   [$1c]
              03e8 011a  ld   [$1a]
              03e9 a002  suba $02         ;vAC is actual address, NEXT adds 2
              03ea c218  st   [$18]
              03eb 0119  ld   [$19]
              03ec c21d  st   [$1d]
              03ed 011b  ld   [$1b]
              03ee d619  st   [$19],y
              03ef fc04  bra  NEXT
              03f0 00f5  ld   $f5
SYS:          03f1 0200  nop
              03f2 0200  nop
              03f3 0200  nop
              * 15 times
font:         0400 007c  ld   $7c         ;Char ' '
              0401 0082  ld   $82
              0402 0082  ld   $82
              0403 0092  ld   $92
              0404 005c  ld   $5c
              0405 0000  ld   $00         ;Char '!'
              0406 0022  ld   $22
              0407 00be  ld   $be
              0408 0002  ld   $02
              0409 0000  ld   $00
              040a 0018  ld   $18         ;Char '"'
              040b 0025  ld   $25
              040c 0025  ld   $25
              040d 0025  ld   $25
              040e 001e  ld   $1e
              040f 0004  ld   $04         ;Char '#'
              0410 002a  ld   $2a
              0411 002a  ld   $2a
              0412 002a  ld   $2a
              0413 001e  ld   $1e
              0414 0020  ld   $20         ;Char '$'
              0415 00fc  ld   $fc
              0416 0022  ld   $22
              0417 0002  ld   $02
              0418 0004  ld   $04
              0419 003e  ld   $3e         ;Char '%'
              041a 0010  ld   $10
              041b 0020  ld   $20
              041c 0020  ld   $20
              041d 0010  ld   $10
              041e 001c  ld   $1c         ;Char '&'
              041f 0022  ld   $22
              0420 0022  ld   $22
              0421 0022  ld   $22
              0422 001c  ld   $1c
              0423 003e  ld   $3e         ;Char "'"
              0424 0010  ld   $10
              0425 0020  ld   $20
              0426 0020  ld   $20
              0427 001e  ld   $1e
              0428 0000  ld   $00         ;Char '('
              0429 0000  ld   $00
              042a 0000  ld   $00
              * 5 times
              042d 0080  ld   $80         ;Char ')'
              042e 0080  ld   $80
              042f 00fe  ld   $fe
              0430 0080  ld   $80
              0431 0080  ld   $80
              0432 0080  ld   $80         ;Char '*'
              0433 0080  ld   $80
              0434 00fe  ld   $fe
              0435 0080  ld   $80
              0436 0080  ld   $80
              0437 00fe  ld   $fe         ;Char '+'
              0438 0002  ld   $02
              0439 0002  ld   $02
              043a 0002  ld   $02
              043b 0002  ld   $02
              043c 0000  ld   $00         ;Char ','
              043d 0000  ld   $00
              043e 0000  ld   $00
              * 5 times
              0441 003e  ld   $3e         ;Char '-'
              0442 0020  ld   $20
              0443 001c  ld   $1c
              0444 0020  ld   $20
              0445 001e  ld   $1e
              0446 0000  ld   $00         ;Char '.'
              0447 0022  ld   $22
              0448 00be  ld   $be
              0449 0002  ld   $02
              044a 0000  ld   $00
              044b 001c  ld   $1c         ;Char '/'
              044c 0022  ld   $22
              044d 0022  ld   $22
              044e 0022  ld   $22
              044f 0014  ld   $14
              0450 003e  ld   $3e         ;Char '0'
              0451 0010  ld   $10
              0452 0020  ld   $20
              0453 0020  ld   $20
              0454 0010  ld   $10
              0455 001c  ld   $1c         ;Char '1'
              0456 0022  ld   $22
              0457 0022  ld   $22
              0458 0022  ld   $22
              0459 001c  ld   $1c
              045a 001c  ld   $1c         ;Char '2'
              045b 0022  ld   $22
              045c 0022  ld   $22
              045d 0022  ld   $22
              045e 0014  ld   $14
              045f 001c  ld   $1c         ;Char '3'
              0460 0022  ld   $22
              0461 0022  ld   $22
              0462 0022  ld   $22
              0463 001c  ld   $1c
              0464 003e  ld   $3e         ;Char '4'
              0465 0020  ld   $20
              0466 001c  ld   $1c
              0467 0020  ld   $20
              0468 001e  ld   $1e
              0469 003f  ld   $3f         ;Char '5'
              046a 0024  ld   $24
              046b 0024  ld   $24
              046c 0024  ld   $24
              046d 0018  ld   $18
              046e 003c  ld   $3c         ;Char '6'
              046f 0002  ld   $02
              0470 0002  ld   $02
              0471 0004  ld   $04
              0472 003e  ld   $3e
              0473 0020  ld   $20         ;Char '7'
              0474 00fc  ld   $fc
              0475 0022  ld   $22
              0476 0002  ld   $02
              0477 0004  ld   $04
              0478 001c  ld   $1c         ;Char '8'
              0479 002a  ld   $2a
              047a 002a  ld   $2a
              047b 002a  ld   $2a
              047c 0018  ld   $18
              047d 003e  ld   $3e         ;Char '9'
              047e 0010  ld   $10
              047f 0020  ld   $20
              0480 0020  ld   $20
              0481 0010  ld   $10
              0482 0000  ld   $00         ;Char ':'
              0483 0000  ld   $00
              0484 0000  ld   $00
              * 5 times
              0487 00fe  ld   $fe         ;Char ';'
              0488 0090  ld   $90
              0489 0098  ld   $98
              048a 0094  ld   $94
              048b 0062  ld   $62
              048c 007c  ld   $7c         ;Char '<'
              048d 0082  ld   $82
              048e 0082  ld   $82
              048f 0082  ld   $82
              0490 007c  ld   $7c
              0491 00fe  ld   $fe         ;Char '='
              0492 0040  ld   $40
              0493 0020  ld   $20
              0494 0040  ld   $40
              0495 00fe  ld   $fe
              0496 0038  ld   $38         ;Char '>'
              0497 0004  ld   $04
              0498 0002  ld   $02
              0499 0004  ld   $04
              049a 0038  ld   $38
              049b 007c  ld   $7c         ;Char '?'
              049c 008a  ld   $8a
              049d 0092  ld   $92
              049e 00a2  ld   $a2
              049f 007c  ld   $7c
              04a0 0200  nop
              04a1 0200  nop
              04a2 0200  nop
              * 91 times
              04fb fe00  bra  ac          ;Trampoline for page $04 lookups
              04fc fcfd  bra  $04fd
              04fd 1403  ld   $03,y
              04fe e0e0  jmp  y,$e0
              04ff 1519  ld   [$19],y
initVcpu:     0500 1000  ld   $00,x
              0501 1404  ld   $04,y
              0502 dc14  st   $14,[y,x++] ;0400 LDI
              0503 dc78  st   $78,[y,x++]
              0504 dc5b  st   $5b,[y,x++] ;0402 ST
              0505 dc12  st   $12,[y,x++]
              0506 dc23  st   $23,[y,x++] ;0404 LD
              0507 dc12  st   $12,[y,x++]
              0508 dc3e  st   $3e,[y,x++] ;0406 SIGNW
              0509 dc5f  st   $5f,[y,x++] ;0407 BNE
              050a dc02  st   $02,[y,x++]
              050b dc14  st   $14,[y,x++] ;0409 LDI
              050c dc3e  st   $3e,[y,x++]
              050d dc34  st   $34,[y,x++] ;040b STW
              050e dc9f  st   $9f,[y,x++] ;040c 'P'
              050f dc1a  st   $1a,[y,x++] ;040d LDWI
              0510 dc00  st   $00,[y,x++]
              0511 dc04  st   $04,[y,x++]
              0512 dc34  st   $34,[y,x++] ;0410 STW
              0513 dc8b  st   $8b,[y,x++] ;0411 'F'
              0514 dc1a  st   $1a,[y,x++] ;0412 LDWI
              0515 dc00  st   $00,[y,x++]
              0516 dc08  st   $08,[y,x++]
              0517 dc34  st   $34,[y,x++] ;0415 STW
              0518 dca5  st   $a5,[y,x++] ;0416 'S'
              0519 dc1a  st   $1a,[y,x++] ;0417 LDWI
              051a dc00  st   $00,[y,x++]
              051b dc10  st   $10,[y,x++]
              051c dc34  st   $34,[y,x++] ;041a STW
              051d dca9  st   $a9,[y,x++] ;041b 'U'
              051e dcd9  st   $d9,[y,x++] ;041c LOOKUP
              051f dc8b  st   $8b,[y,x++] ;041d 'F'
              0520 dc34  st   $34,[y,x++] ;041e STW
              0521 dc81  st   $81,[y,x++] ;041f 'A'
              0522 dc2a  st   $2a,[y,x++] ;0420 LDW
              0523 dca5  st   $a5,[y,x++] ;0421 'S'
              0524 dc34  st   $34,[y,x++] ;0422 STW
              0525 dca7  st   $a7,[y,x++] ;0423 'T'
              0526 dc2a  st   $2a,[y,x++] ;0424 LDW
              0527 dc81  st   $81,[y,x++] ;0425 'A'
              0528 dc68  st   $68,[y,x++] ;0426 ANDI
              0529 dc80  st   $80,[y,x++]
              052a dc3e  st   $3e,[y,x++] ;0428 SIGNW
              052b dc5f  st   $5f,[y,x++] ;0429 BNE
              052c dc2f  st   $2f,[y,x++]
              052d dc14  st   $14,[y,x++] ;042b LDI
              052e dc00  st   $00,[y,x++]
              052f dccd  st   $cd,[y,x++] ;042d POKE
              0530 dca7  st   $a7,[y,x++] ;042e 'T'
              0531 dc88  st   $88,[y,x++] ;042f BRA
              0532 dc33  st   $33,[y,x++]
              0533 dc2a  st   $2a,[y,x++] ;0431 LDW
              0534 dc9f  st   $9f,[y,x++] ;0432 'P'
              0535 dccd  st   $cd,[y,x++] ;0433 POKE
              0536 dca7  st   $a7,[y,x++] ;0434 'T'
              0537 dc2a  st   $2a,[y,x++] ;0435 LDW
              0538 dc81  st   $81,[y,x++] ;0436 'A'
              0539 dc98  st   $98,[y,x++] ;0437 ADDW
              053a dc81  st   $81,[y,x++] ;0438 'A'
              053b dc34  st   $34,[y,x++] ;0439 STW
              053c dc81  st   $81,[y,x++] ;043a 'A'
              053d dc1a  st   $1a,[y,x++] ;043b LDWI
              053e dc00  st   $00,[y,x++]
              053f dc01  st   $01,[y,x++]
              0540 dc98  st   $98,[y,x++] ;043e ADDW
              0541 dca7  st   $a7,[y,x++] ;043f 'T'
              0542 dc34  st   $34,[y,x++] ;0440 STW
              0543 dca7  st   $a7,[y,x++] ;0441 'T'
              0544 dcac  st   $ac,[y,x++] ;0442 SUBW
              0545 dca9  st   $a9,[y,x++] ;0443 'U'
              0546 dc3e  st   $3e,[y,x++] ;0444 SIGNW
              0547 dc7f  st   $7f,[y,x++] ;0445 BLT
              0548 dc22  st   $22,[y,x++]
              0549 dc14  st   $14,[y,x++] ;0447 LDI
              054a dc01  st   $01,[y,x++]
              054b dc98  st   $98,[y,x++] ;0449 ADDW
              054c dc8b  st   $8b,[y,x++] ;044a 'F'
              054d dc34  st   $34,[y,x++] ;044b STW
              054e dc8b  st   $8b,[y,x++] ;044c 'F'
              054f dc14  st   $14,[y,x++] ;044d LDI
              0550 dc01  st   $01,[y,x++]
              0551 dc98  st   $98,[y,x++] ;044f ADDW
              0552 dca5  st   $a5,[y,x++] ;0450 'S'
              0553 dc34  st   $34,[y,x++] ;0451 STW
              0554 dca5  st   $a5,[y,x++] ;0452 'S'
              0555 dc68  st   $68,[y,x++] ;0453 ANDI
              0556 dcff  st   $ff,[y,x++]
              0557 dc72  st   $72,[y,x++] ;0455 XORI
              0558 dca0  st   $a0,[y,x++]
              0559 dc3e  st   $3e,[y,x++] ;0457 SIGNW
              055a dc5f  st   $5f,[y,x++] ;0458 BNE
              055b dc1a  st   $1a,[y,x++]
              055c dc2a  st   $2a,[y,x++] ;045a LDW
              055d dc9f  st   $9f,[y,x++] ;045b 'P'
              055e dc72  st   $72,[y,x++] ;045c XORI
              055f dc08  st   $08,[y,x++]
              0560 dc3e  st   $3e,[y,x++] ;045e SIGNW
              0561 dc54  st   $54,[y,x++] ;045f BEQ
              0562 dc68  st   $68,[y,x++]
              0563 dc1a  st   $1a,[y,x++] ;0461 LDWI
              0564 dcf7  st   $f7,[y,x++]
              0565 dcff  st   $ff,[y,x++]
              0566 dc98  st   $98,[y,x++] ;0464 ADDW
              0567 dc9f  st   $9f,[y,x++] ;0465 'P'
              0568 dc34  st   $34,[y,x++] ;0466 STW
              0569 dc9f  st   $9f,[y,x++] ;0467 'P'
              056a dc88  st   $88,[y,x++] ;0468 BRA
              056b dc0b  st   $0b,[y,x++]
              056c dc1a  st   $1a,[y,x++] ;046a LDWI
              056d dc11  st   $11,[y,x++]
              056e dc01  st   $01,[y,x++]
              056f dc34  st   $34,[y,x++] ;046d STW
              0570 dca5  st   $a5,[y,x++] ;046e 'S'
              0571 dc14  st   $14,[y,x++] ;046f LDI
              0572 dc00  st   $00,[y,x++]
              0573 dc34  st   $34,[y,x++] ;0471 STW
              0574 dca1  st   $a1,[y,x++] ;0472 'Q'
              0575 dc34  st   $34,[y,x++] ;0473 STW
              0576 dca3  st   $a3,[y,x++] ;0474 'R'
              0577 dc34  st   $34,[y,x++] ;0475 STW
              0578 dc93  st   $93,[y,x++] ;0476 'J'
              0579 dc34  st   $34,[y,x++] ;0477 STW
              057a dc89  st   $89,[y,x++] ;0478 'E'
              057b dc34  st   $34,[y,x++] ;0479 STW
              057c dcb3  st   $b3,[y,x++] ;047a 'Z'
              057d dc34  st   $34,[y,x++] ;047b STW
              057e dc8d  st   $8d,[y,x++] ;047c 'G'
              057f dc1a  st   $1a,[y,x++] ;047d LDWI
              0580 dc00  st   $00,[y,x++]
              0581 dc18  st   $18,[y,x++]
              0582 dc34  st   $34,[y,x++] ;0480 STW
              0583 dcb1  st   $b1,[y,x++] ;0481 'Y'
              0584 dc34  st   $34,[y,x++] ;0482 STW
              0585 dc83  st   $83,[y,x++] ;0483 'B'
              0586 dc14  st   $14,[y,x++] ;0484 LDI
              0587 dc5a  st   $5a,[y,x++]
              0588 dc34  st   $34,[y,x++] ;0486 STW
              0589 dc8f  st   $8f,[y,x++] ;0487 'H'
              058a dc1a  st   $1a,[y,x++] ;0488 LDWI
              058b dc00  st   $00,[y,x++]
              058c dc05  st   $05,[y,x++]
              058d dce5  st   $e5,[y,x++] ;048b CALL
              058e 1000  ld   $00,x
              058f 1405  ld   $05,y
              0590 dc2a  st   $2a,[y,x++] ;0500 LDW
              0591 dca5  st   $a5,[y,x++] ;0501 'S'
              0592 dcc1  st   $c1,[y,x++] ;0502 PEEK
              0593 dc34  st   $34,[y,x++] ;0503 STW
              0594 dcaf  st   $af,[y,x++] ;0504 'X'
              0595 dc72  st   $72,[y,x++] ;0505 XORI
              0596 dc78  st   $78,[y,x++]
              0597 dc3e  st   $3e,[y,x++] ;0507 SIGNW
              0598 dc5f  st   $5f,[y,x++] ;0508 BNE
              0599 dc0c  st   $0c,[y,x++]
              059a dc14  st   $14,[y,x++] ;050a LDI
              059b dc0a  st   $0a,[y,x++]
              059c dc34  st   $34,[y,x++] ;050c STW
              059d dc89  st   $89,[y,x++] ;050d 'E'
              059e dc2a  st   $2a,[y,x++] ;050e LDW
              059f dca5  st   $a5,[y,x++] ;050f 'S'
              05a0 dcc1  st   $c1,[y,x++] ;0510 PEEK
              05a1 dc98  st   $98,[y,x++] ;0511 ADDW
              05a2 dca3  st   $a3,[y,x++] ;0512 'R'
              05a3 dc98  st   $98,[y,x++] ;0513 ADDW
              05a4 dca1  st   $a1,[y,x++] ;0514 'Q'
              05a5 dc34  st   $34,[y,x++] ;0515 STW
              05a6 dca3  st   $a3,[y,x++] ;0516 'R'
              05a7 dc98  st   $98,[y,x++] ;0517 ADDW
              05a8 dca1  st   $a1,[y,x++] ;0518 'Q'
              05a9 dc3e  st   $3e,[y,x++] ;0519 SIGNW
              05aa dc8c  st   $8c,[y,x++] ;051a BGE
              05ab dc1c  st   $1c,[y,x++]
              05ac dc72  st   $72,[y,x++] ;051c XORI
              05ad dc2b  st   $2b,[y,x++]
              05ae dc98  st   $98,[y,x++] ;051e ADDW
              05af dca1  st   $a1,[y,x++] ;051f 'Q'
              05b0 dc98  st   $98,[y,x++] ;0520 ADDW
              05b1 dca1  st   $a1,[y,x++] ;0521 'Q'
              05b2 dc34  st   $34,[y,x++] ;0522 STW
              05b3 dca1  st   $a1,[y,x++] ;0523 'Q'
              05b4 dc1a  st   $1a,[y,x++] ;0524 LDWI
              05b5 dcc4  st   $c4,[y,x++]
              05b6 dcff  st   $ff,[y,x++]
              05b7 dc98  st   $98,[y,x++] ;0527 ADDW
              05b8 dc8f  st   $8f,[y,x++] ;0528 'H'
              05b9 dc3e  st   $3e,[y,x++] ;0529 SIGNW
              05ba dc92  st   $92,[y,x++] ;052a BLE
              05bb dc38  st   $38,[y,x++]
              05bc dc2a  st   $2a,[y,x++] ;052c LDW
              05bd dca3  st   $a3,[y,x++] ;052d 'R'
              05be dc68  st   $68,[y,x++] ;052e ANDI
              05bf dc10  st   $10,[y,x++]
              05c0 dc3e  st   $3e,[y,x++] ;0530 SIGNW
              05c1 dc54  st   $54,[y,x++] ;0531 BEQ
              05c2 dc38  st   $38,[y,x++]
              05c3 dc1a  st   $1a,[y,x++] ;0533 LDWI
              05c4 dcff  st   $ff,[y,x++]
              05c5 dcff  st   $ff,[y,x++]
              05c6 dc98  st   $98,[y,x++] ;0536 ADDW
              05c7 dc8f  st   $8f,[y,x++] ;0537 'H'
              05c8 dc34  st   $34,[y,x++] ;0538 STW
              05c9 dc8f  st   $8f,[y,x++] ;0539 'H'
              05ca dc1a  st   $1a,[y,x++] ;053a LDWI
              05cb dca6  st   $a6,[y,x++]
              05cc dcff  st   $ff,[y,x++]
              05cd dc98  st   $98,[y,x++] ;053d ADDW
              05ce dc8f  st   $8f,[y,x++] ;053e 'H'
              05cf dc3e  st   $3e,[y,x++] ;053f SIGNW
              05d0 dc8c  st   $8c,[y,x++] ;0540 BGE
              05d1 dc4d  st   $4d,[y,x++]
              05d2 dc2a  st   $2a,[y,x++] ;0542 LDW
              05d3 dca3  st   $a3,[y,x++] ;0543 'R'
              05d4 dc68  st   $68,[y,x++] ;0544 ANDI
              05d5 dc80  st   $80,[y,x++]
              05d6 dc3e  st   $3e,[y,x++] ;0546 SIGNW
              05d7 dc54  st   $54,[y,x++] ;0547 BEQ
              05d8 dc4d  st   $4d,[y,x++]
              05d9 dc14  st   $14,[y,x++] ;0549 LDI
              05da dc01  st   $01,[y,x++]
              05db dc98  st   $98,[y,x++] ;054b ADDW
              05dc dc8f  st   $8f,[y,x++] ;054c 'H'
              05dd dc34  st   $34,[y,x++] ;054d STW
              05de dc8f  st   $8f,[y,x++] ;054e 'H'
              05df dc14  st   $14,[y,x++] ;054f LDI
              05e0 dc08  st   $08,[y,x++]
              05e1 dc98  st   $98,[y,x++] ;0551 ADDW
              05e2 dc8f  st   $8f,[y,x++] ;0552 'H'
              05e3 dc5b  st   $5b,[y,x++] ;0553 ST
              05e4 dc94  st   $94,[y,x++] ;0554 'J'+1
              05e5 dc14  st   $14,[y,x++] ;0555 LDI
              05e6 dca0  st   $a0,[y,x++]
              05e7 dc98  st   $98,[y,x++] ;0557 ADDW
              05e8 dcaf  st   $af,[y,x++] ;0558 'X'
              05e9 dc68  st   $68,[y,x++] ;0559 ANDI
              05ea dcff  st   $ff,[y,x++]
              05eb dc34  st   $34,[y,x++] ;055b STW
              05ec dca7  st   $a7,[y,x++] ;055c 'T'
              05ed dc1a  st   $1a,[y,x++] ;055d LDWI
              05ee dc00  st   $00,[y,x++]
              05ef dc10  st   $10,[y,x++]
              05f0 dc98  st   $98,[y,x++] ;0560 ADDW
              05f1 dca7  st   $a7,[y,x++] ;0561 'T'
              05f2 dc34  st   $34,[y,x++] ;0562 STW
              05f3 dcab  st   $ab,[y,x++] ;0563 'V'
              05f4 dccd  st   $cd,[y,x++] ;0564 POKE
              05f5 dcab  st   $ab,[y,x++] ;0565 'V'
              05f6 dc1a  st   $1a,[y,x++] ;0566 LDWI
              05f7 dc00  st   $00,[y,x++]
              05f8 dc01  st   $01,[y,x++]
              05f9 dc98  st   $98,[y,x++] ;0569 ADDW
              05fa dcab  st   $ab,[y,x++] ;056a 'V'
              05fb dc34  st   $34,[y,x++] ;056b STW
              05fc dcab  st   $ab,[y,x++] ;056c 'V'
              05fd dc3e  st   $3e,[y,x++] ;056d SIGNW
              05fe dc76  st   $76,[y,x++] ;056e BGT
              05ff dc62  st   $62,[y,x++]
              0600 dc2a  st   $2a,[y,x++] ;0570 LDW
              0601 dca3  st   $a3,[y,x++] ;0571 'R'
              0602 dc68  st   $68,[y,x++] ;0572 ANDI
              0603 dc7f  st   $7f,[y,x++]
              0604 dc34  st   $34,[y,x++] ;0574 STW
              0605 dc81  st   $81,[y,x++] ;0575 'A'
              0606 dc1a  st   $1a,[y,x++] ;0576 LDWI
              0607 dc91  st   $91,[y,x++]
              0608 dcff  st   $ff,[y,x++]
              0609 dc98  st   $98,[y,x++] ;0579 ADDW
              060a dc81  st   $81,[y,x++] ;057a 'A'
              060b dc3e  st   $3e,[y,x++] ;057b SIGNW
              060c dc92  st   $92,[y,x++] ;057c BLE
              060d dc80  st   $80,[y,x++]
              060e dc14  st   $14,[y,x++] ;057e LDI
              060f dc77  st   $77,[y,x++]
              0610 dc34  st   $34,[y,x++] ;0580 STW
              0611 dc81  st   $81,[y,x++] ;0581 'A'
              0612 dc14  st   $14,[y,x++] ;0582 LDI
              0613 dc10  st   $10,[y,x++]
              0614 dc98  st   $98,[y,x++] ;0584 ADDW
              0615 dc81  st   $81,[y,x++] ;0585 'A'
              0616 dc5b  st   $5b,[y,x++] ;0586 ST
              0617 dc82  st   $82,[y,x++] ;0587 'A'+1
              0618 dc2a  st   $2a,[y,x++] ;0588 LDW
              0619 dca7  st   $a7,[y,x++] ;0589 'T'
              061a dc5b  st   $5b,[y,x++] ;058a ST
              061b dc81  st   $81,[y,x++] ;058b 'A'
              061c dc14  st   $14,[y,x++] ;058c LDI
              061d dc02  st   $02,[y,x++]
              061e dccd  st   $cd,[y,x++] ;058e POKE
              061f dc81  st   $81,[y,x++] ;058f 'A'
              0620 dc2a  st   $2a,[y,x++] ;0590 LDW
              0621 dc93  st   $93,[y,x++] ;0591 'J'
              0622 dc98  st   $98,[y,x++] ;0592 ADDW
              0623 dca7  st   $a7,[y,x++] ;0593 'T'
              0624 dc34  st   $34,[y,x++] ;0594 STW
              0625 dcab  st   $ab,[y,x++] ;0595 'V'
              0626 dc14  st   $14,[y,x++] ;0596 LDI
              0627 dc3f  st   $3f,[y,x++]
              0628 dccd  st   $cd,[y,x++] ;0598 POKE
              0629 dcab  st   $ab,[y,x++] ;0599 'V'
              062a dc1a  st   $1a,[y,x++] ;059a LDWI
              062b dc00  st   $00,[y,x++]
              062c dc01  st   $01,[y,x++]
              062d dc98  st   $98,[y,x++] ;059d ADDW
              062e dcab  st   $ab,[y,x++] ;059e 'V'
              062f dc34  st   $34,[y,x++] ;059f STW
              0630 dcab  st   $ab,[y,x++] ;05a0 'V'
              0631 dc2a  st   $2a,[y,x++] ;05a1 LDW
              0632 dcaf  st   $af,[y,x++] ;05a2 'X'
              0633 dc68  st   $68,[y,x++] ;05a3 ANDI
              0634 dc08  st   $08,[y,x++]
              0635 dc98  st   $98,[y,x++] ;05a5 ADDW
              0636 dc8f  st   $8f,[y,x++] ;05a6 'H'
              0637 dc34  st   $34,[y,x++] ;05a7 STW
              0638 dc85  st   $85,[y,x++] ;05a8 'C'
              0639 dc14  st   $14,[y,x++] ;05a9 LDI
              063a dc01  st   $01,[y,x++]
              063b dc98  st   $98,[y,x++] ;05ab ADDW
              063c dc85  st   $85,[y,x++] ;05ac 'C'
              063d dc34  st   $34,[y,x++] ;05ad STW
              063e dc85  st   $85,[y,x++] ;05ae 'C'
              063f dc68  st   $68,[y,x++] ;05af ANDI
              0640 dc08  st   $08,[y,x++]
              0641 dc3e  st   $3e,[y,x++] ;05b1 SIGNW
              0642 dc5f  st   $5f,[y,x++] ;05b2 BNE
              0643 dcb6  st   $b6,[y,x++]
              0644 dc14  st   $14,[y,x++] ;05b4 LDI
              0645 dc2a  st   $2a,[y,x++]
              0646 dc88  st   $88,[y,x++] ;05b6 BRA
              0647 dcb8  st   $b8,[y,x++]
              0648 dc14  st   $14,[y,x++] ;05b8 LDI
              0649 dc20  st   $20,[y,x++]
              064a dccd  st   $cd,[y,x++] ;05ba POKE
              064b dcab  st   $ab,[y,x++] ;05bb 'V'
              064c dc1a  st   $1a,[y,x++] ;05bc LDWI
              064d dc00  st   $00,[y,x++]
              064e dc01  st   $01,[y,x++]
              064f dc98  st   $98,[y,x++] ;05bf ADDW
              0650 dcab  st   $ab,[y,x++] ;05c0 'V'
              0651 dc34  st   $34,[y,x++] ;05c1 STW
              0652 dcab  st   $ab,[y,x++] ;05c2 'V'
              0653 dc3e  st   $3e,[y,x++] ;05c3 SIGNW
              0654 dc76  st   $76,[y,x++] ;05c4 BGT
              0655 dca7  st   $a7,[y,x++]
              0656 dc1a  st   $1a,[y,x++] ;05c6 LDWI
              0657 dc00  st   $00,[y,x++]
              0658 dc06  st   $06,[y,x++]
              0659 dce5  st   $e5,[y,x++] ;05c9 CALL
              065a dc88  st   $88,[y,x++] ;05ca BRA
              065b dcfe  st   $fe,[y,x++]
              065c 1000  ld   $00,x
              065d 1406  ld   $06,y
              065e dc23  st   $23,[y,x++] ;0600 LD
              065f dc08  st   $08,[y,x++]
              0660 dc3e  st   $3e,[y,x++] ;0602 SIGNW
              0661 dc5f  st   $5f,[y,x++] ;0603 BNE
              0662 dcfe  st   $fe,[y,x++]
              0663 dc14  st   $14,[y,x++] ;0605 LDI
              0664 dc01  st   $01,[y,x++]
              0665 dc98  st   $98,[y,x++] ;0607 ADDW
              0666 dcaf  st   $af,[y,x++] ;0608 'X'
              0667 dccd  st   $cd,[y,x++] ;0609 POKE
              0668 dca5  st   $a5,[y,x++] ;060a 'S'
              0669 dc2a  st   $2a,[y,x++] ;060b LDW
              066a dc8d  st   $8d,[y,x++] ;060c 'G'
              066b dccd  st   $cd,[y,x++] ;060d POKE
              066c dc83  st   $83,[y,x++] ;060e 'B'
              066d dc2a  st   $2a,[y,x++] ;060f LDW
              066e dc89  st   $89,[y,x++] ;0610 'E'
              066f dc98  st   $98,[y,x++] ;0611 ADDW
              0670 dcb3  st   $b3,[y,x++] ;0612 'Z'
              0671 dc34  st   $34,[y,x++] ;0613 STW
              0672 dcb3  st   $b3,[y,x++] ;0614 'Z'
              0673 dc2a  st   $2a,[y,x++] ;0615 LDW
              0674 dcb1  st   $b1,[y,x++] ;0616 'Y'
              0675 dc98  st   $98,[y,x++] ;0617 ADDW
              0676 dcb3  st   $b3,[y,x++] ;0618 'Z'
              0677 dc34  st   $34,[y,x++] ;0619 STW
              0678 dcb1  st   $b1,[y,x++] ;061a 'Y'
              0679 dc14  st   $14,[y,x++] ;061b LDI
              067a dc32  st   $32,[y,x++]
              067b dc98  st   $98,[y,x++] ;061d ADDW
              067c dcaf  st   $af,[y,x++] ;061e 'X'
              067d dc34  st   $34,[y,x++] ;061f STW
              067e dc83  st   $83,[y,x++] ;0620 'B'
              067f dc23  st   $23,[y,x++] ;0621 LD
              0680 dcb2  st   $b2,[y,x++] ;0622 'Y'+1
              0681 dc5b  st   $5b,[y,x++] ;0623 ST
              0682 dc84  st   $84,[y,x++] ;0624 'B'+1
              0683 dc2a  st   $2a,[y,x++] ;0625 LDW
              0684 dc83  st   $83,[y,x++] ;0626 'B'
              0685 dcc1  st   $c1,[y,x++] ;0627 PEEK
              0686 dc34  st   $34,[y,x++] ;0628 STW
              0687 dc8d  st   $8d,[y,x++] ;0629 'G'
              0688 dc14  st   $14,[y,x++] ;062a LDI
              0689 dc02  st   $02,[y,x++]
              068a dcac  st   $ac,[y,x++] ;062c SUBW
              068b dc8d  st   $8d,[y,x++] ;062d 'G'
              068c dc3e  st   $3e,[y,x++] ;062e SIGNW
              068d dc8c  st   $8c,[y,x++] ;062f BGE
              068e dc4e  st   $4e,[y,x++]
              068f dc14  st   $14,[y,x++] ;0631 LDI
              0690 dc00  st   $00,[y,x++]
              0691 dcac  st   $ac,[y,x++] ;0633 SUBW
              0692 dcb3  st   $b3,[y,x++] ;0634 'Z'
              0693 dc34  st   $34,[y,x++] ;0635 STW
              0694 dcb3  st   $b3,[y,x++] ;0636 'Z'
              0695 dc2a  st   $2a,[y,x++] ;0637 LDW
              0696 dcb1  st   $b1,[y,x++] ;0638 'Y'
              0697 dc98  st   $98,[y,x++] ;0639 ADDW
              0698 dcb3  st   $b3,[y,x++] ;063a 'Z'
              0699 dc34  st   $34,[y,x++] ;063b STW
              069a dcb1  st   $b1,[y,x++] ;063c 'Y'
              069b dc14  st   $14,[y,x++] ;063d LDI
              069c dc0a  st   $0a,[y,x++]
              069d dc5b  st   $5b,[y,x++] ;063f ST
              069e dc12  st   $12,[y,x++]
              069f dc14  st   $14,[y,x++] ;0641 LDI
              06a0 dc32  st   $32,[y,x++]
              06a1 dc98  st   $98,[y,x++] ;0643 ADDW
              06a2 dcaf  st   $af,[y,x++] ;0644 'X'
              06a3 dc34  st   $34,[y,x++] ;0645 STW
              06a4 dc83  st   $83,[y,x++] ;0646 'B'
              06a5 dc23  st   $23,[y,x++] ;0647 LD
              06a6 dcb2  st   $b2,[y,x++] ;0648 'Y'+1
              06a7 dc5b  st   $5b,[y,x++] ;0649 ST
              06a8 dc84  st   $84,[y,x++] ;064a 'B'+1
              06a9 dc2a  st   $2a,[y,x++] ;064b LDW
              06aa dc83  st   $83,[y,x++] ;064c 'B'
              06ab dcc1  st   $c1,[y,x++] ;064d PEEK
              06ac dc34  st   $34,[y,x++] ;064e STW
              06ad dc8d  st   $8d,[y,x++] ;064f 'G'
              06ae dc14  st   $14,[y,x++] ;0650 LDI
              06af dc3f  st   $3f,[y,x++]
              06b0 dccd  st   $cd,[y,x++] ;0652 POKE
              06b1 dc83  st   $83,[y,x++] ;0653 'B'
              06b2 dc2a  st   $2a,[y,x++] ;0654 LDW
              06b3 dc1c  st   $1c,[y,x++] ;0655 vRT
              06b4 dce5  st   $e5,[y,x++] ;0656 CALL
              06b5 00fe  ld   $fe
              06b6 c218  st   [$18]
              06b7 0004  ld   $04
              06b8 c219  st   [$19]
              06b9 150f  ld   [$0f],y
              06ba e10e  jmp  y,[$0e]
              06bb 0200  nop
              06bc
