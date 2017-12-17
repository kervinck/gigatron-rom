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
              001e d220  st   [$20],x
              001f d621  st   [$21],y
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
              002a 0120  ld   [$20]
              002b 8001  adda $01
              002c ec20  bne  .initEnt0
              002d d220  st   [$20],x
              002e 0121  ld   [$21]
              002f 8001  adda $01
              0030 ec20  bne  .initEnt0
              0031 d621  st   [$21],y
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
.loop:        004f c220  st   [$20]
              0050 8200  adda ac
              0051 9200  adda ac,x
              0052 0120  ld   [$20]
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
              0079 1408  ld   $08,y
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
ENTER:        0300 fc04  bra  .next2      ;vCPU interpreter
              0301 1519  ld   [$19],y
NEXT:         0302 8120  adda [$20]       ;Track elapsed ticks
              0303 e80c  blt  EXIT        ;Escape near time out
.next2:       0304 c220  st   [$20]
              0305 0118  ld   [$18]       ;Advance vPC
              0306 8002  adda $02
              0307 d218  st   [$18],x
              0308 0d00  ld   [y,x]       ;Fetch opcode
              0309 de00  st   [y,x++]
              030a fe00  bra  ac          ;Dispatch
              030b 0d00  ld   [y,x]       ;Prefetch operand
EXIT:         030c 800e  adda $0e
              030d e40d  bgt  $030d       ;Resync
              030e a001  suba $01
              030f 150f  ld   [$0f],y
              0310 e10e  jmp  y,[$0e]     ;Return to caller
              0311 0200  nop
LDWI:         0312 c21a  st   [$1a]
              0313 de00  st   [y,x++]
              0314 0d00  ld   [y,x]
              0315 c21b  st   [$1b]
              0316 0118  ld   [$18]
              0317 8001  adda $01
              0318 c218  st   [$18]
              0319 00f6  ld   $f6
              031a fc02  bra  NEXT
LD:           031b 1200  ld   ac,x
              031c 0500  ld   [x]
              031d c21a  st   [$1a]
              031e 0000  ld   $00
              031f c21b  st   [$1b]
              0320 00f7  ld   $f7
              0321 fc02  bra  NEXT
LDW:          0322 1200  ld   ac,x
              0323 8001  adda $01
              0324 c221  st   [$21]
              0325 0500  ld   [x]
              0326 c21a  st   [$1a]
              0327 1121  ld   [$21],x
              0328 0500  ld   [x]
              0329 c21b  st   [$1b]
              032a fc02  bra  NEXT
              032b 00f6  ld   $f6
STW:          032c 1200  ld   ac,x
              032d 8001  adda $01
              032e c221  st   [$21]
              032f 011a  ld   [$1a]
              0330 c600  st   [x]
              0331 1121  ld   [$21],x
              0332 011b  ld   [$1b]
              0333 c600  st   [x]
              0334 fc02  bra  NEXT
              0335 00f6  ld   $f6
COND:         0336 011b  ld   [$1b]
              0337 ec40  bne  .cond2
              0338 c221  st   [$21]
              0339 011a  ld   [$1a]
              033a f043  beq  .cond3
              033b 0001  ld   $01
              033c c221  st   [$21]
              033d 0d00  ld   [y,x]
.cond1:       033e fe00  bra  ac
              033f 0121  ld   [$21]
.cond2:       0340 0200  nop
              0341 0200  nop
              0342 0200  nop
.cond3:       0343 fc3e  bra  .cond1
              0344 0d00  ld   [y,x]
.cond4:       0345 0118  ld   [$18]       ;False condition
              0346 fc4a  bra  .cond6
              0347 8001  adda $01
.cond5:       0348 de00  st   [y,x++]     ;True condition
              0349 0d00  ld   [y,x]
.cond6:       034a c218  st   [$18]
              034b fc02  bra  NEXT
              034c 00f2  ld   $f2
EQ:           034d ec45  bne  .cond4
              034e f048  beq  .cond5
              034f 0d00  ld   [y,x]
NE:           0350 f045  beq  .cond4
              0351 ec48  bne  .cond5
              0352 0d00  ld   [y,x]
GT:           0353 f845  ble  .cond4
              0354 e448  bgt  .cond5
              0355 0d00  ld   [y,x]
LT:           0356 f445  bge  .cond4
              0357 e848  blt  .cond5
              0358 0d00  ld   [y,x]
GE:           0359 e845  blt  .cond4
              035a f448  bge  .cond5
              035b 0d00  ld   [y,x]
LE:           035c e445  bgt  .cond4
              035d f848  ble  .cond5
              035e 0d00  ld   [y,x]
LDI:          035f c21a  st   [$1a]
              0360 0000  ld   $00
              0361 c21b  st   [$1b]
              0362 00f8  ld   $f8
              0363 fc02  bra  NEXT
ST:           0364 1200  ld   ac,x
              0365 011a  ld   [$1a]
              0366 c61a  st   [x]
              0367 00f8  ld   $f8
              0368 fc02  bra  NEXT
PULL:         0369 111e  ld   [$1e],x
              036a 0500  ld   [x]
              036b c21a  st   [$1a]
              036c 011e  ld   [$1e]
              036d 9001  adda $01,x
              036e 0500  ld   [x]
              036f c21b  st   [$1b]
              0370 011e  ld   [$1e]
              0371 8002  adda $02
              0372 c21e  st   [$1e]
next1:        0373 0118  ld   [$18]
              0374 a001  suba $01
              0375 c218  st   [$18]
              0376 00f3  ld   $f3
              0377 fc02  bra  NEXT
LOOKUP:       0378 c221  st   [$21]
              0379 9001  adda $01,x
              037a 0500  ld   [x]
              037b 1600  ld   ac,y
              037c 1121  ld   [$21],x
              037d e0fb  jmp  y,$fb       ;Trampoline offset
              037e 0500  ld   [x]
.lookup0:     037f c21a  st   [$1a]
              0380 0000  ld   $00
              0381 c21b  st   [$1b]
              0382 fc02  bra  NEXT
              0383 00f2  ld   $f2
PUSH:         0384 011e  ld   [$1e]
              0385 b001  suba $01,x
              0386 011d  ld   [$1d]
              0387 c600  st   [x]
              0388 011e  ld   [$1e]
              0389 a002  suba $02
              038a d21e  st   [$1e],x
              038b 011c  ld   [$1c]
              038c fc73  bra  next1
              038d c600  st   [x]
ANDI:         038e 211a  anda [$1a]
              038f c21a  st   [$1a]
              0390 0000  ld   $00
              0391 c21b  st   [$1b]
              0392 fc02  bra  NEXT
              0393 00f8  ld   $f8
ORI:          0394 411a  ora  [$1a]
              0395 c21a  st   [$1a]
              0396 fc02  bra  NEXT
              0397 00f9  ld   $f9
XORI:         0398 611a  xora [$1a]
              0399 c21a  st   [$1a]
              039a fc02  bra  NEXT
              039b 00f9  ld   $f9
BRA:          039c c218  st   [$18]
              039d 00f9  ld   $f9
              039e fc02  bra  NEXT
POKE:         039f c221  st   [$21]
              03a0 9001  adda $01,x
              03a1 0500  ld   [x]
              03a2 1600  ld   ac,y
              03a3 1121  ld   [$21],x
              03a4 0500  ld   [x]
              03a5 1200  ld   ac,x
              03a6 011a  ld   [$1a]
              03a7 ce00  st   [y,x]
              03a8 1519  ld   [$19],y
              03a9 fc02  bra  NEXT
              03aa 00f5  ld   $f5
retry:        03ab 0118  ld   [$18]       ;Retry until sufficient time
              03ac a002  suba $02
              03ad c218  st   [$18]
              03ae fce6  bra  RETURN
              03af 00f6  ld   $f6
SYS:          03b0 8120  adda [$20]
              03b1 e8ab  blt  retry
              03b2 151b  ld   [$1b],y
              03b3 e11a  jmp  y,[$1a]
SUBW:         03b4 1200  ld   ac,x
              03b5 8001  adda $01
              03b6 c221  st   [$21]
              03b7 011a  ld   [$1a]
              03b8 e8bd  blt  .subw0
              03b9 a500  suba [x]
              03ba c21a  st   [$1a]
              03bb fcc0  bra  .subw1
              03bc 4500  ora  [x]
.subw0:       03bd c21a  st   [$1a]
              03be 2500  anda [x]
              03bf 0200  nop
.subw1:       03c0 3080  anda $80,x
              03c1 011b  ld   [$1b]
              03c2 a500  suba [x]
              03c3 1121  ld   [$21],x
              03c4 a500  suba [x]
              03c5 c21b  st   [$1b]
              03c6 00f2  ld   $f2
              03c7 fc02  bra  NEXT
ADDW:         03c8 1200  ld   ac,x
              03c9 8001  adda $01
              03ca c221  st   [$21]
              03cb 011a  ld   [$1a]
              03cc 8500  adda [x]
              03cd c21a  st   [$1a]
              03ce e8d2  blt  .addw0
              03cf a500  suba [x]
              03d0 fcd4  bra  .addw1
              03d1 4500  ora  [x]
.addw0:       03d2 2500  anda [x]
              03d3 0200  nop
.addw1:       03d4 3080  anda $80,x
              03d5 0500  ld   [x]
              03d6 811b  adda [$1b]
              03d7 1121  ld   [$21],x
              03d8 8500  adda [x]
              03d9 c21b  st   [$1b]
              03da fc02  bra  NEXT
              03db 00f2  ld   $f2
PEEK:         03dc 0118  ld   [$18]
              03dd a001  suba $01
              03de c218  st   [$18]
              03df 111a  ld   [$1a],x
              03e0 151b  ld   [$1b],y
              03e1 0d00  ld   [y,x]
              03e2 c21a  st   [$1a]
              03e3 0000  ld   $00
              03e4 c21b  st   [$1b]
              03e5 00f5  ld   $f5
RETURN:       03e6 fc02  bra  NEXT        ;Return from SYS calls
              03e7 1519  ld   [$19],y
CALL:         03e8 0118  ld   [$18]
              03e9 8001  adda $01         ;CALL has no operand, advances PC by 1
              03ea c21c  st   [$1c]
              03eb 011a  ld   [$1a]
              03ec a002  suba $02         ;vAC is actual address, NEXT adds 2
              03ed c218  st   [$18]
              03ee 0119  ld   [$19]
              03ef c21d  st   [$1d]
              03f0 011b  ld   [$1b]
              03f1 d619  st   [$19],y
              03f2 fc02  bra  NEXT
              03f3 00f5  ld   $f5
ADDI:         03f4 1404  ld   $04,y
              03f5 e000  jmp  y,$00
              03f6 c221  st   [$21]
SUBI:         03f7 1404  ld   $04,y
              03f8 e00f  jmp  y,$0f
              03f9 c221  st   [$21]
INC:          03fa 1200  ld   ac,x
              03fb 0500  ld   [x]
              03fc 8001  adda $01
              03fd c600  st   [x]
              03fe fc02  bra  NEXT
              03ff 00f8  ld   $f8
addi:         0400 811a  adda [$1a]
              0401 c21a  st   [$1a]
              0402 e806  blt  .addi0
              0403 a121  suba [$21]
              0404 fc08  bra  .addi1
              0405 4121  ora  [$21]
.addi0:       0406 2121  anda [$21]
              0407 0200  nop
.addi1:       0408 3080  anda $80,x
              0409 0500  ld   [x]
              040a 811b  adda [$1b]
              040b c21b  st   [$1b]
              040c 1403  ld   $03,y
              040d e0e6  jmp  y,$e6
              040e 00f2  ld   $f2
subi:         040f 011a  ld   [$1a]
              0410 e815  blt  .subi0
              0411 a121  suba [$21]
              0412 c21a  st   [$1a]
              0413 fc18  bra  .subi1
              0414 4121  ora  [$21]
.subi0:       0415 c21a  st   [$1a]
              0416 2121  anda [$21]
              0417 0200  nop
.subi1:       0418 3080  anda $80,x
              0419 011b  ld   [$1b]
              041a a500  suba [x]
              041b c21b  st   [$1b]
              041c 1403  ld   $03,y
              041d e0e6  jmp  y,$e6
              041e 00f2  ld   $f2
              041f 0200  nop
              0420 0200  nop
              0421 0200  nop
              * 225 times
              0500 0000  ld   $00         ;Char ' '
              0501 0000  ld   $00
              0502 0000  ld   $00
              * 5 times
              0505 0000  ld   $00         ;Char '!'
              0506 0000  ld   $00
              0507 00fa  ld   $fa
              0508 0000  ld   $00
              0509 0000  ld   $00
              050a 00a0  ld   $a0         ;Char '"'
              050b 00c0  ld   $c0
              050c 0000  ld   $00
              050d 00a0  ld   $a0
              050e 00c0  ld   $c0
              050f 0028  ld   $28         ;Char '#'
              0510 00fe  ld   $fe
              0511 0028  ld   $28
              0512 00fe  ld   $fe
              0513 0028  ld   $28
              0514 0024  ld   $24         ;Char '$'
              0515 0054  ld   $54
              0516 00fe  ld   $fe
              0517 0054  ld   $54
              0518 0048  ld   $48
              0519 00c4  ld   $c4         ;Char '%'
              051a 00c8  ld   $c8
              051b 0010  ld   $10
              051c 0026  ld   $26
              051d 0046  ld   $46
              051e 006c  ld   $6c         ;Char '&'
              051f 0092  ld   $92
              0520 006a  ld   $6a
              0521 0004  ld   $04
              0522 000a  ld   $0a
              0523 0000  ld   $00         ;Char "'"
              0524 00a0  ld   $a0
              0525 00c0  ld   $c0
              0526 0000  ld   $00
              0527 0000  ld   $00
              0528 0000  ld   $00         ;Char '('
              0529 0038  ld   $38
              052a 0044  ld   $44
              052b 0082  ld   $82
              052c 0000  ld   $00
              052d 0000  ld   $00         ;Char ')'
              052e 0082  ld   $82
              052f 0044  ld   $44
              0530 0038  ld   $38
              0531 0000  ld   $00
              0532 0028  ld   $28         ;Char '*'
              0533 0010  ld   $10
              0534 007c  ld   $7c
              0535 0010  ld   $10
              0536 0028  ld   $28
              0537 0010  ld   $10         ;Char '+'
              0538 0010  ld   $10
              0539 007c  ld   $7c
              053a 0010  ld   $10
              053b 0010  ld   $10
              053c 0000  ld   $00         ;Char ','
              053d 0005  ld   $05
              053e 0006  ld   $06
              053f 0000  ld   $00
              0540 0000  ld   $00
              0541 0010  ld   $10         ;Char '-'
              0542 0010  ld   $10
              0543 0010  ld   $10
              * 5 times
              0546 0000  ld   $00         ;Char '.'
              0547 0002  ld   $02
              0548 0002  ld   $02
              0549 0000  ld   $00
              054a 0000  ld   $00
              054b 0000  ld   $00         ;Char '/'
              054c 0006  ld   $06
              054d 0018  ld   $18
              054e 0060  ld   $60
              054f 0000  ld   $00
              0550 007c  ld   $7c         ;Char '0'
              0551 008a  ld   $8a
              0552 0092  ld   $92
              0553 00a2  ld   $a2
              0554 007c  ld   $7c
              0555 0022  ld   $22         ;Char '1'
              0556 0042  ld   $42
              0557 00fe  ld   $fe
              0558 0002  ld   $02
              0559 0002  ld   $02
              055a 0046  ld   $46         ;Char '2'
              055b 008a  ld   $8a
              055c 0092  ld   $92
              055d 0092  ld   $92
              055e 0062  ld   $62
              055f 0044  ld   $44         ;Char '3'
              0560 0082  ld   $82
              0561 0092  ld   $92
              0562 0092  ld   $92
              0563 006c  ld   $6c
              0564 0018  ld   $18         ;Char '4'
              0565 0028  ld   $28
              0566 0048  ld   $48
              0567 00fe  ld   $fe
              0568 0008  ld   $08
              0569 00e4  ld   $e4         ;Char '5'
              056a 00a2  ld   $a2
              056b 00a2  ld   $a2
              056c 00a2  ld   $a2
              056d 009c  ld   $9c
              056e 007c  ld   $7c         ;Char '6'
              056f 0092  ld   $92
              0570 0092  ld   $92
              0571 0092  ld   $92
              0572 004c  ld   $4c
              0573 0080  ld   $80         ;Char '7'
              0574 008e  ld   $8e
              0575 0090  ld   $90
              0576 00a0  ld   $a0
              0577 00c0  ld   $c0
              0578 006c  ld   $6c         ;Char '8'
              0579 0092  ld   $92
              057a 0092  ld   $92
              057b 0092  ld   $92
              057c 006c  ld   $6c
              057d 0064  ld   $64         ;Char '9'
              057e 0092  ld   $92
              057f 0092  ld   $92
              0580 0092  ld   $92
              0581 007c  ld   $7c
              0582 0000  ld   $00         ;Char ':'
              0583 0012  ld   $12
              0584 0012  ld   $12
              0585 0000  ld   $00
              0586 0000  ld   $00
              0587 0000  ld   $00         ;Char ';'
              0588 0015  ld   $15
              0589 0016  ld   $16
              058a 0000  ld   $00
              058b 0000  ld   $00
              058c 0010  ld   $10         ;Char '<'
              058d 0028  ld   $28
              058e 0044  ld   $44
              058f 0082  ld   $82
              0590 0000  ld   $00
              0591 0024  ld   $24         ;Char '='
              0592 0024  ld   $24
              0593 0024  ld   $24
              * 5 times
              0596 0000  ld   $00         ;Char '>'
              0597 0082  ld   $82
              0598 0044  ld   $44
              0599 0028  ld   $28
              059a 0010  ld   $10
              059b 0040  ld   $40         ;Char '?'
              059c 0080  ld   $80
              059d 008a  ld   $8a
              059e 0090  ld   $90
              059f 0060  ld   $60
              05a0 004c  ld   $4c         ;Char '@'
              05a1 0092  ld   $92
              05a2 009e  ld   $9e
              05a3 0082  ld   $82
              05a4 007c  ld   $7c
              05a5 003e  ld   $3e         ;Char 'A'
              05a6 0048  ld   $48
              05a7 0088  ld   $88
              05a8 0048  ld   $48
              05a9 003e  ld   $3e
              05aa 00fe  ld   $fe         ;Char 'B'
              05ab 0092  ld   $92
              05ac 0092  ld   $92
              05ad 0092  ld   $92
              05ae 006c  ld   $6c
              05af 007c  ld   $7c         ;Char 'C'
              05b0 0082  ld   $82
              05b1 0082  ld   $82
              05b2 0082  ld   $82
              05b3 0044  ld   $44
              05b4 00fe  ld   $fe         ;Char 'D'
              05b5 0082  ld   $82
              05b6 0082  ld   $82
              05b7 0044  ld   $44
              05b8 0038  ld   $38
              05b9 00fe  ld   $fe         ;Char 'E'
              05ba 0092  ld   $92
              05bb 0092  ld   $92
              05bc 0092  ld   $92
              05bd 0082  ld   $82
              05be 00fe  ld   $fe         ;Char 'F'
              05bf 0090  ld   $90
              05c0 0090  ld   $90
              05c1 0090  ld   $90
              05c2 0080  ld   $80
              05c3 007c  ld   $7c         ;Char 'G'
              05c4 0082  ld   $82
              05c5 0082  ld   $82
              05c6 0092  ld   $92
              05c7 005c  ld   $5c
              05c8 00fe  ld   $fe         ;Char 'H'
              05c9 0010  ld   $10
              05ca 0010  ld   $10
              05cb 0010  ld   $10
              05cc 00fe  ld   $fe
              05cd 0000  ld   $00         ;Char 'I'
              05ce 0082  ld   $82
              05cf 00fe  ld   $fe
              05d0 0082  ld   $82
              05d1 0000  ld   $00
              05d2 0004  ld   $04         ;Char 'J'
              05d3 0002  ld   $02
              05d4 0082  ld   $82
              05d5 00fc  ld   $fc
              05d6 0080  ld   $80
              05d7 00fe  ld   $fe         ;Char 'K'
              05d8 0010  ld   $10
              05d9 0028  ld   $28
              05da 0044  ld   $44
              05db 0082  ld   $82
              05dc 00fe  ld   $fe         ;Char 'L'
              05dd 0002  ld   $02
              05de 0002  ld   $02
              05df 0002  ld   $02
              05e0 0002  ld   $02
              05e1 00fe  ld   $fe         ;Char 'M'
              05e2 0040  ld   $40
              05e3 0020  ld   $20
              05e4 0040  ld   $40
              05e5 00fe  ld   $fe
              05e6 00fe  ld   $fe         ;Char 'N'
              05e7 0020  ld   $20
              05e8 0010  ld   $10
              05e9 0008  ld   $08
              05ea 00fe  ld   $fe
              05eb 007c  ld   $7c         ;Char 'O'
              05ec 0082  ld   $82
              05ed 0082  ld   $82
              05ee 0082  ld   $82
              05ef 007c  ld   $7c
              05f0 00fe  ld   $fe         ;Char 'P'
              05f1 0090  ld   $90
              05f2 0090  ld   $90
              05f3 0090  ld   $90
              05f4 0060  ld   $60
              05f5 007c  ld   $7c         ;Char 'Q'
              05f6 0082  ld   $82
              05f7 008a  ld   $8a
              05f8 0084  ld   $84
              05f9 007a  ld   $7a
              05fa 0200  nop
              05fb fe00  bra  ac          ;Trampoline for page $0500 lookups
              05fc fcfd  bra  $05fd
              05fd 1403  ld   $03,y
              05fe e07f  jmp  y,$7f
              05ff 1519  ld   [$19],y
              0600 00fe  ld   $fe         ;Char 'R'
              0601 0090  ld   $90
              0602 0098  ld   $98
              0603 0094  ld   $94
              0604 0062  ld   $62
              0605 0062  ld   $62         ;Char 'S'
              0606 0092  ld   $92
              0607 0092  ld   $92
              0608 0092  ld   $92
              0609 008c  ld   $8c
              060a 0080  ld   $80         ;Char 'T'
              060b 0080  ld   $80
              060c 00fe  ld   $fe
              060d 0080  ld   $80
              060e 0080  ld   $80
              060f 00fc  ld   $fc         ;Char 'U'
              0610 0002  ld   $02
              0611 0002  ld   $02
              0612 0002  ld   $02
              0613 00fc  ld   $fc
              0614 00f0  ld   $f0         ;Char 'V'
              0615 000c  ld   $0c
              0616 0002  ld   $02
              0617 000c  ld   $0c
              0618 00f0  ld   $f0
              0619 00fe  ld   $fe         ;Char 'W'
              061a 0004  ld   $04
              061b 0008  ld   $08
              061c 0004  ld   $04
              061d 00fe  ld   $fe
              061e 00c6  ld   $c6         ;Char 'X'
              061f 0028  ld   $28
              0620 0010  ld   $10
              0621 0028  ld   $28
              0622 00c6  ld   $c6
              0623 00e0  ld   $e0         ;Char 'Y'
              0624 0010  ld   $10
              0625 000e  ld   $0e
              0626 0010  ld   $10
              0627 00e0  ld   $e0
              0628 0086  ld   $86         ;Char 'Z'
              0629 008a  ld   $8a
              062a 0092  ld   $92
              062b 00a2  ld   $a2
              062c 00c2  ld   $c2
              062d 0000  ld   $00         ;Char '['
              062e 00fe  ld   $fe
              062f 0082  ld   $82
              0630 0082  ld   $82
              0631 0000  ld   $00
              0632 0000  ld   $00         ;Char '\\'
              0633 0060  ld   $60
              0634 0018  ld   $18
              0635 0006  ld   $06
              0636 0000  ld   $00
              0637 0000  ld   $00         ;Char ']'
              0638 0082  ld   $82
              0639 0082  ld   $82
              063a 00fe  ld   $fe
              063b 0000  ld   $00
              063c 0020  ld   $20         ;Char '^'
              063d 0040  ld   $40
              063e 0080  ld   $80
              063f 0040  ld   $40
              0640 0020  ld   $20
              0641 0002  ld   $02         ;Char '_'
              0642 0002  ld   $02
              0643 0002  ld   $02
              * 5 times
              0646 0000  ld   $00         ;Char '`'
              0647 0000  ld   $00
              0648 00c0  ld   $c0
              0649 00a0  ld   $a0
              064a 0000  ld   $00
              064b 0004  ld   $04         ;Char 'a'
              064c 002a  ld   $2a
              064d 002a  ld   $2a
              064e 002a  ld   $2a
              064f 001e  ld   $1e
              0650 00fe  ld   $fe         ;Char 'b'
              0651 0022  ld   $22
              0652 0022  ld   $22
              0653 0022  ld   $22
              0654 001c  ld   $1c
              0655 001c  ld   $1c         ;Char 'c'
              0656 0022  ld   $22
              0657 0022  ld   $22
              0658 0022  ld   $22
              0659 0014  ld   $14
              065a 001c  ld   $1c         ;Char 'd'
              065b 0022  ld   $22
              065c 0022  ld   $22
              065d 0022  ld   $22
              065e 00fe  ld   $fe
              065f 001c  ld   $1c         ;Char 'e'
              0660 002a  ld   $2a
              0661 002a  ld   $2a
              0662 002a  ld   $2a
              0663 0018  ld   $18
              0664 0010  ld   $10         ;Char 'f'
              0665 007e  ld   $7e
              0666 0090  ld   $90
              0667 0080  ld   $80
              0668 0040  ld   $40
              0669 0018  ld   $18         ;Char 'g'
              066a 0025  ld   $25
              066b 0025  ld   $25
              066c 0025  ld   $25
              066d 001e  ld   $1e
              066e 00fe  ld   $fe         ;Char 'h'
              066f 0020  ld   $20
              0670 0020  ld   $20
              0671 0020  ld   $20
              0672 001e  ld   $1e
              0673 0000  ld   $00         ;Char 'i'
              0674 0022  ld   $22
              0675 00be  ld   $be
              0676 0002  ld   $02
              0677 0000  ld   $00
              0678 0002  ld   $02         ;Char 'j'
              0679 0001  ld   $01
              067a 0021  ld   $21
              067b 00be  ld   $be
              067c 0000  ld   $00
              067d 00fe  ld   $fe         ;Char 'k'
              067e 0008  ld   $08
              067f 0018  ld   $18
              0680 0024  ld   $24
              0681 0002  ld   $02
              0682 0000  ld   $00         ;Char 'l'
              0683 0082  ld   $82
              0684 00fe  ld   $fe
              0685 0002  ld   $02
              0686 0000  ld   $00
              0687 003e  ld   $3e         ;Char 'm'
              0688 0020  ld   $20
              0689 001c  ld   $1c
              068a 0020  ld   $20
              068b 001e  ld   $1e
              068c 003e  ld   $3e         ;Char 'n'
              068d 0010  ld   $10
              068e 0020  ld   $20
              068f 0020  ld   $20
              0690 001e  ld   $1e
              0691 001c  ld   $1c         ;Char 'o'
              0692 0022  ld   $22
              0693 0022  ld   $22
              0694 0022  ld   $22
              0695 001c  ld   $1c
              0696 003f  ld   $3f         ;Char 'p'
              0697 0024  ld   $24
              0698 0024  ld   $24
              0699 0024  ld   $24
              069a 0018  ld   $18
              069b 0018  ld   $18         ;Char 'q'
              069c 0024  ld   $24
              069d 0024  ld   $24
              069e 0024  ld   $24
              069f 003f  ld   $3f
              06a0 003e  ld   $3e         ;Char 'r'
              06a1 0010  ld   $10
              06a2 0020  ld   $20
              06a3 0020  ld   $20
              06a4 0010  ld   $10
              06a5 0012  ld   $12         ;Char 's'
              06a6 002a  ld   $2a
              06a7 002a  ld   $2a
              06a8 002a  ld   $2a
              06a9 0004  ld   $04
              06aa 0020  ld   $20         ;Char 't'
              06ab 00fc  ld   $fc
              06ac 0022  ld   $22
              06ad 0002  ld   $02
              06ae 0004  ld   $04
              06af 003c  ld   $3c         ;Char 'u'
              06b0 0002  ld   $02
              06b1 0002  ld   $02
              06b2 0004  ld   $04
              06b3 003e  ld   $3e
              06b4 0038  ld   $38         ;Char 'v'
              06b5 0004  ld   $04
              06b6 0002  ld   $02
              06b7 0004  ld   $04
              06b8 0038  ld   $38
              06b9 003c  ld   $3c         ;Char 'w'
              06ba 0002  ld   $02
              06bb 000c  ld   $0c
              06bc 0002  ld   $02
              06bd 003c  ld   $3c
              06be 0022  ld   $22         ;Char 'x'
              06bf 0014  ld   $14
              06c0 0008  ld   $08
              06c1 0014  ld   $14
              06c2 0022  ld   $22
              06c3 0038  ld   $38         ;Char 'y'
              06c4 0005  ld   $05
              06c5 0005  ld   $05
              06c6 0005  ld   $05
              06c7 003e  ld   $3e
              06c8 0022  ld   $22         ;Char 'z'
              06c9 0026  ld   $26
              06ca 002a  ld   $2a
              06cb 0032  ld   $32
              06cc 0022  ld   $22
              06cd 0010  ld   $10         ;Char '{'
              06ce 006c  ld   $6c
              06cf 0082  ld   $82
              06d0 0082  ld   $82
              06d1 0000  ld   $00
              06d2 0000  ld   $00         ;Char '|'
              06d3 0000  ld   $00
              06d4 00fe  ld   $fe
              06d5 0000  ld   $00
              06d6 0000  ld   $00
              06d7 0000  ld   $00         ;Char '}'
              06d8 0082  ld   $82
              06d9 0082  ld   $82
              06da 006c  ld   $6c
              06db 0010  ld   $10
              06dc 0040  ld   $40         ;Char '~'
              06dd 0080  ld   $80
              06de 0040  ld   $40
              06df 0020  ld   $20
              06e0 0040  ld   $40
              06e1 00fe  ld   $fe         ;Char '\x7f'
              06e2 00fe  ld   $fe
              06e3 00fe  ld   $fe
              * 5 times
              06e6 0200  nop
              06e7 0200  nop
              06e8 0200  nop
              * 21 times
              06fb fe00  bra  ac          ;Trampoline for page $0600 lookups
              06fc fcfd  bra  $06fd
              06fd 1403  ld   $03,y
              06fe e07f  jmp  y,$7f
              06ff 1519  ld   [$19],y
              0700 0047  ld   $47
              0701 0069  ld   $69
              0702 0067  ld   $67
              0703 0061  ld   $61
              0704 0074  ld   $74
              0705 0072  ld   $72
              0706 006f  ld   $6f
              0707 006e  ld   $6e
              0708 0020  ld   $20
              0709 0054  ld   $54
              070a 0054  ld   $54
              070b 004c  ld   $4c
              070c 0020  ld   $20
              070d 0063  ld   $63
              070e 006f  ld   $6f
              070f 006d  ld   $6d
              0710 0070  ld   $70
              0711 0075  ld   $75
              0712 0074  ld   $74
              0713 0065  ld   $65
              0714 0072  ld   $72
              0715 0020  ld   $20
              0716 0052  ld   $52
              0717 004f  ld   $4f
              0718 004d  ld   $4d
              0719 0030  ld   $30
              071a 0020  ld   $20
              071b 0000  ld   $00
              071c 0200  nop
              071d 0200  nop
              071e 0200  nop
              * 223 times
              07fb fe00  bra  ac          ;Trampoline for page $0700 lookups
              07fc fcfd  bra  $07fd
              07fd 1403  ld   $03,y
              07fe e07f  jmp  y,$7f
              07ff 1519  ld   [$19],y
initVcpu:     0800 1000  ld   $00,x
              0801 1404  ld   $04,y
              0802 dc5f  st   $5f,[y,x++] ;0400 LDI
              0803 dc78  st   $78,[y,x++]
              0804 dc64  st   $64,[y,x++] ;0402 ST
              0805 dc12  st   $12,[y,x++]
              0806 dc1b  st   $1b,[y,x++] ;0404 LD
              0807 dc12  st   $12,[y,x++]
              0808 dc36  st   $36,[y,x++] ;0406 COND
              0809 dc50  st   $50,[y,x++] ;0407 NE
              080a dc02  st   $02,[y,x++]
              080b dc5f  st   $5f,[y,x++] ;0409 LDI
              080c dc3e  st   $3e,[y,x++]
              080d dc2c  st   $2c,[y,x++] ;040b STW
              080e dc9f  st   $9f,[y,x++] ;040c 'P'
              080f dc12  st   $12,[y,x++] ;040d LDWI
              0810 dc00  st   $00,[y,x++]
              0811 dc07  st   $07,[y,x++]
              0812 dc2c  st   $2c,[y,x++] ;0410 STW
              0813 dc85  st   $85,[y,x++] ;0411 'C'
              0814 dc12  st   $12,[y,x++] ;0412 LDWI
              0815 dc00  st   $00,[y,x++]
              0816 dc05  st   $05,[y,x++]
              0817 dc2c  st   $2c,[y,x++] ;0415 STW
              0818 dc8b  st   $8b,[y,x++] ;0416 'F'
              0819 dc12  st   $12,[y,x++] ;0417 LDWI
              081a dc00  st   $00,[y,x++]
              081b dc08  st   $08,[y,x++]
              081c dc2c  st   $2c,[y,x++] ;041a STW
              081d dca5  st   $a5,[y,x++] ;041b 'S'
              081e dc12  st   $12,[y,x++] ;041c LDWI
              081f dc00  st   $00,[y,x++]
              0820 dc10  st   $10,[y,x++]
              0821 dc2c  st   $2c,[y,x++] ;041f STW
              0822 dca9  st   $a9,[y,x++] ;0420 'U'
              0823 dc78  st   $78,[y,x++] ;0421 LOOKUP
              0824 dc85  st   $85,[y,x++] ;0422 'C'
              0825 dc2c  st   $2c,[y,x++] ;0423 STW
              0826 dc87  st   $87,[y,x++] ;0424 'D'
              0827 dc36  st   $36,[y,x++] ;0425 COND
              0828 dc4d  st   $4d,[y,x++] ;0426 EQ
              0829 dc99  st   $99,[y,x++]
              082a dc22  st   $22,[y,x++] ;0428 LDW
              082b dc87  st   $87,[y,x++] ;0429 'D'
              082c dcf7  st   $f7,[y,x++] ;042a SUBI
              082d dc52  st   $52,[y,x++]
              082e dc36  st   $36,[y,x++] ;042c COND
              082f dc59  st   $59,[y,x++] ;042d GE
              0830 dc36  st   $36,[y,x++]
              0831 dcf4  st   $f4,[y,x++] ;042f ADDI
              0832 dc32  st   $32,[y,x++]
              0833 dc2c  st   $2c,[y,x++] ;0431 STW
              0834 dc89  st   $89,[y,x++] ;0432 'E'
              0835 dc12  st   $12,[y,x++] ;0433 LDWI
              0836 dc00  st   $00,[y,x++]
              0837 dc05  st   $05,[y,x++]
              0838 dc9c  st   $9c,[y,x++] ;0436 BRA
              0839 dc3b  st   $3b,[y,x++]
              083a dc2c  st   $2c,[y,x++] ;0438 STW
              083b dc89  st   $89,[y,x++] ;0439 'E'
              083c dc12  st   $12,[y,x++] ;043a LDWI
              083d dc00  st   $00,[y,x++]
              083e dc06  st   $06,[y,x++]
              083f dc2c  st   $2c,[y,x++] ;043d STW
              0840 dc8b  st   $8b,[y,x++] ;043e 'F'
              0841 dc22  st   $22,[y,x++] ;043f LDW
              0842 dc89  st   $89,[y,x++] ;0440 'E'
              0843 dcc8  st   $c8,[y,x++] ;0441 ADDW
              0844 dc89  st   $89,[y,x++] ;0442 'E'
              0845 dc2c  st   $2c,[y,x++] ;0443 STW
              0846 dca7  st   $a7,[y,x++] ;0444 'T'
              0847 dcc8  st   $c8,[y,x++] ;0445 ADDW
              0848 dca7  st   $a7,[y,x++] ;0446 'T'
              0849 dcc8  st   $c8,[y,x++] ;0447 ADDW
              084a dc89  st   $89,[y,x++] ;0448 'E'
              084b dcc8  st   $c8,[y,x++] ;0449 ADDW
              084c dc8b  st   $8b,[y,x++] ;044a 'F'
              084d dc2c  st   $2c,[y,x++] ;044b STW
              084e dc8b  st   $8b,[y,x++] ;044c 'F'
              084f dc5f  st   $5f,[y,x++] ;044d LDI
              0850 dc05  st   $05,[y,x++]
              0851 dc2c  st   $2c,[y,x++] ;044f STW
              0852 dc91  st   $91,[y,x++] ;0450 'I'
              0853 dc22  st   $22,[y,x++] ;0451 LDW
              0854 dc91  st   $91,[y,x++] ;0452 'I'
              0855 dc36  st   $36,[y,x++] ;0453 COND
              0856 dc4d  st   $4d,[y,x++] ;0454 EQ
              0857 dc58  st   $58,[y,x++]
              0858 dc78  st   $78,[y,x++] ;0456 LOOKUP
              0859 dc8b  st   $8b,[y,x++] ;0457 'F'
              085a dc9c  st   $9c,[y,x++] ;0458 BRA
              085b dc5a  st   $5a,[y,x++]
              085c dc5f  st   $5f,[y,x++] ;045a LDI
              085d dc00  st   $00,[y,x++]
              085e dc2c  st   $2c,[y,x++] ;045c STW
              085f dc81  st   $81,[y,x++] ;045d 'A'
              0860 dc22  st   $22,[y,x++] ;045e LDW
              0861 dca5  st   $a5,[y,x++] ;045f 'S'
              0862 dc2c  st   $2c,[y,x++] ;0460 STW
              0863 dca7  st   $a7,[y,x++] ;0461 'T'
              0864 dc22  st   $22,[y,x++] ;0462 LDW
              0865 dc81  st   $81,[y,x++] ;0463 'A'
              0866 dc8e  st   $8e,[y,x++] ;0464 ANDI
              0867 dc80  st   $80,[y,x++]
              0868 dc36  st   $36,[y,x++] ;0466 COND
              0869 dc50  st   $50,[y,x++] ;0467 NE
              086a dc6d  st   $6d,[y,x++]
              086b dc5f  st   $5f,[y,x++] ;0469 LDI
              086c dc00  st   $00,[y,x++]
              086d dc9f  st   $9f,[y,x++] ;046b POKE
              086e dca7  st   $a7,[y,x++] ;046c 'T'
              086f dc9c  st   $9c,[y,x++] ;046d BRA
              0870 dc71  st   $71,[y,x++]
              0871 dc22  st   $22,[y,x++] ;046f LDW
              0872 dc9f  st   $9f,[y,x++] ;0470 'P'
              0873 dc9f  st   $9f,[y,x++] ;0471 POKE
              0874 dca7  st   $a7,[y,x++] ;0472 'T'
              0875 dc22  st   $22,[y,x++] ;0473 LDW
              0876 dc81  st   $81,[y,x++] ;0474 'A'
              0877 dcc8  st   $c8,[y,x++] ;0475 ADDW
              0878 dc81  st   $81,[y,x++] ;0476 'A'
              0879 dc2c  st   $2c,[y,x++] ;0477 STW
              087a dc81  st   $81,[y,x++] ;0478 'A'
              087b dcfa  st   $fa,[y,x++] ;0479 INC
              087c dca8  st   $a8,[y,x++] ;047a 'T'+1
              087d dc22  st   $22,[y,x++] ;047b LDW
              087e dca7  st   $a7,[y,x++] ;047c 'T'
              087f dcb4  st   $b4,[y,x++] ;047d SUBW
              0880 dca9  st   $a9,[y,x++] ;047e 'U'
              0881 dc36  st   $36,[y,x++] ;047f COND
              0882 dc56  st   $56,[y,x++] ;0480 LT
              0883 dc60  st   $60,[y,x++]
              0884 dc22  st   $22,[y,x++] ;0482 LDW
              0885 dc8b  st   $8b,[y,x++] ;0483 'F'
              0886 dcf4  st   $f4,[y,x++] ;0484 ADDI
              0887 dc01  st   $01,[y,x++]
              0888 dc2c  st   $2c,[y,x++] ;0486 STW
              0889 dc8b  st   $8b,[y,x++] ;0487 'F'
              088a dc22  st   $22,[y,x++] ;0488 LDW
              088b dca5  st   $a5,[y,x++] ;0489 'S'
              088c dcf4  st   $f4,[y,x++] ;048a ADDI
              088d dc01  st   $01,[y,x++]
              088e dc2c  st   $2c,[y,x++] ;048c STW
              088f dca5  st   $a5,[y,x++] ;048d 'S'
              0890 dc22  st   $22,[y,x++] ;048e LDW
              0891 dc91  st   $91,[y,x++] ;048f 'I'
              0892 dcf7  st   $f7,[y,x++] ;0490 SUBI
              0893 dc01  st   $01,[y,x++]
              0894 dc2c  st   $2c,[y,x++] ;0492 STW
              0895 dc91  st   $91,[y,x++] ;0493 'I'
              0896 dc36  st   $36,[y,x++] ;0494 COND
              0897 dc59  st   $59,[y,x++] ;0495 GE
              0898 dc4f  st   $4f,[y,x++]
              0899 dcfa  st   $fa,[y,x++] ;0497 INC
              089a dc85  st   $85,[y,x++] ;0498 'C'
              089b dc9c  st   $9c,[y,x++] ;0499 BRA
              089c dc1f  st   $1f,[y,x++]
              089d dc22  st   $22,[y,x++] ;049b LDW
              089e dc9f  st   $9f,[y,x++] ;049c 'P'
              089f dc98  st   $98,[y,x++] ;049d XORI
              08a0 dc08  st   $08,[y,x++]
              08a1 dc36  st   $36,[y,x++] ;049f COND
              08a2 dc4d  st   $4d,[y,x++] ;04a0 EQ
              08a3 dca8  st   $a8,[y,x++]
              08a4 dc22  st   $22,[y,x++] ;04a2 LDW
              08a5 dc9f  st   $9f,[y,x++] ;04a3 'P'
              08a6 dcf7  st   $f7,[y,x++] ;04a4 SUBI
              08a7 dc09  st   $09,[y,x++]
              08a8 dc2c  st   $2c,[y,x++] ;04a6 STW
              08a9 dc9f  st   $9f,[y,x++] ;04a7 'P'
              08aa dc9c  st   $9c,[y,x++] ;04a8 BRA
              08ab dc0b  st   $0b,[y,x++]
              08ac dc12  st   $12,[y,x++] ;04aa LDWI
              08ad dc11  st   $11,[y,x++]
              08ae dc01  st   $01,[y,x++]
              08af dc2c  st   $2c,[y,x++] ;04ad STW
              08b0 dca5  st   $a5,[y,x++] ;04ae 'S'
              08b1 dc5f  st   $5f,[y,x++] ;04af LDI
              08b2 dc00  st   $00,[y,x++]
              08b3 dc2c  st   $2c,[y,x++] ;04b1 STW
              08b4 dca1  st   $a1,[y,x++] ;04b2 'Q'
              08b5 dc2c  st   $2c,[y,x++] ;04b3 STW
              08b6 dca3  st   $a3,[y,x++] ;04b4 'R'
              08b7 dc2c  st   $2c,[y,x++] ;04b5 STW
              08b8 dc93  st   $93,[y,x++] ;04b6 'J'
              08b9 dc2c  st   $2c,[y,x++] ;04b7 STW
              08ba dc89  st   $89,[y,x++] ;04b8 'E'
              08bb dc2c  st   $2c,[y,x++] ;04b9 STW
              08bc dcb3  st   $b3,[y,x++] ;04ba 'Z'
              08bd dc2c  st   $2c,[y,x++] ;04bb STW
              08be dc8d  st   $8d,[y,x++] ;04bc 'G'
              08bf dc12  st   $12,[y,x++] ;04bd LDWI
              08c0 dc00  st   $00,[y,x++]
              08c1 dc18  st   $18,[y,x++]
              08c2 dc2c  st   $2c,[y,x++] ;04c0 STW
              08c3 dcb1  st   $b1,[y,x++] ;04c1 'Y'
              08c4 dc2c  st   $2c,[y,x++] ;04c2 STW
              08c5 dc83  st   $83,[y,x++] ;04c3 'B'
              08c6 dc5f  st   $5f,[y,x++] ;04c4 LDI
              08c7 dc5a  st   $5a,[y,x++]
              08c8 dc2c  st   $2c,[y,x++] ;04c6 STW
              08c9 dc8f  st   $8f,[y,x++] ;04c7 'H'
              08ca dc12  st   $12,[y,x++] ;04c8 LDWI
              08cb dc00  st   $00,[y,x++]
              08cc dc05  st   $05,[y,x++]
              08cd dce8  st   $e8,[y,x++] ;04cb CALL
              08ce 1000  ld   $00,x
              08cf 1405  ld   $05,y
              08d0 dc22  st   $22,[y,x++] ;0500 LDW
              08d1 dca5  st   $a5,[y,x++] ;0501 'S'
              08d2 dcdc  st   $dc,[y,x++] ;0502 PEEK
              08d3 dc2c  st   $2c,[y,x++] ;0503 STW
              08d4 dcaf  st   $af,[y,x++] ;0504 'X'
              08d5 dc98  st   $98,[y,x++] ;0505 XORI
              08d6 dc78  st   $78,[y,x++]
              08d7 dc36  st   $36,[y,x++] ;0507 COND
              08d8 dc50  st   $50,[y,x++] ;0508 NE
              08d9 dc0c  st   $0c,[y,x++]
              08da dc5f  st   $5f,[y,x++] ;050a LDI
              08db dc0a  st   $0a,[y,x++]
              08dc dc2c  st   $2c,[y,x++] ;050c STW
              08dd dc89  st   $89,[y,x++] ;050d 'E'
              08de dc22  st   $22,[y,x++] ;050e LDW
              08df dca5  st   $a5,[y,x++] ;050f 'S'
              08e0 dcdc  st   $dc,[y,x++] ;0510 PEEK
              08e1 dcc8  st   $c8,[y,x++] ;0511 ADDW
              08e2 dca3  st   $a3,[y,x++] ;0512 'R'
              08e3 dcc8  st   $c8,[y,x++] ;0513 ADDW
              08e4 dca1  st   $a1,[y,x++] ;0514 'Q'
              08e5 dc2c  st   $2c,[y,x++] ;0515 STW
              08e6 dca3  st   $a3,[y,x++] ;0516 'R'
              08e7 dcc8  st   $c8,[y,x++] ;0517 ADDW
              08e8 dca1  st   $a1,[y,x++] ;0518 'Q'
              08e9 dc36  st   $36,[y,x++] ;0519 COND
              08ea dc59  st   $59,[y,x++] ;051a GE
              08eb dc1c  st   $1c,[y,x++]
              08ec dc98  st   $98,[y,x++] ;051c XORI
              08ed dc2b  st   $2b,[y,x++]
              08ee dcc8  st   $c8,[y,x++] ;051e ADDW
              08ef dca1  st   $a1,[y,x++] ;051f 'Q'
              08f0 dcc8  st   $c8,[y,x++] ;0520 ADDW
              08f1 dca1  st   $a1,[y,x++] ;0521 'Q'
              08f2 dc2c  st   $2c,[y,x++] ;0522 STW
              08f3 dca1  st   $a1,[y,x++] ;0523 'Q'
              08f4 dc22  st   $22,[y,x++] ;0524 LDW
              08f5 dc8f  st   $8f,[y,x++] ;0525 'H'
              08f6 dcf7  st   $f7,[y,x++] ;0526 SUBI
              08f7 dc58  st   $58,[y,x++]
              08f8 dc36  st   $36,[y,x++] ;0528 COND
              08f9 dc5c  st   $5c,[y,x++] ;0529 LE
              08fa dc36  st   $36,[y,x++]
              08fb dc22  st   $22,[y,x++] ;052b LDW
              08fc dca3  st   $a3,[y,x++] ;052c 'R'
              08fd dc8e  st   $8e,[y,x++] ;052d ANDI
              08fe dc10  st   $10,[y,x++]
              08ff dc36  st   $36,[y,x++] ;052f COND
              0900 dc4d  st   $4d,[y,x++] ;0530 EQ
              0901 dc36  st   $36,[y,x++]
              0902 dc22  st   $22,[y,x++] ;0532 LDW
              0903 dc8f  st   $8f,[y,x++] ;0533 'H'
              0904 dcf7  st   $f7,[y,x++] ;0534 SUBI
              0905 dc01  st   $01,[y,x++]
              0906 dc2c  st   $2c,[y,x++] ;0536 STW
              0907 dc8f  st   $8f,[y,x++] ;0537 'H'
              0908 dc22  st   $22,[y,x++] ;0538 LDW
              0909 dc8f  st   $8f,[y,x++] ;0539 'H'
              090a dcf7  st   $f7,[y,x++] ;053a SUBI
              090b dc76  st   $76,[y,x++]
              090c dc36  st   $36,[y,x++] ;053c COND
              090d dc59  st   $59,[y,x++] ;053d GE
              090e dc4a  st   $4a,[y,x++]
              090f dc22  st   $22,[y,x++] ;053f LDW
              0910 dca3  st   $a3,[y,x++] ;0540 'R'
              0911 dc8e  st   $8e,[y,x++] ;0541 ANDI
              0912 dc80  st   $80,[y,x++]
              0913 dc36  st   $36,[y,x++] ;0543 COND
              0914 dc4d  st   $4d,[y,x++] ;0544 EQ
              0915 dc4a  st   $4a,[y,x++]
              0916 dc22  st   $22,[y,x++] ;0546 LDW
              0917 dc8f  st   $8f,[y,x++] ;0547 'H'
              0918 dcf4  st   $f4,[y,x++] ;0548 ADDI
              0919 dc01  st   $01,[y,x++]
              091a dc2c  st   $2c,[y,x++] ;054a STW
              091b dc8f  st   $8f,[y,x++] ;054b 'H'
              091c dc22  st   $22,[y,x++] ;054c LDW
              091d dc8f  st   $8f,[y,x++] ;054d 'H'
              091e dcf4  st   $f4,[y,x++] ;054e ADDI
              091f dc08  st   $08,[y,x++]
              0920 dc64  st   $64,[y,x++] ;0550 ST
              0921 dc94  st   $94,[y,x++] ;0551 'J'+1
              0922 dc22  st   $22,[y,x++] ;0552 LDW
              0923 dcaf  st   $af,[y,x++] ;0553 'X'
              0924 dcf4  st   $f4,[y,x++] ;0554 ADDI
              0925 dca0  st   $a0,[y,x++]
              0926 dc8e  st   $8e,[y,x++] ;0556 ANDI
              0927 dcff  st   $ff,[y,x++]
              0928 dc2c  st   $2c,[y,x++] ;0558 STW
              0929 dca7  st   $a7,[y,x++] ;0559 'T'
              092a dc12  st   $12,[y,x++] ;055a LDWI
              092b dc00  st   $00,[y,x++]
              092c dc10  st   $10,[y,x++]
              092d dcc8  st   $c8,[y,x++] ;055d ADDW
              092e dca7  st   $a7,[y,x++] ;055e 'T'
              092f dc2c  st   $2c,[y,x++] ;055f STW
              0930 dcab  st   $ab,[y,x++] ;0560 'V'
              0931 dc5f  st   $5f,[y,x++] ;0561 LDI
              0932 dc01  st   $01,[y,x++]
              0933 dc9f  st   $9f,[y,x++] ;0563 POKE
              0934 dcab  st   $ab,[y,x++] ;0564 'V'
              0935 dcfa  st   $fa,[y,x++] ;0565 INC
              0936 dcac  st   $ac,[y,x++] ;0566 'V'+1
              0937 dc22  st   $22,[y,x++] ;0567 LDW
              0938 dcab  st   $ab,[y,x++] ;0568 'V'
              0939 dc36  st   $36,[y,x++] ;0569 COND
              093a dc53  st   $53,[y,x++] ;056a GT
              093b dc5f  st   $5f,[y,x++]
              093c dc22  st   $22,[y,x++] ;056c LDW
              093d dca3  st   $a3,[y,x++] ;056d 'R'
              093e dc8e  st   $8e,[y,x++] ;056e ANDI
              093f dc7f  st   $7f,[y,x++]
              0940 dc2c  st   $2c,[y,x++] ;0570 STW
              0941 dc81  st   $81,[y,x++] ;0571 'A'
              0942 dc22  st   $22,[y,x++] ;0572 LDW
              0943 dc81  st   $81,[y,x++] ;0573 'A'
              0944 dcf7  st   $f7,[y,x++] ;0574 SUBI
              0945 dc6f  st   $6f,[y,x++]
              0946 dc36  st   $36,[y,x++] ;0576 COND
              0947 dc5c  st   $5c,[y,x++] ;0577 LE
              0948 dc7b  st   $7b,[y,x++]
              0949 dc5f  st   $5f,[y,x++] ;0579 LDI
              094a dc77  st   $77,[y,x++]
              094b dc2c  st   $2c,[y,x++] ;057b STW
              094c dc81  st   $81,[y,x++] ;057c 'A'
              094d dc22  st   $22,[y,x++] ;057d LDW
              094e dc81  st   $81,[y,x++] ;057e 'A'
              094f dcf4  st   $f4,[y,x++] ;057f ADDI
              0950 dc10  st   $10,[y,x++]
              0951 dc64  st   $64,[y,x++] ;0581 ST
              0952 dc82  st   $82,[y,x++] ;0582 'A'+1
              0953 dc22  st   $22,[y,x++] ;0583 LDW
              0954 dca7  st   $a7,[y,x++] ;0584 'T'
              0955 dc64  st   $64,[y,x++] ;0585 ST
              0956 dc81  st   $81,[y,x++] ;0586 'A'
              0957 dc5f  st   $5f,[y,x++] ;0587 LDI
              0958 dc02  st   $02,[y,x++]
              0959 dc9f  st   $9f,[y,x++] ;0589 POKE
              095a dc81  st   $81,[y,x++] ;058a 'A'
              095b dc22  st   $22,[y,x++] ;058b LDW
              095c dc93  st   $93,[y,x++] ;058c 'J'
              095d dcc8  st   $c8,[y,x++] ;058d ADDW
              095e dca7  st   $a7,[y,x++] ;058e 'T'
              095f dc2c  st   $2c,[y,x++] ;058f STW
              0960 dcab  st   $ab,[y,x++] ;0590 'V'
              0961 dc5f  st   $5f,[y,x++] ;0591 LDI
              0962 dc3f  st   $3f,[y,x++]
              0963 dc9f  st   $9f,[y,x++] ;0593 POKE
              0964 dcab  st   $ab,[y,x++] ;0594 'V'
              0965 dcfa  st   $fa,[y,x++] ;0595 INC
              0966 dcac  st   $ac,[y,x++] ;0596 'V'+1
              0967 dc22  st   $22,[y,x++] ;0597 LDW
              0968 dcaf  st   $af,[y,x++] ;0598 'X'
              0969 dc8e  st   $8e,[y,x++] ;0599 ANDI
              096a dc08  st   $08,[y,x++]
              096b dcc8  st   $c8,[y,x++] ;059b ADDW
              096c dc8f  st   $8f,[y,x++] ;059c 'H'
              096d dc2c  st   $2c,[y,x++] ;059d STW
              096e dc85  st   $85,[y,x++] ;059e 'C'
              096f dc22  st   $22,[y,x++] ;059f LDW
              0970 dc85  st   $85,[y,x++] ;05a0 'C'
              0971 dcf4  st   $f4,[y,x++] ;05a1 ADDI
              0972 dc01  st   $01,[y,x++]
              0973 dc2c  st   $2c,[y,x++] ;05a3 STW
              0974 dc85  st   $85,[y,x++] ;05a4 'C'
              0975 dc8e  st   $8e,[y,x++] ;05a5 ANDI
              0976 dc08  st   $08,[y,x++]
              0977 dc36  st   $36,[y,x++] ;05a7 COND
              0978 dc50  st   $50,[y,x++] ;05a8 NE
              0979 dcac  st   $ac,[y,x++]
              097a dc5f  st   $5f,[y,x++] ;05aa LDI
              097b dc2a  st   $2a,[y,x++]
              097c dc9c  st   $9c,[y,x++] ;05ac BRA
              097d dcae  st   $ae,[y,x++]
              097e dc5f  st   $5f,[y,x++] ;05ae LDI
              097f dc20  st   $20,[y,x++]
              0980 dc9f  st   $9f,[y,x++] ;05b0 POKE
              0981 dcab  st   $ab,[y,x++] ;05b1 'V'
              0982 dcfa  st   $fa,[y,x++] ;05b2 INC
              0983 dcac  st   $ac,[y,x++] ;05b3 'V'+1
              0984 dc22  st   $22,[y,x++] ;05b4 LDW
              0985 dcab  st   $ab,[y,x++] ;05b5 'V'
              0986 dc36  st   $36,[y,x++] ;05b6 COND
              0987 dc53  st   $53,[y,x++] ;05b7 GT
              0988 dc9d  st   $9d,[y,x++]
              0989 dc12  st   $12,[y,x++] ;05b9 LDWI
              098a dc00  st   $00,[y,x++]
              098b dc06  st   $06,[y,x++]
              098c dce8  st   $e8,[y,x++] ;05bc CALL
              098d dc9c  st   $9c,[y,x++] ;05bd BRA
              098e dcfe  st   $fe,[y,x++]
              098f 1000  ld   $00,x
              0990 1406  ld   $06,y
              0991 dc1b  st   $1b,[y,x++] ;0600 LD
              0992 dc08  st   $08,[y,x++]
              0993 dc36  st   $36,[y,x++] ;0602 COND
              0994 dc50  st   $50,[y,x++] ;0603 NE
              0995 dcfe  st   $fe,[y,x++]
              0996 dc22  st   $22,[y,x++] ;0605 LDW
              0997 dcaf  st   $af,[y,x++] ;0606 'X'
              0998 dcf4  st   $f4,[y,x++] ;0607 ADDI
              0999 dc01  st   $01,[y,x++]
              099a dc9f  st   $9f,[y,x++] ;0609 POKE
              099b dca5  st   $a5,[y,x++] ;060a 'S'
              099c dc22  st   $22,[y,x++] ;060b LDW
              099d dc8d  st   $8d,[y,x++] ;060c 'G'
              099e dc9f  st   $9f,[y,x++] ;060d POKE
              099f dc83  st   $83,[y,x++] ;060e 'B'
              09a0 dc22  st   $22,[y,x++] ;060f LDW
              09a1 dc89  st   $89,[y,x++] ;0610 'E'
              09a2 dcc8  st   $c8,[y,x++] ;0611 ADDW
              09a3 dcb3  st   $b3,[y,x++] ;0612 'Z'
              09a4 dc2c  st   $2c,[y,x++] ;0613 STW
              09a5 dcb3  st   $b3,[y,x++] ;0614 'Z'
              09a6 dc22  st   $22,[y,x++] ;0615 LDW
              09a7 dcb1  st   $b1,[y,x++] ;0616 'Y'
              09a8 dcc8  st   $c8,[y,x++] ;0617 ADDW
              09a9 dcb3  st   $b3,[y,x++] ;0618 'Z'
              09aa dc2c  st   $2c,[y,x++] ;0619 STW
              09ab dcb1  st   $b1,[y,x++] ;061a 'Y'
              09ac dc22  st   $22,[y,x++] ;061b LDW
              09ad dcaf  st   $af,[y,x++] ;061c 'X'
              09ae dcf4  st   $f4,[y,x++] ;061d ADDI
              09af dc32  st   $32,[y,x++]
              09b0 dc2c  st   $2c,[y,x++] ;061f STW
              09b1 dc83  st   $83,[y,x++] ;0620 'B'
              09b2 dc1b  st   $1b,[y,x++] ;0621 LD
              09b3 dcb2  st   $b2,[y,x++] ;0622 'Y'+1
              09b4 dc64  st   $64,[y,x++] ;0623 ST
              09b5 dc84  st   $84,[y,x++] ;0624 'B'+1
              09b6 dc22  st   $22,[y,x++] ;0625 LDW
              09b7 dc83  st   $83,[y,x++] ;0626 'B'
              09b8 dcdc  st   $dc,[y,x++] ;0627 PEEK
              09b9 dc2c  st   $2c,[y,x++] ;0628 STW
              09ba dc8d  st   $8d,[y,x++] ;0629 'G'
              09bb dc22  st   $22,[y,x++] ;062a LDW
              09bc dc8d  st   $8d,[y,x++] ;062b 'G'
              09bd dcf7  st   $f7,[y,x++] ;062c SUBI
              09be dc02  st   $02,[y,x++]
              09bf dc36  st   $36,[y,x++] ;062e COND
              09c0 dc5c  st   $5c,[y,x++] ;062f LE
              09c1 dc4e  st   $4e,[y,x++]
              09c2 dc5f  st   $5f,[y,x++] ;0631 LDI
              09c3 dc00  st   $00,[y,x++]
              09c4 dcb4  st   $b4,[y,x++] ;0633 SUBW
              09c5 dcb3  st   $b3,[y,x++] ;0634 'Z'
              09c6 dc2c  st   $2c,[y,x++] ;0635 STW
              09c7 dcb3  st   $b3,[y,x++] ;0636 'Z'
              09c8 dc22  st   $22,[y,x++] ;0637 LDW
              09c9 dcb1  st   $b1,[y,x++] ;0638 'Y'
              09ca dcc8  st   $c8,[y,x++] ;0639 ADDW
              09cb dcb3  st   $b3,[y,x++] ;063a 'Z'
              09cc dc2c  st   $2c,[y,x++] ;063b STW
              09cd dcb1  st   $b1,[y,x++] ;063c 'Y'
              09ce dc5f  st   $5f,[y,x++] ;063d LDI
              09cf dc0a  st   $0a,[y,x++]
              09d0 dc64  st   $64,[y,x++] ;063f ST
              09d1 dc12  st   $12,[y,x++]
              09d2 dc22  st   $22,[y,x++] ;0641 LDW
              09d3 dcaf  st   $af,[y,x++] ;0642 'X'
              09d4 dcf4  st   $f4,[y,x++] ;0643 ADDI
              09d5 dc32  st   $32,[y,x++]
              09d6 dc2c  st   $2c,[y,x++] ;0645 STW
              09d7 dc83  st   $83,[y,x++] ;0646 'B'
              09d8 dc1b  st   $1b,[y,x++] ;0647 LD
              09d9 dcb2  st   $b2,[y,x++] ;0648 'Y'+1
              09da dc64  st   $64,[y,x++] ;0649 ST
              09db dc84  st   $84,[y,x++] ;064a 'B'+1
              09dc dc22  st   $22,[y,x++] ;064b LDW
              09dd dc83  st   $83,[y,x++] ;064c 'B'
              09de dcdc  st   $dc,[y,x++] ;064d PEEK
              09df dc2c  st   $2c,[y,x++] ;064e STW
              09e0 dc8d  st   $8d,[y,x++] ;064f 'G'
              09e1 dc5f  st   $5f,[y,x++] ;0650 LDI
              09e2 dc3f  st   $3f,[y,x++]
              09e3 dc9f  st   $9f,[y,x++] ;0652 POKE
              09e4 dc83  st   $83,[y,x++] ;0653 'B'
              09e5 dc22  st   $22,[y,x++] ;0654 LDW
              09e6 dc1c  st   $1c,[y,x++] ;0655 vRT
              09e7 dce8  st   $e8,[y,x++] ;0656 CALL
              09e8 00fe  ld   $fe
              09e9 c218  st   [$18]
              09ea 0004  ld   $04
              09eb c219  st   [$19]
              09ec 0000  ld   $00
              09ed c21e  st   [$1e]
              09ee c21f  st   [$1f]
              09ef 150f  ld   [$0f],y
              09f0 e10e  jmp  y,[$0e]
              09f1 0200  nop
              09f2
