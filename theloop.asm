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
              0079 1406  ld   $06,y
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
RETRY:        03ab 0118  ld   [$18]       ;Retry until sufficient time
              03ac a002  suba $02
              03ad c218  st   [$18]
              03ae fce6  bra  RETURN
              03af 00f6  ld   $f6
SYS:          03b0 8120  adda [$20]
              03b1 e8ab  blt  RETRY
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
              05fd 1403  ld   $03,y
              05fe e07f  jmp  y,$7f
              05ff 1519  ld   [$19],y
initVcpu:     0600 1000  ld   $00,x
              0601 1404  ld   $04,y
              0602 dc5f  st   $5f,[y,x++] ;0400 LDI
              0603 dc78  st   $78,[y,x++]
              0604 dc64  st   $64,[y,x++] ;0402 ST
              0605 dc12  st   $12,[y,x++]
              0606 dc1b  st   $1b,[y,x++] ;0404 LD
              0607 dc12  st   $12,[y,x++]
              0608 dc36  st   $36,[y,x++] ;0406 COND
              0609 dc50  st   $50,[y,x++] ;0407 NE
              060a dc02  st   $02,[y,x++]
              060b dc5f  st   $5f,[y,x++] ;0409 LDI
              060c dc3e  st   $3e,[y,x++]
              060d dc2c  st   $2c,[y,x++] ;040b STW
              060e dc9f  st   $9f,[y,x++] ;040c 'P'
              060f dc12  st   $12,[y,x++] ;040d LDWI
              0610 dc00  st   $00,[y,x++]
              0611 dc05  st   $05,[y,x++]
              0612 dc2c  st   $2c,[y,x++] ;0410 STW
              0613 dc8b  st   $8b,[y,x++] ;0411 'F'
              0614 dc12  st   $12,[y,x++] ;0412 LDWI
              0615 dc00  st   $00,[y,x++]
              0616 dc08  st   $08,[y,x++]
              0617 dc2c  st   $2c,[y,x++] ;0415 STW
              0618 dca5  st   $a5,[y,x++] ;0416 'S'
              0619 dc12  st   $12,[y,x++] ;0417 LDWI
              061a dc00  st   $00,[y,x++]
              061b dc10  st   $10,[y,x++]
              061c dc2c  st   $2c,[y,x++] ;041a STW
              061d dca9  st   $a9,[y,x++] ;041b 'U'
              061e dc78  st   $78,[y,x++] ;041c LOOKUP
              061f dc8b  st   $8b,[y,x++] ;041d 'F'
              0620 dc2c  st   $2c,[y,x++] ;041e STW
              0621 dc81  st   $81,[y,x++] ;041f 'A'
              0622 dc22  st   $22,[y,x++] ;0420 LDW
              0623 dca5  st   $a5,[y,x++] ;0421 'S'
              0624 dc2c  st   $2c,[y,x++] ;0422 STW
              0625 dca7  st   $a7,[y,x++] ;0423 'T'
              0626 dc22  st   $22,[y,x++] ;0424 LDW
              0627 dc81  st   $81,[y,x++] ;0425 'A'
              0628 dc8e  st   $8e,[y,x++] ;0426 ANDI
              0629 dc80  st   $80,[y,x++]
              062a dc36  st   $36,[y,x++] ;0428 COND
              062b dc50  st   $50,[y,x++] ;0429 NE
              062c dc2f  st   $2f,[y,x++]
              062d dc5f  st   $5f,[y,x++] ;042b LDI
              062e dc00  st   $00,[y,x++]
              062f dc9f  st   $9f,[y,x++] ;042d POKE
              0630 dca7  st   $a7,[y,x++] ;042e 'T'
              0631 dc9c  st   $9c,[y,x++] ;042f BRA
              0632 dc33  st   $33,[y,x++]
              0633 dc22  st   $22,[y,x++] ;0431 LDW
              0634 dc9f  st   $9f,[y,x++] ;0432 'P'
              0635 dc9f  st   $9f,[y,x++] ;0433 POKE
              0636 dca7  st   $a7,[y,x++] ;0434 'T'
              0637 dc22  st   $22,[y,x++] ;0435 LDW
              0638 dc81  st   $81,[y,x++] ;0436 'A'
              0639 dcc8  st   $c8,[y,x++] ;0437 ADDW
              063a dc81  st   $81,[y,x++] ;0438 'A'
              063b dc2c  st   $2c,[y,x++] ;0439 STW
              063c dc81  st   $81,[y,x++] ;043a 'A'
              063d dcfa  st   $fa,[y,x++] ;043b INC
              063e dca8  st   $a8,[y,x++] ;043c 'T'+1
              063f dc22  st   $22,[y,x++] ;043d LDW
              0640 dca7  st   $a7,[y,x++] ;043e 'T'
              0641 dcb4  st   $b4,[y,x++] ;043f SUBW
              0642 dca9  st   $a9,[y,x++] ;0440 'U'
              0643 dc36  st   $36,[y,x++] ;0441 COND
              0644 dc56  st   $56,[y,x++] ;0442 LT
              0645 dc22  st   $22,[y,x++]
              0646 dc22  st   $22,[y,x++] ;0444 LDW
              0647 dc8b  st   $8b,[y,x++] ;0445 'F'
              0648 dcf4  st   $f4,[y,x++] ;0446 ADDI
              0649 dc01  st   $01,[y,x++]
              064a dc2c  st   $2c,[y,x++] ;0448 STW
              064b dc8b  st   $8b,[y,x++] ;0449 'F'
              064c dc22  st   $22,[y,x++] ;044a LDW
              064d dca5  st   $a5,[y,x++] ;044b 'S'
              064e dcf4  st   $f4,[y,x++] ;044c ADDI
              064f dc01  st   $01,[y,x++]
              0650 dc2c  st   $2c,[y,x++] ;044e STW
              0651 dca5  st   $a5,[y,x++] ;044f 'S'
              0652 dc8e  st   $8e,[y,x++] ;0450 ANDI
              0653 dcff  st   $ff,[y,x++]
              0654 dc98  st   $98,[y,x++] ;0452 XORI
              0655 dca0  st   $a0,[y,x++]
              0656 dc36  st   $36,[y,x++] ;0454 COND
              0657 dc50  st   $50,[y,x++] ;0455 NE
              0658 dc1a  st   $1a,[y,x++]
              0659 dc22  st   $22,[y,x++] ;0457 LDW
              065a dc9f  st   $9f,[y,x++] ;0458 'P'
              065b dc98  st   $98,[y,x++] ;0459 XORI
              065c dc08  st   $08,[y,x++]
              065d dc36  st   $36,[y,x++] ;045b COND
              065e dc4d  st   $4d,[y,x++] ;045c EQ
              065f dc64  st   $64,[y,x++]
              0660 dc22  st   $22,[y,x++] ;045e LDW
              0661 dc9f  st   $9f,[y,x++] ;045f 'P'
              0662 dcf7  st   $f7,[y,x++] ;0460 SUBI
              0663 dc09  st   $09,[y,x++]
              0664 dc2c  st   $2c,[y,x++] ;0462 STW
              0665 dc9f  st   $9f,[y,x++] ;0463 'P'
              0666 dc9c  st   $9c,[y,x++] ;0464 BRA
              0667 dc0b  st   $0b,[y,x++]
              0668 dc12  st   $12,[y,x++] ;0466 LDWI
              0669 dc11  st   $11,[y,x++]
              066a dc01  st   $01,[y,x++]
              066b dc2c  st   $2c,[y,x++] ;0469 STW
              066c dca5  st   $a5,[y,x++] ;046a 'S'
              066d dc5f  st   $5f,[y,x++] ;046b LDI
              066e dc00  st   $00,[y,x++]
              066f dc2c  st   $2c,[y,x++] ;046d STW
              0670 dca1  st   $a1,[y,x++] ;046e 'Q'
              0671 dc2c  st   $2c,[y,x++] ;046f STW
              0672 dca3  st   $a3,[y,x++] ;0470 'R'
              0673 dc2c  st   $2c,[y,x++] ;0471 STW
              0674 dc93  st   $93,[y,x++] ;0472 'J'
              0675 dc2c  st   $2c,[y,x++] ;0473 STW
              0676 dc89  st   $89,[y,x++] ;0474 'E'
              0677 dc2c  st   $2c,[y,x++] ;0475 STW
              0678 dcb3  st   $b3,[y,x++] ;0476 'Z'
              0679 dc2c  st   $2c,[y,x++] ;0477 STW
              067a dc8d  st   $8d,[y,x++] ;0478 'G'
              067b dc12  st   $12,[y,x++] ;0479 LDWI
              067c dc00  st   $00,[y,x++]
              067d dc18  st   $18,[y,x++]
              067e dc2c  st   $2c,[y,x++] ;047c STW
              067f dcb1  st   $b1,[y,x++] ;047d 'Y'
              0680 dc2c  st   $2c,[y,x++] ;047e STW
              0681 dc83  st   $83,[y,x++] ;047f 'B'
              0682 dc5f  st   $5f,[y,x++] ;0480 LDI
              0683 dc5a  st   $5a,[y,x++]
              0684 dc2c  st   $2c,[y,x++] ;0482 STW
              0685 dc8f  st   $8f,[y,x++] ;0483 'H'
              0686 dc12  st   $12,[y,x++] ;0484 LDWI
              0687 dc00  st   $00,[y,x++]
              0688 dc05  st   $05,[y,x++]
              0689 dce8  st   $e8,[y,x++] ;0487 CALL
              068a 1000  ld   $00,x
              068b 1405  ld   $05,y
              068c dc22  st   $22,[y,x++] ;0500 LDW
              068d dca5  st   $a5,[y,x++] ;0501 'S'
              068e dcdc  st   $dc,[y,x++] ;0502 PEEK
              068f dc2c  st   $2c,[y,x++] ;0503 STW
              0690 dcaf  st   $af,[y,x++] ;0504 'X'
              0691 dc98  st   $98,[y,x++] ;0505 XORI
              0692 dc78  st   $78,[y,x++]
              0693 dc36  st   $36,[y,x++] ;0507 COND
              0694 dc50  st   $50,[y,x++] ;0508 NE
              0695 dc0c  st   $0c,[y,x++]
              0696 dc5f  st   $5f,[y,x++] ;050a LDI
              0697 dc0a  st   $0a,[y,x++]
              0698 dc2c  st   $2c,[y,x++] ;050c STW
              0699 dc89  st   $89,[y,x++] ;050d 'E'
              069a dc22  st   $22,[y,x++] ;050e LDW
              069b dca5  st   $a5,[y,x++] ;050f 'S'
              069c dcdc  st   $dc,[y,x++] ;0510 PEEK
              069d dcc8  st   $c8,[y,x++] ;0511 ADDW
              069e dca3  st   $a3,[y,x++] ;0512 'R'
              069f dcc8  st   $c8,[y,x++] ;0513 ADDW
              06a0 dca1  st   $a1,[y,x++] ;0514 'Q'
              06a1 dc2c  st   $2c,[y,x++] ;0515 STW
              06a2 dca3  st   $a3,[y,x++] ;0516 'R'
              06a3 dcc8  st   $c8,[y,x++] ;0517 ADDW
              06a4 dca1  st   $a1,[y,x++] ;0518 'Q'
              06a5 dc36  st   $36,[y,x++] ;0519 COND
              06a6 dc59  st   $59,[y,x++] ;051a GE
              06a7 dc1c  st   $1c,[y,x++]
              06a8 dc98  st   $98,[y,x++] ;051c XORI
              06a9 dc2b  st   $2b,[y,x++]
              06aa dcc8  st   $c8,[y,x++] ;051e ADDW
              06ab dca1  st   $a1,[y,x++] ;051f 'Q'
              06ac dcc8  st   $c8,[y,x++] ;0520 ADDW
              06ad dca1  st   $a1,[y,x++] ;0521 'Q'
              06ae dc2c  st   $2c,[y,x++] ;0522 STW
              06af dca1  st   $a1,[y,x++] ;0523 'Q'
              06b0 dc22  st   $22,[y,x++] ;0524 LDW
              06b1 dc8f  st   $8f,[y,x++] ;0525 'H'
              06b2 dcf7  st   $f7,[y,x++] ;0526 SUBI
              06b3 dc58  st   $58,[y,x++]
              06b4 dc36  st   $36,[y,x++] ;0528 COND
              06b5 dc5c  st   $5c,[y,x++] ;0529 LE
              06b6 dc36  st   $36,[y,x++]
              06b7 dc22  st   $22,[y,x++] ;052b LDW
              06b8 dca3  st   $a3,[y,x++] ;052c 'R'
              06b9 dc8e  st   $8e,[y,x++] ;052d ANDI
              06ba dc10  st   $10,[y,x++]
              06bb dc36  st   $36,[y,x++] ;052f COND
              06bc dc4d  st   $4d,[y,x++] ;0530 EQ
              06bd dc36  st   $36,[y,x++]
              06be dc22  st   $22,[y,x++] ;0532 LDW
              06bf dc8f  st   $8f,[y,x++] ;0533 'H'
              06c0 dcf7  st   $f7,[y,x++] ;0534 SUBI
              06c1 dc01  st   $01,[y,x++]
              06c2 dc2c  st   $2c,[y,x++] ;0536 STW
              06c3 dc8f  st   $8f,[y,x++] ;0537 'H'
              06c4 dc22  st   $22,[y,x++] ;0538 LDW
              06c5 dc8f  st   $8f,[y,x++] ;0539 'H'
              06c6 dcf7  st   $f7,[y,x++] ;053a SUBI
              06c7 dc76  st   $76,[y,x++]
              06c8 dc36  st   $36,[y,x++] ;053c COND
              06c9 dc59  st   $59,[y,x++] ;053d GE
              06ca dc4a  st   $4a,[y,x++]
              06cb dc22  st   $22,[y,x++] ;053f LDW
              06cc dca3  st   $a3,[y,x++] ;0540 'R'
              06cd dc8e  st   $8e,[y,x++] ;0541 ANDI
              06ce dc80  st   $80,[y,x++]
              06cf dc36  st   $36,[y,x++] ;0543 COND
              06d0 dc4d  st   $4d,[y,x++] ;0544 EQ
              06d1 dc4a  st   $4a,[y,x++]
              06d2 dc22  st   $22,[y,x++] ;0546 LDW
              06d3 dc8f  st   $8f,[y,x++] ;0547 'H'
              06d4 dcf4  st   $f4,[y,x++] ;0548 ADDI
              06d5 dc01  st   $01,[y,x++]
              06d6 dc2c  st   $2c,[y,x++] ;054a STW
              06d7 dc8f  st   $8f,[y,x++] ;054b 'H'
              06d8 dc22  st   $22,[y,x++] ;054c LDW
              06d9 dc8f  st   $8f,[y,x++] ;054d 'H'
              06da dcf4  st   $f4,[y,x++] ;054e ADDI
              06db dc08  st   $08,[y,x++]
              06dc dc64  st   $64,[y,x++] ;0550 ST
              06dd dc94  st   $94,[y,x++] ;0551 'J'+1
              06de dc22  st   $22,[y,x++] ;0552 LDW
              06df dcaf  st   $af,[y,x++] ;0553 'X'
              06e0 dcf4  st   $f4,[y,x++] ;0554 ADDI
              06e1 dca0  st   $a0,[y,x++]
              06e2 dc8e  st   $8e,[y,x++] ;0556 ANDI
              06e3 dcff  st   $ff,[y,x++]
              06e4 dc2c  st   $2c,[y,x++] ;0558 STW
              06e5 dca7  st   $a7,[y,x++] ;0559 'T'
              06e6 dc12  st   $12,[y,x++] ;055a LDWI
              06e7 dc00  st   $00,[y,x++]
              06e8 dc10  st   $10,[y,x++]
              06e9 dcc8  st   $c8,[y,x++] ;055d ADDW
              06ea dca7  st   $a7,[y,x++] ;055e 'T'
              06eb dc2c  st   $2c,[y,x++] ;055f STW
              06ec dcab  st   $ab,[y,x++] ;0560 'V'
              06ed dc5f  st   $5f,[y,x++] ;0561 LDI
              06ee dc01  st   $01,[y,x++]
              06ef dc9f  st   $9f,[y,x++] ;0563 POKE
              06f0 dcab  st   $ab,[y,x++] ;0564 'V'
              06f1 dcfa  st   $fa,[y,x++] ;0565 INC
              06f2 dcac  st   $ac,[y,x++] ;0566 'V'+1
              06f3 dc22  st   $22,[y,x++] ;0567 LDW
              06f4 dcab  st   $ab,[y,x++] ;0568 'V'
              06f5 dc36  st   $36,[y,x++] ;0569 COND
              06f6 dc53  st   $53,[y,x++] ;056a GT
              06f7 dc5f  st   $5f,[y,x++]
              06f8 dc22  st   $22,[y,x++] ;056c LDW
              06f9 dca3  st   $a3,[y,x++] ;056d 'R'
              06fa dc8e  st   $8e,[y,x++] ;056e ANDI
              06fb dc7f  st   $7f,[y,x++]
              06fc dc2c  st   $2c,[y,x++] ;0570 STW
              06fd dc81  st   $81,[y,x++] ;0571 'A'
              06fe dc22  st   $22,[y,x++] ;0572 LDW
              06ff dc81  st   $81,[y,x++] ;0573 'A'
              0700 dcf7  st   $f7,[y,x++] ;0574 SUBI
              0701 dc6f  st   $6f,[y,x++]
              0702 dc36  st   $36,[y,x++] ;0576 COND
              0703 dc5c  st   $5c,[y,x++] ;0577 LE
              0704 dc7b  st   $7b,[y,x++]
              0705 dc5f  st   $5f,[y,x++] ;0579 LDI
              0706 dc77  st   $77,[y,x++]
              0707 dc2c  st   $2c,[y,x++] ;057b STW
              0708 dc81  st   $81,[y,x++] ;057c 'A'
              0709 dc22  st   $22,[y,x++] ;057d LDW
              070a dc81  st   $81,[y,x++] ;057e 'A'
              070b dcf4  st   $f4,[y,x++] ;057f ADDI
              070c dc10  st   $10,[y,x++]
              070d dc64  st   $64,[y,x++] ;0581 ST
              070e dc82  st   $82,[y,x++] ;0582 'A'+1
              070f dc22  st   $22,[y,x++] ;0583 LDW
              0710 dca7  st   $a7,[y,x++] ;0584 'T'
              0711 dc64  st   $64,[y,x++] ;0585 ST
              0712 dc81  st   $81,[y,x++] ;0586 'A'
              0713 dc5f  st   $5f,[y,x++] ;0587 LDI
              0714 dc02  st   $02,[y,x++]
              0715 dc9f  st   $9f,[y,x++] ;0589 POKE
              0716 dc81  st   $81,[y,x++] ;058a 'A'
              0717 dc22  st   $22,[y,x++] ;058b LDW
              0718 dc93  st   $93,[y,x++] ;058c 'J'
              0719 dcc8  st   $c8,[y,x++] ;058d ADDW
              071a dca7  st   $a7,[y,x++] ;058e 'T'
              071b dc2c  st   $2c,[y,x++] ;058f STW
              071c dcab  st   $ab,[y,x++] ;0590 'V'
              071d dc5f  st   $5f,[y,x++] ;0591 LDI
              071e dc3f  st   $3f,[y,x++]
              071f dc9f  st   $9f,[y,x++] ;0593 POKE
              0720 dcab  st   $ab,[y,x++] ;0594 'V'
              0721 dcfa  st   $fa,[y,x++] ;0595 INC
              0722 dcac  st   $ac,[y,x++] ;0596 'V'+1
              0723 dc22  st   $22,[y,x++] ;0597 LDW
              0724 dcaf  st   $af,[y,x++] ;0598 'X'
              0725 dc8e  st   $8e,[y,x++] ;0599 ANDI
              0726 dc08  st   $08,[y,x++]
              0727 dcc8  st   $c8,[y,x++] ;059b ADDW
              0728 dc8f  st   $8f,[y,x++] ;059c 'H'
              0729 dc2c  st   $2c,[y,x++] ;059d STW
              072a dc85  st   $85,[y,x++] ;059e 'C'
              072b dc22  st   $22,[y,x++] ;059f LDW
              072c dc85  st   $85,[y,x++] ;05a0 'C'
              072d dcf4  st   $f4,[y,x++] ;05a1 ADDI
              072e dc01  st   $01,[y,x++]
              072f dc2c  st   $2c,[y,x++] ;05a3 STW
              0730 dc85  st   $85,[y,x++] ;05a4 'C'
              0731 dc8e  st   $8e,[y,x++] ;05a5 ANDI
              0732 dc08  st   $08,[y,x++]
              0733 dc36  st   $36,[y,x++] ;05a7 COND
              0734 dc50  st   $50,[y,x++] ;05a8 NE
              0735 dcac  st   $ac,[y,x++]
              0736 dc5f  st   $5f,[y,x++] ;05aa LDI
              0737 dc2a  st   $2a,[y,x++]
              0738 dc9c  st   $9c,[y,x++] ;05ac BRA
              0739 dcae  st   $ae,[y,x++]
              073a dc5f  st   $5f,[y,x++] ;05ae LDI
              073b dc20  st   $20,[y,x++]
              073c dc9f  st   $9f,[y,x++] ;05b0 POKE
              073d dcab  st   $ab,[y,x++] ;05b1 'V'
              073e dcfa  st   $fa,[y,x++] ;05b2 INC
              073f dcac  st   $ac,[y,x++] ;05b3 'V'+1
              0740 dc22  st   $22,[y,x++] ;05b4 LDW
              0741 dcab  st   $ab,[y,x++] ;05b5 'V'
              0742 dc36  st   $36,[y,x++] ;05b6 COND
              0743 dc53  st   $53,[y,x++] ;05b7 GT
              0744 dc9d  st   $9d,[y,x++]
              0745 dc12  st   $12,[y,x++] ;05b9 LDWI
              0746 dc00  st   $00,[y,x++]
              0747 dc06  st   $06,[y,x++]
              0748 dce8  st   $e8,[y,x++] ;05bc CALL
              0749 dc9c  st   $9c,[y,x++] ;05bd BRA
              074a dcfe  st   $fe,[y,x++]
              074b 1000  ld   $00,x
              074c 1406  ld   $06,y
              074d dc1b  st   $1b,[y,x++] ;0600 LD
              074e dc08  st   $08,[y,x++]
              074f dc36  st   $36,[y,x++] ;0602 COND
              0750 dc50  st   $50,[y,x++] ;0603 NE
              0751 dcfe  st   $fe,[y,x++]
              0752 dc22  st   $22,[y,x++] ;0605 LDW
              0753 dcaf  st   $af,[y,x++] ;0606 'X'
              0754 dcf4  st   $f4,[y,x++] ;0607 ADDI
              0755 dc01  st   $01,[y,x++]
              0756 dc9f  st   $9f,[y,x++] ;0609 POKE
              0757 dca5  st   $a5,[y,x++] ;060a 'S'
              0758 dc22  st   $22,[y,x++] ;060b LDW
              0759 dc8d  st   $8d,[y,x++] ;060c 'G'
              075a dc9f  st   $9f,[y,x++] ;060d POKE
              075b dc83  st   $83,[y,x++] ;060e 'B'
              075c dc22  st   $22,[y,x++] ;060f LDW
              075d dc89  st   $89,[y,x++] ;0610 'E'
              075e dcc8  st   $c8,[y,x++] ;0611 ADDW
              075f dcb3  st   $b3,[y,x++] ;0612 'Z'
              0760 dc2c  st   $2c,[y,x++] ;0613 STW
              0761 dcb3  st   $b3,[y,x++] ;0614 'Z'
              0762 dc22  st   $22,[y,x++] ;0615 LDW
              0763 dcb1  st   $b1,[y,x++] ;0616 'Y'
              0764 dcc8  st   $c8,[y,x++] ;0617 ADDW
              0765 dcb3  st   $b3,[y,x++] ;0618 'Z'
              0766 dc2c  st   $2c,[y,x++] ;0619 STW
              0767 dcb1  st   $b1,[y,x++] ;061a 'Y'
              0768 dc22  st   $22,[y,x++] ;061b LDW
              0769 dcaf  st   $af,[y,x++] ;061c 'X'
              076a dcf4  st   $f4,[y,x++] ;061d ADDI
              076b dc32  st   $32,[y,x++]
              076c dc2c  st   $2c,[y,x++] ;061f STW
              076d dc83  st   $83,[y,x++] ;0620 'B'
              076e dc1b  st   $1b,[y,x++] ;0621 LD
              076f dcb2  st   $b2,[y,x++] ;0622 'Y'+1
              0770 dc64  st   $64,[y,x++] ;0623 ST
              0771 dc84  st   $84,[y,x++] ;0624 'B'+1
              0772 dc22  st   $22,[y,x++] ;0625 LDW
              0773 dc83  st   $83,[y,x++] ;0626 'B'
              0774 dcdc  st   $dc,[y,x++] ;0627 PEEK
              0775 dc2c  st   $2c,[y,x++] ;0628 STW
              0776 dc8d  st   $8d,[y,x++] ;0629 'G'
              0777 dc22  st   $22,[y,x++] ;062a LDW
              0778 dc8d  st   $8d,[y,x++] ;062b 'G'
              0779 dcf7  st   $f7,[y,x++] ;062c SUBI
              077a dc02  st   $02,[y,x++]
              077b dc36  st   $36,[y,x++] ;062e COND
              077c dc5c  st   $5c,[y,x++] ;062f LE
              077d dc4e  st   $4e,[y,x++]
              077e dc5f  st   $5f,[y,x++] ;0631 LDI
              077f dc00  st   $00,[y,x++]
              0780 dcb4  st   $b4,[y,x++] ;0633 SUBW
              0781 dcb3  st   $b3,[y,x++] ;0634 'Z'
              0782 dc2c  st   $2c,[y,x++] ;0635 STW
              0783 dcb3  st   $b3,[y,x++] ;0636 'Z'
              0784 dc22  st   $22,[y,x++] ;0637 LDW
              0785 dcb1  st   $b1,[y,x++] ;0638 'Y'
              0786 dcc8  st   $c8,[y,x++] ;0639 ADDW
              0787 dcb3  st   $b3,[y,x++] ;063a 'Z'
              0788 dc2c  st   $2c,[y,x++] ;063b STW
              0789 dcb1  st   $b1,[y,x++] ;063c 'Y'
              078a dc5f  st   $5f,[y,x++] ;063d LDI
              078b dc0a  st   $0a,[y,x++]
              078c dc64  st   $64,[y,x++] ;063f ST
              078d dc12  st   $12,[y,x++]
              078e dc22  st   $22,[y,x++] ;0641 LDW
              078f dcaf  st   $af,[y,x++] ;0642 'X'
              0790 dcf4  st   $f4,[y,x++] ;0643 ADDI
              0791 dc32  st   $32,[y,x++]
              0792 dc2c  st   $2c,[y,x++] ;0645 STW
              0793 dc83  st   $83,[y,x++] ;0646 'B'
              0794 dc1b  st   $1b,[y,x++] ;0647 LD
              0795 dcb2  st   $b2,[y,x++] ;0648 'Y'+1
              0796 dc64  st   $64,[y,x++] ;0649 ST
              0797 dc84  st   $84,[y,x++] ;064a 'B'+1
              0798 dc22  st   $22,[y,x++] ;064b LDW
              0799 dc83  st   $83,[y,x++] ;064c 'B'
              079a dcdc  st   $dc,[y,x++] ;064d PEEK
              079b dc2c  st   $2c,[y,x++] ;064e STW
              079c dc8d  st   $8d,[y,x++] ;064f 'G'
              079d dc5f  st   $5f,[y,x++] ;0650 LDI
              079e dc3f  st   $3f,[y,x++]
              079f dc9f  st   $9f,[y,x++] ;0652 POKE
              07a0 dc83  st   $83,[y,x++] ;0653 'B'
              07a1 dc22  st   $22,[y,x++] ;0654 LDW
              07a2 dc1c  st   $1c,[y,x++] ;0655 vRT
              07a3 dce8  st   $e8,[y,x++] ;0656 CALL
              07a4 00fe  ld   $fe
              07a5 c218  st   [$18]
              07a6 0004  ld   $04
              07a7 c219  st   [$19]
              07a8 0000  ld   $00
              07a9 c21e  st   [$1e]
              07aa c21f  st   [$1f]
              07ab 150f  ld   [$0f],y
              07ac e10e  jmp  y,[$0e]
              07ad 0200  nop
              07ae
