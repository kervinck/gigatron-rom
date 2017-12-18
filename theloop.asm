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
              001e d224  st   [$24],x
              001f d625  st   [$25],y
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
              002a 0124  ld   [$24]
              002b 8001  adda $01
              002c ec20  bne  .initEnt0
              002d d224  st   [$24],x
              002e 0125  ld   [$25]
              002f 8001  adda $01
              0030 ec20  bne  .initEnt0
              0031 d625  st   [$25],y
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
.loop:        004f c224  st   [$24]
              0050 8200  adda ac
              0051 9200  adda ac,x
              0052 0124  ld   [$24]
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
              0079 147f  ld   $7f,y
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
              01fb 0200  nop              ;Run vCPU for 163 cycles (line41-521 typeF)
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
              0210 0017  ld   $17         ;Run vCPU for 126 cycles (line0)
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
              0294 009b  ld   $9b         ;Run vCPU for 144 cycles (line1-39 typeC)
              0295 c20e  st   [$0e]
              0296 0002  ld   $02
              0297 c20f  st   [$0f]
              0298 1403  ld   $03,y
              0299 e000  jmp  y,$00
              029a 0032  ld   $32
              029b fc58  bra  sound1
              029c 1909  ld   [$09],out   ;<New scanline start>
vBlankNormal: 029d 00a4  ld   $a4         ;Run vCPU for 148 cycles (line1-39 typeABD)
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
              02b8 0200  nop              ;Run vCPU for 151 cycles (line40)
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
GT:           0350 f845  ble  .cond4
              0351 e448  bgt  .cond5
              0352 0d00  ld   [y,x]
LT:           0353 f445  bge  .cond4
              0354 e848  blt  .cond5
              0355 0d00  ld   [y,x]
GE:           0356 e845  blt  .cond4
              0357 f448  bge  .cond5
              0358 0d00  ld   [y,x]
LE:           0359 e445  bgt  .cond4
              035a f848  ble  .cond5
              035b 0d00  ld   [y,x]
LDI:          035c c21a  st   [$1a]
              035d 0000  ld   $00
              035e c21b  st   [$1b]
              035f 00f8  ld   $f8
              0360 fc02  bra  NEXT
ST:           0361 1200  ld   ac,x
              0362 011a  ld   [$1a]
              0363 c61a  st   [x]
              0364 00f8  ld   $f8
              0365 fc02  bra  NEXT
POP:          0366 111e  ld   [$1e],x
              0367 0500  ld   [x]
              0368 c21a  st   [$1a]
              0369 011e  ld   [$1e]
              036a 9001  adda $01,x
              036b 0500  ld   [x]
              036c c21b  st   [$1b]
              036d 011e  ld   [$1e]
              036e 8002  adda $02
              036f c21e  st   [$1e]
next1:        0370 0118  ld   [$18]
              0371 a001  suba $01
              0372 c218  st   [$18]
              0373 00f3  ld   $f3
              0374 fc02  bra  NEXT
NE:           0375 f045  beq  .cond4
              0376 ec48  bne  .cond5
              0377 0d00  ld   [y,x]
LOOKUP:       0378 951b  adda [$1b],y
              0379 e0fb  jmp  y,$fb       ;Trampoline offset
              037a 011a  ld   [$1a]
rLookup:      037b c21a  st   [$1a]
              037c 0000  ld   $00
              037d c21b  st   [$1b]
              037e fc02  bra  NEXT
              037f 00f4  ld   $f4
PUSH:         0380 011e  ld   [$1e]
              0381 b001  suba $01,x
              0382 011d  ld   [$1d]
              0383 c600  st   [x]
              0384 011e  ld   [$1e]
              0385 a002  suba $02
              0386 d21e  st   [$1e],x
              0387 011c  ld   [$1c]
              0388 fc70  bra  next1
              0389 c600  st   [x]
ANDI:         038a 211a  anda [$1a]
              038b c21a  st   [$1a]
              038c 0000  ld   $00
              038d c21b  st   [$1b]
              038e fc02  bra  NEXT
              038f 00f8  ld   $f8
ORI:          0390 411a  ora  [$1a]
              0391 c21a  st   [$1a]
              0392 fc02  bra  NEXT
              0393 00f9  ld   $f9
XORI:         0394 611a  xora [$1a]
              0395 c21a  st   [$1a]
              0396 fc02  bra  NEXT
              0397 00f9  ld   $f9
BRA:          0398 c218  st   [$18]
              0399 00f9  ld   $f9
              039a fc02  bra  NEXT
POKE:         039b c221  st   [$21]
              039c 9001  adda $01,x
              039d 0500  ld   [x]
              039e 1600  ld   ac,y
              039f 1121  ld   [$21],x
              03a0 0500  ld   [x]
              03a1 1200  ld   ac,x
              03a2 011a  ld   [$1a]
              03a3 ce00  st   [y,x]
              03a4 1519  ld   [$19],y
              03a5 fc02  bra  NEXT
              03a6 00f5  ld   $f5
retry:        03a7 0118  ld   [$18]       ;Retry until sufficient time
              03a8 a002  suba $02
              03a9 c218  st   [$18]
              03aa fce2  bra  REENTER
              03ab 00f6  ld   $f6
SYS:          03ac 8120  adda [$20]
              03ad e8a7  blt  retry
              03ae 151b  ld   [$1b],y
              03af e11a  jmp  y,[$1a]
SUBW:         03b0 1200  ld   ac,x
              03b1 8001  adda $01
              03b2 c221  st   [$21]
              03b3 011a  ld   [$1a]
              03b4 e8b9  blt  .subw0
              03b5 a500  suba [x]
              03b6 c21a  st   [$1a]
              03b7 fcbc  bra  .subw1
              03b8 4500  ora  [x]
.subw0:       03b9 c21a  st   [$1a]
              03ba 2500  anda [x]
              03bb 0200  nop
.subw1:       03bc 3080  anda $80,x
              03bd 011b  ld   [$1b]
              03be a500  suba [x]
              03bf 1121  ld   [$21],x
              03c0 a500  suba [x]
              03c1 c21b  st   [$1b]
              03c2 00f2  ld   $f2
              03c3 fc02  bra  NEXT
ADDW:         03c4 1200  ld   ac,x
              03c5 8001  adda $01
              03c6 c221  st   [$21]
              03c7 011a  ld   [$1a]
              03c8 8500  adda [x]
              03c9 c21a  st   [$1a]
              03ca e8ce  blt  .addw0
              03cb a500  suba [x]
              03cc fcd0  bra  .addw1
              03cd 4500  ora  [x]
.addw0:       03ce 2500  anda [x]
              03cf 0200  nop
.addw1:       03d0 3080  anda $80,x
              03d1 0500  ld   [x]
              03d2 811b  adda [$1b]
              03d3 1121  ld   [$21],x
              03d4 8500  adda [x]
              03d5 c21b  st   [$1b]
              03d6 fc02  bra  NEXT
              03d7 00f2  ld   $f2
PEEK:         03d8 0118  ld   [$18]
              03d9 a001  suba $01
              03da c218  st   [$18]
              03db 111a  ld   [$1a],x
              03dc 151b  ld   [$1b],y
              03dd 0d00  ld   [y,x]
              03de c21a  st   [$1a]
              03df 0000  ld   $00
              03e0 c21b  st   [$1b]
              03e1 00f5  ld   $f5
REENTER:      03e2 fc02  bra  NEXT        ;Return from SYS calls
              03e3 1519  ld   [$19],y
CALL:         03e4 0118  ld   [$18]
              03e5 8001  adda $01         ;CALL has no operand, advances PC by 1
              03e6 c21c  st   [$1c]
              03e7 011a  ld   [$1a]
              03e8 a002  suba $02         ;vAC is actual address, NEXT adds 2
              03e9 c218  st   [$18]
              03ea 0119  ld   [$19]
              03eb c21d  st   [$1d]
              03ec 011b  ld   [$1b]
              03ed d619  st   [$19],y
              03ee fc02  bra  NEXT
              03ef 00f5  ld   $f5
ADDI:         03f0 1404  ld   $04,y
              03f1 e007  jmp  y,$07
              03f2 c221  st   [$21]
SUBI:         03f3 1404  ld   $04,y
              03f4 e016  jmp  y,$16
              03f5 c221  st   [$21]
INC:          03f6 1200  ld   ac,x
              03f7 0500  ld   [x]
              03f8 8001  adda $01
              03f9 c600  st   [x]
              03fa fc02  bra  NEXT
              03fb 00f8  ld   $f8
DEF:          03fc 1404  ld   $04,y
              03fd e026  jmp  y,$26
              03fe c221  st   [$21]
RET:          03ff 011c  ld   [$1c]
              0400 a002  suba $02
              0401 c218  st   [$18]
              0402 011d  ld   [$1d]
              0403 c219  st   [$19]
              0404 1403  ld   $03,y
              0405 e0e2  jmp  y,$e2
              0406 00f6  ld   $f6
addi:         0407 811a  adda [$1a]
              0408 c21a  st   [$1a]
              0409 e80d  blt  .addi0
              040a a121  suba [$21]
              040b fc0f  bra  .addi1
              040c 4121  ora  [$21]
.addi0:       040d 2121  anda [$21]
              040e 0200  nop
.addi1:       040f 3080  anda $80,x
              0410 0500  ld   [x]
              0411 811b  adda [$1b]
              0412 c21b  st   [$1b]
              0413 1403  ld   $03,y
              0414 e0e2  jmp  y,$e2
              0415 00f2  ld   $f2
subi:         0416 011a  ld   [$1a]
              0417 e81c  blt  .subi0
              0418 a121  suba [$21]
              0419 c21a  st   [$1a]
              041a fc1f  bra  .subi1
              041b 4121  ora  [$21]
.subi0:       041c c21a  st   [$1a]
              041d 2121  anda [$21]
              041e 0200  nop
.subi1:       041f 3080  anda $80,x
              0420 011b  ld   [$1b]
              0421 a500  suba [x]
              0422 c21b  st   [$1b]
              0423 1403  ld   $03,y
              0424 e0e2  jmp  y,$e2
              0425 00f2  ld   $f2
def:          0426 0118  ld   [$18]
              0427 8002  adda $02
              0428 c21a  st   [$1a]
              0429 0119  ld   [$19]
              042a c21b  st   [$1b]
              042b 0121  ld   [$21]
              042c c218  st   [$18]
              042d 1403  ld   $03,y
              042e 00f3  ld   $f3
              042f e0e2  jmp  y,$e2
              0430 0200  nop
SYS_38_VCLEAR8: 0431 1122  ld   [$22],x
              0432 0123  ld   [$23]
              0433 9400  adda $00,y
              0434 cc00  st   $00,[y,x]
              0435 9401  adda $01,y
              0436 cc00  st   $00,[y,x]
              0437 9402  adda $02,y
              0438 cc00  st   $00,[y,x]
              0439 9403  adda $03,y
              043a cc00  st   $00,[y,x]
              043b 9404  adda $04,y
              043c cc00  st   $00,[y,x]
              043d 9405  adda $05,y
              043e cc00  st   $00,[y,x]
              043f 9406  adda $06,y
              0440 cc00  st   $00,[y,x]
              0441 9407  adda $07,y
              0442 cc00  st   $00,[y,x]
              0443 1403  ld   $03,y
              0444 e0e2  jmp  y,$e2
              0445 00ed  ld   $ed
              0446 0200  nop
              0447 0200  nop
              0448 0200  nop
              * 186 times
font32up:     0500 0000  ld   $00         ;Char ' '
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
              05fe e07b  jmp  y,$7b
              05ff 1519  ld   [$19],y
font82up:     0600 00fe  ld   $fe         ;Char 'R'
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
              06fe e07b  jmp  y,$7b
              06ff 1519  ld   [$19],y
jupiter0:     0700 0000  ld   $00
              0701 0000  ld   $00
              0702 0000  ld   $00
              * 160 times
              07a0 0200  nop
              07a1 0200  nop
              07a2 0200  nop
              * 91 times
              07fb fe00  bra  ac          ;Trampoline for page $0700 lookups
              07fc fcfd  bra  $07fd
              07fd 1403  ld   $03,y
              07fe e07b  jmp  y,$7b
              07ff 1519  ld   [$19],y
jupiter1:     0800 0000  ld   $00
              0801 0000  ld   $00
              0802 0000  ld   $00
              * 160 times
              08a0 0200  nop
              08a1 0200  nop
              08a2 0200  nop
              * 91 times
              08fb fe00  bra  ac          ;Trampoline for page $0800 lookups
              08fc fcfd  bra  $08fd
              08fd 1403  ld   $03,y
              08fe e07b  jmp  y,$7b
              08ff 1519  ld   [$19],y
jupiter2:     0900 0000  ld   $00
              0901 0000  ld   $00
              0902 0000  ld   $00
              * 160 times
              09a0 0200  nop
              09a1 0200  nop
              09a2 0200  nop
              * 91 times
              09fb fe00  bra  ac          ;Trampoline for page $0900 lookups
              09fc fcfd  bra  $09fd
              09fd 1403  ld   $03,y
              09fe e07b  jmp  y,$7b
              09ff 1519  ld   [$19],y
jupiter3:     0a00 0000  ld   $00
              0a01 0000  ld   $00
              0a02 0000  ld   $00
              * 69 times
              0a45 0001  ld   $01
              0a46 0010  ld   $10
              0a47 0005  ld   $05
              0a48 0011  ld   $11
              0a49 0005  ld   $05
              0a4a 0015  ld   $15
              0a4b 0015  ld   $15
              0a4c 0015  ld   $15
              * 10 times
              0a54 0005  ld   $05
              0a55 0015  ld   $15
              0a56 0001  ld   $01
              0a57 0015  ld   $15
              0a58 0001  ld   $01
              0a59 0000  ld   $00
              0a5a 0000  ld   $00
              0a5b 0000  ld   $00
              * 71 times
              0aa0 0200  nop
              0aa1 0200  nop
              0aa2 0200  nop
              * 91 times
              0afb fe00  bra  ac          ;Trampoline for page $0a00 lookups
              0afc fcfd  bra  $0afd
              0afd 1403  ld   $03,y
              0afe e07b  jmp  y,$7b
              0aff 1519  ld   [$19],y
jupiter4:     0b00 0000  ld   $00
              0b01 0000  ld   $00
              0b02 0000  ld   $00
              * 64 times
              0b40 0001  ld   $01
              0b41 0015  ld   $15
              0b42 0015  ld   $15
              0b43 0015  ld   $15
              * 5 times
              0b46 001a  ld   $1a
              0b47 0015  ld   $15
              0b48 0016  ld   $16
              0b49 0015  ld   $15
              0b4a 0016  ld   $16
              0b4b 0016  ld   $16
              0b4c 0016  ld   $16
              * 8 times
              0b52 0015  ld   $15
              0b53 0016  ld   $16
              0b54 0015  ld   $15
              0b55 0015  ld   $15
              0b56 0015  ld   $15
              * 8 times
              0b5c 0001  ld   $01
              0b5d 0014  ld   $14
              0b5e 0001  ld   $01
              0b5f 0000  ld   $00
              0b60 0000  ld   $00
              0b61 0000  ld   $00
              * 65 times
              0ba0 0200  nop
              0ba1 0200  nop
              0ba2 0200  nop
              * 91 times
              0bfb fe00  bra  ac          ;Trampoline for page $0b00 lookups
              0bfc fcfd  bra  $0bfd
              0bfd 1403  ld   $03,y
              0bfe e07b  jmp  y,$7b
              0bff 1519  ld   [$19],y
jupiter5:     0c00 0000  ld   $00
              0c01 0000  ld   $00
              0c02 0000  ld   $00
              * 60 times
              0c3c 0010  ld   $10
              0c3d 0001  ld   $01
              0c3e 0015  ld   $15
              0c3f 0019  ld   $19
              0c40 0016  ld   $16
              0c41 0015  ld   $15
              0c42 0016  ld   $16
              0c43 0016  ld   $16
              0c44 001a  ld   $1a
              0c45 0016  ld   $16
              0c46 0015  ld   $15
              0c47 0016  ld   $16
              0c48 001a  ld   $1a
              0c49 0016  ld   $16
              0c4a 001a  ld   $1a
              0c4b 0015  ld   $15
              0c4c 001a  ld   $1a
              0c4d 0015  ld   $15
              0c4e 001a  ld   $1a
              0c4f 0015  ld   $15
              0c50 0019  ld   $19
              0c51 0016  ld   $16
              0c52 0019  ld   $19
              0c53 0016  ld   $16
              0c54 001a  ld   $1a
              0c55 0015  ld   $15
              0c56 0016  ld   $16
              0c57 001a  ld   $1a
              0c58 0015  ld   $15
              0c59 0016  ld   $16
              0c5a 0015  ld   $15
              0c5b 0016  ld   $16
              0c5c 0015  ld   $15
              0c5d 0015  ld   $15
              0c5e 0015  ld   $15
              * 6 times
              0c62 0000  ld   $00
              0c63 0000  ld   $00
              0c64 0000  ld   $00
              * 62 times
              0ca0 0200  nop
              0ca1 0200  nop
              0ca2 0200  nop
              * 91 times
              0cfb fe00  bra  ac          ;Trampoline for page $0c00 lookups
              0cfc fcfd  bra  $0cfd
              0cfd 1403  ld   $03,y
              0cfe e07b  jmp  y,$7b
              0cff 1519  ld   [$19],y
jupiter6:     0d00 0000  ld   $00
              0d01 0000  ld   $00
              0d02 0000  ld   $00
              * 57 times
              0d39 0001  ld   $01
              0d3a 0015  ld   $15
              0d3b 0016  ld   $16
              0d3c 001a  ld   $1a
              0d3d 0016  ld   $16
              0d3e 002a  ld   $2a
              0d3f 0016  ld   $16
              0d40 0016  ld   $16
              0d41 001a  ld   $1a
              0d42 0016  ld   $16
              0d43 001a  ld   $1a
              0d44 0016  ld   $16
              0d45 001a  ld   $1a
              0d46 0016  ld   $16
              0d47 001a  ld   $1a
              0d48 0015  ld   $15
              0d49 001a  ld   $1a
              0d4a 0016  ld   $16
              0d4b 001a  ld   $1a
              0d4c 0016  ld   $16
              0d4d 001a  ld   $1a
              0d4e 0016  ld   $16
              0d4f 0016  ld   $16
              0d50 001a  ld   $1a
              0d51 0016  ld   $16
              0d52 0016  ld   $16
              0d53 0015  ld   $15
              0d54 0016  ld   $16
              0d55 0016  ld   $16
              0d56 0015  ld   $15
              0d57 0015  ld   $15
              0d58 0016  ld   $16
              0d59 0005  ld   $05
              0d5a 0015  ld   $15
              0d5b 0015  ld   $15
              0d5c 0016  ld   $16
              0d5d 0005  ld   $05
              0d5e 0016  ld   $16
              0d5f 0015  ld   $15
              0d60 0005  ld   $05
              0d61 0015  ld   $15
              0d62 0015  ld   $15
              0d63 0015  ld   $15
              0d64 0001  ld   $01
              0d65 0000  ld   $00
              0d66 0000  ld   $00
              0d67 0000  ld   $00
              * 59 times
              0da0 0200  nop
              0da1 0200  nop
              0da2 0200  nop
              * 91 times
              0dfb fe00  bra  ac          ;Trampoline for page $0d00 lookups
              0dfc fcfd  bra  $0dfd
              0dfd 1403  ld   $03,y
              0dfe e07b  jmp  y,$7b
              0dff 1519  ld   [$19],y
jupiter7:     0e00 0000  ld   $00
              0e01 0000  ld   $00
              0e02 0000  ld   $00
              * 55 times
              0e37 0005  ld   $05
              0e38 0016  ld   $16
              0e39 0019  ld   $19
              0e3a 0016  ld   $16
              0e3b 001a  ld   $1a
              0e3c 0016  ld   $16
              0e3d 001a  ld   $1a
              0e3e 0016  ld   $16
              0e3f 001a  ld   $1a
              0e40 0016  ld   $16
              0e41 001a  ld   $1a
              0e42 002a  ld   $2a
              0e43 0016  ld   $16
              0e44 001a  ld   $1a
              0e45 0026  ld   $26
              0e46 001a  ld   $1a
              0e47 0016  ld   $16
              0e48 002a  ld   $2a
              0e49 0016  ld   $16
              0e4a 001a  ld   $1a
              0e4b 0016  ld   $16
              0e4c 001a  ld   $1a
              0e4d 0016  ld   $16
              0e4e 001a  ld   $1a
              0e4f 0015  ld   $15
              0e50 001a  ld   $1a
              0e51 0015  ld   $15
              0e52 001a  ld   $1a
              0e53 0016  ld   $16
              0e54 001a  ld   $1a
              0e55 0015  ld   $15
              0e56 001a  ld   $1a
              0e57 0005  ld   $05
              0e58 0016  ld   $16
              0e59 0015  ld   $15
              0e5a 0016  ld   $16
              0e5b 0006  ld   $06
              0e5c 0015  ld   $15
              0e5d 0016  ld   $16
              0e5e 0015  ld   $15
              0e5f 0005  ld   $05
              0e60 0015  ld   $15
              0e61 0015  ld   $15
              0e62 0005  ld   $05
              0e63 0015  ld   $15
              0e64 0015  ld   $15
              0e65 0015  ld   $15
              0e66 0005  ld   $05
              0e67 0015  ld   $15
              0e68 0000  ld   $00
              0e69 0000  ld   $00
              0e6a 0000  ld   $00
              * 56 times
              0ea0 0200  nop
              0ea1 0200  nop
              0ea2 0200  nop
              * 91 times
              0efb fe00  bra  ac          ;Trampoline for page $0e00 lookups
              0efc fcfd  bra  $0efd
              0efd 1403  ld   $03,y
              0efe e07b  jmp  y,$7b
              0eff 1519  ld   [$19],y
jupiter8:     0f00 0000  ld   $00
              0f01 0000  ld   $00
              0f02 0000  ld   $00
              * 53 times
              0f35 0015  ld   $15
              0f36 0016  ld   $16
              0f37 0016  ld   $16
              0f38 001a  ld   $1a
              0f39 0016  ld   $16
              0f3a 001a  ld   $1a
              0f3b 0016  ld   $16
              0f3c 001a  ld   $1a
              0f3d 0026  ld   $26
              0f3e 001a  ld   $1a
              0f3f 001a  ld   $1a
              0f40 002a  ld   $2a
              0f41 0016  ld   $16
              0f42 001a  ld   $1a
              0f43 0016  ld   $16
              0f44 001a  ld   $1a
              0f45 0016  ld   $16
              0f46 001a  ld   $1a
              0f47 0016  ld   $16
              0f48 001a  ld   $1a
              0f49 0016  ld   $16
              0f4a 0016  ld   $16
              0f4b 0019  ld   $19
              0f4c 0016  ld   $16
              0f4d 0016  ld   $16
              0f4e 001a  ld   $1a
              0f4f 0016  ld   $16
              0f50 0016  ld   $16
              0f51 001a  ld   $1a
              0f52 0016  ld   $16
              0f53 0015  ld   $15
              0f54 0016  ld   $16
              0f55 0016  ld   $16
              0f56 0015  ld   $15
              0f57 0016  ld   $16
              0f58 000a  ld   $0a
              0f59 0015  ld   $15
              0f5a 0006  ld   $06
              0f5b 0015  ld   $15
              0f5c 0006  ld   $06
              0f5d 0015  ld   $15
              0f5e 0006  ld   $06
              0f5f 0016  ld   $16
              0f60 0015  ld   $15
              0f61 0006  ld   $06
              0f62 0015  ld   $15
              0f63 0005  ld   $05
              0f64 0015  ld   $15
              0f65 0005  ld   $05
              0f66 0011  ld   $11
              0f67 0005  ld   $05
              0f68 0015  ld   $15
              0f69 0000  ld   $00
              0f6a 0000  ld   $00
              0f6b 0000  ld   $00
              * 55 times
              0fa0 0200  nop
              0fa1 0200  nop
              0fa2 0200  nop
              * 91 times
              0ffb fe00  bra  ac          ;Trampoline for page $0f00 lookups
              0ffc fcfd  bra  $0ffd
              0ffd 1403  ld   $03,y
              0ffe e07b  jmp  y,$7b
              0fff 1519  ld   [$19],y
jupiter9:     1000 0000  ld   $00
              1001 0000  ld   $00
              1002 0000  ld   $00
              * 50 times
              1032 0001  ld   $01
              1033 0015  ld   $15
              1034 0015  ld   $15
              1035 001a  ld   $1a
              1036 0026  ld   $26
              1037 001a  ld   $1a
              1038 0016  ld   $16
              1039 001a  ld   $1a
              103a 0016  ld   $16
              103b 0016  ld   $16
              103c 001a  ld   $1a
              103d 0016  ld   $16
              103e 001a  ld   $1a
              103f 0016  ld   $16
              1040 0016  ld   $16
              1041 001a  ld   $1a
              1042 0016  ld   $16
              1043 001a  ld   $1a
              1044 0016  ld   $16
              1045 001a  ld   $1a
              1046 0016  ld   $16
              1047 001a  ld   $1a
              1048 0016  ld   $16
              1049 0016  ld   $16
              104a 001a  ld   $1a
              104b 0016  ld   $16
              104c 0016  ld   $16
              104d 001a  ld   $1a
              104e 0016  ld   $16
              104f 0016  ld   $16
              1050 0019  ld   $19
              1051 0016  ld   $16
              1052 0015  ld   $15
              1053 001a  ld   $1a
              1054 0016  ld   $16
              1055 0019  ld   $19
              1056 0006  ld   $06
              1057 0016  ld   $16
              1058 0005  ld   $05
              1059 0016  ld   $16
              105a 0005  ld   $05
              105b 0016  ld   $16
              105c 0015  ld   $15
              105d 0006  ld   $06
              105e 0015  ld   $15
              105f 0015  ld   $15
              1060 0006  ld   $06
              1061 0015  ld   $15
              1062 0006  ld   $06
              1063 0015  ld   $15
              1064 0005  ld   $05
              1065 0005  ld   $05
              1066 0015  ld   $15
              1067 0005  ld   $05
              1068 0011  ld   $11
              1069 0005  ld   $05
              106a 0015  ld   $15
              106b 0005  ld   $05
              106c 0000  ld   $00
              106d 0000  ld   $00
              106e 0000  ld   $00
              * 52 times
              10a0 0200  nop
              10a1 0200  nop
              10a2 0200  nop
              * 91 times
              10fb fe00  bra  ac          ;Trampoline for page $1000 lookups
              10fc fcfd  bra  $10fd
              10fd 1403  ld   $03,y
              10fe e07b  jmp  y,$7b
              10ff 1519  ld   [$19],y
jupiter10:    1100 0000  ld   $00
              1101 0000  ld   $00
              1102 0000  ld   $00
              * 48 times
              1130 0001  ld   $01
              1131 0015  ld   $15
              1132 0015  ld   $15
              1133 001a  ld   $1a
              1134 0016  ld   $16
              1135 001a  ld   $1a
              1136 0016  ld   $16
              1137 001a  ld   $1a
              1138 0016  ld   $16
              1139 0016  ld   $16
              113a 001a  ld   $1a
              113b 0016  ld   $16
              113c 0016  ld   $16
              113d 001a  ld   $1a
              113e 0016  ld   $16
              113f 001a  ld   $1a
              1140 001a  ld   $1a
              1141 0016  ld   $16
              1142 001a  ld   $1a
              1143 0016  ld   $16
              1144 001a  ld   $1a
              1145 0016  ld   $16
              1146 001a  ld   $1a
              1147 0016  ld   $16
              1148 001a  ld   $1a
              1149 001a  ld   $1a
              114a 0016  ld   $16
              114b 001a  ld   $1a
              114c 001a  ld   $1a
              114d 0016  ld   $16
              114e 001a  ld   $1a
              114f 0016  ld   $16
              1150 0016  ld   $16
              1151 001a  ld   $1a
              1152 0016  ld   $16
              1153 0016  ld   $16
              1154 001a  ld   $1a
              1155 0016  ld   $16
              1156 0016  ld   $16
              1157 0015  ld   $15
              1158 0016  ld   $16
              1159 0005  ld   $05
              115a 0016  ld   $16
              115b 0005  ld   $05
              115c 0016  ld   $16
              115d 0005  ld   $05
              115e 0016  ld   $16
              115f 0005  ld   $05
              1160 0016  ld   $16
              1161 0015  ld   $15
              1162 0015  ld   $15
              1163 0005  ld   $05
              1164 0012  ld   $12
              1165 0005  ld   $05
              1166 0015  ld   $15
              1167 0001  ld   $01
              1168 0015  ld   $15
              1169 0005  ld   $05
              116a 0011  ld   $11
              116b 0001  ld   $01
              116c 0015  ld   $15
              116d 0001  ld   $01
              116e 0000  ld   $00
              116f 0000  ld   $00
              1170 0000  ld   $00
              * 50 times
              11a0 0200  nop
              11a1 0200  nop
              11a2 0200  nop
              * 91 times
              11fb fe00  bra  ac          ;Trampoline for page $1100 lookups
              11fc fcfd  bra  $11fd
              11fd 1403  ld   $03,y
              11fe e07b  jmp  y,$7b
              11ff 1519  ld   [$19],y
jupiter11:    1200 0000  ld   $00
              1201 0000  ld   $00
              1202 0000  ld   $00
              * 47 times
              122f 0005  ld   $05
              1230 0015  ld   $15
              1231 0016  ld   $16
              1232 001a  ld   $1a
              1233 0016  ld   $16
              1234 001a  ld   $1a
              1235 0016  ld   $16
              1236 001a  ld   $1a
              1237 0016  ld   $16
              1238 001a  ld   $1a
              1239 001a  ld   $1a
              123a 0016  ld   $16
              123b 001a  ld   $1a
              123c 0016  ld   $16
              123d 001a  ld   $1a
              123e 0016  ld   $16
              123f 001a  ld   $1a
              1240 0016  ld   $16
              1241 001a  ld   $1a
              1242 001a  ld   $1a
              1243 0016  ld   $16
              1244 001a  ld   $1a
              1245 001a  ld   $1a
              1246 0016  ld   $16
              1247 001a  ld   $1a
              1248 0016  ld   $16
              1249 001a  ld   $1a
              124a 0016  ld   $16
              124b 001a  ld   $1a
              124c 0016  ld   $16
              124d 0016  ld   $16
              124e 001a  ld   $1a
              124f 0016  ld   $16
              1250 001a  ld   $1a
              1251 0016  ld   $16
              1252 0016  ld   $16
              1253 001a  ld   $1a
              1254 0006  ld   $06
              1255 0015  ld   $15
              1256 0016  ld   $16
              1257 001a  ld   $1a
              1258 0006  ld   $06
              1259 001a  ld   $1a
              125a 0016  ld   $16
              125b 0016  ld   $16
              125c 0015  ld   $15
              125d 0016  ld   $16
              125e 0015  ld   $15
              125f 0016  ld   $16
              1260 0005  ld   $05
              1261 0016  ld   $16
              1262 0005  ld   $05
              1263 0016  ld   $16
              1264 0015  ld   $15
              1265 0005  ld   $05
              1266 0015  ld   $15
              1267 0006  ld   $06
              1268 0005  ld   $05
              1269 0015  ld   $15
              126a 0005  ld   $05
              126b 0005  ld   $05
              126c 0011  ld   $11
              126d 0005  ld   $05
              126e 0001  ld   $01
              126f 0015  ld   $15
              1270 0000  ld   $00
              1271 0000  ld   $00
              1272 0000  ld   $00
              * 48 times
              12a0 0200  nop
              12a1 0200  nop
              12a2 0200  nop
              * 91 times
              12fb fe00  bra  ac          ;Trampoline for page $1200 lookups
              12fc fcfd  bra  $12fd
              12fd 1403  ld   $03,y
              12fe e07b  jmp  y,$7b
              12ff 1519  ld   [$19],y
jupiter12:    1300 0000  ld   $00
              1301 0000  ld   $00
              1302 0000  ld   $00
              * 45 times
              132d 0005  ld   $05
              132e 0015  ld   $15
              132f 001a  ld   $1a
              1330 0016  ld   $16
              1331 001a  ld   $1a
              1332 001a  ld   $1a
              1333 001b  ld   $1b
              1334 0016  ld   $16
              1335 001b  ld   $1b
              1336 0016  ld   $16
              1337 001a  ld   $1a
              1338 0016  ld   $16
              1339 0016  ld   $16
              133a 001a  ld   $1a
              133b 0016  ld   $16
              133c 0016  ld   $16
              133d 001a  ld   $1a
              133e 0017  ld   $17
              133f 001a  ld   $1a
              1340 0016  ld   $16
              1341 001a  ld   $1a
              1342 0017  ld   $17
              1343 001a  ld   $1a
              1344 001a  ld   $1a
              1345 0016  ld   $16
              1346 001a  ld   $1a
              1347 0016  ld   $16
              1348 001a  ld   $1a
              1349 0016  ld   $16
              134a 001a  ld   $1a
              134b 0016  ld   $16
              134c 001a  ld   $1a
              134d 001a  ld   $1a
              134e 0016  ld   $16
              134f 001a  ld   $1a
              1350 0016  ld   $16
              1351 0019  ld   $19
              1352 0016  ld   $16
              1353 001a  ld   $1a
              1354 0016  ld   $16
              1355 001a  ld   $1a
              1356 0016  ld   $16
              1357 0015  ld   $15
              1358 0016  ld   $16
              1359 0015  ld   $15
              135a 0006  ld   $06
              135b 0015  ld   $15
              135c 0006  ld   $06
              135d 0006  ld   $06
              135e 0015  ld   $15
              135f 0016  ld   $16
              1360 0015  ld   $15
              1361 0016  ld   $16
              1362 0015  ld   $15
              1363 0005  ld   $05
              1364 0016  ld   $16
              1365 0015  ld   $15
              1366 0016  ld   $16
              1367 0005  ld   $05
              1368 0015  ld   $15
              1369 0001  ld   $01
              136a 0015  ld   $15
              136b 0005  ld   $05
              136c 0015  ld   $15
              136d 0005  ld   $05
              136e 0014  ld   $14
              136f 0001  ld   $01
              1370 0015  ld   $15
              1371 0000  ld   $00
              1372 0000  ld   $00
              1373 0000  ld   $00
              * 47 times
              13a0 0200  nop
              13a1 0200  nop
              13a2 0200  nop
              * 91 times
              13fb fe00  bra  ac          ;Trampoline for page $1300 lookups
              13fc fcfd  bra  $13fd
              13fd 1403  ld   $03,y
              13fe e07b  jmp  y,$7b
              13ff 1519  ld   [$19],y
jupiter13:    1400 0000  ld   $00
              1401 0000  ld   $00
              1402 0000  ld   $00
              * 43 times
              142b 0001  ld   $01
              142c 0015  ld   $15
              142d 001a  ld   $1a
              142e 0016  ld   $16
              142f 001a  ld   $1a
              1430 001a  ld   $1a
              1431 001b  ld   $1b
              1432 0016  ld   $16
              1433 001a  ld   $1a
              1434 0017  ld   $17
              1435 001a  ld   $1a
              1436 001a  ld   $1a
              1437 0017  ld   $17
              1438 001a  ld   $1a
              1439 001a  ld   $1a
              143a 0016  ld   $16
              143b 001b  ld   $1b
              143c 001a  ld   $1a
              143d 0016  ld   $16
              143e 001a  ld   $1a
              143f 001b  ld   $1b
              1440 0016  ld   $16
              1441 001a  ld   $1a
              1442 0016  ld   $16
              1443 001b  ld   $1b
              1444 002a  ld   $2a
              1445 001b  ld   $1b
              1446 0016  ld   $16
              1447 001a  ld   $1a
              1448 002a  ld   $2a
              1449 001a  ld   $1a
              144a 0016  ld   $16
              144b 001a  ld   $1a
              144c 0016  ld   $16
              144d 001a  ld   $1a
              144e 0016  ld   $16
              144f 001a  ld   $1a
              1450 0016  ld   $16
              1451 001a  ld   $1a
              1452 0016  ld   $16
              1453 001a  ld   $1a
              1454 0016  ld   $16
              1455 001a  ld   $1a
              1456 0016  ld   $16
              1457 001a  ld   $1a
              1458 0006  ld   $06
              1459 001a  ld   $1a
              145a 0016  ld   $16
              145b 001a  ld   $1a
              145c 0016  ld   $16
              145d 0019  ld   $19
              145e 0016  ld   $16
              145f 0006  ld   $06
              1460 0016  ld   $16
              1461 0005  ld   $05
              1462 0016  ld   $16
              1463 0016  ld   $16
              1464 0015  ld   $15
              1465 0016  ld   $16
              1466 0015  ld   $15
              1467 0005  ld   $05
              1468 0016  ld   $16
              1469 0005  ld   $05
              146a 0016  ld   $16
              146b 0005  ld   $05
              146c 0015  ld   $15
              146d 0001  ld   $01
              146e 0015  ld   $15
              146f 0005  ld   $05
              1470 0015  ld   $15
              1471 0001  ld   $01
              1472 0015  ld   $15
              1473 0001  ld   $01
              1474 0000  ld   $00
              1475 0000  ld   $00
              1476 0000  ld   $00
              * 44 times
              14a0 0200  nop
              14a1 0200  nop
              14a2 0200  nop
              * 91 times
              14fb fe00  bra  ac          ;Trampoline for page $1400 lookups
              14fc fcfd  bra  $14fd
              14fd 1403  ld   $03,y
              14fe e07b  jmp  y,$7b
              14ff 1519  ld   [$19],y
jupiter14:    1500 0000  ld   $00
              1501 0000  ld   $00
              1502 0000  ld   $00
              * 42 times
              152a 0015  ld   $15
              152b 0015  ld   $15
              152c 001a  ld   $1a
              152d 001b  ld   $1b
              152e 001a  ld   $1a
              152f 001b  ld   $1b
              1530 002b  ld   $2b
              1531 001a  ld   $1a
              1532 001b  ld   $1b
              1533 001a  ld   $1a
              1534 001a  ld   $1a
              1535 0016  ld   $16
              1536 001b  ld   $1b
              1537 001a  ld   $1a
              1538 0016  ld   $16
              1539 001b  ld   $1b
              153a 001a  ld   $1a
              153b 001a  ld   $1a
              153c 0017  ld   $17
              153d 001a  ld   $1a
              153e 001b  ld   $1b
              153f 0016  ld   $16
              1540 001a  ld   $1a
              1541 001a  ld   $1a
              1542 002b  ld   $2b
              1543 001a  ld   $1a
              1544 0016  ld   $16
              1545 001b  ld   $1b
              1546 001a  ld   $1a
              1547 0016  ld   $16
              1548 001b  ld   $1b
              1549 0016  ld   $16
              154a 001a  ld   $1a
              154b 001a  ld   $1a
              154c 002a  ld   $2a
              154d 0016  ld   $16
              154e 001a  ld   $1a
              154f 001a  ld   $1a
              1550 0016  ld   $16
              1551 001a  ld   $1a
              1552 0016  ld   $16
              1553 001a  ld   $1a
              1554 0016  ld   $16
              1555 001a  ld   $1a
              1556 0016  ld   $16
              1557 001a  ld   $1a
              1558 0016  ld   $16
              1559 0015  ld   $15
              155a 0016  ld   $16
              155b 0005  ld   $05
              155c 001a  ld   $1a
              155d 0016  ld   $16
              155e 0006  ld   $06
              155f 0019  ld   $19
              1560 0016  ld   $16
              1561 0015  ld   $15
              1562 001a  ld   $1a
              1563 0015  ld   $15
              1564 000a  ld   $0a
              1565 0015  ld   $15
              1566 0016  ld   $16
              1567 0016  ld   $16
              1568 0015  ld   $15
              1569 0015  ld   $15
              156a 0015  ld   $15
              156b 0015  ld   $15
              156c 0005  ld   $05
              156d 0015  ld   $15
              156e 0005  ld   $05
              156f 0015  ld   $15
              1570 0001  ld   $01
              1571 0005  ld   $05
              1572 0015  ld   $15
              1573 0004  ld   $04
              1574 0011  ld   $11
              1575 0000  ld   $00
              1576 0000  ld   $00
              1577 0000  ld   $00
              * 43 times
              15a0 0200  nop
              15a1 0200  nop
              15a2 0200  nop
              * 91 times
              15fb fe00  bra  ac          ;Trampoline for page $1500 lookups
              15fc fcfd  bra  $15fd
              15fd 1403  ld   $03,y
              15fe e07b  jmp  y,$7b
              15ff 1519  ld   [$19],y
jupiter15:    1600 0000  ld   $00
              1601 0000  ld   $00
              1602 0000  ld   $00
              * 40 times
              1628 0001  ld   $01
              1629 0015  ld   $15
              162a 001a  ld   $1a
              162b 001a  ld   $1a
              162c 001b  ld   $1b
              162d 002a  ld   $2a
              162e 001b  ld   $1b
              162f 002b  ld   $2b
              1630 001a  ld   $1a
              1631 002b  ld   $2b
              1632 001a  ld   $1a
              1633 002b  ld   $2b
              1634 001b  ld   $1b
              1635 001b  ld   $1b
              1636 0016  ld   $16
              1637 001a  ld   $1a
              1638 001b  ld   $1b
              1639 001a  ld   $1a
              163a 001a  ld   $1a
              163b 001b  ld   $1b
              163c 001a  ld   $1a
              163d 002b  ld   $2b
              163e 0016  ld   $16
              163f 001a  ld   $1a
              1640 0016  ld   $16
              1641 0016  ld   $16
              1642 0016  ld   $16
              1643 001b  ld   $1b
              1644 002a  ld   $2a
              1645 001a  ld   $1a
              1646 002b  ld   $2b
              1647 001a  ld   $1a
              1648 0016  ld   $16
              1649 001a  ld   $1a
              164a 002a  ld   $2a
              164b 0016  ld   $16
              164c 001b  ld   $1b
              164d 001a  ld   $1a
              164e 001b  ld   $1b
              164f 0016  ld   $16
              1650 001a  ld   $1a
              1651 002a  ld   $2a
              1652 001a  ld   $1a
              1653 0016  ld   $16
              1654 001a  ld   $1a
              1655 0016  ld   $16
              1656 001a  ld   $1a
              1657 0016  ld   $16
              1658 001a  ld   $1a
              1659 001a  ld   $1a
              165a 0016  ld   $16
              165b 001a  ld   $1a
              165c 0016  ld   $16
              165d 001a  ld   $1a
              165e 0016  ld   $16
              165f 0016  ld   $16
              1660 001a  ld   $1a
              1661 0016  ld   $16
              1662 0016  ld   $16
              1663 0015  ld   $15
              1664 0016  ld   $16
              1665 0016  ld   $16
              1666 0005  ld   $05
              1667 0016  ld   $16
              1668 0005  ld   $05
              1669 0016  ld   $16
              166a 0006  ld   $06
              166b 0016  ld   $16
              166c 0015  ld   $15
              166d 0016  ld   $16
              166e 0015  ld   $15
              166f 0005  ld   $05
              1670 0015  ld   $15
              1671 0005  ld   $05
              1672 0015  ld   $15
              1673 0001  ld   $01
              1674 0015  ld   $15
              1675 0005  ld   $05
              1676 0000  ld   $00
              1677 0000  ld   $00
              1678 0000  ld   $00
              * 42 times
              16a0 0200  nop
              16a1 0200  nop
              16a2 0200  nop
              * 91 times
              16fb fe00  bra  ac          ;Trampoline for page $1600 lookups
              16fc fcfd  bra  $16fd
              16fd 1403  ld   $03,y
              16fe e07b  jmp  y,$7b
              16ff 1519  ld   [$19],y
jupiter16:    1700 0000  ld   $00
              1701 0000  ld   $00
              1702 0000  ld   $00
              * 39 times
              1727 0011  ld   $11
              1728 0019  ld   $19
              1729 001b  ld   $1b
              172a 002a  ld   $2a
              172b 001b  ld   $1b
              172c 001a  ld   $1a
              172d 001b  ld   $1b
              172e 002a  ld   $2a
              172f 001b  ld   $1b
              1730 002a  ld   $2a
              1731 001b  ld   $1b
              1732 002a  ld   $2a
              1733 001b  ld   $1b
              1734 001a  ld   $1a
              1735 002a  ld   $2a
              1736 001b  ld   $1b
              1737 001a  ld   $1a
              1738 002b  ld   $2b
              1739 0016  ld   $16
              173a 0017  ld   $17
              173b 0016  ld   $16
              173c 001a  ld   $1a
              173d 0016  ld   $16
              173e 001a  ld   $1a
              173f 0016  ld   $16
              1740 001b  ld   $1b
              1741 001a  ld   $1a
              1742 001a  ld   $1a
              1743 0016  ld   $16
              1744 001b  ld   $1b
              1745 0016  ld   $16
              1746 001a  ld   $1a
              1747 0016  ld   $16
              1748 001b  ld   $1b
              1749 001a  ld   $1a
              174a 001b  ld   $1b
              174b 001a  ld   $1a
              174c 002b  ld   $2b
              174d 0016  ld   $16
              174e 002a  ld   $2a
              174f 001a  ld   $1a
              1750 001b  ld   $1b
              1751 0016  ld   $16
              1752 001a  ld   $1a
              1753 001a  ld   $1a
              1754 0016  ld   $16
              1755 001a  ld   $1a
              1756 0016  ld   $16
              1757 001a  ld   $1a
              1758 0016  ld   $16
              1759 001a  ld   $1a
              175a 0016  ld   $16
              175b 001a  ld   $1a
              175c 0016  ld   $16
              175d 0015  ld   $15
              175e 0016  ld   $16
              175f 0019  ld   $19
              1760 0016  ld   $16
              1761 0015  ld   $15
              1762 000a  ld   $0a
              1763 0016  ld   $16
              1764 0015  ld   $15
              1765 001a  ld   $1a
              1766 0015  ld   $15
              1767 0016  ld   $16
              1768 0019  ld   $19
              1769 0016  ld   $16
              176a 0015  ld   $15
              176b 0015  ld   $15
              176c 0006  ld   $06
              176d 0015  ld   $15
              176e 0005  ld   $05
              176f 0016  ld   $16
              1770 0005  ld   $05
              1771 0015  ld   $15
              1772 0005  ld   $05
              1773 0015  ld   $15
              1774 0001  ld   $01
              1775 0015  ld   $15
              1776 0005  ld   $05
              1777 0010  ld   $10
              1778 0000  ld   $00
              1779 0000  ld   $00
              177a 0000  ld   $00
              * 40 times
              17a0 0200  nop
              17a1 0200  nop
              17a2 0200  nop
              * 91 times
              17fb fe00  bra  ac          ;Trampoline for page $1700 lookups
              17fc fcfd  bra  $17fd
              17fd 1403  ld   $03,y
              17fe e07b  jmp  y,$7b
              17ff 1519  ld   [$19],y
jupiter17:    1800 0000  ld   $00
              1801 0000  ld   $00
              1802 0000  ld   $00
              * 39 times
              1827 001a  ld   $1a
              1828 002b  ld   $2b
              1829 001a  ld   $1a
              182a 002b  ld   $2b
              182b 001a  ld   $1a
              182c 002b  ld   $2b
              182d 001a  ld   $1a
              182e 001b  ld   $1b
              182f 002a  ld   $2a
              1830 001b  ld   $1b
              1831 002a  ld   $2a
              1832 001b  ld   $1b
              1833 001a  ld   $1a
              1834 002b  ld   $2b
              1835 001a  ld   $1a
              1836 001b  ld   $1b
              1837 001a  ld   $1a
              1838 0017  ld   $17
              1839 001a  ld   $1a
              183a 001a  ld   $1a
              183b 0016  ld   $16
              183c 001a  ld   $1a
              183d 0016  ld   $16
              183e 001b  ld   $1b
              183f 002b  ld   $2b
              1840 002b  ld   $2b
              1841 001b  ld   $1b
              1842 001a  ld   $1a
              1843 0016  ld   $16
              1844 001a  ld   $1a
              1845 001b  ld   $1b
              1846 0016  ld   $16
              1847 001b  ld   $1b
              1848 001a  ld   $1a
              1849 002b  ld   $2b
              184a 001a  ld   $1a
              184b 0017  ld   $17
              184c 001a  ld   $1a
              184d 001a  ld   $1a
              184e 001b  ld   $1b
              184f 0016  ld   $16
              1850 001a  ld   $1a
              1851 0016  ld   $16
              1852 001a  ld   $1a
              1853 0016  ld   $16
              1854 001a  ld   $1a
              1855 0016  ld   $16
              1856 001a  ld   $1a
              1857 0016  ld   $16
              1858 001a  ld   $1a
              1859 0016  ld   $16
              185a 001a  ld   $1a
              185b 0016  ld   $16
              185c 001a  ld   $1a
              185d 0006  ld   $06
              185e 001a  ld   $1a
              185f 0016  ld   $16
              1860 0006  ld   $06
              1861 0016  ld   $16
              1862 0016  ld   $16
              1863 0015  ld   $15
              1864 0016  ld   $16
              1865 0016  ld   $16
              1866 001a  ld   $1a
              1867 0005  ld   $05
              1868 0016  ld   $16
              1869 0005  ld   $05
              186a 0016  ld   $16
              186b 0009  ld   $09
              186c 0016  ld   $16
              186d 0015  ld   $15
              186e 0016  ld   $16
              186f 0005  ld   $05
              1870 0015  ld   $15
              1871 0001  ld   $01
              1872 0015  ld   $15
              1873 0015  ld   $15
              1874 0005  ld   $05
              1875 0015  ld   $15
              1876 0001  ld   $01
              1877 0015  ld   $15
              1878 0005  ld   $05
              1879 0000  ld   $00
              187a 0000  ld   $00
              187b 0000  ld   $00
              * 39 times
              18a0 0200  nop
              18a1 0200  nop
              18a2 0200  nop
              * 91 times
              18fb fe00  bra  ac          ;Trampoline for page $1800 lookups
              18fc fcfd  bra  $18fd
              18fd 1403  ld   $03,y
              18fe e07b  jmp  y,$7b
              18ff 1519  ld   [$19],y
jupiter18:    1900 0000  ld   $00
              1901 0000  ld   $00
              1902 0000  ld   $00
              * 37 times
              1925 0015  ld   $15
              1926 002b  ld   $2b
              1927 002a  ld   $2a
              1928 002b  ld   $2b
              1929 001b  ld   $1b
              192a 002b  ld   $2b
              192b 001b  ld   $1b
              192c 001b  ld   $1b
              192d 001a  ld   $1a
              192e 002b  ld   $2b
              192f 001b  ld   $1b
              1930 001b  ld   $1b
              1931 001a  ld   $1a
              1932 002b  ld   $2b
              1933 001b  ld   $1b
              1934 0017  ld   $17
              1935 001b  ld   $1b
              1936 0016  ld   $16
              1937 0016  ld   $16
              1938 001a  ld   $1a
              1939 0016  ld   $16
              193a 0017  ld   $17
              193b 001a  ld   $1a
              193c 001b  ld   $1b
              193d 001b  ld   $1b
              193e 001a  ld   $1a
              193f 001b  ld   $1b
              1940 001a  ld   $1a
              1941 0016  ld   $16
              1942 0016  ld   $16
              1943 001a  ld   $1a
              1944 001b  ld   $1b
              1945 0016  ld   $16
              1946 001a  ld   $1a
              1947 001a  ld   $1a
              1948 0017  ld   $17
              1949 001a  ld   $1a
              194a 0016  ld   $16
              194b 001a  ld   $1a
              194c 001a  ld   $1a
              194d 0016  ld   $16
              194e 001a  ld   $1a
              194f 001b  ld   $1b
              1950 001a  ld   $1a
              1951 001a  ld   $1a
              1952 0016  ld   $16
              1953 001a  ld   $1a
              1954 001a  ld   $1a
              1955 0016  ld   $16
              1956 001a  ld   $1a
              1957 0016  ld   $16
              1958 001a  ld   $1a
              1959 0006  ld   $06
              195a 0016  ld   $16
              195b 0016  ld   $16
              195c 0005  ld   $05
              195d 0016  ld   $16
              195e 0015  ld   $15
              195f 0006  ld   $06
              1960 001a  ld   $1a
              1961 0015  ld   $15
              1962 001a  ld   $1a
              1963 0006  ld   $06
              1964 001a  ld   $1a
              1965 0005  ld   $05
              1966 0016  ld   $16
              1967 0015  ld   $15
              1968 0016  ld   $16
              1969 0015  ld   $15
              196a 0006  ld   $06
              196b 0015  ld   $15
              196c 0005  ld   $05
              196d 0006  ld   $06
              196e 0015  ld   $15
              196f 0005  ld   $05
              1970 0015  ld   $15
              1971 0005  ld   $05
              1972 0005  ld   $05
              1973 0005  ld   $05
              1974 0015  ld   $15
              1975 0001  ld   $01
              1976 0005  ld   $05
              1977 0001  ld   $01
              1978 0015  ld   $15
              1979 0000  ld   $00
              197a 0000  ld   $00
              197b 0000  ld   $00
              * 39 times
              19a0 0200  nop
              19a1 0200  nop
              19a2 0200  nop
              * 91 times
              19fb fe00  bra  ac          ;Trampoline for page $1900 lookups
              19fc fcfd  bra  $19fd
              19fd 1403  ld   $03,y
              19fe e07b  jmp  y,$7b
              19ff 1519  ld   [$19],y
jupiter19:    1a00 0000  ld   $00
              1a01 0000  ld   $00
              1a02 0000  ld   $00
              * 36 times
              1a24 0015  ld   $15
              1a25 002a  ld   $2a
              1a26 002b  ld   $2b
              1a27 001b  ld   $1b
              1a28 001b  ld   $1b
              1a29 002a  ld   $2a
              1a2a 001b  ld   $1b
              1a2b 001b  ld   $1b
              1a2c 002a  ld   $2a
              1a2d 001b  ld   $1b
              1a2e 001b  ld   $1b
              1a2f 001b  ld   $1b
              1a30 002a  ld   $2a
              1a31 001b  ld   $1b
              1a32 001b  ld   $1b
              1a33 001a  ld   $1a
              1a34 002b  ld   $2b
              1a35 001a  ld   $1a
              1a36 001b  ld   $1b
              1a37 001a  ld   $1a
              1a38 0017  ld   $17
              1a39 001b  ld   $1b
              1a3a 001a  ld   $1a
              1a3b 0017  ld   $17
              1a3c 001a  ld   $1a
              1a3d 0017  ld   $17
              1a3e 001a  ld   $1a
              1a3f 0017  ld   $17
              1a40 001a  ld   $1a
              1a41 0016  ld   $16
              1a42 001b  ld   $1b
              1a43 001a  ld   $1a
              1a44 0016  ld   $16
              1a45 001b  ld   $1b
              1a46 001a  ld   $1a
              1a47 001b  ld   $1b
              1a48 001a  ld   $1a
              1a49 001b  ld   $1b
              1a4a 001a  ld   $1a
              1a4b 001b  ld   $1b
              1a4c 0016  ld   $16
              1a4d 001b  ld   $1b
              1a4e 001a  ld   $1a
              1a4f 001a  ld   $1a
              1a50 0016  ld   $16
              1a51 0016  ld   $16
              1a52 001a  ld   $1a
              1a53 0016  ld   $16
              1a54 001a  ld   $1a
              1a55 0016  ld   $16
              1a56 000a  ld   $0a
              1a57 0016  ld   $16
              1a58 0006  ld   $06
              1a59 0016  ld   $16
              1a5a 0005  ld   $05
              1a5b 001a  ld   $1a
              1a5c 0016  ld   $16
              1a5d 000a  ld   $0a
              1a5e 0016  ld   $16
              1a5f 0006  ld   $06
              1a60 0015  ld   $15
              1a61 0006  ld   $06
              1a62 0015  ld   $15
              1a63 0006  ld   $06
              1a64 0015  ld   $15
              1a65 0006  ld   $06
              1a66 0015  ld   $15
              1a67 0006  ld   $06
              1a68 0019  ld   $19
              1a69 0006  ld   $06
              1a6a 0015  ld   $15
              1a6b 0006  ld   $06
              1a6c 0015  ld   $15
              1a6d 0005  ld   $05
              1a6e 0015  ld   $15
              1a6f 0006  ld   $06
              1a70 0015  ld   $15
              1a71 0005  ld   $05
              1a72 0015  ld   $15
              1a73 0001  ld   $01
              1a74 0005  ld   $05
              1a75 0015  ld   $15
              1a76 0005  ld   $05
              1a77 0011  ld   $11
              1a78 0005  ld   $05
              1a79 0001  ld   $01
              1a7a 0015  ld   $15
              1a7b 0000  ld   $00
              1a7c 0000  ld   $00
              1a7d 0000  ld   $00
              * 37 times
              1aa0 0200  nop
              1aa1 0200  nop
              1aa2 0200  nop
              * 91 times
              1afb fe00  bra  ac          ;Trampoline for page $1a00 lookups
              1afc fcfd  bra  $1afd
              1afd 1403  ld   $03,y
              1afe e07b  jmp  y,$7b
              1aff 1519  ld   [$19],y
jupiter20:    1b00 0000  ld   $00
              1b01 0000  ld   $00
              1b02 0000  ld   $00
              * 35 times
              1b23 0005  ld   $05
              1b24 002a  ld   $2a
              1b25 001b  ld   $1b
              1b26 001b  ld   $1b
              1b27 002a  ld   $2a
              1b28 001b  ld   $1b
              1b29 001b  ld   $1b
              1b2a 001b  ld   $1b
              1b2b 001a  ld   $1a
              1b2c 001b  ld   $1b
              1b2d 001b  ld   $1b
              1b2e 001a  ld   $1a
              1b2f 001b  ld   $1b
              1b30 0016  ld   $16
              1b31 001b  ld   $1b
              1b32 001a  ld   $1a
              1b33 001b  ld   $1b
              1b34 001a  ld   $1a
              1b35 001b  ld   $1b
              1b36 001b  ld   $1b
              1b37 0016  ld   $16
              1b38 001b  ld   $1b
              1b39 001a  ld   $1a
              1b3a 001b  ld   $1b
              1b3b 001a  ld   $1a
              1b3c 001b  ld   $1b
              1b3d 001a  ld   $1a
              1b3e 001b  ld   $1b
              1b3f 001a  ld   $1a
              1b40 001b  ld   $1b
              1b41 001a  ld   $1a
              1b42 001b  ld   $1b
              1b43 0016  ld   $16
              1b44 001b  ld   $1b
              1b45 001a  ld   $1a
              1b46 0017  ld   $17
              1b47 001a  ld   $1a
              1b48 001a  ld   $1a
              1b49 0017  ld   $17
              1b4a 001a  ld   $1a
              1b4b 0016  ld   $16
              1b4c 001b  ld   $1b
              1b4d 001a  ld   $1a
              1b4e 0017  ld   $17
              1b4f 001a  ld   $1a
              1b50 001b  ld   $1b
              1b51 001a  ld   $1a
              1b52 0017  ld   $17
              1b53 001a  ld   $1a
              1b54 001b  ld   $1b
              1b55 0016  ld   $16
              1b56 001a  ld   $1a
              1b57 0016  ld   $16
              1b58 001a  ld   $1a
              1b59 001a  ld   $1a
              1b5a 0016  ld   $16
              1b5b 0006  ld   $06
              1b5c 0016  ld   $16
              1b5d 0016  ld   $16
              1b5e 0005  ld   $05
              1b5f 001a  ld   $1a
              1b60 0016  ld   $16
              1b61 000a  ld   $0a
              1b62 0016  ld   $16
              1b63 0005  ld   $05
              1b64 0016  ld   $16
              1b65 0015  ld   $15
              1b66 0006  ld   $06
              1b67 0015  ld   $15
              1b68 0006  ld   $06
              1b69 0015  ld   $15
              1b6a 0005  ld   $05
              1b6b 0015  ld   $15
              1b6c 0006  ld   $06
              1b6d 0005  ld   $05
              1b6e 0016  ld   $16
              1b6f 0005  ld   $05
              1b70 0015  ld   $15
              1b71 0002  ld   $02
              1b72 0005  ld   $05
              1b73 0015  ld   $15
              1b74 0001  ld   $01
              1b75 0005  ld   $05
              1b76 0001  ld   $01
              1b77 0004  ld   $04
              1b78 0005  ld   $05
              1b79 0015  ld   $15
              1b7a 0001  ld   $01
              1b7b 0004  ld   $04
              1b7c 0000  ld   $00
              1b7d 0000  ld   $00
              1b7e 0000  ld   $00
              * 36 times
              1ba0 0200  nop
              1ba1 0200  nop
              1ba2 0200  nop
              * 91 times
              1bfb fe00  bra  ac          ;Trampoline for page $1b00 lookups
              1bfc fcfd  bra  $1bfd
              1bfd 1403  ld   $03,y
              1bfe e07b  jmp  y,$7b
              1bff 1519  ld   [$19],y
jupiter21:    1c00 0000  ld   $00
              1c01 0000  ld   $00
              1c02 0000  ld   $00
              * 34 times
              1c22 0015  ld   $15
              1c23 001a  ld   $1a
              1c24 002b  ld   $2b
              1c25 001a  ld   $1a
              1c26 002b  ld   $2b
              1c27 001b  ld   $1b
              1c28 001b  ld   $1b
              1c29 001a  ld   $1a
              1c2a 002b  ld   $2b
              1c2b 001b  ld   $1b
              1c2c 002b  ld   $2b
              1c2d 001b  ld   $1b
              1c2e 0016  ld   $16
              1c2f 001a  ld   $1a
              1c30 001b  ld   $1b
              1c31 001b  ld   $1b
              1c32 002b  ld   $2b
              1c33 001b  ld   $1b
              1c34 0016  ld   $16
              1c35 002b  ld   $2b
              1c36 001b  ld   $1b
              1c37 001b  ld   $1b
              1c38 001a  ld   $1a
              1c39 001b  ld   $1b
              1c3a 001b  ld   $1b
              1c3b 001a  ld   $1a
              1c3c 0017  ld   $17
              1c3d 001a  ld   $1a
              1c3e 0017  ld   $17
              1c3f 001a  ld   $1a
              1c40 0017  ld   $17
              1c41 001a  ld   $1a
              1c42 0006  ld   $06
              1c43 0016  ld   $16
              1c44 001b  ld   $1b
              1c45 001a  ld   $1a
              1c46 001a  ld   $1a
              1c47 001b  ld   $1b
              1c48 0016  ld   $16
              1c49 001a  ld   $1a
              1c4a 001b  ld   $1b
              1c4b 001a  ld   $1a
              1c4c 001a  ld   $1a
              1c4d 001a  ld   $1a
              * 5 times
              1c50 0016  ld   $16
              1c51 0006  ld   $06
              1c52 001a  ld   $1a
              1c53 0016  ld   $16
              1c54 001a  ld   $1a
              1c55 0016  ld   $16
              1c56 001a  ld   $1a
              1c57 0006  ld   $06
              1c58 0016  ld   $16
              1c59 0006  ld   $06
              1c5a 0016  ld   $16
              1c5b 001a  ld   $1a
              1c5c 001a  ld   $1a
              1c5d 001a  ld   $1a
              1c5e 001a  ld   $1a
              1c5f 0016  ld   $16
              1c60 001a  ld   $1a
              1c61 0016  ld   $16
              1c62 001a  ld   $1a
              1c63 0016  ld   $16
              1c64 001a  ld   $1a
              1c65 0016  ld   $16
              1c66 0009  ld   $09
              1c67 0016  ld   $16
              1c68 0016  ld   $16
              1c69 0005  ld   $05
              1c6a 0016  ld   $16
              1c6b 0006  ld   $06
              1c6c 0015  ld   $15
              1c6d 0016  ld   $16
              1c6e 0005  ld   $05
              1c6f 0016  ld   $16
              1c70 0005  ld   $05
              1c71 0015  ld   $15
              1c72 0005  ld   $05
              1c73 0015  ld   $15
              1c74 0005  ld   $05
              1c75 0015  ld   $15
              1c76 0005  ld   $05
              1c77 0011  ld   $11
              1c78 0005  ld   $05
              1c79 0000  ld   $00
              1c7a 0005  ld   $05
              1c7b 0015  ld   $15
              1c7c 0001  ld   $01
              1c7d 0000  ld   $00
              1c7e 0000  ld   $00
              1c7f 0000  ld   $00
              * 35 times
              1ca0 0200  nop
              1ca1 0200  nop
              1ca2 0200  nop
              * 91 times
              1cfb fe00  bra  ac          ;Trampoline for page $1c00 lookups
              1cfc fcfd  bra  $1cfd
              1cfd 1403  ld   $03,y
              1cfe e07b  jmp  y,$7b
              1cff 1519  ld   [$19],y
jupiter22:    1d00 0000  ld   $00
              1d01 0000  ld   $00
              1d02 0000  ld   $00
              * 33 times
              1d21 0015  ld   $15
              1d22 001a  ld   $1a
              1d23 002b  ld   $2b
              1d24 001a  ld   $1a
              1d25 001b  ld   $1b
              1d26 002b  ld   $2b
              1d27 001a  ld   $1a
              1d28 001a  ld   $1a
              1d29 001b  ld   $1b
              1d2a 001a  ld   $1a
              1d2b 001b  ld   $1b
              1d2c 001a  ld   $1a
              1d2d 001b  ld   $1b
              1d2e 001a  ld   $1a
              1d2f 001b  ld   $1b
              1d30 002b  ld   $2b
              1d31 001a  ld   $1a
              1d32 001b  ld   $1b
              1d33 002b  ld   $2b
              1d34 001b  ld   $1b
              1d35 001a  ld   $1a
              1d36 002b  ld   $2b
              1d37 001a  ld   $1a
              1d38 001b  ld   $1b
              1d39 001b  ld   $1b
              1d3a 001a  ld   $1a
              1d3b 001b  ld   $1b
              1d3c 001b  ld   $1b
              1d3d 001b  ld   $1b
              1d3e 001a  ld   $1a
              1d3f 001b  ld   $1b
              1d40 001a  ld   $1a
              1d41 0016  ld   $16
              1d42 0006  ld   $06
              1d43 001a  ld   $1a
              1d44 001a  ld   $1a
              1d45 0017  ld   $17
              1d46 001b  ld   $1b
              1d47 001a  ld   $1a
              1d48 001a  ld   $1a
              1d49 002b  ld   $2b
              1d4a 001a  ld   $1a
              1d4b 0017  ld   $17
              1d4c 002b  ld   $2b
              1d4d 001a  ld   $1a
              1d4e 0017  ld   $17
              1d4f 001a  ld   $1a
              1d50 0006  ld   $06
              1d51 0005  ld   $05
              1d52 0016  ld   $16
              1d53 001a  ld   $1a
              1d54 0016  ld   $16
              1d55 001a  ld   $1a
              1d56 001a  ld   $1a
              1d57 0016  ld   $16
              1d58 0005  ld   $05
              1d59 0016  ld   $16
              1d5a 000a  ld   $0a
              1d5b 0016  ld   $16
              1d5c 0016  ld   $16
              1d5d 0006  ld   $06
              1d5e 0016  ld   $16
              1d5f 001a  ld   $1a
              1d60 0016  ld   $16
              1d61 0005  ld   $05
              1d62 0016  ld   $16
              1d63 001a  ld   $1a
              1d64 0006  ld   $06
              1d65 0016  ld   $16
              1d66 0016  ld   $16
              1d67 0015  ld   $15
              1d68 001a  ld   $1a
              1d69 0015  ld   $15
              1d6a 0006  ld   $06
              1d6b 0015  ld   $15
              1d6c 0005  ld   $05
              1d6d 0005  ld   $05
              1d6e 0016  ld   $16
              1d6f 0015  ld   $15
              1d70 0016  ld   $16
              1d71 0005  ld   $05
              1d72 0015  ld   $15
              1d73 0001  ld   $01
              1d74 0005  ld   $05
              1d75 0015  ld   $15
              1d76 0005  ld   $05
              1d77 0005  ld   $05
              1d78 0005  ld   $05
              1d79 0015  ld   $15
              1d7a 0001  ld   $01
              1d7b 0005  ld   $05
              1d7c 0015  ld   $15
              1d7d 0000  ld   $00
              1d7e 0000  ld   $00
              1d7f 0000  ld   $00
              * 35 times
              1da0 0200  nop
              1da1 0200  nop
              1da2 0200  nop
              * 91 times
              1dfb fe00  bra  ac          ;Trampoline for page $1d00 lookups
              1dfc fcfd  bra  $1dfd
              1dfd 1403  ld   $03,y
              1dfe e07b  jmp  y,$7b
              1dff 1519  ld   [$19],y
jupiter23:    1e00 0000  ld   $00
              1e01 0000  ld   $00
              1e02 0000  ld   $00
              * 32 times
              1e20 0010  ld   $10
              1e21 001b  ld   $1b
              1e22 002a  ld   $2a
              1e23 001b  ld   $1b
              1e24 002b  ld   $2b
              1e25 001b  ld   $1b
              1e26 001b  ld   $1b
              1e27 001a  ld   $1a
              1e28 001b  ld   $1b
              1e29 002b  ld   $2b
              1e2a 002b  ld   $2b
              1e2b 002f  ld   $2f
              1e2c 001b  ld   $1b
              1e2d 002b  ld   $2b
              1e2e 002b  ld   $2b
              1e2f 001b  ld   $1b
              1e30 002b  ld   $2b
              1e31 001b  ld   $1b
              1e32 002b  ld   $2b
              1e33 002b  ld   $2b
              1e34 001b  ld   $1b
              1e35 002b  ld   $2b
              1e36 002f  ld   $2f
              1e37 002b  ld   $2b
              1e38 002b  ld   $2b
              1e39 002b  ld   $2b
              * 5 times
              1e3c 002a  ld   $2a
              1e3d 002b  ld   $2b
              1e3e 002b  ld   $2b
              1e3f 002b  ld   $2b
              1e40 001b  ld   $1b
              1e41 001b  ld   $1b
              1e42 001a  ld   $1a
              1e43 001a  ld   $1a
              1e44 002b  ld   $2b
              1e45 001a  ld   $1a
              1e46 002b  ld   $2b
              1e47 001a  ld   $1a
              1e48 002b  ld   $2b
              1e49 001b  ld   $1b
              1e4a 001b  ld   $1b
              1e4b 002a  ld   $2a
              1e4c 001b  ld   $1b
              1e4d 001a  ld   $1a
              1e4e 002b  ld   $2b
              1e4f 001a  ld   $1a
              1e50 001a  ld   $1a
              1e51 0016  ld   $16
              1e52 001a  ld   $1a
              1e53 001a  ld   $1a
              1e54 001b  ld   $1b
              1e55 001a  ld   $1a
              1e56 001b  ld   $1b
              1e57 0016  ld   $16
              1e58 001a  ld   $1a
              1e59 0016  ld   $16
              1e5a 0016  ld   $16
              1e5b 001a  ld   $1a
              1e5c 001a  ld   $1a
              1e5d 001a  ld   $1a
              1e5e 001a  ld   $1a
              1e5f 0016  ld   $16
              1e60 001a  ld   $1a
              1e61 001a  ld   $1a
              1e62 0016  ld   $16
              1e63 001a  ld   $1a
              1e64 0016  ld   $16
              1e65 0019  ld   $19
              1e66 0016  ld   $16
              1e67 001a  ld   $1a
              1e68 0016  ld   $16
              1e69 0016  ld   $16
              1e6a 0016  ld   $16
              1e6b 0015  ld   $15
              1e6c 0016  ld   $16
              1e6d 0016  ld   $16
              1e6e 0005  ld   $05
              1e6f 0016  ld   $16
              1e70 0015  ld   $15
              1e71 0016  ld   $16
              1e72 0005  ld   $05
              1e73 0016  ld   $16
              1e74 0015  ld   $15
              1e75 0001  ld   $01
              1e76 0015  ld   $15
              1e77 0005  ld   $05
              1e78 0011  ld   $11
              1e79 0005  ld   $05
              1e7a 0015  ld   $15
              1e7b 0001  ld   $01
              1e7c 0015  ld   $15
              1e7d 0005  ld   $05
              1e7e 0011  ld   $11
              1e7f 0000  ld   $00
              1e80 0000  ld   $00
              1e81 0000  ld   $00
              * 33 times
              1ea0 0200  nop
              1ea1 0200  nop
              1ea2 0200  nop
              * 91 times
              1efb fe00  bra  ac          ;Trampoline for page $1e00 lookups
              1efc fcfd  bra  $1efd
              1efd 1403  ld   $03,y
              1efe e07b  jmp  y,$7b
              1eff 1519  ld   [$19],y
jupiter24:    1f00 0000  ld   $00
              1f01 0000  ld   $00
              1f02 0000  ld   $00
              * 31 times
              1f1f 0015  ld   $15
              1f20 001a  ld   $1a
              1f21 002b  ld   $2b
              1f22 002b  ld   $2b
              1f23 001b  ld   $1b
              1f24 002f  ld   $2f
              1f25 001a  ld   $1a
              1f26 001b  ld   $1b
              1f27 001a  ld   $1a
              1f28 002b  ld   $2b
              1f29 002f  ld   $2f
              1f2a 001b  ld   $1b
              1f2b 002b  ld   $2b
              1f2c 002f  ld   $2f
              1f2d 002b  ld   $2b
              1f2e 002f  ld   $2f
              1f2f 002b  ld   $2b
              1f30 002f  ld   $2f
              1f31 002b  ld   $2b
              1f32 002f  ld   $2f
              1f33 002f  ld   $2f
              1f34 002b  ld   $2b
              1f35 002f  ld   $2f
              1f36 001b  ld   $1b
              1f37 002f  ld   $2f
              1f38 002b  ld   $2b
              1f39 001f  ld   $1f
              1f3a 002b  ld   $2b
              1f3b 001f  ld   $1f
              1f3c 002b  ld   $2b
              1f3d 001f  ld   $1f
              1f3e 002b  ld   $2b
              1f3f 002f  ld   $2f
              1f40 001a  ld   $1a
              1f41 002b  ld   $2b
              1f42 001b  ld   $1b
              1f43 002b  ld   $2b
              1f44 001b  ld   $1b
              1f45 002b  ld   $2b
              1f46 002b  ld   $2b
              1f47 002b  ld   $2b
              1f48 001a  ld   $1a
              1f49 002b  ld   $2b
              1f4a 002a  ld   $2a
              1f4b 001b  ld   $1b
              1f4c 002a  ld   $2a
              1f4d 002b  ld   $2b
              1f4e 001a  ld   $1a
              1f4f 002b  ld   $2b
              1f50 001a  ld   $1a
              1f51 002b  ld   $2b
              1f52 001a  ld   $1a
              1f53 002b  ld   $2b
              1f54 001a  ld   $1a
              1f55 002a  ld   $2a
              1f56 001a  ld   $1a
              1f57 001a  ld   $1a
              1f58 001b  ld   $1b
              1f59 001a  ld   $1a
              1f5a 001a  ld   $1a
              1f5b 001a  ld   $1a
              1f5c 0016  ld   $16
              1f5d 001a  ld   $1a
              1f5e 0016  ld   $16
              1f5f 001a  ld   $1a
              1f60 0016  ld   $16
              1f61 001a  ld   $1a
              1f62 001a  ld   $1a
              1f63 0016  ld   $16
              1f64 001a  ld   $1a
              1f65 001a  ld   $1a
              1f66 0016  ld   $16
              1f67 001a  ld   $1a
              1f68 0015  ld   $15
              1f69 001a  ld   $1a
              1f6a 0019  ld   $19
              1f6b 0016  ld   $16
              1f6c 0015  ld   $15
              1f6d 001a  ld   $1a
              1f6e 0016  ld   $16
              1f6f 0015  ld   $15
              1f70 0016  ld   $16
              1f71 0015  ld   $15
              1f72 0016  ld   $16
              1f73 0015  ld   $15
              1f74 0005  ld   $05
              1f75 0015  ld   $15
              1f76 0005  ld   $05
              1f77 0015  ld   $15
              1f78 0005  ld   $05
              1f79 0005  ld   $05
              1f7a 0015  ld   $15
              1f7b 0005  ld   $05
              1f7c 0015  ld   $15
              1f7d 0005  ld   $05
              1f7e 0015  ld   $15
              1f7f 0000  ld   $00
              1f80 0000  ld   $00
              1f81 0000  ld   $00
              * 33 times
              1fa0 0200  nop
              1fa1 0200  nop
              1fa2 0200  nop
              * 91 times
              1ffb fe00  bra  ac          ;Trampoline for page $1f00 lookups
              1ffc fcfd  bra  $1ffd
              1ffd 1403  ld   $03,y
              1ffe e07b  jmp  y,$7b
              1fff 1519  ld   [$19],y
jupiter25:    2000 0000  ld   $00
              2001 0000  ld   $00
              2002 0000  ld   $00
              * 30 times
              201e 0001  ld   $01
              201f 001a  ld   $1a
              2020 002b  ld   $2b
              2021 001e  ld   $1e
              2022 002b  ld   $2b
              2023 002f  ld   $2f
              2024 002b  ld   $2b
              2025 002b  ld   $2b
              2026 002f  ld   $2f
              2027 002b  ld   $2b
              2028 002f  ld   $2f
              2029 002b  ld   $2b
              202a 002f  ld   $2f
              202b 002b  ld   $2b
              202c 002f  ld   $2f
              202d 002b  ld   $2b
              202e 002f  ld   $2f
              202f 002b  ld   $2b
              2030 002f  ld   $2f
              2031 002b  ld   $2b
              2032 002f  ld   $2f
              2033 002b  ld   $2b
              2034 002f  ld   $2f
              2035 002b  ld   $2b
              2036 002f  ld   $2f
              2037 002b  ld   $2b
              2038 002f  ld   $2f
              2039 002a  ld   $2a
              203a 002f  ld   $2f
              203b 002b  ld   $2b
              203c 002f  ld   $2f
              203d 002b  ld   $2b
              203e 002f  ld   $2f
              203f 002b  ld   $2b
              2040 002b  ld   $2b
              2041 002f  ld   $2f
              2042 002b  ld   $2b
              2043 002b  ld   $2b
              2044 002f  ld   $2f
              2045 002b  ld   $2b
              2046 001f  ld   $1f
              2047 002b  ld   $2b
              2048 002b  ld   $2b
              2049 001f  ld   $1f
              204a 002b  ld   $2b
              204b 002b  ld   $2b
              204c 002f  ld   $2f
              204d 001b  ld   $1b
              204e 002b  ld   $2b
              204f 002a  ld   $2a
              2050 002b  ld   $2b
              2051 001b  ld   $1b
              2052 002a  ld   $2a
              2053 001b  ld   $1b
              2054 002a  ld   $2a
              2055 001b  ld   $1b
              2056 002b  ld   $2b
              2057 002b  ld   $2b
              2058 002a  ld   $2a
              2059 001a  ld   $1a
              205a 002a  ld   $2a
              205b 002b  ld   $2b
              205c 001a  ld   $1a
              205d 002b  ld   $2b
              205e 001a  ld   $1a
              205f 001a  ld   $1a
              2060 002a  ld   $2a
              2061 0016  ld   $16
              2062 002a  ld   $2a
              2063 001a  ld   $1a
              2064 0016  ld   $16
              2065 001a  ld   $1a
              2066 001a  ld   $1a
              2067 0016  ld   $16
              2068 001a  ld   $1a
              2069 0016  ld   $16
              206a 001a  ld   $1a
              206b 0016  ld   $16
              206c 001a  ld   $1a
              206d 0016  ld   $16
              206e 0016  ld   $16
              206f 0019  ld   $19
              2070 0016  ld   $16
              2071 0019  ld   $19
              2072 0016  ld   $16
              2073 0015  ld   $15
              2074 0016  ld   $16
              2075 0015  ld   $15
              2076 0016  ld   $16
              2077 0015  ld   $15
              2078 0015  ld   $15
              2079 0001  ld   $01
              207a 0015  ld   $15
              207b 0015  ld   $15
              207c 0005  ld   $05
              207d 0011  ld   $11
              207e 0005  ld   $05
              207f 0015  ld   $15
              2080 0005  ld   $05
              2081 0000  ld   $00
              2082 0000  ld   $00
              2083 0000  ld   $00
              * 31 times
              20a0 0200  nop
              20a1 0200  nop
              20a2 0200  nop
              * 91 times
              20fb fe00  bra  ac          ;Trampoline for page $2000 lookups
              20fc fcfd  bra  $20fd
              20fd 1403  ld   $03,y
              20fe e07b  jmp  y,$7b
              20ff 1519  ld   [$19],y
jupiter26:    2100 0000  ld   $00
              2101 0000  ld   $00
              2102 0000  ld   $00
              * 30 times
              211e 002a  ld   $2a
              211f 002b  ld   $2b
              2120 002f  ld   $2f
              2121 002b  ld   $2b
              2122 002f  ld   $2f
              2123 002b  ld   $2b
              2124 002f  ld   $2f
              2125 002f  ld   $2f
              2126 002b  ld   $2b
              2127 002f  ld   $2f
              2128 002f  ld   $2f
              2129 002b  ld   $2b
              212a 002f  ld   $2f
              212b 002f  ld   $2f
              212c 002f  ld   $2f
              212d 002b  ld   $2b
              212e 002f  ld   $2f
              212f 002f  ld   $2f
              2130 002f  ld   $2f
              2131 002f  ld   $2f
              2132 002b  ld   $2b
              2133 002f  ld   $2f
              2134 002f  ld   $2f
              2135 002f  ld   $2f
              2136 002f  ld   $2f
              2137 002b  ld   $2b
              2138 002f  ld   $2f
              2139 002f  ld   $2f
              213a 002b  ld   $2b
              213b 002f  ld   $2f
              213c 002b  ld   $2b
              213d 002f  ld   $2f
              213e 002b  ld   $2b
              213f 002f  ld   $2f
              2140 002a  ld   $2a
              2141 002f  ld   $2f
              2142 002b  ld   $2b
              2143 002f  ld   $2f
              2144 002b  ld   $2b
              2145 002a  ld   $2a
              2146 002f  ld   $2f
              2147 002b  ld   $2b
              2148 002a  ld   $2a
              2149 002f  ld   $2f
              214a 002a  ld   $2a
              214b 002f  ld   $2f
              214c 002a  ld   $2a
              214d 002b  ld   $2b
              214e 002f  ld   $2f
              214f 001b  ld   $1b
              2150 002e  ld   $2e
              2151 002b  ld   $2b
              2152 002a  ld   $2a
              2153 001b  ld   $1b
              2154 002a  ld   $2a
              2155 002a  ld   $2a
              2156 001a  ld   $1a
              2157 002a  ld   $2a
              2158 001b  ld   $1b
              2159 002a  ld   $2a
              215a 001b  ld   $1b
              215b 001a  ld   $1a
              215c 0026  ld   $26
              215d 001a  ld   $1a
              215e 001a  ld   $1a
              215f 0026  ld   $26
              2160 001a  ld   $1a
              2161 001a  ld   $1a
              2162 001a  ld   $1a
              2163 0016  ld   $16
              2164 002a  ld   $2a
              2165 001a  ld   $1a
              2166 0016  ld   $16
              2167 001a  ld   $1a
              2168 001a  ld   $1a
              2169 0015  ld   $15
              216a 001a  ld   $1a
              216b 0015  ld   $15
              216c 001a  ld   $1a
              216d 0015  ld   $15
              216e 001a  ld   $1a
              216f 0016  ld   $16
              2170 0015  ld   $15
              2171 0016  ld   $16
              2172 0005  ld   $05
              2173 0016  ld   $16
              2174 0015  ld   $15
              2175 0005  ld   $05
              2176 0015  ld   $15
              2177 0005  ld   $05
              2178 0015  ld   $15
              2179 0015  ld   $15
              217a 0005  ld   $05
              217b 0015  ld   $15
              217c 0015  ld   $15
              217d 0005  ld   $05
              217e 0015  ld   $15
              217f 0015  ld   $15
              2180 0011  ld   $11
              2181 0000  ld   $00
              2182 0000  ld   $00
              2183 0000  ld   $00
              * 31 times
              21a0 0200  nop
              21a1 0200  nop
              21a2 0200  nop
              * 91 times
              21fb fe00  bra  ac          ;Trampoline for page $2100 lookups
              21fc fcfd  bra  $21fd
              21fd 1403  ld   $03,y
              21fe e07b  jmp  y,$7b
              21ff 1519  ld   [$19],y
jupiter27:    2200 0000  ld   $00
              2201 0000  ld   $00
              2202 0000  ld   $00
              * 29 times
              221d 0016  ld   $16
              221e 002f  ld   $2f
              221f 002b  ld   $2b
              2220 002f  ld   $2f
              2221 002f  ld   $2f
              2222 002b  ld   $2b
              2223 002f  ld   $2f
              2224 002f  ld   $2f
              2225 002b  ld   $2b
              2226 002f  ld   $2f
              2227 002f  ld   $2f
              2228 002b  ld   $2b
              2229 002f  ld   $2f
              222a 002f  ld   $2f
              222b 002b  ld   $2b
              222c 002f  ld   $2f
              222d 002f  ld   $2f
              222e 002f  ld   $2f
              222f 002b  ld   $2b
              2230 002f  ld   $2f
              2231 002b  ld   $2b
              2232 002f  ld   $2f
              2233 002e  ld   $2e
              2234 002f  ld   $2f
              2235 002b  ld   $2b
              2236 002f  ld   $2f
              2237 002f  ld   $2f
              2238 002b  ld   $2b
              2239 002f  ld   $2f
              223a 002b  ld   $2b
              223b 002f  ld   $2f
              223c 002f  ld   $2f
              223d 002b  ld   $2b
              223e 002f  ld   $2f
              223f 002f  ld   $2f
              2240 002f  ld   $2f
              2241 002b  ld   $2b
              2242 002f  ld   $2f
              2243 002f  ld   $2f
              2244 002b  ld   $2b
              2245 002f  ld   $2f
              2246 002b  ld   $2b
              2247 002b  ld   $2b
              2248 002f  ld   $2f
              2249 002b  ld   $2b
              224a 002b  ld   $2b
              224b 002b  ld   $2b
              224c 001b  ld   $1b
              224d 002f  ld   $2f
              224e 002a  ld   $2a
              224f 002f  ld   $2f
              2250 002b  ld   $2b
              2251 001a  ld   $1a
              2252 002f  ld   $2f
              2253 002b  ld   $2b
              2254 002a  ld   $2a
              2255 001b  ld   $1b
              2256 002b  ld   $2b
              2257 001b  ld   $1b
              2258 002a  ld   $2a
              2259 001b  ld   $1b
              225a 002a  ld   $2a
              225b 002b  ld   $2b
              225c 001a  ld   $1a
              225d 001a  ld   $1a
              225e 002b  ld   $2b
              225f 001a  ld   $1a
              2260 002b  ld   $2b
              2261 001a  ld   $1a
              2262 002a  ld   $2a
              2263 001a  ld   $1a
              2264 001a  ld   $1a
              2265 0026  ld   $26
              2266 001a  ld   $1a
              2267 001a  ld   $1a
              2268 0026  ld   $26
              2269 001a  ld   $1a
              226a 0016  ld   $16
              226b 001a  ld   $1a
              226c 0016  ld   $16
              226d 001a  ld   $1a
              226e 0016  ld   $16
              226f 001a  ld   $1a
              2270 0016  ld   $16
              2271 0019  ld   $19
              2272 0016  ld   $16
              2273 0019  ld   $19
              2274 0016  ld   $16
              2275 0015  ld   $15
              2276 0016  ld   $16
              2277 0015  ld   $15
              2278 0016  ld   $16
              2279 0015  ld   $15
              227a 0005  ld   $05
              227b 0015  ld   $15
              227c 0001  ld   $01
              227d 0015  ld   $15
              227e 0005  ld   $05
              227f 0015  ld   $15
              2280 0005  ld   $05
              2281 0015  ld   $15
              2282 0000  ld   $00
              2283 0000  ld   $00
              2284 0000  ld   $00
              * 30 times
              22a0 0200  nop
              22a1 0200  nop
              22a2 0200  nop
              * 91 times
              22fb fe00  bra  ac          ;Trampoline for page $2200 lookups
              22fc fcfd  bra  $22fd
              22fd 1403  ld   $03,y
              22fe e07b  jmp  y,$7b
              22ff 1519  ld   [$19],y
jupiter28:    2300 0000  ld   $00
              2301 0000  ld   $00
              2302 0000  ld   $00
              * 28 times
              231c 0015  ld   $15
              231d 002f  ld   $2f
              231e 002f  ld   $2f
              231f 002f  ld   $2f
              2320 002b  ld   $2b
              2321 002f  ld   $2f
              2322 002f  ld   $2f
              2323 002f  ld   $2f
              2324 002b  ld   $2b
              2325 002f  ld   $2f
              2326 002f  ld   $2f
              2327 003f  ld   $3f
              2328 002f  ld   $2f
              2329 002f  ld   $2f
              232a 003f  ld   $3f
              232b 002f  ld   $2f
              232c 002f  ld   $2f
              232d 002b  ld   $2b
              232e 002f  ld   $2f
              232f 002f  ld   $2f
              2330 003f  ld   $3f
              2331 002f  ld   $2f
              2332 002f  ld   $2f
              2333 002b  ld   $2b
              2334 002f  ld   $2f
              2335 002f  ld   $2f
              2336 002f  ld   $2f
              * 5 times
              2339 003f  ld   $3f
              233a 002f  ld   $2f
              233b 002b  ld   $2b
              233c 002f  ld   $2f
              233d 002f  ld   $2f
              233e 002b  ld   $2b
              233f 002f  ld   $2f
              2340 002b  ld   $2b
              2341 002f  ld   $2f
              2342 002b  ld   $2b
              2343 002f  ld   $2f
              2344 002b  ld   $2b
              2345 002f  ld   $2f
              2346 002b  ld   $2b
              2347 002f  ld   $2f
              2348 002b  ld   $2b
              2349 002f  ld   $2f
              234a 002f  ld   $2f
              234b 002a  ld   $2a
              234c 002f  ld   $2f
              234d 002b  ld   $2b
              234e 002b  ld   $2b
              234f 002a  ld   $2a
              2350 002b  ld   $2b
              2351 002f  ld   $2f
              2352 002a  ld   $2a
              2353 001b  ld   $1b
              2354 002e  ld   $2e
              2355 002b  ld   $2b
              2356 002a  ld   $2a
              2357 002a  ld   $2a
              2358 001b  ld   $1b
              2359 002a  ld   $2a
              235a 001a  ld   $1a
              235b 002a  ld   $2a
              235c 001a  ld   $1a
              235d 0027  ld   $27
              235e 001a  ld   $1a
              235f 001a  ld   $1a
              2360 001a  ld   $1a
              2361 0026  ld   $26
              2362 001a  ld   $1a
              2363 0026  ld   $26
              2364 001a  ld   $1a
              2365 001a  ld   $1a
              2366 0016  ld   $16
              2367 002a  ld   $2a
              2368 001a  ld   $1a
              2369 0016  ld   $16
              236a 0029  ld   $29
              236b 0016  ld   $16
              236c 0029  ld   $29
              236d 0016  ld   $16
              236e 0019  ld   $19
              236f 0016  ld   $16
              2370 0015  ld   $15
              2371 0016  ld   $16
              2372 0015  ld   $15
              2373 0016  ld   $16
              2374 0015  ld   $15
              2375 0016  ld   $16
              2376 0015  ld   $15
              2377 0015  ld   $15
              2378 0005  ld   $05
              2379 0015  ld   $15
              237a 0015  ld   $15
              237b 0015  ld   $15
              237c 0015  ld   $15
              237d 0005  ld   $05
              237e 0015  ld   $15
              237f 0015  ld   $15
              2380 0015  ld   $15
              2381 0015  ld   $15
              2382 0005  ld   $05
              2383 0000  ld   $00
              2384 0000  ld   $00
              2385 0000  ld   $00
              * 29 times
              23a0 0200  nop
              23a1 0200  nop
              23a2 0200  nop
              * 91 times
              23fb fe00  bra  ac          ;Trampoline for page $2300 lookups
              23fc fcfd  bra  $23fd
              23fd 1403  ld   $03,y
              23fe e07b  jmp  y,$7b
              23ff 1519  ld   [$19],y
jupiter29:    2400 0000  ld   $00
              2401 0000  ld   $00
              2402 0000  ld   $00
              * 27 times
              241b 0001  ld   $01
              241c 002a  ld   $2a
              241d 002b  ld   $2b
              241e 002f  ld   $2f
              241f 002b  ld   $2b
              2420 003f  ld   $3f
              2421 002f  ld   $2f
              2422 003b  ld   $3b
              2423 002f  ld   $2f
              2424 003f  ld   $3f
              2425 002f  ld   $2f
              2426 003b  ld   $3b
              2427 002f  ld   $2f
              2428 002b  ld   $2b
              2429 003f  ld   $3f
              242a 002b  ld   $2b
              242b 002f  ld   $2f
              242c 003f  ld   $3f
              242d 002f  ld   $2f
              242e 003b  ld   $3b
              242f 002f  ld   $2f
              2430 002f  ld   $2f
              2431 003b  ld   $3b
              2432 002f  ld   $2f
              2433 002f  ld   $2f
              2434 003b  ld   $3b
              2435 002f  ld   $2f
              2436 002b  ld   $2b
              2437 003f  ld   $3f
              2438 002b  ld   $2b
              2439 002f  ld   $2f
              243a 002b  ld   $2b
              243b 002f  ld   $2f
              243c 002b  ld   $2b
              243d 002f  ld   $2f
              243e 003b  ld   $3b
              243f 002f  ld   $2f
              2440 002f  ld   $2f
              2441 002b  ld   $2b
              2442 002f  ld   $2f
              2443 002b  ld   $2b
              2444 002f  ld   $2f
              2445 002b  ld   $2b
              2446 002f  ld   $2f
              2447 002b  ld   $2b
              2448 002f  ld   $2f
              2449 002a  ld   $2a
              244a 002b  ld   $2b
              244b 002f  ld   $2f
              244c 002b  ld   $2b
              244d 002a  ld   $2a
              244e 002f  ld   $2f
              244f 002b  ld   $2b
              2450 002e  ld   $2e
              2451 002b  ld   $2b
              2452 002a  ld   $2a
              2453 002b  ld   $2b
              2454 002a  ld   $2a
              2455 001b  ld   $1b
              2456 002a  ld   $2a
              2457 001b  ld   $1b
              2458 002a  ld   $2a
              2459 002a  ld   $2a
              245a 002b  ld   $2b
              245b 001a  ld   $1a
              245c 002b  ld   $2b
              245d 001a  ld   $1a
              245e 002a  ld   $2a
              245f 001b  ld   $1b
              2460 002a  ld   $2a
              2461 001a  ld   $1a
              2462 002b  ld   $2b
              2463 001a  ld   $1a
              2464 001a  ld   $1a
              2465 0026  ld   $26
              2466 001a  ld   $1a
              2467 001a  ld   $1a
              2468 0026  ld   $26
              2469 001a  ld   $1a
              246a 0016  ld   $16
              246b 001a  ld   $1a
              246c 001a  ld   $1a
              246d 0016  ld   $16
              246e 002a  ld   $2a
              246f 0016  ld   $16
              2470 001a  ld   $1a
              2471 0019  ld   $19
              2472 0016  ld   $16
              2473 001a  ld   $1a
              2474 0015  ld   $15
              2475 001a  ld   $1a
              2476 0015  ld   $15
              2477 0016  ld   $16
              2478 0015  ld   $15
              2479 0016  ld   $16
              247a 0015  ld   $15
              247b 0006  ld   $06
              247c 0015  ld   $15
              247d 0016  ld   $16
              247e 0015  ld   $15
              247f 0015  ld   $15
              2480 0005  ld   $05
              2481 0015  ld   $15
              2482 0015  ld   $15
              2483 0001  ld   $01
              2484 0000  ld   $00
              2485 0000  ld   $00
              2486 0000  ld   $00
              * 28 times
              24a0 0200  nop
              24a1 0200  nop
              24a2 0200  nop
              * 91 times
              24fb fe00  bra  ac          ;Trampoline for page $2400 lookups
              24fc fcfd  bra  $24fd
              24fd 1403  ld   $03,y
              24fe e07b  jmp  y,$7b
              24ff 1519  ld   [$19],y
jupiter30:    2500 0000  ld   $00
              2501 0000  ld   $00
              2502 0000  ld   $00
              * 27 times
              251b 001a  ld   $1a
              251c 003f  ld   $3f
              251d 002f  ld   $2f
              251e 002e  ld   $2e
              251f 002f  ld   $2f
              2520 002f  ld   $2f
              2521 002b  ld   $2b
              2522 002f  ld   $2f
              2523 002f  ld   $2f
              2524 002f  ld   $2f
              * 5 times
              2527 003f  ld   $3f
              2528 002f  ld   $2f
              2529 002f  ld   $2f
              252a 002f  ld   $2f
              252b 003b  ld   $3b
              252c 002f  ld   $2f
              252d 002f  ld   $2f
              252e 003e  ld   $3e
              252f 002b  ld   $2b
              2530 002f  ld   $2f
              2531 002f  ld   $2f
              2532 002f  ld   $2f
              * 5 times
              2535 002b  ld   $2b
              2536 002f  ld   $2f
              2537 002f  ld   $2f
              2538 002b  ld   $2b
              2539 002f  ld   $2f
              253a 002f  ld   $2f
              253b 002b  ld   $2b
              253c 002f  ld   $2f
              253d 002b  ld   $2b
              253e 002f  ld   $2f
              253f 002b  ld   $2b
              2540 002f  ld   $2f
              2541 002b  ld   $2b
              2542 002f  ld   $2f
              2543 002b  ld   $2b
              2544 002e  ld   $2e
              2545 002b  ld   $2b
              2546 002f  ld   $2f
              2547 002a  ld   $2a
              2548 002f  ld   $2f
              2549 002b  ld   $2b
              254a 002f  ld   $2f
              254b 002b  ld   $2b
              254c 002e  ld   $2e
              254d 002b  ld   $2b
              254e 001f  ld   $1f
              254f 002a  ld   $2a
              2550 002b  ld   $2b
              2551 002b  ld   $2b
              2552 001a  ld   $1a
              2553 002b  ld   $2b
              2554 002a  ld   $2a
              2555 002b  ld   $2b
              2556 001a  ld   $1a
              2557 002f  ld   $2f
              2558 002a  ld   $2a
              2559 001b  ld   $1b
              255a 002a  ld   $2a
              255b 001a  ld   $1a
              255c 002a  ld   $2a
              255d 001a  ld   $1a
              255e 0026  ld   $26
              255f 001a  ld   $1a
              2560 001a  ld   $1a
              2561 002b  ld   $2b
              2562 001a  ld   $1a
              2563 002a  ld   $2a
              2564 001a  ld   $1a
              2565 002a  ld   $2a
              2566 0016  ld   $16
              2567 002a  ld   $2a
              2568 0016  ld   $16
              2569 001a  ld   $1a
              256a 0029  ld   $29
              256b 0016  ld   $16
              256c 001a  ld   $1a
              256d 0015  ld   $15
              256e 001a  ld   $1a
              256f 0015  ld   $15
              2570 0016  ld   $16
              2571 0016  ld   $16
              2572 0015  ld   $15
              2573 0016  ld   $16
              2574 0015  ld   $15
              2575 0016  ld   $16
              2576 0015  ld   $15
              2577 0015  ld   $15
              2578 0016  ld   $16
              2579 0005  ld   $05
              257a 0015  ld   $15
              257b 0015  ld   $15
              257c 0015  ld   $15
              257d 0005  ld   $05
              257e 0015  ld   $15
              257f 0015  ld   $15
              2580 0015  ld   $15
              2581 0005  ld   $05
              2582 0015  ld   $15
              2583 0015  ld   $15
              2584 0000  ld   $00
              2585 0000  ld   $00
              2586 0000  ld   $00
              * 28 times
              25a0 0200  nop
              25a1 0200  nop
              25a2 0200  nop
              * 91 times
              25fb fe00  bra  ac          ;Trampoline for page $2500 lookups
              25fc fcfd  bra  $25fd
              25fd 1403  ld   $03,y
              25fe e07b  jmp  y,$7b
              25ff 1519  ld   [$19],y
jupiter31:    2600 0000  ld   $00
              2601 0000  ld   $00
              2602 0000  ld   $00
              * 26 times
              261a 0015  ld   $15
              261b 002f  ld   $2f
              261c 002b  ld   $2b
              261d 002b  ld   $2b
              261e 002b  ld   $2b
              261f 002f  ld   $2f
              2620 003b  ld   $3b
              2621 002f  ld   $2f
              2622 002f  ld   $2f
              2623 003b  ld   $3b
              2624 002f  ld   $2f
              2625 003b  ld   $3b
              2626 002b  ld   $2b
              2627 002f  ld   $2f
              2628 002b  ld   $2b
              2629 003b  ld   $3b
              262a 002e  ld   $2e
              262b 002f  ld   $2f
              262c 003b  ld   $3b
              262d 002f  ld   $2f
              262e 002b  ld   $2b
              262f 002f  ld   $2f
              2630 002f  ld   $2f
              2631 002b  ld   $2b
              2632 002f  ld   $2f
              2633 002b  ld   $2b
              2634 002b  ld   $2b
              2635 002f  ld   $2f
              2636 002b  ld   $2b
              2637 002f  ld   $2f
              2638 002b  ld   $2b
              2639 002e  ld   $2e
              263a 002b  ld   $2b
              263b 002f  ld   $2f
              263c 002b  ld   $2b
              263d 002f  ld   $2f
              263e 002b  ld   $2b
              263f 002f  ld   $2f
              2640 002b  ld   $2b
              2641 002f  ld   $2f
              2642 002a  ld   $2a
              2643 002f  ld   $2f
              2644 002b  ld   $2b
              2645 002b  ld   $2b
              2646 002b  ld   $2b
              2647 002f  ld   $2f
              2648 002b  ld   $2b
              2649 002b  ld   $2b
              264a 002b  ld   $2b
              264b 002e  ld   $2e
              264c 001b  ld   $1b
              264d 002b  ld   $2b
              264e 002a  ld   $2a
              264f 002b  ld   $2b
              2650 002b  ld   $2b
              2651 002a  ld   $2a
              2652 002f  ld   $2f
              2653 002a  ld   $2a
              2654 001b  ld   $1b
              2655 002a  ld   $2a
              2656 002a  ld   $2a
              2657 001b  ld   $1b
              2658 002a  ld   $2a
              2659 001a  ld   $1a
              265a 002a  ld   $2a
              265b 001a  ld   $1a
              265c 0027  ld   $27
              265d 001a  ld   $1a
              265e 002b  ld   $2b
              265f 001a  ld   $1a
              2660 002a  ld   $2a
              2661 0016  ld   $16
              2662 001a  ld   $1a
              2663 0016  ld   $16
              2664 002a  ld   $2a
              2665 0016  ld   $16
              2666 001a  ld   $1a
              2667 0016  ld   $16
              2668 002a  ld   $2a
              2669 0016  ld   $16
              266a 001a  ld   $1a
              266b 0016  ld   $16
              266c 002a  ld   $2a
              266d 0015  ld   $15
              266e 001a  ld   $1a
              266f 0016  ld   $16
              2670 0019  ld   $19
              2671 0016  ld   $16
              2672 0019  ld   $19
              2673 0016  ld   $16
              2674 0019  ld   $19
              2675 0016  ld   $16
              2676 0005  ld   $05
              2677 0016  ld   $16
              2678 0015  ld   $15
              2679 0015  ld   $15
              267a 0015  ld   $15
              267b 0015  ld   $15
              267c 0005  ld   $05
              267d 0015  ld   $15
              267e 0012  ld   $12
              267f 0015  ld   $15
              2680 0005  ld   $05
              2681 0015  ld   $15
              2682 0015  ld   $15
              2683 0015  ld   $15
              2684 0015  ld   $15
              2685 0000  ld   $00
              2686 0000  ld   $00
              2687 0000  ld   $00
              * 27 times
              26a0 0200  nop
              26a1 0200  nop
              26a2 0200  nop
              * 91 times
              26fb fe00  bra  ac          ;Trampoline for page $2600 lookups
              26fc fcfd  bra  $26fd
              26fd 1403  ld   $03,y
              26fe e07b  jmp  y,$7b
              26ff 1519  ld   [$19],y
jupiter32:    2700 0000  ld   $00
              2701 0000  ld   $00
              2702 0000  ld   $00
              * 26 times
              271a 002a  ld   $2a
              271b 002b  ld   $2b
              271c 002f  ld   $2f
              271d 002b  ld   $2b
              271e 002f  ld   $2f
              271f 002b  ld   $2b
              2720 002f  ld   $2f
              2721 002b  ld   $2b
              2722 002b  ld   $2b
              2723 002f  ld   $2f
              2724 002b  ld   $2b
              2725 002f  ld   $2f
              2726 002b  ld   $2b
              2727 002f  ld   $2f
              2728 002b  ld   $2b
              2729 002f  ld   $2f
              272a 002b  ld   $2b
              272b 002b  ld   $2b
              272c 002f  ld   $2f
              272d 002b  ld   $2b
              272e 002f  ld   $2f
              272f 002b  ld   $2b
              2730 002b  ld   $2b
              2731 002f  ld   $2f
              2732 002b  ld   $2b
              2733 002f  ld   $2f
              2734 002f  ld   $2f
              2735 002b  ld   $2b
              2736 002f  ld   $2f
              2737 002b  ld   $2b
              2738 002f  ld   $2f
              2739 002b  ld   $2b
              273a 002f  ld   $2f
              273b 002b  ld   $2b
              273c 002a  ld   $2a
              273d 002f  ld   $2f
              273e 002b  ld   $2b
              273f 002a  ld   $2a
              2740 002b  ld   $2b
              2741 002f  ld   $2f
              2742 002b  ld   $2b
              2743 002b  ld   $2b
              2744 002a  ld   $2a
              2745 002f  ld   $2f
              2746 001a  ld   $1a
              2747 002b  ld   $2b
              2748 001a  ld   $1a
              2749 002a  ld   $2a
              274a 001b  ld   $1b
              274b 002b  ld   $2b
              274c 002a  ld   $2a
              274d 001f  ld   $1f
              274e 002a  ld   $2a
              274f 001b  ld   $1b
              2750 002a  ld   $2a
              2751 001a  ld   $1a
              2752 002b  ld   $2b
              2753 001a  ld   $1a
              2754 002b  ld   $2b
              2755 002a  ld   $2a
              2756 001b  ld   $1b
              2757 002a  ld   $2a
              2758 002b  ld   $2b
              2759 001a  ld   $1a
              275a 002b  ld   $2b
              275b 001a  ld   $1a
              275c 002a  ld   $2a
              275d 001a  ld   $1a
              275e 0016  ld   $16
              275f 002a  ld   $2a
              2760 0016  ld   $16
              2761 002a  ld   $2a
              2762 001a  ld   $1a
              2763 0026  ld   $26
              2764 001a  ld   $1a
              2765 0016  ld   $16
              2766 001a  ld   $1a
              2767 0016  ld   $16
              2768 0019  ld   $19
              2769 0016  ld   $16
              276a 001a  ld   $1a
              276b 0015  ld   $15
              276c 0016  ld   $16
              276d 001a  ld   $1a
              276e 0015  ld   $15
              276f 0016  ld   $16
              2770 0015  ld   $15
              2771 0016  ld   $16
              2772 0015  ld   $15
              2773 0015  ld   $15
              2774 0016  ld   $16
              2775 0015  ld   $15
              2776 0015  ld   $15
              2777 0015  ld   $15
              2778 0015  ld   $15
              2779 0006  ld   $06
              277a 0015  ld   $15
              277b 0015  ld   $15
              277c 0015  ld   $15
              277d 0005  ld   $05
              277e 0015  ld   $15
              277f 0005  ld   $05
              2780 0015  ld   $15
              2781 0011  ld   $11
              2782 0005  ld   $05
              2783 0015  ld   $15
              2784 0005  ld   $05
              2785 0000  ld   $00
              2786 0000  ld   $00
              2787 0000  ld   $00
              * 27 times
              27a0 0200  nop
              27a1 0200  nop
              27a2 0200  nop
              * 91 times
              27fb fe00  bra  ac          ;Trampoline for page $2700 lookups
              27fc fcfd  bra  $27fd
              27fd 1403  ld   $03,y
              27fe e07b  jmp  y,$7b
              27ff 1519  ld   [$19],y
jupiter33:    2800 0000  ld   $00
              2801 0000  ld   $00
              2802 0000  ld   $00
              * 25 times
              2819 0015  ld   $15
              281a 002f  ld   $2f
              281b 002b  ld   $2b
              281c 002f  ld   $2f
              281d 002b  ld   $2b
              281e 002f  ld   $2f
              281f 002b  ld   $2b
              2820 002f  ld   $2f
              2821 002a  ld   $2a
              2822 002f  ld   $2f
              2823 002b  ld   $2b
              2824 002f  ld   $2f
              2825 002b  ld   $2b
              2826 002e  ld   $2e
              2827 002b  ld   $2b
              2828 002f  ld   $2f
              2829 002b  ld   $2b
              282a 002f  ld   $2f
              282b 002f  ld   $2f
              282c 002b  ld   $2b
              282d 002f  ld   $2f
              282e 002b  ld   $2b
              282f 002f  ld   $2f
              2830 002f  ld   $2f
              2831 002a  ld   $2a
              2832 002f  ld   $2f
              2833 002b  ld   $2b
              2834 002b  ld   $2b
              2835 002e  ld   $2e
              2836 002b  ld   $2b
              2837 002f  ld   $2f
              2838 002b  ld   $2b
              2839 002b  ld   $2b
              283a 002e  ld   $2e
              283b 002b  ld   $2b
              283c 002f  ld   $2f
              283d 002b  ld   $2b
              283e 002f  ld   $2f
              283f 002b  ld   $2b
              2840 002f  ld   $2f
              2841 002b  ld   $2b
              2842 002a  ld   $2a
              2843 002f  ld   $2f
              2844 002b  ld   $2b
              2845 001b  ld   $1b
              2846 002b  ld   $2b
              2847 002b  ld   $2b
              2848 002b  ld   $2b
              2849 002b  ld   $2b
              284a 002e  ld   $2e
              284b 001b  ld   $1b
              284c 002a  ld   $2a
              284d 002b  ld   $2b
              284e 002a  ld   $2a
              284f 002b  ld   $2b
              2850 002e  ld   $2e
              2851 002b  ld   $2b
              2852 002b  ld   $2b
              2853 002a  ld   $2a
              2854 002b  ld   $2b
              2855 002a  ld   $2a
              2856 002b  ld   $2b
              2857 001a  ld   $1a
              2858 002a  ld   $2a
              2859 002b  ld   $2b
              285a 001a  ld   $1a
              285b 002a  ld   $2a
              285c 001b  ld   $1b
              285d 0026  ld   $26
              285e 001a  ld   $1a
              285f 002a  ld   $2a
              2860 001a  ld   $1a
              2861 001a  ld   $1a
              2862 0026  ld   $26
              2863 001a  ld   $1a
              2864 001a  ld   $1a
              2865 002a  ld   $2a
              2866 0016  ld   $16
              2867 002a  ld   $2a
              2868 001a  ld   $1a
              2869 0025  ld   $25
              286a 001a  ld   $1a
              286b 0026  ld   $26
              286c 001a  ld   $1a
              286d 0015  ld   $15
              286e 0016  ld   $16
              286f 001a  ld   $1a
              2870 0015  ld   $15
              2871 001a  ld   $1a
              2872 0015  ld   $15
              2873 001a  ld   $1a
              2874 0015  ld   $15
              2875 0016  ld   $16
              2876 0015  ld   $15
              2877 0016  ld   $16
              2878 0015  ld   $15
              2879 0015  ld   $15
              287a 0015  ld   $15
              287b 0001  ld   $01
              287c 0015  ld   $15
              287d 0015  ld   $15
              287e 0001  ld   $01
              287f 0015  ld   $15
              2880 0015  ld   $15
              2881 0005  ld   $05
              2882 0015  ld   $15
              2883 0005  ld   $05
              2884 0015  ld   $15
              2885 0015  ld   $15
              2886 0000  ld   $00
              2887 0000  ld   $00
              2888 0000  ld   $00
              * 26 times
              28a0 0200  nop
              28a1 0200  nop
              28a2 0200  nop
              * 91 times
              28fb fe00  bra  ac          ;Trampoline for page $2800 lookups
              28fc fcfd  bra  $28fd
              28fd 1403  ld   $03,y
              28fe e07b  jmp  y,$7b
              28ff 1519  ld   [$19],y
jupiter34:    2900 0000  ld   $00
              2901 0000  ld   $00
              2902 0000  ld   $00
              * 24 times
              2918 0005  ld   $05
              2919 002b  ld   $2b
              291a 002b  ld   $2b
              291b 002f  ld   $2f
              291c 002b  ld   $2b
              291d 002f  ld   $2f
              291e 002a  ld   $2a
              291f 002f  ld   $2f
              2920 003f  ld   $3f
              2921 002b  ld   $2b
              2922 003f  ld   $3f
              2923 002f  ld   $2f
              2924 003b  ld   $3b
              2925 002f  ld   $2f
              2926 002f  ld   $2f
              2927 003f  ld   $3f
              2928 002b  ld   $2b
              2929 002f  ld   $2f
              292a 002f  ld   $2f
              292b 002b  ld   $2b
              292c 002f  ld   $2f
              292d 003f  ld   $3f
              292e 002f  ld   $2f
              292f 002b  ld   $2b
              2930 003f  ld   $3f
              2931 002f  ld   $2f
              2932 002f  ld   $2f
              2933 003f  ld   $3f
              2934 002f  ld   $2f
              2935 002b  ld   $2b
              2936 003f  ld   $3f
              2937 002f  ld   $2f
              2938 002b  ld   $2b
              2939 002f  ld   $2f
              293a 002b  ld   $2b
              293b 002f  ld   $2f
              293c 002b  ld   $2b
              293d 002f  ld   $2f
              293e 002b  ld   $2b
              293f 002f  ld   $2f
              2940 002a  ld   $2a
              2941 002f  ld   $2f
              2942 002f  ld   $2f
              2943 002b  ld   $2b
              2944 002f  ld   $2f
              2945 002f  ld   $2f
              2946 002f  ld   $2f
              2947 002a  ld   $2a
              2948 002f  ld   $2f
              2949 002b  ld   $2b
              294a 002f  ld   $2f
              294b 002b  ld   $2b
              294c 002f  ld   $2f
              294d 002a  ld   $2a
              294e 001f  ld   $1f
              294f 002b  ld   $2b
              2950 002b  ld   $2b
              2951 002a  ld   $2a
              2952 002f  ld   $2f
              2953 002b  ld   $2b
              2954 002e  ld   $2e
              2955 001b  ld   $1b
              2956 002b  ld   $2b
              2957 002a  ld   $2a
              2958 002b  ld   $2b
              2959 002a  ld   $2a
              295a 002b  ld   $2b
              295b 002a  ld   $2a
              295c 001b  ld   $1b
              295d 002a  ld   $2a
              295e 001b  ld   $1b
              295f 002a  ld   $2a
              2960 002b  ld   $2b
              2961 001a  ld   $1a
              2962 002a  ld   $2a
              2963 001a  ld   $1a
              2964 0026  ld   $26
              2965 001a  ld   $1a
              2966 002a  ld   $2a
              2967 0016  ld   $16
              2968 001a  ld   $1a
              2969 0016  ld   $16
              296a 001a  ld   $1a
              296b 001a  ld   $1a
              296c 0015  ld   $15
              296d 002a  ld   $2a
              296e 001a  ld   $1a
              296f 0025  ld   $25
              2970 001a  ld   $1a
              2971 0016  ld   $16
              2972 0015  ld   $15
              2973 0016  ld   $16
              2974 0015  ld   $15
              2975 0019  ld   $19
              2976 0006  ld   $06
              2977 0015  ld   $15
              2978 0015  ld   $15
              2979 0005  ld   $05
              297a 0015  ld   $15
              297b 0015  ld   $15
              297c 0005  ld   $05
              297d 0015  ld   $15
              297e 0015  ld   $15
              297f 0015  ld   $15
              2980 0005  ld   $05
              2981 0015  ld   $15
              2982 0011  ld   $11
              2983 0015  ld   $15
              2984 0005  ld   $05
              2985 0011  ld   $11
              2986 0004  ld   $04
              2987 0000  ld   $00
              2988 0000  ld   $00
              2989 0000  ld   $00
              * 25 times
              29a0 0200  nop
              29a1 0200  nop
              29a2 0200  nop
              * 91 times
              29fb fe00  bra  ac          ;Trampoline for page $2900 lookups
              29fc fcfd  bra  $29fd
              29fd 1403  ld   $03,y
              29fe e07b  jmp  y,$7b
              29ff 1519  ld   [$19],y
jupiter35:    2a00 0000  ld   $00
              2a01 0000  ld   $00
              2a02 0000  ld   $00
              * 24 times
              2a18 0025  ld   $25
              2a19 002f  ld   $2f
              2a1a 003e  ld   $3e
              2a1b 002b  ld   $2b
              2a1c 002f  ld   $2f
              2a1d 002f  ld   $2f
              2a1e 003f  ld   $3f
              2a1f 002b  ld   $2b
              2a20 003f  ld   $3f
              2a21 002f  ld   $2f
              2a22 002f  ld   $2f
              2a23 003f  ld   $3f
              2a24 002f  ld   $2f
              2a25 002f  ld   $2f
              2a26 003b  ld   $3b
              2a27 002f  ld   $2f
              2a28 003f  ld   $3f
              2a29 002f  ld   $2f
              2a2a 003f  ld   $3f
              2a2b 003f  ld   $3f
              2a2c 002f  ld   $2f
              2a2d 002f  ld   $2f
              2a2e 003f  ld   $3f
              2a2f 002f  ld   $2f
              2a30 002f  ld   $2f
              2a31 003b  ld   $3b
              2a32 002f  ld   $2f
              2a33 002b  ld   $2b
              2a34 003f  ld   $3f
              2a35 002f  ld   $2f
              2a36 002b  ld   $2b
              2a37 003f  ld   $3f
              2a38 002f  ld   $2f
              2a39 003f  ld   $3f
              2a3a 002f  ld   $2f
              2a3b 003f  ld   $3f
              2a3c 002f  ld   $2f
              2a3d 002b  ld   $2b
              2a3e 002f  ld   $2f
              2a3f 002f  ld   $2f
              2a40 002b  ld   $2b
              2a41 002f  ld   $2f
              2a42 002b  ld   $2b
              2a43 002f  ld   $2f
              2a44 002b  ld   $2b
              2a45 002b  ld   $2b
              2a46 002f  ld   $2f
              2a47 002b  ld   $2b
              2a48 002f  ld   $2f
              2a49 002b  ld   $2b
              2a4a 002f  ld   $2f
              2a4b 002b  ld   $2b
              2a4c 002b  ld   $2b
              2a4d 002f  ld   $2f
              2a4e 002b  ld   $2b
              2a4f 002b  ld   $2b
              2a50 002f  ld   $2f
              2a51 002b  ld   $2b
              2a52 002e  ld   $2e
              2a53 002b  ld   $2b
              2a54 002b  ld   $2b
              2a55 002a  ld   $2a
              2a56 002f  ld   $2f
              2a57 002a  ld   $2a
              2a58 002b  ld   $2b
              2a59 002b  ld   $2b
              2a5a 001a  ld   $1a
              2a5b 002b  ld   $2b
              2a5c 002a  ld   $2a
              2a5d 002a  ld   $2a
              2a5e 002b  ld   $2b
              2a5f 001a  ld   $1a
              2a60 002a  ld   $2a
              2a61 001b  ld   $1b
              2a62 002a  ld   $2a
              2a63 002b  ld   $2b
              2a64 001a  ld   $1a
              2a65 002a  ld   $2a
              2a66 001a  ld   $1a
              2a67 002a  ld   $2a
              2a68 002a  ld   $2a
              2a69 001a  ld   $1a
              2a6a 002a  ld   $2a
              2a6b 0016  ld   $16
              2a6c 002a  ld   $2a
              2a6d 0016  ld   $16
              2a6e 001a  ld   $1a
              2a6f 0016  ld   $16
              2a70 001a  ld   $1a
              2a71 0016  ld   $16
              2a72 001a  ld   $1a
              2a73 0015  ld   $15
              2a74 001a  ld   $1a
              2a75 0016  ld   $16
              2a76 0015  ld   $15
              2a77 0016  ld   $16
              2a78 0015  ld   $15
              2a79 0016  ld   $16
              2a7a 0015  ld   $15
              2a7b 0016  ld   $16
              2a7c 0015  ld   $15
              2a7d 0005  ld   $05
              2a7e 0015  ld   $15
              2a7f 0015  ld   $15
              2a80 0015  ld   $15
              2a81 0015  ld   $15
              2a82 0006  ld   $06
              2a83 0015  ld   $15
              2a84 0015  ld   $15
              2a85 0015  ld   $15
              2a86 0015  ld   $15
              2a87 0000  ld   $00
              2a88 0000  ld   $00
              2a89 0000  ld   $00
              * 25 times
              2aa0 0200  nop
              2aa1 0200  nop
              2aa2 0200  nop
              * 91 times
              2afb fe00  bra  ac          ;Trampoline for page $2a00 lookups
              2afc fcfd  bra  $2afd
              2afd 1403  ld   $03,y
              2afe e07b  jmp  y,$7b
              2aff 1519  ld   [$19],y
jupiter36:    2b00 0000  ld   $00
              2b01 0000  ld   $00
              2b02 0000  ld   $00
              * 23 times
              2b17 0005  ld   $05
              2b18 002b  ld   $2b
              2b19 002f  ld   $2f
              2b1a 002f  ld   $2f
              2b1b 003f  ld   $3f
              2b1c 002f  ld   $2f
              2b1d 002b  ld   $2b
              2b1e 002f  ld   $2f
              2b1f 003f  ld   $3f
              2b20 002f  ld   $2f
              2b21 003f  ld   $3f
              2b22 002f  ld   $2f
              2b23 003f  ld   $3f
              2b24 002f  ld   $2f
              2b25 003f  ld   $3f
              2b26 002f  ld   $2f
              2b27 002f  ld   $2f
              2b28 003f  ld   $3f
              2b29 002f  ld   $2f
              2b2a 003f  ld   $3f
              2b2b 002f  ld   $2f
              2b2c 003f  ld   $3f
              2b2d 003b  ld   $3b
              2b2e 002f  ld   $2f
              2b2f 003f  ld   $3f
              2b30 002f  ld   $2f
              2b31 003f  ld   $3f
              2b32 002f  ld   $2f
              2b33 003f  ld   $3f
              2b34 002f  ld   $2f
              2b35 003f  ld   $3f
              2b36 002f  ld   $2f
              2b37 002f  ld   $2f
              2b38 003b  ld   $3b
              2b39 002f  ld   $2f
              2b3a 002b  ld   $2b
              2b3b 002f  ld   $2f
              2b3c 002b  ld   $2b
              2b3d 003f  ld   $3f
              2b3e 002f  ld   $2f
              2b3f 003b  ld   $3b
              2b40 002f  ld   $2f
              2b41 003f  ld   $3f
              2b42 002f  ld   $2f
              2b43 003b  ld   $3b
              2b44 002f  ld   $2f
              2b45 002f  ld   $2f
              2b46 002b  ld   $2b
              2b47 002f  ld   $2f
              2b48 002b  ld   $2b
              2b49 002f  ld   $2f
              2b4a 002b  ld   $2b
              2b4b 002f  ld   $2f
              2b4c 002e  ld   $2e
              2b4d 002b  ld   $2b
              2b4e 002f  ld   $2f
              2b4f 002a  ld   $2a
              2b50 002f  ld   $2f
              2b51 002b  ld   $2b
              2b52 002b  ld   $2b
              2b53 002f  ld   $2f
              2b54 002a  ld   $2a
              2b55 002b  ld   $2b
              2b56 002b  ld   $2b
              2b57 002b  ld   $2b
              2b58 002e  ld   $2e
              2b59 002b  ld   $2b
              2b5a 002a  ld   $2a
              2b5b 002b  ld   $2b
              2b5c 001a  ld   $1a
              2b5d 002b  ld   $2b
              2b5e 002a  ld   $2a
              2b5f 002a  ld   $2a
              2b60 002b  ld   $2b
              2b61 0016  ld   $16
              2b62 002a  ld   $2a
              2b63 001a  ld   $1a
              2b64 002a  ld   $2a
              2b65 001a  ld   $1a
              2b66 0026  ld   $26
              2b67 001a  ld   $1a
              2b68 0016  ld   $16
              2b69 002a  ld   $2a
              2b6a 0016  ld   $16
              2b6b 001a  ld   $1a
              2b6c 001a  ld   $1a
              2b6d 0025  ld   $25
              2b6e 001a  ld   $1a
              2b6f 0025  ld   $25
              2b70 001a  ld   $1a
              2b71 0025  ld   $25
              2b72 001a  ld   $1a
              2b73 0016  ld   $16
              2b74 0015  ld   $15
              2b75 0015  ld   $15
              2b76 001a  ld   $1a
              2b77 0015  ld   $15
              2b78 0015  ld   $15
              2b79 0015  ld   $15
              2b7a 0005  ld   $05
              2b7b 0015  ld   $15
              2b7c 0015  ld   $15
              2b7d 0015  ld   $15
              2b7e 0015  ld   $15
              2b7f 0005  ld   $05
              2b80 0015  ld   $15
              2b81 0015  ld   $15
              2b82 0015  ld   $15
              2b83 0015  ld   $15
              2b84 0005  ld   $05
              2b85 0015  ld   $15
              2b86 0015  ld   $15
              2b87 0000  ld   $00
              2b88 0000  ld   $00
              2b89 0000  ld   $00
              * 25 times
              2ba0 0200  nop
              2ba1 0200  nop
              2ba2 0200  nop
              * 91 times
              2bfb fe00  bra  ac          ;Trampoline for page $2b00 lookups
              2bfc fcfd  bra  $2bfd
              2bfd 1403  ld   $03,y
              2bfe e07b  jmp  y,$7b
              2bff 1519  ld   [$19],y
jupiter37:    2c00 0000  ld   $00
              2c01 0000  ld   $00
              2c02 0000  ld   $00
              * 23 times
              2c17 0015  ld   $15
              2c18 002f  ld   $2f
              2c19 003b  ld   $3b
              2c1a 002f  ld   $2f
              2c1b 002b  ld   $2b
              2c1c 002f  ld   $2f
              2c1d 003f  ld   $3f
              2c1e 002f  ld   $2f
              2c1f 002f  ld   $2f
              2c20 003f  ld   $3f
              2c21 002f  ld   $2f
              2c22 003b  ld   $3b
              2c23 002f  ld   $2f
              2c24 002f  ld   $2f
              2c25 003f  ld   $3f
              2c26 002f  ld   $2f
              2c27 003f  ld   $3f
              2c28 002f  ld   $2f
              2c29 003f  ld   $3f
              2c2a 002b  ld   $2b
              2c2b 003f  ld   $3f
              2c2c 002f  ld   $2f
              2c2d 003f  ld   $3f
              2c2e 002f  ld   $2f
              2c2f 003f  ld   $3f
              2c30 002f  ld   $2f
              2c31 003f  ld   $3f
              2c32 002f  ld   $2f
              2c33 003f  ld   $3f
              2c34 002f  ld   $2f
              2c35 002f  ld   $2f
              2c36 003b  ld   $3b
              2c37 002f  ld   $2f
              2c38 002f  ld   $2f
              2c39 003f  ld   $3f
              2c3a 002f  ld   $2f
              2c3b 003f  ld   $3f
              2c3c 002f  ld   $2f
              2c3d 002f  ld   $2f
              2c3e 002b  ld   $2b
              2c3f 002f  ld   $2f
              2c40 003b  ld   $3b
              2c41 002f  ld   $2f
              2c42 003b  ld   $3b
              2c43 002f  ld   $2f
              2c44 002f  ld   $2f
              2c45 003b  ld   $3b
              2c46 002f  ld   $2f
              2c47 003b  ld   $3b
              2c48 002f  ld   $2f
              2c49 002b  ld   $2b
              2c4a 003f  ld   $3f
              2c4b 002b  ld   $2b
              2c4c 002f  ld   $2f
              2c4d 002a  ld   $2a
              2c4e 002b  ld   $2b
              2c4f 002f  ld   $2f
              2c50 002b  ld   $2b
              2c51 002e  ld   $2e
              2c52 002b  ld   $2b
              2c53 002a  ld   $2a
              2c54 002f  ld   $2f
              2c55 002b  ld   $2b
              2c56 002e  ld   $2e
              2c57 001a  ld   $1a
              2c58 002b  ld   $2b
              2c59 002a  ld   $2a
              2c5a 001b  ld   $1b
              2c5b 002a  ld   $2a
              2c5c 002b  ld   $2b
              2c5d 002a  ld   $2a
              2c5e 001b  ld   $1b
              2c5f 002a  ld   $2a
              2c60 001a  ld   $1a
              2c61 002a  ld   $2a
              2c62 002b  ld   $2b
              2c63 001a  ld   $1a
              2c64 002a  ld   $2a
              2c65 001b  ld   $1b
              2c66 002a  ld   $2a
              2c67 001a  ld   $1a
              2c68 002a  ld   $2a
              2c69 001a  ld   $1a
              2c6a 002a  ld   $2a
              2c6b 0016  ld   $16
              2c6c 002a  ld   $2a
              2c6d 001a  ld   $1a
              2c6e 0016  ld   $16
              2c6f 001a  ld   $1a
              2c70 0016  ld   $16
              2c71 001a  ld   $1a
              2c72 0016  ld   $16
              2c73 0019  ld   $19
              2c74 0016  ld   $16
              2c75 0019  ld   $19
              2c76 0016  ld   $16
              2c77 0015  ld   $15
              2c78 0016  ld   $16
              2c79 0015  ld   $15
              2c7a 0016  ld   $16
              2c7b 0015  ld   $15
              2c7c 0015  ld   $15
              2c7d 0006  ld   $06
              2c7e 0015  ld   $15
              2c7f 0015  ld   $15
              2c80 0012  ld   $12
              2c81 0015  ld   $15
              2c82 0005  ld   $05
              2c83 0015  ld   $15
              2c84 0016  ld   $16
              2c85 0005  ld   $05
              2c86 0015  ld   $15
              2c87 0015  ld   $15
              2c88 0000  ld   $00
              2c89 0000  ld   $00
              2c8a 0000  ld   $00
              * 24 times
              2ca0 0200  nop
              2ca1 0200  nop
              2ca2 0200  nop
              * 91 times
              2cfb fe00  bra  ac          ;Trampoline for page $2c00 lookups
              2cfc fcfd  bra  $2cfd
              2cfd 1403  ld   $03,y
              2cfe e07b  jmp  y,$7b
              2cff 1519  ld   [$19],y
jupiter38:    2d00 0000  ld   $00
              2d01 0000  ld   $00
              2d02 0000  ld   $00
              * 22 times
              2d16 0011  ld   $11
              2d17 002e  ld   $2e
              2d18 002b  ld   $2b
              2d19 002f  ld   $2f
              2d1a 002f  ld   $2f
              2d1b 003f  ld   $3f
              2d1c 002f  ld   $2f
              2d1d 002b  ld   $2b
              2d1e 003f  ld   $3f
              2d1f 002f  ld   $2f
              2d20 002f  ld   $2f
              2d21 002f  ld   $2f
              2d22 003f  ld   $3f
              2d23 002f  ld   $2f
              2d24 003f  ld   $3f
              2d25 002f  ld   $2f
              2d26 003f  ld   $3f
              2d27 002b  ld   $2b
              2d28 002f  ld   $2f
              2d29 003f  ld   $3f
              2d2a 002f  ld   $2f
              2d2b 002f  ld   $2f
              2d2c 002f  ld   $2f
              2d2d 003f  ld   $3f
              2d2e 002f  ld   $2f
              2d2f 002f  ld   $2f
              2d30 003b  ld   $3b
              2d31 002f  ld   $2f
              2d32 003f  ld   $3f
              2d33 002b  ld   $2b
              2d34 003f  ld   $3f
              2d35 002f  ld   $2f
              2d36 003f  ld   $3f
              2d37 002f  ld   $2f
              2d38 002b  ld   $2b
              2d39 002f  ld   $2f
              2d3a 002b  ld   $2b
              2d3b 002f  ld   $2f
              2d3c 002b  ld   $2b
              2d3d 003f  ld   $3f
              2d3e 002f  ld   $2f
              2d3f 002f  ld   $2f
              2d40 002b  ld   $2b
              2d41 002f  ld   $2f
              2d42 002b  ld   $2b
              2d43 002f  ld   $2f
              2d44 002b  ld   $2b
              2d45 002f  ld   $2f
              2d46 002b  ld   $2b
              2d47 002f  ld   $2f
              2d48 002a  ld   $2a
              2d49 002f  ld   $2f
              2d4a 002b  ld   $2b
              2d4b 002e  ld   $2e
              2d4c 002b  ld   $2b
              2d4d 002f  ld   $2f
              2d4e 002b  ld   $2b
              2d4f 002a  ld   $2a
              2d50 002f  ld   $2f
              2d51 002b  ld   $2b
              2d52 002b  ld   $2b
              2d53 002f  ld   $2f
              2d54 002a  ld   $2a
              2d55 002b  ld   $2b
              2d56 002a  ld   $2a
              2d57 002b  ld   $2b
              2d58 002a  ld   $2a
              2d59 002b  ld   $2b
              2d5a 002a  ld   $2a
              2d5b 002b  ld   $2b
              2d5c 001a  ld   $1a
              2d5d 002a  ld   $2a
              2d5e 002b  ld   $2b
              2d5f 001a  ld   $1a
              2d60 002b  ld   $2b
              2d61 002a  ld   $2a
              2d62 001a  ld   $1a
              2d63 002a  ld   $2a
              2d64 0016  ld   $16
              2d65 002a  ld   $2a
              2d66 001a  ld   $1a
              2d67 002a  ld   $2a
              2d68 0016  ld   $16
              2d69 002a  ld   $2a
              2d6a 001a  ld   $1a
              2d6b 002a  ld   $2a
              2d6c 0015  ld   $15
              2d6d 001a  ld   $1a
              2d6e 0029  ld   $29
              2d6f 0016  ld   $16
              2d70 0019  ld   $19
              2d71 0026  ld   $26
              2d72 0015  ld   $15
              2d73 0016  ld   $16
              2d74 0019  ld   $19
              2d75 0016  ld   $16
              2d76 0015  ld   $15
              2d77 001a  ld   $1a
              2d78 0015  ld   $15
              2d79 0015  ld   $15
              2d7a 0015  ld   $15
              2d7b 0016  ld   $16
              2d7c 0015  ld   $15
              2d7d 0015  ld   $15
              2d7e 0015  ld   $15
              2d7f 0015  ld   $15
              2d80 0005  ld   $05
              2d81 0015  ld   $15
              2d82 0015  ld   $15
              2d83 0011  ld   $11
              2d84 0005  ld   $05
              2d85 0015  ld   $15
              2d86 0016  ld   $16
              2d87 0015  ld   $15
              2d88 0000  ld   $00
              2d89 0000  ld   $00
              2d8a 0000  ld   $00
              * 24 times
              2da0 0200  nop
              2da1 0200  nop
              2da2 0200  nop
              * 91 times
              2dfb fe00  bra  ac          ;Trampoline for page $2d00 lookups
              2dfc fcfd  bra  $2dfd
              2dfd 1403  ld   $03,y
              2dfe e07b  jmp  y,$7b
              2dff 1519  ld   [$19],y
jupiter39:    2e00 0000  ld   $00
              2e01 0000  ld   $00
              2e02 0000  ld   $00
              * 22 times
              2e16 0015  ld   $15
              2e17 002f  ld   $2f
              2e18 002b  ld   $2b
              2e19 002f  ld   $2f
              2e1a 002f  ld   $2f
              2e1b 002f  ld   $2f
              2e1c 002f  ld   $2f
              2e1d 003f  ld   $3f
              2e1e 002f  ld   $2f
              2e1f 002f  ld   $2f
              2e20 003b  ld   $3b
              2e21 002f  ld   $2f
              2e22 002f  ld   $2f
              2e23 003b  ld   $3b
              2e24 002f  ld   $2f
              2e25 002f  ld   $2f
              2e26 002f  ld   $2f
              2e27 003f  ld   $3f
              2e28 002f  ld   $2f
              2e29 002f  ld   $2f
              2e2a 002f  ld   $2f
              2e2b 003f  ld   $3f
              2e2c 002f  ld   $2f
              2e2d 002b  ld   $2b
              2e2e 002f  ld   $2f
              2e2f 003f  ld   $3f
              2e30 002f  ld   $2f
              2e31 002f  ld   $2f
              2e32 002f  ld   $2f
              * 5 times
              2e35 002b  ld   $2b
              2e36 002f  ld   $2f
              2e37 003f  ld   $3f
              2e38 002f  ld   $2f
              2e39 003b  ld   $3b
              2e3a 002f  ld   $2f
              2e3b 003f  ld   $3f
              2e3c 002f  ld   $2f
              2e3d 002b  ld   $2b
              2e3e 002e  ld   $2e
              2e3f 002b  ld   $2b
              2e40 002f  ld   $2f
              2e41 002f  ld   $2f
              2e42 002f  ld   $2f
              2e43 002b  ld   $2b
              2e44 002f  ld   $2f
              2e45 002b  ld   $2b
              2e46 002f  ld   $2f
              2e47 002b  ld   $2b
              2e48 002f  ld   $2f
              2e49 002b  ld   $2b
              2e4a 002f  ld   $2f
              2e4b 002b  ld   $2b
              2e4c 002b  ld   $2b
              2e4d 002f  ld   $2f
              2e4e 002b  ld   $2b
              2e4f 002f  ld   $2f
              2e50 002b  ld   $2b
              2e51 002b  ld   $2b
              2e52 002e  ld   $2e
              2e53 001b  ld   $1b
              2e54 002a  ld   $2a
              2e55 002b  ld   $2b
              2e56 002b  ld   $2b
              2e57 001a  ld   $1a
              2e58 002b  ld   $2b
              2e59 002f  ld   $2f
              2e5a 001a  ld   $1a
              2e5b 002b  ld   $2b
              2e5c 002a  ld   $2a
              2e5d 002b  ld   $2b
              2e5e 001a  ld   $1a
              2e5f 002a  ld   $2a
              2e60 002b  ld   $2b
              2e61 001a  ld   $1a
              2e62 002a  ld   $2a
              2e63 001a  ld   $1a
              2e64 002b  ld   $2b
              2e65 002a  ld   $2a
              2e66 001a  ld   $1a
              2e67 002a  ld   $2a
              2e68 001a  ld   $1a
              2e69 0026  ld   $26
              2e6a 001a  ld   $1a
              2e6b 001a  ld   $1a
              2e6c 0026  ld   $26
              2e6d 001a  ld   $1a
              2e6e 0016  ld   $16
              2e6f 001a  ld   $1a
              2e70 0016  ld   $16
              2e71 0019  ld   $19
              2e72 0016  ld   $16
              2e73 001a  ld   $1a
              2e74 0016  ld   $16
              2e75 0015  ld   $15
              2e76 0016  ld   $16
              2e77 0015  ld   $15
              2e78 0015  ld   $15
              2e79 0016  ld   $16
              2e7a 0015  ld   $15
              2e7b 0015  ld   $15
              2e7c 0005  ld   $05
              2e7d 0015  ld   $15
              2e7e 0005  ld   $05
              2e7f 0015  ld   $15
              2e80 0015  ld   $15
              2e81 0015  ld   $15
              2e82 0005  ld   $05
              2e83 0016  ld   $16
              2e84 0015  ld   $15
              2e85 0015  ld   $15
              2e86 0015  ld   $15
              * 5 times
              2e89 0000  ld   $00
              2e8a 0000  ld   $00
              2e8b 0000  ld   $00
              * 23 times
              2ea0 0200  nop
              2ea1 0200  nop
              2ea2 0200  nop
              * 91 times
              2efb fe00  bra  ac          ;Trampoline for page $2e00 lookups
              2efc fcfd  bra  $2efd
              2efd 1403  ld   $03,y
              2efe e07b  jmp  y,$7b
              2eff 1519  ld   [$19],y
jupiter40:    2f00 0000  ld   $00
              2f01 0000  ld   $00
              2f02 0000  ld   $00
              * 22 times
              2f16 001a  ld   $1a
              2f17 002b  ld   $2b
              2f18 002f  ld   $2f
              2f19 002b  ld   $2b
              2f1a 002f  ld   $2f
              2f1b 002a  ld   $2a
              2f1c 002f  ld   $2f
              2f1d 002f  ld   $2f
              2f1e 002b  ld   $2b
              2f1f 002f  ld   $2f
              2f20 002f  ld   $2f
              2f21 002f  ld   $2f
              2f22 002b  ld   $2b
              2f23 002f  ld   $2f
              2f24 002f  ld   $2f
              2f25 002f  ld   $2f
              * 5 times
              2f28 002b  ld   $2b
              2f29 002e  ld   $2e
              2f2a 002b  ld   $2b
              2f2b 002f  ld   $2f
              2f2c 002f  ld   $2f
              2f2d 002f  ld   $2f
              2f2e 002b  ld   $2b
              2f2f 002f  ld   $2f
              2f30 002b  ld   $2b
              2f31 002f  ld   $2f
              2f32 002b  ld   $2b
              2f33 002f  ld   $2f
              2f34 002f  ld   $2f
              2f35 002f  ld   $2f
              2f36 002b  ld   $2b
              2f37 002f  ld   $2f
              2f38 002b  ld   $2b
              2f39 002f  ld   $2f
              2f3a 002b  ld   $2b
              2f3b 002f  ld   $2f
              2f3c 002f  ld   $2f
              2f3d 002b  ld   $2b
              2f3e 002f  ld   $2f
              2f3f 003f  ld   $3f
              2f40 002b  ld   $2b
              2f41 002f  ld   $2f
              2f42 002b  ld   $2b
              2f43 002f  ld   $2f
              2f44 002f  ld   $2f
              2f45 002b  ld   $2b
              2f46 002f  ld   $2f
              2f47 002b  ld   $2b
              2f48 002f  ld   $2f
              2f49 002a  ld   $2a
              2f4a 002b  ld   $2b
              2f4b 002f  ld   $2f
              2f4c 002a  ld   $2a
              2f4d 002b  ld   $2b
              2f4e 002e  ld   $2e
              2f4f 002b  ld   $2b
              2f50 002e  ld   $2e
              2f51 002b  ld   $2b
              2f52 002b  ld   $2b
              2f53 002b  ld   $2b
              2f54 001e  ld   $1e
              2f55 002b  ld   $2b
              2f56 002e  ld   $2e
              2f57 002b  ld   $2b
              2f58 001a  ld   $1a
              2f59 002b  ld   $2b
              2f5a 001a  ld   $1a
              2f5b 002b  ld   $2b
              2f5c 001a  ld   $1a
              2f5d 002b  ld   $2b
              2f5e 001a  ld   $1a
              2f5f 002a  ld   $2a
              2f60 001a  ld   $1a
              2f61 0016  ld   $16
              2f62 002b  ld   $2b
              2f63 001a  ld   $1a
              2f64 0016  ld   $16
              2f65 001a  ld   $1a
              2f66 0026  ld   $26
              2f67 001a  ld   $1a
              2f68 0016  ld   $16
              2f69 001a  ld   $1a
              2f6a 002a  ld   $2a
              2f6b 0016  ld   $16
              2f6c 001a  ld   $1a
              2f6d 0016  ld   $16
              2f6e 001a  ld   $1a
              2f6f 0015  ld   $15
              2f70 0016  ld   $16
              2f71 001a  ld   $1a
              2f72 0015  ld   $15
              2f73 0016  ld   $16
              2f74 0015  ld   $15
              2f75 001a  ld   $1a
              2f76 0016  ld   $16
              2f77 0019  ld   $19
              2f78 0016  ld   $16
              2f79 0015  ld   $15
              2f7a 0015  ld   $15
              2f7b 0016  ld   $16
              2f7c 0015  ld   $15
              2f7d 0015  ld   $15
              2f7e 0015  ld   $15
              2f7f 0016  ld   $16
              2f80 0005  ld   $05
              2f81 0015  ld   $15
              2f82 0015  ld   $15
              2f83 0005  ld   $05
              2f84 0015  ld   $15
              2f85 0005  ld   $05
              2f86 0015  ld   $15
              2f87 0005  ld   $05
              2f88 0015  ld   $15
              2f89 0001  ld   $01
              2f8a 0000  ld   $00
              2f8b 0000  ld   $00
              2f8c 0000  ld   $00
              * 22 times
              2fa0 0200  nop
              2fa1 0200  nop
              2fa2 0200  nop
              * 91 times
              2ffb fe00  bra  ac          ;Trampoline for page $2f00 lookups
              2ffc fcfd  bra  $2ffd
              2ffd 1403  ld   $03,y
              2ffe e07b  jmp  y,$7b
              2fff 1519  ld   [$19],y
jupiter41:    3000 0000  ld   $00
              3001 0000  ld   $00
              3002 0000  ld   $00
              * 21 times
              3015 0001  ld   $01
              3016 002a  ld   $2a
              3017 001b  ld   $1b
              3018 002b  ld   $2b
              3019 002b  ld   $2b
              301a 002f  ld   $2f
              301b 002f  ld   $2f
              301c 002b  ld   $2b
              301d 002f  ld   $2f
              301e 002b  ld   $2b
              301f 002f  ld   $2f
              3020 002b  ld   $2b
              3021 002b  ld   $2b
              3022 002f  ld   $2f
              3023 002b  ld   $2b
              3024 002f  ld   $2f
              3025 002b  ld   $2b
              3026 002b  ld   $2b
              3027 002b  ld   $2b
              3028 002f  ld   $2f
              3029 002b  ld   $2b
              302a 002f  ld   $2f
              302b 002f  ld   $2f
              302c 002b  ld   $2b
              302d 002a  ld   $2a
              302e 002f  ld   $2f
              302f 002b  ld   $2b
              3030 002f  ld   $2f
              3031 002a  ld   $2a
              3032 002f  ld   $2f
              3033 002b  ld   $2b
              3034 002b  ld   $2b
              3035 002f  ld   $2f
              3036 002b  ld   $2b
              3037 002b  ld   $2b
              3038 002e  ld   $2e
              3039 002b  ld   $2b
              303a 002f  ld   $2f
              303b 002a  ld   $2a
              303c 002b  ld   $2b
              303d 002f  ld   $2f
              303e 002b  ld   $2b
              303f 001b  ld   $1b
              3040 002f  ld   $2f
              3041 002b  ld   $2b
              3042 002f  ld   $2f
              3043 001a  ld   $1a
              3044 002b  ld   $2b
              3045 002b  ld   $2b
              3046 001a  ld   $1a
              3047 002b  ld   $2b
              3048 002b  ld   $2b
              3049 002f  ld   $2f
              304a 002f  ld   $2f
              304b 002b  ld   $2b
              304c 002b  ld   $2b
              304d 002f  ld   $2f
              304e 002b  ld   $2b
              304f 002b  ld   $2b
              3050 001b  ld   $1b
              3051 002a  ld   $2a
              3052 001b  ld   $1b
              3053 002a  ld   $2a
              3054 001b  ld   $1b
              3055 002a  ld   $2a
              3056 001b  ld   $1b
              3057 001a  ld   $1a
              3058 002b  ld   $2b
              3059 001a  ld   $1a
              305a 002b  ld   $2b
              305b 001a  ld   $1a
              305c 001a  ld   $1a
              305d 002b  ld   $2b
              305e 001a  ld   $1a
              305f 001b  ld   $1b
              3060 001a  ld   $1a
              3061 001b  ld   $1b
              3062 001a  ld   $1a
              3063 001a  ld   $1a
              3064 002a  ld   $2a
              3065 001a  ld   $1a
              3066 001a  ld   $1a
              3067 0016  ld   $16
              3068 001a  ld   $1a
              3069 0016  ld   $16
              306a 001a  ld   $1a
              306b 0016  ld   $16
              306c 001a  ld   $1a
              306d 0016  ld   $16
              306e 0015  ld   $15
              306f 001a  ld   $1a
              3070 0016  ld   $16
              3071 0015  ld   $15
              3072 0016  ld   $16
              3073 0019  ld   $19
              3074 0016  ld   $16
              3075 0015  ld   $15
              3076 0015  ld   $15
              3077 0016  ld   $16
              3078 0015  ld   $15
              3079 0016  ld   $16
              307a 0015  ld   $15
              307b 0005  ld   $05
              307c 0015  ld   $15
              307d 0005  ld   $05
              307e 0015  ld   $15
              307f 0001  ld   $01
              3080 0015  ld   $15
              3081 0016  ld   $16
              3082 0005  ld   $05
              3083 0011  ld   $11
              3084 0015  ld   $15
              3085 0005  ld   $05
              3086 0011  ld   $11
              3087 0005  ld   $05
              3088 0015  ld   $15
              3089 0004  ld   $04
              308a 0000  ld   $00
              308b 0000  ld   $00
              308c 0000  ld   $00
              * 22 times
              30a0 0200  nop
              30a1 0200  nop
              30a2 0200  nop
              * 91 times
              30fb fe00  bra  ac          ;Trampoline for page $3000 lookups
              30fc fcfd  bra  $30fd
              30fd 1403  ld   $03,y
              30fe e07b  jmp  y,$7b
              30ff 1519  ld   [$19],y
jupiter42:    3100 0000  ld   $00
              3101 0000  ld   $00
              3102 0000  ld   $00
              * 21 times
              3115 0015  ld   $15
              3116 001b  ld   $1b
              3117 001a  ld   $1a
              3118 002f  ld   $2f
              3119 002b  ld   $2b
              311a 002f  ld   $2f
              311b 002b  ld   $2b
              311c 002f  ld   $2f
              311d 002b  ld   $2b
              311e 001a  ld   $1a
              311f 002b  ld   $2b
              3120 001b  ld   $1b
              3121 001b  ld   $1b
              3122 002b  ld   $2b
              3123 001b  ld   $1b
              3124 002b  ld   $2b
              3125 002b  ld   $2b
              3126 002f  ld   $2f
              3127 002b  ld   $2b
              3128 002f  ld   $2f
              3129 002b  ld   $2b
              312a 002b  ld   $2b
              312b 002b  ld   $2b
              312c 002f  ld   $2f
              312d 002f  ld   $2f
              312e 002b  ld   $2b
              312f 002f  ld   $2f
              3130 001b  ld   $1b
              3131 002b  ld   $2b
              3132 001b  ld   $1b
              3133 002b  ld   $2b
              3134 002f  ld   $2f
              3135 002b  ld   $2b
              3136 001b  ld   $1b
              3137 002f  ld   $2f
              3138 001b  ld   $1b
              3139 001f  ld   $1f
              313a 002b  ld   $2b
              313b 001f  ld   $1f
              313c 002b  ld   $2b
              313d 002f  ld   $2f
              313e 001a  ld   $1a
              313f 002b  ld   $2b
              3140 001a  ld   $1a
              3141 002f  ld   $2f
              3142 001b  ld   $1b
              3143 002b  ld   $2b
              3144 001f  ld   $1f
              3145 001b  ld   $1b
              3146 002f  ld   $2f
              3147 001b  ld   $1b
              3148 002b  ld   $2b
              3149 001b  ld   $1b
              314a 002b  ld   $2b
              314b 001b  ld   $1b
              314c 002b  ld   $2b
              314d 001b  ld   $1b
              314e 001b  ld   $1b
              314f 002f  ld   $2f
              3150 002a  ld   $2a
              3151 001b  ld   $1b
              3152 002e  ld   $2e
              3153 001b  ld   $1b
              3154 002a  ld   $2a
              3155 001b  ld   $1b
              3156 001a  ld   $1a
              3157 001a  ld   $1a
              3158 001a  ld   $1a
              3159 001a  ld   $1a
              315a 001b  ld   $1b
              315b 001a  ld   $1a
              315c 001b  ld   $1b
              315d 001a  ld   $1a
              315e 0016  ld   $16
              315f 001a  ld   $1a
              3160 0026  ld   $26
              3161 001a  ld   $1a
              3162 0016  ld   $16
              3163 0016  ld   $16
              3164 001a  ld   $1a
              3165 0016  ld   $16
              3166 001a  ld   $1a
              3167 0016  ld   $16
              3168 001a  ld   $1a
              3169 0016  ld   $16
              316a 0016  ld   $16
              316b 0016  ld   $16
              316c 0015  ld   $15
              316d 0016  ld   $16
              316e 0016  ld   $16
              316f 0015  ld   $15
              3170 0016  ld   $16
              3171 0015  ld   $15
              3172 0016  ld   $16
              3173 0015  ld   $15
              3174 0016  ld   $16
              3175 0015  ld   $15
              3176 0006  ld   $06
              3177 0015  ld   $15
              3178 0015  ld   $15
              3179 0006  ld   $06
              317a 0015  ld   $15
              317b 0015  ld   $15
              317c 0006  ld   $06
              317d 0015  ld   $15
              317e 0015  ld   $15
              317f 0005  ld   $05
              3180 0015  ld   $15
              3181 0001  ld   $01
              3182 0015  ld   $15
              3183 0005  ld   $05
              3184 0015  ld   $15
              3185 0005  ld   $05
              3186 0015  ld   $15
              3187 0001  ld   $01
              3188 0015  ld   $15
              3189 0001  ld   $01
              318a 0000  ld   $00
              318b 0000  ld   $00
              318c 0000  ld   $00
              * 22 times
              31a0 0200  nop
              31a1 0200  nop
              31a2 0200  nop
              * 91 times
              31fb fe00  bra  ac          ;Trampoline for page $3100 lookups
              31fc fcfd  bra  $31fd
              31fd 1403  ld   $03,y
              31fe e07b  jmp  y,$7b
              31ff 1519  ld   [$19],y
jupiter43:    3200 0000  ld   $00
              3201 0000  ld   $00
              3202 0000  ld   $00
              * 21 times
              3215 0016  ld   $16
              3216 001a  ld   $1a
              3217 001b  ld   $1b
              3218 001b  ld   $1b
              3219 001f  ld   $1f
              321a 002b  ld   $2b
              321b 001b  ld   $1b
              321c 002f  ld   $2f
              321d 001b  ld   $1b
              321e 002b  ld   $2b
              321f 001a  ld   $1a
              3220 001b  ld   $1b
              3221 001a  ld   $1a
              3222 001b  ld   $1b
              3223 001a  ld   $1a
              3224 001b  ld   $1b
              3225 001a  ld   $1a
              3226 001b  ld   $1b
              3227 001a  ld   $1a
              3228 001b  ld   $1b
              3229 002f  ld   $2f
              322a 001b  ld   $1b
              322b 002f  ld   $2f
              322c 001b  ld   $1b
              322d 002b  ld   $2b
              322e 002f  ld   $2f
              322f 001b  ld   $1b
              3230 002a  ld   $2a
              3231 002b  ld   $2b
              3232 002f  ld   $2f
              3233 002b  ld   $2b
              3234 001a  ld   $1a
              3235 002b  ld   $2b
              3236 001b  ld   $1b
              3237 001a  ld   $1a
              3238 002b  ld   $2b
              3239 002b  ld   $2b
              323a 001f  ld   $1f
              323b 002b  ld   $2b
              323c 002b  ld   $2b
              323d 001b  ld   $1b
              323e 002b  ld   $2b
              323f 001b  ld   $1b
              3240 001a  ld   $1a
              3241 001b  ld   $1b
              3242 002b  ld   $2b
              3243 001b  ld   $1b
              3244 002b  ld   $2b
              3245 001a  ld   $1a
              3246 002b  ld   $2b
              3247 002f  ld   $2f
              3248 001b  ld   $1b
              3249 002a  ld   $2a
              324a 001a  ld   $1a
              324b 001b  ld   $1b
              324c 002e  ld   $2e
              324d 001b  ld   $1b
              324e 002a  ld   $2a
              324f 001b  ld   $1b
              3250 001b  ld   $1b
              3251 002a  ld   $2a
              3252 001b  ld   $1b
              3253 001a  ld   $1a
              3254 001b  ld   $1b
              3255 001a  ld   $1a
              3256 001a  ld   $1a
              3257 0017  ld   $17
              3258 001a  ld   $1a
              3259 001b  ld   $1b
              325a 0026  ld   $26
              325b 001a  ld   $1a
              325c 0016  ld   $16
              325d 001a  ld   $1a
              325e 001a  ld   $1a
              325f 0016  ld   $16
              3260 001a  ld   $1a
              3261 0016  ld   $16
              3262 0016  ld   $16
              3263 001a  ld   $1a
              3264 0016  ld   $16
              3265 0006  ld   $06
              3266 001a  ld   $1a
              3267 0016  ld   $16
              3268 0016  ld   $16
              3269 0016  ld   $16
              326a 001a  ld   $1a
              326b 0015  ld   $15
              326c 000a  ld   $0a
              326d 0016  ld   $16
              326e 0005  ld   $05
              326f 001a  ld   $1a
              3270 0006  ld   $06
              3271 0005  ld   $05
              3272 0016  ld   $16
              3273 0005  ld   $05
              3274 0015  ld   $15
              3275 0005  ld   $05
              3276 0015  ld   $15
              3277 0006  ld   $06
              3278 0015  ld   $15
              3279 0005  ld   $05
              327a 0016  ld   $16
              327b 0005  ld   $05
              327c 0015  ld   $15
              327d 0005  ld   $05
              327e 0015  ld   $15
              327f 0001  ld   $01
              3280 0015  ld   $15
              3281 0005  ld   $05
              3282 0015  ld   $15
              3283 0005  ld   $05
              3284 0015  ld   $15
              3285 0001  ld   $01
              3286 0015  ld   $15
              3287 0005  ld   $05
              3288 0005  ld   $05
              3289 0015  ld   $15
              328a 0001  ld   $01
              328b 0000  ld   $00
              328c 0000  ld   $00
              328d 0000  ld   $00
              * 21 times
              32a0 0200  nop
              32a1 0200  nop
              32a2 0200  nop
              * 91 times
              32fb fe00  bra  ac          ;Trampoline for page $3200 lookups
              32fc fcfd  bra  $32fd
              32fd 1403  ld   $03,y
              32fe e07b  jmp  y,$7b
              32ff 1519  ld   [$19],y
jupiter44:    3300 0000  ld   $00
              3301 0000  ld   $00
              3302 0000  ld   $00
              * 20 times
              3314 0005  ld   $05
              3315 0016  ld   $16
              3316 0006  ld   $06
              3317 0016  ld   $16
              3318 0016  ld   $16
              3319 001a  ld   $1a
              331a 001b  ld   $1b
              331b 002f  ld   $2f
              331c 002b  ld   $2b
              331d 001b  ld   $1b
              331e 001b  ld   $1b
              331f 002b  ld   $2b
              3320 001b  ld   $1b
              3321 001b  ld   $1b
              3322 002b  ld   $2b
              3323 001b  ld   $1b
              3324 001b  ld   $1b
              3325 001a  ld   $1a
              3326 001b  ld   $1b
              3327 001b  ld   $1b
              3328 001a  ld   $1a
              3329 001b  ld   $1b
              332a 002b  ld   $2b
              332b 001b  ld   $1b
              332c 002b  ld   $2b
              332d 001f  ld   $1f
              332e 002b  ld   $2b
              332f 001f  ld   $1f
              3330 002f  ld   $2f
              3331 002f  ld   $2f
              3332 001a  ld   $1a
              3333 001b  ld   $1b
              3334 002f  ld   $2f
              3335 002b  ld   $2b
              3336 002f  ld   $2f
              3337 001b  ld   $1b
              3338 002b  ld   $2b
              3339 001b  ld   $1b
              333a 002b  ld   $2b
              333b 001a  ld   $1a
              333c 002f  ld   $2f
              333d 002f  ld   $2f
              333e 002f  ld   $2f
              333f 002b  ld   $2b
              3340 001b  ld   $1b
              3341 001a  ld   $1a
              3342 002b  ld   $2b
              3343 001f  ld   $1f
              3344 002b  ld   $2b
              3345 001b  ld   $1b
              3346 001b  ld   $1b
              3347 002b  ld   $2b
              3348 001a  ld   $1a
              3349 001b  ld   $1b
              334a 002b  ld   $2b
              334b 001a  ld   $1a
              334c 001b  ld   $1b
              334d 002b  ld   $2b
              334e 001a  ld   $1a
              334f 001b  ld   $1b
              3350 002a  ld   $2a
              3351 001b  ld   $1b
              3352 001b  ld   $1b
              3353 002a  ld   $2a
              3354 001b  ld   $1b
              3355 0016  ld   $16
              3356 001a  ld   $1a
              3357 001a  ld   $1a
              3358 0016  ld   $16
              3359 001a  ld   $1a
              335a 0016  ld   $16
              335b 001b  ld   $1b
              335c 0016  ld   $16
              335d 001a  ld   $1a
              335e 0017  ld   $17
              335f 001a  ld   $1a
              3360 0016  ld   $16
              3361 001a  ld   $1a
              3362 0006  ld   $06
              3363 0016  ld   $16
              3364 001a  ld   $1a
              3365 0006  ld   $06
              3366 0016  ld   $16
              3367 0016  ld   $16
              3368 0019  ld   $19
              3369 0006  ld   $06
              336a 0015  ld   $15
              336b 0016  ld   $16
              336c 0005  ld   $05
              336d 0016  ld   $16
              336e 0005  ld   $05
              336f 0016  ld   $16
              3370 0015  ld   $15
              3371 0016  ld   $16
              3372 0005  ld   $05
              3373 0016  ld   $16
              3374 0016  ld   $16
              3375 0005  ld   $05
              3376 0015  ld   $15
              3377 0005  ld   $05
              3378 0015  ld   $15
              3379 0005  ld   $05
              337a 0015  ld   $15
              337b 0005  ld   $05
              337c 0001  ld   $01
              337d 0005  ld   $05
              337e 0001  ld   $01
              337f 0015  ld   $15
              3380 0001  ld   $01
              3381 0005  ld   $05
              3382 0001  ld   $01
              3383 0015  ld   $15
              3384 0001  ld   $01
              3385 0005  ld   $05
              3386 0015  ld   $15
              3387 0001  ld   $01
              3388 0015  ld   $15
              3389 0001  ld   $01
              338a 0000  ld   $00
              338b 0000  ld   $00
              338c 0000  ld   $00
              * 22 times
              33a0 0200  nop
              33a1 0200  nop
              33a2 0200  nop
              * 91 times
              33fb fe00  bra  ac          ;Trampoline for page $3300 lookups
              33fc fcfd  bra  $33fd
              33fd 1403  ld   $03,y
              33fe e07b  jmp  y,$7b
              33ff 1519  ld   [$19],y
jupiter45:    3400 0000  ld   $00
              3401 0000  ld   $00
              3402 0000  ld   $00
              * 20 times
              3414 0005  ld   $05
              3415 0006  ld   $06
              3416 0016  ld   $16
              3417 0006  ld   $06
              3418 0006  ld   $06
              3419 0016  ld   $16
              341a 001a  ld   $1a
              341b 001b  ld   $1b
              341c 001b  ld   $1b
              341d 001b  ld   $1b
              341e 002f  ld   $2f
              341f 001b  ld   $1b
              3420 001b  ld   $1b
              3421 002b  ld   $2b
              3422 001f  ld   $1f
              3423 002b  ld   $2b
              3424 001b  ld   $1b
              3425 001b  ld   $1b
              3426 001b  ld   $1b
              3427 001b  ld   $1b
              3428 0016  ld   $16
              3429 0017  ld   $17
              342a 001a  ld   $1a
              342b 001a  ld   $1a
              342c 0006  ld   $06
              342d 0016  ld   $16
              342e 001a  ld   $1a
              342f 002f  ld   $2f
              3430 001b  ld   $1b
              3431 002b  ld   $2b
              3432 002f  ld   $2f
              3433 002f  ld   $2f
              3434 001a  ld   $1a
              3435 001b  ld   $1b
              3436 001b  ld   $1b
              3437 001a  ld   $1a
              3438 001b  ld   $1b
              3439 001a  ld   $1a
              343a 001a  ld   $1a
              343b 001b  ld   $1b
              343c 001b  ld   $1b
              343d 001a  ld   $1a
              343e 001b  ld   $1b
              343f 002b  ld   $2b
              3440 001b  ld   $1b
              3441 001b  ld   $1b
              3442 001a  ld   $1a
              3443 001b  ld   $1b
              3444 002b  ld   $2b
              3445 001a  ld   $1a
              3446 002b  ld   $2b
              3447 001f  ld   $1f
              3448 002b  ld   $2b
              3449 002f  ld   $2f
              344a 002b  ld   $2b
              344b 001a  ld   $1a
              344c 001b  ld   $1b
              344d 001a  ld   $1a
              344e 002b  ld   $2b
              344f 001b  ld   $1b
              3450 001a  ld   $1a
              3451 001b  ld   $1b
              3452 001a  ld   $1a
              3453 0016  ld   $16
              3454 0016  ld   $16
              3455 001a  ld   $1a
              3456 0017  ld   $17
              3457 001a  ld   $1a
              3458 0017  ld   $17
              3459 001a  ld   $1a
              345a 0016  ld   $16
              345b 001a  ld   $1a
              345c 0016  ld   $16
              345d 001a  ld   $1a
              345e 0016  ld   $16
              345f 001a  ld   $1a
              3460 0016  ld   $16
              3461 0016  ld   $16
              3462 0006  ld   $06
              3463 0016  ld   $16
              3464 0006  ld   $06
              3465 0015  ld   $15
              3466 0006  ld   $06
              3467 0015  ld   $15
              3468 0006  ld   $06
              3469 0016  ld   $16
              346a 0016  ld   $16
              346b 0005  ld   $05
              346c 0016  ld   $16
              346d 0005  ld   $05
              346e 0016  ld   $16
              346f 0015  ld   $15
              3470 0006  ld   $06
              3471 0016  ld   $16
              3472 001a  ld   $1a
              3473 0016  ld   $16
              3474 0005  ld   $05
              3475 0016  ld   $16
              3476 0015  ld   $15
              3477 0006  ld   $06
              3478 0015  ld   $15
              3479 0006  ld   $06
              347a 0015  ld   $15
              347b 0001  ld   $01
              347c 0015  ld   $15
              347d 0001  ld   $01
              347e 0005  ld   $05
              347f 0001  ld   $01
              3480 0005  ld   $05
              3481 0001  ld   $01
              3482 0005  ld   $05
              3483 0001  ld   $01
              3484 0005  ld   $05
              3485 0015  ld   $15
              3486 0001  ld   $01
              3487 0005  ld   $05
              3488 0001  ld   $01
              3489 0015  ld   $15
              348a 0005  ld   $05
              348b 0000  ld   $00
              348c 0000  ld   $00
              348d 0000  ld   $00
              * 21 times
              34a0 0200  nop
              34a1 0200  nop
              34a2 0200  nop
              * 91 times
              34fb fe00  bra  ac          ;Trampoline for page $3400 lookups
              34fc fcfd  bra  $34fd
              34fd 1403  ld   $03,y
              34fe e07b  jmp  y,$7b
              34ff 1519  ld   [$19],y
jupiter46:    3500 0000  ld   $00
              3501 0000  ld   $00
              3502 0000  ld   $00
              * 20 times
              3514 0016  ld   $16
              3515 0016  ld   $16
              3516 0007  ld   $07
              3517 001b  ld   $1b
              3518 0006  ld   $06
              3519 0017  ld   $17
              351a 0006  ld   $06
              351b 001a  ld   $1a
              351c 001b  ld   $1b
              351d 001b  ld   $1b
              351e 001b  ld   $1b
              351f 001b  ld   $1b
              3520 002b  ld   $2b
              3521 001b  ld   $1b
              3522 001b  ld   $1b
              3523 001b  ld   $1b
              3524 001a  ld   $1a
              3525 001b  ld   $1b
              3526 001b  ld   $1b
              3527 0006  ld   $06
              3528 0006  ld   $06
              3529 001b  ld   $1b
              352a 001b  ld   $1b
              352b 0016  ld   $16
              352c 0006  ld   $06
              352d 0006  ld   $06
              352e 001b  ld   $1b
              352f 002b  ld   $2b
              3530 002f  ld   $2f
              3531 001b  ld   $1b
              3532 002b  ld   $2b
              3533 001b  ld   $1b
              3534 0016  ld   $16
              3535 001b  ld   $1b
              3536 001b  ld   $1b
              3537 0016  ld   $16
              3538 0007  ld   $07
              3539 0006  ld   $06
              353a 0006  ld   $06
              353b 001b  ld   $1b
              353c 002b  ld   $2b
              353d 0016  ld   $16
              353e 001a  ld   $1a
              353f 001b  ld   $1b
              3540 001a  ld   $1a
              3541 0016  ld   $16
              3542 0016  ld   $16
              3543 001b  ld   $1b
              3544 001b  ld   $1b
              3545 001a  ld   $1a
              3546 0016  ld   $16
              3547 001b  ld   $1b
              3548 001a  ld   $1a
              3549 001b  ld   $1b
              354a 001a  ld   $1a
              354b 001b  ld   $1b
              354c 002b  ld   $2b
              354d 001a  ld   $1a
              354e 001b  ld   $1b
              354f 001a  ld   $1a
              3550 001b  ld   $1b
              3551 0016  ld   $16
              3552 001b  ld   $1b
              3553 001a  ld   $1a
              3554 0006  ld   $06
              3555 0016  ld   $16
              3556 001a  ld   $1a
              3557 0016  ld   $16
              3558 001a  ld   $1a
              3559 0016  ld   $16
              355a 0006  ld   $06
              355b 001a  ld   $1a
              355c 0016  ld   $16
              355d 001b  ld   $1b
              355e 0016  ld   $16
              355f 0006  ld   $06
              3560 001a  ld   $1a
              3561 0006  ld   $06
              3562 001a  ld   $1a
              3563 0006  ld   $06
              3564 0016  ld   $16
              3565 001a  ld   $1a
              3566 0006  ld   $06
              3567 0015  ld   $15
              3568 0006  ld   $06
              3569 0015  ld   $15
              356a 0005  ld   $05
              356b 0005  ld   $05
              356c 0006  ld   $06
              356d 0005  ld   $05
              356e 0006  ld   $06
              356f 0005  ld   $05
              3570 0016  ld   $16
              3571 0005  ld   $05
              3572 0015  ld   $15
              3573 0005  ld   $05
              3574 0016  ld   $16
              3575 0005  ld   $05
              3576 0006  ld   $06
              3577 0015  ld   $15
              3578 0005  ld   $05
              3579 0011  ld   $11
              357a 0005  ld   $05
              357b 0001  ld   $01
              357c 0005  ld   $05
              357d 0001  ld   $01
              357e 0015  ld   $15
              357f 0001  ld   $01
              3580 0004  ld   $04
              3581 0001  ld   $01
              3582 0011  ld   $11
              3583 0005  ld   $05
              3584 0001  ld   $01
              3585 0005  ld   $05
              3586 0001  ld   $01
              3587 0015  ld   $15
              3588 0001  ld   $01
              3589 0004  ld   $04
              358a 0001  ld   $01
              358b 0000  ld   $00
              358c 0000  ld   $00
              358d 0000  ld   $00
              * 21 times
              35a0 0200  nop
              35a1 0200  nop
              35a2 0200  nop
              * 91 times
              35fb fe00  bra  ac          ;Trampoline for page $3500 lookups
              35fc fcfd  bra  $35fd
              35fd 1403  ld   $03,y
              35fe e07b  jmp  y,$7b
              35ff 1519  ld   [$19],y
jupiter47:    3600 0000  ld   $00
              3601 0000  ld   $00
              3602 0000  ld   $00
              * 19 times
              3613 0001  ld   $01
              3614 0005  ld   $05
              3615 0006  ld   $06
              3616 0006  ld   $06
              3617 0006  ld   $06
              3618 0016  ld   $16
              3619 0007  ld   $07
              361a 0016  ld   $16
              361b 0007  ld   $07
              361c 0016  ld   $16
              361d 001b  ld   $1b
              361e 0016  ld   $16
              361f 001b  ld   $1b
              3620 001a  ld   $1a
              3621 001a  ld   $1a
              3622 0017  ld   $17
              3623 001a  ld   $1a
              3624 001b  ld   $1b
              3625 0006  ld   $06
              3626 0016  ld   $16
              3627 0016  ld   $16
              3628 0007  ld   $07
              3629 0016  ld   $16
              362a 000b  ld   $0b
              362b 0016  ld   $16
              362c 000b  ld   $0b
              362d 0016  ld   $16
              362e 0016  ld   $16
              362f 0007  ld   $07
              3630 001a  ld   $1a
              3631 0016  ld   $16
              3632 000a  ld   $0a
              3633 0016  ld   $16
              3634 0006  ld   $06
              3635 001b  ld   $1b
              3636 0016  ld   $16
              3637 001b  ld   $1b
              3638 0016  ld   $16
              3639 0006  ld   $06
              363a 0006  ld   $06
              363b 0016  ld   $16
              363c 001b  ld   $1b
              363d 0006  ld   $06
              363e 0006  ld   $06
              363f 001b  ld   $1b
              3640 001b  ld   $1b
              3641 0007  ld   $07
              3642 001a  ld   $1a
              3643 0006  ld   $06
              3644 0006  ld   $06
              3645 0006  ld   $06
              3646 0006  ld   $06
              3647 0016  ld   $16
              3648 0016  ld   $16
              3649 0006  ld   $06
              364a 0006  ld   $06
              364b 001a  ld   $1a
              364c 0016  ld   $16
              364d 001b  ld   $1b
              364e 0017  ld   $17
              364f 001a  ld   $1a
              3650 0016  ld   $16
              3651 001b  ld   $1b
              3652 001a  ld   $1a
              3653 001b  ld   $1b
              3654 001b  ld   $1b
              3655 001a  ld   $1a
              3656 0006  ld   $06
              3657 0016  ld   $16
              3658 0006  ld   $06
              3659 001a  ld   $1a
              365a 0006  ld   $06
              365b 0016  ld   $16
              365c 0006  ld   $06
              365d 0016  ld   $16
              365e 000a  ld   $0a
              365f 0016  ld   $16
              3660 0016  ld   $16
              3661 0006  ld   $06
              3662 0016  ld   $16
              3663 0016  ld   $16
              3664 0005  ld   $05
              3665 0006  ld   $06
              3666 0016  ld   $16
              3667 0006  ld   $06
              3668 0016  ld   $16
              3669 0006  ld   $06
              366a 0005  ld   $05
              366b 0002  ld   $02
              366c 0005  ld   $05
              366d 0015  ld   $15
              366e 0005  ld   $05
              366f 0016  ld   $16
              3670 0005  ld   $05
              3671 0016  ld   $16
              3672 0016  ld   $16
              3673 0001  ld   $01
              3674 0005  ld   $05
              3675 0015  ld   $15
              3676 0006  ld   $06
              3677 0015  ld   $15
              3678 0005  ld   $05
              3679 0005  ld   $05
              367a 0005  ld   $05
              367b 0001  ld   $01
              367c 0005  ld   $05
              367d 0001  ld   $01
              367e 0004  ld   $04
              367f 0001  ld   $01
              3680 0005  ld   $05
              3681 0001  ld   $01
              3682 0004  ld   $04
              3683 0001  ld   $01
              3684 0005  ld   $05
              3685 0001  ld   $01
              3686 0005  ld   $05
              3687 0000  ld   $00
              3688 0005  ld   $05
              3689 0001  ld   $01
              368a 0005  ld   $05
              368b 0000  ld   $00
              368c 0000  ld   $00
              368d 0000  ld   $00
              * 21 times
              36a0 0200  nop
              36a1 0200  nop
              36a2 0200  nop
              * 91 times
              36fb fe00  bra  ac          ;Trampoline for page $3600 lookups
              36fc fcfd  bra  $36fd
              36fd 1403  ld   $03,y
              36fe e07b  jmp  y,$7b
              36ff 1519  ld   [$19],y
jupiter48:    3700 0000  ld   $00
              3701 0000  ld   $00
              3702 0000  ld   $00
              * 19 times
              3713 0001  ld   $01
              3714 0016  ld   $16
              3715 0016  ld   $16
              3716 0006  ld   $06
              3717 0016  ld   $16
              3718 0006  ld   $06
              3719 0006  ld   $06
              371a 001a  ld   $1a
              371b 0007  ld   $07
              371c 0016  ld   $16
              371d 0007  ld   $07
              371e 001a  ld   $1a
              371f 0007  ld   $07
              3720 0017  ld   $17
              3721 0006  ld   $06
              3722 001a  ld   $1a
              3723 0007  ld   $07
              3724 0016  ld   $16
              3725 0007  ld   $07
              3726 0006  ld   $06
              3727 001b  ld   $1b
              3728 001b  ld   $1b
              3729 0016  ld   $16
              372a 0007  ld   $07
              372b 0016  ld   $16
              372c 001b  ld   $1b
              372d 0006  ld   $06
              372e 001b  ld   $1b
              372f 0006  ld   $06
              3730 0006  ld   $06
              3731 0006  ld   $06
              * 5 times
              3734 0016  ld   $16
              3735 0006  ld   $06
              3736 0006  ld   $06
              3737 0006  ld   $06
              * 8 times
              373d 0016  ld   $16
              373e 0006  ld   $06
              373f 0016  ld   $16
              3740 0016  ld   $16
              3741 0006  ld   $06
              3742 0006  ld   $06
              3743 0016  ld   $16
              3744 0006  ld   $06
              3745 0016  ld   $16
              3746 0006  ld   $06
              3747 0006  ld   $06
              3748 0006  ld   $06
              3749 0016  ld   $16
              374a 0006  ld   $06
              374b 0006  ld   $06
              374c 0016  ld   $16
              374d 001a  ld   $1a
              374e 0017  ld   $17
              374f 001a  ld   $1a
              3750 0016  ld   $16
              3751 001a  ld   $1a
              3752 0016  ld   $16
              3753 0016  ld   $16
              3754 001a  ld   $1a
              3755 0016  ld   $16
              3756 001b  ld   $1b
              3757 0016  ld   $16
              3758 0006  ld   $06
              3759 0016  ld   $16
              375a 000a  ld   $0a
              375b 0016  ld   $16
              375c 0006  ld   $06
              375d 0006  ld   $06
              375e 0016  ld   $16
              375f 001a  ld   $1a
              3760 0006  ld   $06
              3761 0015  ld   $15
              3762 0016  ld   $16
              3763 0006  ld   $06
              3764 0016  ld   $16
              3765 0015  ld   $15
              3766 0006  ld   $06
              3767 0005  ld   $05
              3768 0015  ld   $15
              3769 0006  ld   $06
              376a 0015  ld   $15
              376b 0006  ld   $06
              376c 0005  ld   $05
              376d 0002  ld   $02
              376e 0005  ld   $05
              376f 0005  ld   $05
              3770 0005  ld   $05
              3771 0006  ld   $06
              3772 0015  ld   $15
              3773 0006  ld   $06
              3774 0015  ld   $15
              3775 0006  ld   $06
              3776 0011  ld   $11
              3777 0005  ld   $05
              3778 0001  ld   $01
              3779 0011  ld   $11
              377a 0005  ld   $05
              377b 0011  ld   $11
              377c 0005  ld   $05
              377d 0001  ld   $01
              377e 0001  ld   $01
              377f 0005  ld   $05
              3780 0011  ld   $11
              3781 0005  ld   $05
              3782 0001  ld   $01
              3783 0001  ld   $01
              3784 0000  ld   $00
              3785 0005  ld   $05
              3786 0001  ld   $01
              3787 0005  ld   $05
              3788 0000  ld   $00
              3789 0011  ld   $11
              378a 0005  ld   $05
              378b 0000  ld   $00
              378c 0000  ld   $00
              378d 0000  ld   $00
              * 21 times
              37a0 0200  nop
              37a1 0200  nop
              37a2 0200  nop
              * 91 times
              37fb fe00  bra  ac          ;Trampoline for page $3700 lookups
              37fc fcfd  bra  $37fd
              37fd 1403  ld   $03,y
              37fe e07b  jmp  y,$7b
              37ff 1519  ld   [$19],y
jupiter49:    3800 0000  ld   $00
              3801 0000  ld   $00
              3802 0000  ld   $00
              * 19 times
              3813 0011  ld   $11
              3814 0006  ld   $06
              3815 0016  ld   $16
              3816 0006  ld   $06
              3817 0016  ld   $16
              3818 0007  ld   $07
              3819 0016  ld   $16
              381a 0007  ld   $07
              381b 0016  ld   $16
              381c 001b  ld   $1b
              381d 001a  ld   $1a
              381e 0017  ld   $17
              381f 000a  ld   $0a
              3820 001b  ld   $1b
              3821 0017  ld   $17
              3822 001b  ld   $1b
              3823 0017  ld   $17
              3824 000a  ld   $0a
              3825 0016  ld   $16
              3826 001b  ld   $1b
              3827 0016  ld   $16
              3828 001b  ld   $1b
              3829 001b  ld   $1b
              382a 001b  ld   $1b
              382b 0017  ld   $17
              382c 001b  ld   $1b
              382d 001b  ld   $1b
              382e 001b  ld   $1b
              382f 0006  ld   $06
              3830 0016  ld   $16
              3831 0007  ld   $07
              3832 0016  ld   $16
              3833 0006  ld   $06
              3834 0006  ld   $06
              3835 0002  ld   $02
              3836 0006  ld   $06
              3837 0006  ld   $06
              3838 0006  ld   $06
              3839 0016  ld   $16
              383a 0016  ld   $16
              383b 0016  ld   $16
              383c 0006  ld   $06
              383d 0006  ld   $06
              383e 0006  ld   $06
              * 5 times
              3841 0016  ld   $16
              3842 0006  ld   $06
              3843 0006  ld   $06
              3844 0006  ld   $06
              * 5 times
              3847 0016  ld   $16
              3848 0006  ld   $06
              3849 0006  ld   $06
              384a 0016  ld   $16
              384b 0006  ld   $06
              384c 0006  ld   $06
              384d 0006  ld   $06
              384e 0006  ld   $06
              384f 0016  ld   $16
              3850 001b  ld   $1b
              3851 0006  ld   $06
              3852 0016  ld   $16
              3853 001b  ld   $1b
              3854 0006  ld   $06
              3855 0016  ld   $16
              3856 0006  ld   $06
              3857 0006  ld   $06
              3858 0016  ld   $16
              3859 0006  ld   $06
              385a 0016  ld   $16
              385b 0006  ld   $06
              385c 0016  ld   $16
              385d 001a  ld   $1a
              385e 0016  ld   $16
              385f 0006  ld   $06
              3860 0016  ld   $16
              3861 0006  ld   $06
              3862 0006  ld   $06
              3863 0015  ld   $15
              3864 0006  ld   $06
              3865 0006  ld   $06
              3866 0005  ld   $05
              3867 0006  ld   $06
              3868 0016  ld   $16
              3869 0006  ld   $06
              386a 0015  ld   $15
              386b 0006  ld   $06
              386c 0015  ld   $15
              386d 0005  ld   $05
              386e 0016  ld   $16
              386f 0005  ld   $05
              3870 0001  ld   $01
              3871 0005  ld   $05
              3872 0016  ld   $16
              3873 0005  ld   $05
              3874 0005  ld   $05
              3875 0005  ld   $05
              3876 0016  ld   $16
              3877 0005  ld   $05
              3878 0005  ld   $05
              3879 0001  ld   $01
              387a 0005  ld   $05
              387b 0001  ld   $01
              387c 0005  ld   $05
              387d 0011  ld   $11
              387e 0005  ld   $05
              387f 0001  ld   $01
              3880 0005  ld   $05
              3881 0000  ld   $00
              3882 0005  ld   $05
              3883 0005  ld   $05
              3884 0001  ld   $01
              3885 0005  ld   $05
              3886 0011  ld   $11
              3887 0000  ld   $00
              3888 0005  ld   $05
              3889 0001  ld   $01
              388a 0015  ld   $15
              388b 0000  ld   $00
              388c 0001  ld   $01
              388d 0000  ld   $00
              388e 0000  ld   $00
              388f 0000  ld   $00
              * 19 times
              38a0 0200  nop
              38a1 0200  nop
              38a2 0200  nop
              * 91 times
              38fb fe00  bra  ac          ;Trampoline for page $3800 lookups
              38fc fcfd  bra  $38fd
              38fd 1403  ld   $03,y
              38fe e07b  jmp  y,$7b
              38ff 1519  ld   [$19],y
jupiter50:    3900 0000  ld   $00
              3901 0000  ld   $00
              3902 0000  ld   $00
              * 19 times
              3913 0005  ld   $05
              3914 0016  ld   $16
              3915 0006  ld   $06
              3916 0006  ld   $06
              3917 0017  ld   $17
              3918 000a  ld   $0a
              3919 001b  ld   $1b
              391a 0006  ld   $06
              391b 001b  ld   $1b
              391c 0007  ld   $07
              391d 0017  ld   $17
              391e 000a  ld   $0a
              391f 0017  ld   $17
              3920 001b  ld   $1b
              3921 000a  ld   $0a
              3922 0017  ld   $17
              3923 0006  ld   $06
              3924 001b  ld   $1b
              3925 0007  ld   $07
              3926 0006  ld   $06
              3927 001b  ld   $1b
              3928 0007  ld   $07
              3929 0016  ld   $16
              392a 0006  ld   $06
              392b 0006  ld   $06
              392c 0017  ld   $17
              392d 0006  ld   $06
              392e 001b  ld   $1b
              392f 0017  ld   $17
              3930 000a  ld   $0a
              3931 0016  ld   $16
              3932 001b  ld   $1b
              3933 0016  ld   $16
              3934 0016  ld   $16
              3935 001a  ld   $1a
              3936 0016  ld   $16
              3937 0016  ld   $16
              3938 001b  ld   $1b
              3939 002b  ld   $2b
              393a 001b  ld   $1b
              393b 002f  ld   $2f
              393c 001a  ld   $1a
              393d 0016  ld   $16
              393e 0006  ld   $06
              393f 0006  ld   $06
              3940 0006  ld   $06
              * 5 times
              3943 0016  ld   $16
              3944 0006  ld   $06
              3945 0006  ld   $06
              3946 0016  ld   $16
              3947 0006  ld   $06
              3948 0006  ld   $06
              3949 0016  ld   $16
              394a 0006  ld   $06
              394b 0006  ld   $06
              394c 0016  ld   $16
              394d 0006  ld   $06
              394e 0006  ld   $06
              394f 0006  ld   $06
              3950 0006  ld   $06
              3951 0016  ld   $16
              3952 0006  ld   $06
              3953 0006  ld   $06
              3954 0016  ld   $16
              3955 0006  ld   $06
              3956 001a  ld   $1a
              3957 0006  ld   $06
              3958 0016  ld   $16
              3959 0006  ld   $06
              395a 0006  ld   $06
              395b 0016  ld   $16
              395c 000a  ld   $0a
              395d 0016  ld   $16
              395e 0006  ld   $06
              395f 0016  ld   $16
              3960 0005  ld   $05
              3961 001a  ld   $1a
              3962 0016  ld   $16
              3963 0006  ld   $06
              3964 0015  ld   $15
              3965 0016  ld   $16
              3966 0006  ld   $06
              3967 0015  ld   $15
              3968 0006  ld   $06
              3969 0016  ld   $16
              396a 0006  ld   $06
              396b 0015  ld   $15
              396c 0006  ld   $06
              396d 0005  ld   $05
              396e 0005  ld   $05
              396f 0012  ld   $12
              3970 0005  ld   $05
              3971 0005  ld   $05
              3972 0001  ld   $01
              3973 0005  ld   $05
              3974 0012  ld   $12
              3975 0005  ld   $05
              3976 0005  ld   $05
              3977 0016  ld   $16
              3978 0015  ld   $15
              3979 0005  ld   $05
              397a 0001  ld   $01
              397b 0005  ld   $05
              397c 0000  ld   $00
              397d 0001  ld   $01
              397e 0004  ld   $04
              397f 0001  ld   $01
              3980 0001  ld   $01
              3981 0005  ld   $05
              3982 0011  ld   $11
              3983 0004  ld   $04
              3984 0001  ld   $01
              3985 0005  ld   $05
              3986 0001  ld   $01
              3987 0005  ld   $05
              3988 0011  ld   $11
              3989 0004  ld   $04
              398a 0005  ld   $05
              398b 0011  ld   $11
              398c 0000  ld   $00
              398d 0000  ld   $00
              398e 0000  ld   $00
              * 20 times
              39a0 0200  nop
              39a1 0200  nop
              39a2 0200  nop
              * 91 times
              39fb fe00  bra  ac          ;Trampoline for page $3900 lookups
              39fc fcfd  bra  $39fd
              39fd 1403  ld   $03,y
              39fe e07b  jmp  y,$7b
              39ff 1519  ld   [$19],y
jupiter51:    3a00 0000  ld   $00
              3a01 0000  ld   $00
              3a02 0000  ld   $00
              * 19 times
              3a13 0015  ld   $15
              3a14 0006  ld   $06
              3a15 0016  ld   $16
              3a16 0006  ld   $06
              3a17 0006  ld   $06
              3a18 0016  ld   $16
              3a19 001b  ld   $1b
              3a1a 0017  ld   $17
              3a1b 001a  ld   $1a
              3a1c 001b  ld   $1b
              3a1d 001a  ld   $1a
              3a1e 0017  ld   $17
              3a1f 001b  ld   $1b
              3a20 001b  ld   $1b
              3a21 001b  ld   $1b
              3a22 0006  ld   $06
              3a23 001b  ld   $1b
              3a24 0006  ld   $06
              3a25 001b  ld   $1b
              3a26 0016  ld   $16
              3a27 001b  ld   $1b
              3a28 0016  ld   $16
              3a29 000b  ld   $0b
              3a2a 001b  ld   $1b
              3a2b 0016  ld   $16
              3a2c 000a  ld   $0a
              3a2d 0017  ld   $17
              3a2e 0006  ld   $06
              3a2f 001a  ld   $1a
              3a30 0006  ld   $06
              3a31 0017  ld   $17
              3a32 0006  ld   $06
              3a33 001b  ld   $1b
              3a34 001b  ld   $1b
              3a35 002b  ld   $2b
              3a36 001b  ld   $1b
              3a37 002f  ld   $2f
              3a38 002b  ld   $2b
              3a39 002f  ld   $2f
              3a3a 002f  ld   $2f
              3a3b 003f  ld   $3f
              3a3c 002b  ld   $2b
              3a3d 001b  ld   $1b
              3a3e 001b  ld   $1b
              3a3f 0016  ld   $16
              3a40 0016  ld   $16
              3a41 0006  ld   $06
              3a42 0006  ld   $06
              3a43 0016  ld   $16
              3a44 0006  ld   $06
              3a45 001b  ld   $1b
              3a46 0016  ld   $16
              3a47 001b  ld   $1b
              3a48 001a  ld   $1a
              3a49 000b  ld   $0b
              3a4a 0016  ld   $16
              3a4b 0016  ld   $16
              3a4c 0016  ld   $16
              * 6 times
              3a50 0006  ld   $06
              3a51 0016  ld   $16
              3a52 0006  ld   $06
              3a53 0016  ld   $16
              3a54 0006  ld   $06
              3a55 0006  ld   $06
              3a56 0006  ld   $06
              3a57 0016  ld   $16
              3a58 0006  ld   $06
              3a59 0015  ld   $15
              3a5a 0006  ld   $06
              3a5b 0016  ld   $16
              3a5c 0006  ld   $06
              3a5d 0016  ld   $16
              3a5e 000a  ld   $0a
              3a5f 0016  ld   $16
              3a60 0006  ld   $06
              3a61 0016  ld   $16
              3a62 0006  ld   $06
              3a63 001a  ld   $1a
              3a64 0006  ld   $06
              3a65 0016  ld   $16
              3a66 0005  ld   $05
              3a67 0016  ld   $16
              3a68 000a  ld   $0a
              3a69 0015  ld   $15
              3a6a 0016  ld   $16
              3a6b 0005  ld   $05
              3a6c 0016  ld   $16
              3a6d 0006  ld   $06
              3a6e 0015  ld   $15
              3a6f 0005  ld   $05
              3a70 0006  ld   $06
              3a71 0015  ld   $15
              3a72 0005  ld   $05
              3a73 0001  ld   $01
              3a74 0005  ld   $05
              3a75 0005  ld   $05
              3a76 0001  ld   $01
              3a77 0005  ld   $05
              3a78 0001  ld   $01
              3a79 0015  ld   $15
              3a7a 0005  ld   $05
              3a7b 0011  ld   $11
              3a7c 0005  ld   $05
              3a7d 0005  ld   $05
              3a7e 0001  ld   $01
              3a7f 0011  ld   $11
              3a80 0005  ld   $05
              3a81 0000  ld   $00
              3a82 0005  ld   $05
              3a83 0001  ld   $01
              3a84 0015  ld   $15
              3a85 0005  ld   $05
              3a86 0015  ld   $15
              3a87 0000  ld   $00
              3a88 0005  ld   $05
              3a89 0011  ld   $11
              3a8a 0005  ld   $05
              3a8b 0015  ld   $15
              3a8c 0004  ld   $04
              3a8d 0000  ld   $00
              3a8e 0000  ld   $00
              3a8f 0000  ld   $00
              * 19 times
              3aa0 0200  nop
              3aa1 0200  nop
              3aa2 0200  nop
              * 91 times
              3afb fe00  bra  ac          ;Trampoline for page $3a00 lookups
              3afc fcfd  bra  $3afd
              3afd 1403  ld   $03,y
              3afe e07b  jmp  y,$7b
              3aff 1519  ld   [$19],y
jupiter52:    3b00 0000  ld   $00
              3b01 0000  ld   $00
              3b02 0000  ld   $00
              * 18 times
              3b12 0001  ld   $01
              3b13 0016  ld   $16
              3b14 0006  ld   $06
              3b15 0017  ld   $17
              3b16 000a  ld   $0a
              3b17 0017  ld   $17
              3b18 001b  ld   $1b
              3b19 001b  ld   $1b
              3b1a 001b  ld   $1b
              * 13 times
              3b25 001a  ld   $1a
              3b26 001b  ld   $1b
              3b27 001b  ld   $1b
              3b28 001b  ld   $1b
              3b29 001b  ld   $1b
              3b2a 0016  ld   $16
              3b2b 001b  ld   $1b
              3b2c 001b  ld   $1b
              3b2d 0016  ld   $16
              3b2e 001b  ld   $1b
              3b2f 0017  ld   $17
              3b30 001a  ld   $1a
              3b31 001b  ld   $1b
              3b32 001a  ld   $1a
              3b33 0017  ld   $17
              3b34 001b  ld   $1b
              3b35 002f  ld   $2f
              3b36 002f  ld   $2f
              3b37 002b  ld   $2b
              3b38 002f  ld   $2f
              3b39 002f  ld   $2f
              3b3a 002b  ld   $2b
              3b3b 002f  ld   $2f
              3b3c 002b  ld   $2b
              3b3d 002a  ld   $2a
              3b3e 001b  ld   $1b
              3b3f 000a  ld   $0a
              3b40 0017  ld   $17
              3b41 0016  ld   $16
              3b42 001b  ld   $1b
              3b43 0016  ld   $16
              3b44 001b  ld   $1b
              3b45 001a  ld   $1a
              3b46 001b  ld   $1b
              3b47 0017  ld   $17
              3b48 001a  ld   $1a
              3b49 0017  ld   $17
              3b4a 001b  ld   $1b
              3b4b 001b  ld   $1b
              3b4c 001b  ld   $1b
              3b4d 001a  ld   $1a
              3b4e 001b  ld   $1b
              3b4f 001a  ld   $1a
              3b50 001b  ld   $1b
              3b51 001a  ld   $1a
              3b52 001b  ld   $1b
              3b53 0016  ld   $16
              3b54 0016  ld   $16
              3b55 0016  ld   $16
              3b56 0006  ld   $06
              3b57 001a  ld   $1a
              3b58 0006  ld   $06
              3b59 001a  ld   $1a
              3b5a 0006  ld   $06
              3b5b 0016  ld   $16
              3b5c 0006  ld   $06
              3b5d 0016  ld   $16
              3b5e 0016  ld   $16
              3b5f 0006  ld   $06
              3b60 0019  ld   $19
              3b61 0016  ld   $16
              3b62 0006  ld   $06
              3b63 0016  ld   $16
              3b64 001a  ld   $1a
              3b65 0016  ld   $16
              3b66 0016  ld   $16
              3b67 0006  ld   $06
              3b68 0015  ld   $15
              3b69 0016  ld   $16
              3b6a 000a  ld   $0a
              3b6b 0016  ld   $16
              3b6c 0015  ld   $15
              3b6d 0016  ld   $16
              3b6e 0016  ld   $16
              3b6f 0005  ld   $05
              3b70 0016  ld   $16
              3b71 0005  ld   $05
              3b72 0016  ld   $16
              3b73 0005  ld   $05
              3b74 0016  ld   $16
              3b75 0005  ld   $05
              3b76 0015  ld   $15
              3b77 0005  ld   $05
              3b78 0005  ld   $05
              3b79 0005  ld   $05
              3b7a 0015  ld   $15
              3b7b 0005  ld   $05
              3b7c 0001  ld   $01
              3b7d 0005  ld   $05
              3b7e 0005  ld   $05
              3b7f 0005  ld   $05
              3b80 0011  ld   $11
              3b81 0005  ld   $05
              3b82 0015  ld   $15
              3b83 0015  ld   $15
              3b84 0005  ld   $05
              3b85 0015  ld   $15
              3b86 0001  ld   $01
              3b87 0005  ld   $05
              3b88 0005  ld   $05
              3b89 0005  ld   $05
              3b8a 0015  ld   $15
              3b8b 0001  ld   $01
              3b8c 0000  ld   $00
              3b8d 0000  ld   $00
              3b8e 0000  ld   $00
              * 20 times
              3ba0 0200  nop
              3ba1 0200  nop
              3ba2 0200  nop
              * 91 times
              3bfb fe00  bra  ac          ;Trampoline for page $3b00 lookups
              3bfc fcfd  bra  $3bfd
              3bfd 1403  ld   $03,y
              3bfe e07b  jmp  y,$7b
              3bff 1519  ld   [$19],y
jupiter53:    3c00 0000  ld   $00
              3c01 0000  ld   $00
              3c02 0000  ld   $00
              * 18 times
              3c12 0004  ld   $04
              3c13 001b  ld   $1b
              3c14 001a  ld   $1a
              3c15 001b  ld   $1b
              3c16 001b  ld   $1b
              3c17 001b  ld   $1b
              3c18 001a  ld   $1a
              3c19 001b  ld   $1b
              3c1a 001b  ld   $1b
              3c1b 001b  ld   $1b
              3c1c 002b  ld   $2b
              3c1d 001f  ld   $1f
              3c1e 002b  ld   $2b
              3c1f 001f  ld   $1f
              3c20 002b  ld   $2b
              3c21 001b  ld   $1b
              3c22 002b  ld   $2b
              3c23 001b  ld   $1b
              3c24 001b  ld   $1b
              3c25 001b  ld   $1b
              * 18 times
              3c35 002b  ld   $2b
              3c36 002f  ld   $2f
              3c37 002f  ld   $2f
              3c38 002f  ld   $2f
              * 7 times
              3c3d 001f  ld   $1f
              3c3e 0016  ld   $16
              3c3f 001b  ld   $1b
              3c40 0016  ld   $16
              3c41 001b  ld   $1b
              3c42 001a  ld   $1a
              3c43 001b  ld   $1b
              3c44 001b  ld   $1b
              3c45 001a  ld   $1a
              3c46 001b  ld   $1b
              3c47 001a  ld   $1a
              3c48 001b  ld   $1b
              3c49 001b  ld   $1b
              3c4a 001a  ld   $1a
              3c4b 001b  ld   $1b
              3c4c 001a  ld   $1a
              3c4d 001b  ld   $1b
              3c4e 001a  ld   $1a
              3c4f 0017  ld   $17
              3c50 001a  ld   $1a
              3c51 0017  ld   $17
              3c52 001a  ld   $1a
              3c53 001a  ld   $1a
              3c54 001a  ld   $1a
              3c55 001b  ld   $1b
              3c56 001a  ld   $1a
              3c57 001a  ld   $1a
              3c58 001b  ld   $1b
              3c59 001a  ld   $1a
              3c5a 001a  ld   $1a
              3c5b 001a  ld   $1a
              * 5 times
              3c5e 0006  ld   $06
              3c5f 001a  ld   $1a
              3c60 0016  ld   $16
              3c61 000a  ld   $0a
              3c62 0016  ld   $16
              3c63 001a  ld   $1a
              3c64 0006  ld   $06
              3c65 0016  ld   $16
              3c66 000a  ld   $0a
              3c67 0016  ld   $16
              3c68 001a  ld   $1a
              3c69 0016  ld   $16
              3c6a 0015  ld   $15
              3c6b 001a  ld   $1a
              3c6c 0006  ld   $06
              3c6d 001a  ld   $1a
              3c6e 0005  ld   $05
              3c6f 0016  ld   $16
              3c70 0015  ld   $15
              3c71 0016  ld   $16
              3c72 0005  ld   $05
              3c73 0016  ld   $16
              3c74 0015  ld   $15
              3c75 0006  ld   $06
              3c76 0015  ld   $15
              3c77 0002  ld   $02
              3c78 0015  ld   $15
              3c79 0006  ld   $06
              3c7a 0001  ld   $01
              3c7b 0015  ld   $15
              3c7c 0005  ld   $05
              3c7d 0015  ld   $15
              3c7e 0011  ld   $11
              3c7f 0005  ld   $05
              3c80 0015  ld   $15
              3c81 0005  ld   $05
              3c82 0015  ld   $15
              3c83 0005  ld   $05
              3c84 0015  ld   $15
              3c85 0015  ld   $15
              3c86 0005  ld   $05
              3c87 0011  ld   $11
              3c88 0005  ld   $05
              3c89 0011  ld   $11
              3c8a 0005  ld   $05
              3c8b 0015  ld   $15
              3c8c 0005  ld   $05
              3c8d 0000  ld   $00
              3c8e 0000  ld   $00
              3c8f 0000  ld   $00
              * 19 times
              3ca0 0200  nop
              3ca1 0200  nop
              3ca2 0200  nop
              * 91 times
              3cfb fe00  bra  ac          ;Trampoline for page $3c00 lookups
              3cfc fcfd  bra  $3cfd
              3cfd 1403  ld   $03,y
              3cfe e07b  jmp  y,$7b
              3cff 1519  ld   [$19],y
jupiter54:    3d00 0000  ld   $00
              3d01 0000  ld   $00
              3d02 0000  ld   $00
              * 18 times
              3d12 0011  ld   $11
              3d13 002f  ld   $2f
              3d14 001b  ld   $1b
              3d15 0017  ld   $17
              3d16 001a  ld   $1a
              3d17 001b  ld   $1b
              3d18 001b  ld   $1b
              3d19 001b  ld   $1b
              3d1a 002b  ld   $2b
              3d1b 001f  ld   $1f
              3d1c 002b  ld   $2b
              3d1d 001f  ld   $1f
              3d1e 002b  ld   $2b
              3d1f 002f  ld   $2f
              3d20 001b  ld   $1b
              3d21 002f  ld   $2f
              3d22 001b  ld   $1b
              3d23 002b  ld   $2b
              3d24 001b  ld   $1b
              3d25 002b  ld   $2b
              3d26 001b  ld   $1b
              3d27 002b  ld   $2b
              3d28 001b  ld   $1b
              3d29 002f  ld   $2f
              3d2a 001b  ld   $1b
              3d2b 002b  ld   $2b
              3d2c 001f  ld   $1f
              3d2d 002b  ld   $2b
              3d2e 001b  ld   $1b
              3d2f 001b  ld   $1b
              3d30 002b  ld   $2b
              3d31 001b  ld   $1b
              3d32 001b  ld   $1b
              3d33 002b  ld   $2b
              3d34 002f  ld   $2f
              3d35 002f  ld   $2f
              3d36 002b  ld   $2b
              3d37 002f  ld   $2f
              3d38 002b  ld   $2b
              3d39 002f  ld   $2f
              3d3a 002b  ld   $2b
              3d3b 002b  ld   $2b
              3d3c 001b  ld   $1b
              3d3d 0016  ld   $16
              3d3e 001b  ld   $1b
              3d3f 0006  ld   $06
              3d40 001b  ld   $1b
              3d41 001b  ld   $1b
              3d42 001b  ld   $1b
              * 7 times
              3d47 002a  ld   $2a
              3d48 001b  ld   $1b
              3d49 002a  ld   $2a
              3d4a 001b  ld   $1b
              3d4b 002b  ld   $2b
              3d4c 001b  ld   $1b
              3d4d 002b  ld   $2b
              3d4e 001b  ld   $1b
              3d4f 002a  ld   $2a
              3d50 001b  ld   $1b
              3d51 002a  ld   $2a
              3d52 001b  ld   $1b
              3d53 002b  ld   $2b
              3d54 001b  ld   $1b
              3d55 002a  ld   $2a
              3d56 001b  ld   $1b
              3d57 002a  ld   $2a
              3d58 001a  ld   $1a
              3d59 0017  ld   $17
              3d5a 001a  ld   $1a
              3d5b 0016  ld   $16
              3d5c 0016  ld   $16
              3d5d 001a  ld   $1a
              3d5e 001a  ld   $1a
              3d5f 0016  ld   $16
              3d60 000a  ld   $0a
              3d61 0016  ld   $16
              3d62 001a  ld   $1a
              3d63 0016  ld   $16
              3d64 001a  ld   $1a
              3d65 0016  ld   $16
              3d66 001a  ld   $1a
              3d67 0015  ld   $15
              3d68 000a  ld   $0a
              3d69 0016  ld   $16
              3d6a 001a  ld   $1a
              3d6b 0016  ld   $16
              3d6c 0015  ld   $15
              3d6d 0016  ld   $16
              3d6e 0016  ld   $16
              3d6f 0019  ld   $19
              3d70 0006  ld   $06
              3d71 0015  ld   $15
              3d72 0016  ld   $16
              3d73 0015  ld   $15
              3d74 0006  ld   $06
              3d75 0015  ld   $15
              3d76 0016  ld   $16
              3d77 0005  ld   $05
              3d78 0015  ld   $15
              3d79 0015  ld   $15
              3d7a 0015  ld   $15
              3d7b 0006  ld   $06
              3d7c 0015  ld   $15
              3d7d 0005  ld   $05
              3d7e 0005  ld   $05
              3d7f 0015  ld   $15
              3d80 0005  ld   $05
              3d81 0015  ld   $15
              3d82 0001  ld   $01
              3d83 0016  ld   $16
              3d84 0005  ld   $05
              3d85 0015  ld   $15
              3d86 0005  ld   $05
              3d87 0015  ld   $15
              3d88 0001  ld   $01
              3d89 0005  ld   $05
              3d8a 0015  ld   $15
              3d8b 0015  ld   $15
              3d8c 0000  ld   $00
              3d8d 0000  ld   $00
              3d8e 0000  ld   $00
              * 20 times
              3da0 0200  nop
              3da1 0200  nop
              3da2 0200  nop
              * 91 times
              3dfb fe00  bra  ac          ;Trampoline for page $3d00 lookups
              3dfc fcfd  bra  $3dfd
              3dfd 1403  ld   $03,y
              3dfe e07b  jmp  y,$7b
              3dff 1519  ld   [$19],y
jupiter55:    3e00 0000  ld   $00
              3e01 0000  ld   $00
              3e02 0000  ld   $00
              * 18 times
              3e12 0005  ld   $05
              3e13 002b  ld   $2b
              3e14 001a  ld   $1a
              3e15 001b  ld   $1b
              3e16 001b  ld   $1b
              3e17 001b  ld   $1b
              3e18 001f  ld   $1f
              3e19 002b  ld   $2b
              3e1a 001f  ld   $1f
              3e1b 002b  ld   $2b
              3e1c 001f  ld   $1f
              3e1d 002b  ld   $2b
              3e1e 002f  ld   $2f
              3e1f 001b  ld   $1b
              3e20 002f  ld   $2f
              3e21 002b  ld   $2b
              3e22 001f  ld   $1f
              3e23 001b  ld   $1b
              3e24 002b  ld   $2b
              3e25 001f  ld   $1f
              3e26 001b  ld   $1b
              3e27 001b  ld   $1b
              3e28 001f  ld   $1f
              3e29 002b  ld   $2b
              3e2a 001b  ld   $1b
              3e2b 002f  ld   $2f
              3e2c 001b  ld   $1b
              3e2d 001b  ld   $1b
              3e2e 001b  ld   $1b
              3e2f 002f  ld   $2f
              3e30 001b  ld   $1b
              3e31 001f  ld   $1f
              3e32 002b  ld   $2b
              3e33 001f  ld   $1f
              3e34 001b  ld   $1b
              3e35 002b  ld   $2b
              3e36 002f  ld   $2f
              3e37 002f  ld   $2f
              3e38 002f  ld   $2f
              3e39 002b  ld   $2b
              3e3a 002f  ld   $2f
              3e3b 001a  ld   $1a
              3e3c 001b  ld   $1b
              3e3d 001b  ld   $1b
              3e3e 001a  ld   $1a
              3e3f 001b  ld   $1b
              3e40 001b  ld   $1b
              3e41 001b  ld   $1b
              3e42 001a  ld   $1a
              3e43 002b  ld   $2b
              3e44 001b  ld   $1b
              3e45 002a  ld   $2a
              3e46 001b  ld   $1b
              3e47 002b  ld   $2b
              3e48 001b  ld   $1b
              3e49 002b  ld   $2b
              3e4a 001a  ld   $1a
              3e4b 002b  ld   $2b
              3e4c 001a  ld   $1a
              3e4d 001b  ld   $1b
              3e4e 002a  ld   $2a
              3e4f 001b  ld   $1b
              3e50 002b  ld   $2b
              3e51 001b  ld   $1b
              3e52 002a  ld   $2a
              3e53 001b  ld   $1b
              3e54 002a  ld   $2a
              3e55 001a  ld   $1a
              3e56 001b  ld   $1b
              3e57 001a  ld   $1a
              3e58 001b  ld   $1b
              3e59 001a  ld   $1a
              3e5a 0016  ld   $16
              3e5b 001a  ld   $1a
              3e5c 001a  ld   $1a
              3e5d 0016  ld   $16
              3e5e 001a  ld   $1a
              3e5f 0016  ld   $16
              3e60 001a  ld   $1a
              3e61 001a  ld   $1a
              3e62 0016  ld   $16
              3e63 001a  ld   $1a
              3e64 0006  ld   $06
              3e65 001a  ld   $1a
              3e66 0016  ld   $16
              3e67 001a  ld   $1a
              3e68 0016  ld   $16
              3e69 001a  ld   $1a
              3e6a 0016  ld   $16
              3e6b 001a  ld   $1a
              3e6c 001a  ld   $1a
              3e6d 0016  ld   $16
              3e6e 0019  ld   $19
              3e6f 0016  ld   $16
              3e70 001a  ld   $1a
              3e71 0016  ld   $16
              3e72 0016  ld   $16
              3e73 0009  ld   $09
              3e74 0016  ld   $16
              3e75 0015  ld   $15
              3e76 0006  ld   $06
              3e77 0015  ld   $15
              3e78 0005  ld   $05
              3e79 0016  ld   $16
              3e7a 0005  ld   $05
              3e7b 0015  ld   $15
              3e7c 0005  ld   $05
              3e7d 0015  ld   $15
              3e7e 0015  ld   $15
              3e7f 0001  ld   $01
              3e80 0015  ld   $15
              3e81 0016  ld   $16
              3e82 0005  ld   $05
              3e83 0015  ld   $15
              3e84 0015  ld   $15
              3e85 0005  ld   $05
              3e86 0015  ld   $15
              3e87 0001  ld   $01
              3e88 0005  ld   $05
              3e89 0015  ld   $15
              3e8a 0005  ld   $05
              3e8b 0015  ld   $15
              3e8c 0015  ld   $15
              3e8d 0000  ld   $00
              3e8e 0000  ld   $00
              3e8f 0000  ld   $00
              * 19 times
              3ea0 0200  nop
              3ea1 0200  nop
              3ea2 0200  nop
              * 91 times
              3efb fe00  bra  ac          ;Trampoline for page $3e00 lookups
              3efc fcfd  bra  $3efd
              3efd 1403  ld   $03,y
              3efe e07b  jmp  y,$7b
              3eff 1519  ld   [$19],y
jupiter56:    3f00 0000  ld   $00
              3f01 0000  ld   $00
              3f02 0000  ld   $00
              * 18 times
              3f12 0015  ld   $15
              3f13 001b  ld   $1b
              3f14 001b  ld   $1b
              3f15 001a  ld   $1a
              3f16 002b  ld   $2b
              3f17 001b  ld   $1b
              3f18 002b  ld   $2b
              3f19 002f  ld   $2f
              3f1a 001b  ld   $1b
              3f1b 002f  ld   $2f
              3f1c 002f  ld   $2f
              3f1d 002b  ld   $2b
              3f1e 001f  ld   $1f
              3f1f 002f  ld   $2f
              3f20 001b  ld   $1b
              3f21 001a  ld   $1a
              3f22 002b  ld   $2b
              3f23 001b  ld   $1b
              3f24 001b  ld   $1b
              3f25 001b  ld   $1b
              3f26 001a  ld   $1a
              3f27 002b  ld   $2b
              3f28 001b  ld   $1b
              3f29 001b  ld   $1b
              3f2a 002f  ld   $2f
              3f2b 001b  ld   $1b
              3f2c 002f  ld   $2f
              3f2d 001b  ld   $1b
              3f2e 002f  ld   $2f
              3f2f 001b  ld   $1b
              3f30 001b  ld   $1b
              3f31 002b  ld   $2b
              3f32 001b  ld   $1b
              3f33 002b  ld   $2b
              3f34 002f  ld   $2f
              3f35 001b  ld   $1b
              3f36 002b  ld   $2b
              3f37 002b  ld   $2b
              3f38 002f  ld   $2f
              3f39 002b  ld   $2b
              3f3a 001b  ld   $1b
              3f3b 001b  ld   $1b
              3f3c 001b  ld   $1b
              * 5 times
              3f3f 002b  ld   $2b
              3f40 001b  ld   $1b
              3f41 002b  ld   $2b
              3f42 001b  ld   $1b
              3f43 002b  ld   $2b
              3f44 001b  ld   $1b
              3f45 002f  ld   $2f
              3f46 001b  ld   $1b
              3f47 002f  ld   $2f
              3f48 001b  ld   $1b
              3f49 002f  ld   $2f
              3f4a 001b  ld   $1b
              3f4b 002f  ld   $2f
              3f4c 001b  ld   $1b
              3f4d 002e  ld   $2e
              3f4e 001b  ld   $1b
              3f4f 002b  ld   $2b
              3f50 001e  ld   $1e
              3f51 002b  ld   $2b
              3f52 001b  ld   $1b
              3f53 001a  ld   $1a
              3f54 001b  ld   $1b
              3f55 002a  ld   $2a
              3f56 001b  ld   $1b
              3f57 0016  ld   $16
              3f58 001a  ld   $1a
              3f59 0016  ld   $16
              3f5a 001b  ld   $1b
              3f5b 001a  ld   $1a
              3f5c 0017  ld   $17
              3f5d 001a  ld   $1a
              3f5e 001b  ld   $1b
              3f5f 0016  ld   $16
              3f60 001a  ld   $1a
              3f61 0016  ld   $16
              3f62 001a  ld   $1a
              3f63 0016  ld   $16
              3f64 001a  ld   $1a
              3f65 0016  ld   $16
              3f66 001a  ld   $1a
              3f67 0016  ld   $16
              3f68 001a  ld   $1a
              3f69 001a  ld   $1a
              3f6a 0016  ld   $16
              3f6b 0019  ld   $19
              3f6c 0016  ld   $16
              3f6d 001a  ld   $1a
              3f6e 0006  ld   $06
              3f6f 0015  ld   $15
              3f70 0016  ld   $16
              3f71 0005  ld   $05
              3f72 0015  ld   $15
              3f73 0016  ld   $16
              3f74 0005  ld   $05
              3f75 0015  ld   $15
              3f76 0005  ld   $05
              3f77 0016  ld   $16
              3f78 0005  ld   $05
              3f79 0015  ld   $15
              3f7a 0005  ld   $05
              3f7b 0015  ld   $15
              3f7c 0005  ld   $05
              3f7d 0015  ld   $15
              3f7e 0005  ld   $05
              3f7f 0015  ld   $15
              3f80 0005  ld   $05
              3f81 0015  ld   $15
              3f82 0015  ld   $15
              3f83 0005  ld   $05
              3f84 0015  ld   $15
              3f85 0005  ld   $05
              3f86 0001  ld   $01
              3f87 0015  ld   $15
              3f88 0005  ld   $05
              3f89 0015  ld   $15
              3f8a 0001  ld   $01
              3f8b 0015  ld   $15
              3f8c 0001  ld   $01
              3f8d 0000  ld   $00
              3f8e 0000  ld   $00
              3f8f 0000  ld   $00
              * 19 times
              3fa0 0200  nop
              3fa1 0200  nop
              3fa2 0200  nop
              * 91 times
              3ffb fe00  bra  ac          ;Trampoline for page $3f00 lookups
              3ffc fcfd  bra  $3ffd
              3ffd 1403  ld   $03,y
              3ffe e07b  jmp  y,$7b
              3fff 1519  ld   [$19],y
jupiter57:    4000 0000  ld   $00
              4001 0000  ld   $00
              4002 0000  ld   $00
              * 18 times
              4012 0016  ld   $16
              4013 002b  ld   $2b
              4014 001a  ld   $1a
              4015 001b  ld   $1b
              4016 001b  ld   $1b
              4017 001a  ld   $1a
              4018 001f  ld   $1f
              4019 001b  ld   $1b
              401a 002b  ld   $2b
              401b 001f  ld   $1f
              401c 002b  ld   $2b
              401d 001f  ld   $1f
              401e 002b  ld   $2b
              401f 001f  ld   $1f
              4020 002b  ld   $2b
              4021 001b  ld   $1b
              4022 001f  ld   $1f
              4023 001b  ld   $1b
              4024 001b  ld   $1b
              4025 001b  ld   $1b
              * 5 times
              4028 001f  ld   $1f
              4029 002b  ld   $2b
              402a 001b  ld   $1b
              402b 001b  ld   $1b
              402c 001f  ld   $1f
              402d 002b  ld   $2b
              402e 001b  ld   $1b
              402f 001b  ld   $1b
              4030 001b  ld   $1b
              4031 001a  ld   $1a
              4032 001f  ld   $1f
              4033 001b  ld   $1b
              4034 001b  ld   $1b
              4035 002b  ld   $2b
              4036 001f  ld   $1f
              4037 002b  ld   $2b
              4038 001a  ld   $1a
              4039 001b  ld   $1b
              403a 002b  ld   $2b
              403b 001b  ld   $1b
              403c 002b  ld   $2b
              403d 001b  ld   $1b
              403e 002b  ld   $2b
              403f 001a  ld   $1a
              4040 001b  ld   $1b
              4041 002b  ld   $2b
              4042 001b  ld   $1b
              4043 002b  ld   $2b
              4044 001f  ld   $1f
              4045 002b  ld   $2b
              4046 002b  ld   $2b
              4047 001e  ld   $1e
              4048 002b  ld   $2b
              4049 002b  ld   $2b
              404a 002e  ld   $2e
              404b 001b  ld   $1b
              404c 002a  ld   $2a
              404d 001b  ld   $1b
              404e 002b  ld   $2b
              404f 001a  ld   $1a
              4050 002b  ld   $2b
              4051 001a  ld   $1a
              4052 002b  ld   $2b
              4053 001a  ld   $1a
              4054 002b  ld   $2b
              4055 001a  ld   $1a
              4056 001a  ld   $1a
              4057 001b  ld   $1b
              4058 001a  ld   $1a
              4059 001a  ld   $1a
              405a 0016  ld   $16
              405b 001a  ld   $1a
              405c 001a  ld   $1a
              405d 0016  ld   $16
              405e 001a  ld   $1a
              405f 0016  ld   $16
              4060 001a  ld   $1a
              4061 0006  ld   $06
              4062 001a  ld   $1a
              4063 0006  ld   $06
              4064 001a  ld   $1a
              4065 0016  ld   $16
              4066 000a  ld   $0a
              4067 0015  ld   $15
              4068 001a  ld   $1a
              4069 0016  ld   $16
              406a 001a  ld   $1a
              406b 0016  ld   $16
              406c 001a  ld   $1a
              406d 0015  ld   $15
              406e 0016  ld   $16
              406f 000a  ld   $0a
              4070 0015  ld   $15
              4071 0016  ld   $16
              4072 0019  ld   $19
              4073 0006  ld   $06
              4074 0015  ld   $15
              4075 0006  ld   $06
              4076 0005  ld   $05
              4077 0015  ld   $15
              4078 0005  ld   $05
              4079 0016  ld   $16
              407a 0005  ld   $05
              407b 0005  ld   $05
              407c 0001  ld   $01
              407d 0005  ld   $05
              407e 0015  ld   $15
              407f 0005  ld   $05
              4080 0015  ld   $15
              4081 0005  ld   $05
              4082 0016  ld   $16
              4083 0005  ld   $05
              4084 0015  ld   $15
              4085 0005  ld   $05
              4086 0001  ld   $01
              4087 0015  ld   $15
              4088 0005  ld   $05
              4089 0015  ld   $15
              408a 0005  ld   $05
              408b 0015  ld   $15
              408c 0015  ld   $15
              408d 0000  ld   $00
              408e 0000  ld   $00
              408f 0000  ld   $00
              * 19 times
              40a0 0200  nop
              40a1 0200  nop
              40a2 0200  nop
              * 91 times
              40fb fe00  bra  ac          ;Trampoline for page $4000 lookups
              40fc fcfd  bra  $40fd
              40fd 1403  ld   $03,y
              40fe e07b  jmp  y,$7b
              40ff 1519  ld   [$19],y
jupiter58:    4100 0000  ld   $00
              4101 0000  ld   $00
              4102 0000  ld   $00
              * 18 times
              4112 0015  ld   $15
              4113 001b  ld   $1b
              4114 001b  ld   $1b
              4115 001a  ld   $1a
              4116 002b  ld   $2b
              4117 001b  ld   $1b
              4118 002b  ld   $2b
              4119 001b  ld   $1b
              411a 002f  ld   $2f
              411b 002b  ld   $2b
              411c 001f  ld   $1f
              411d 002b  ld   $2b
              411e 001b  ld   $1b
              411f 002b  ld   $2b
              4120 001b  ld   $1b
              4121 001b  ld   $1b
              4122 002b  ld   $2b
              4123 001b  ld   $1b
              4124 001b  ld   $1b
              4125 002b  ld   $2b
              4126 001b  ld   $1b
              4127 001b  ld   $1b
              4128 001b  ld   $1b
              4129 001b  ld   $1b
              412a 002f  ld   $2f
              412b 001b  ld   $1b
              412c 002b  ld   $2b
              412d 001b  ld   $1b
              412e 001b  ld   $1b
              412f 001a  ld   $1a
              4130 001b  ld   $1b
              4131 001b  ld   $1b
              4132 001b  ld   $1b
              4133 002a  ld   $2a
              4134 001b  ld   $1b
              4135 001b  ld   $1b
              4136 001b  ld   $1b
              4137 001b  ld   $1b
              4138 002b  ld   $2b
              4139 001b  ld   $1b
              413a 002b  ld   $2b
              413b 001b  ld   $1b
              413c 001a  ld   $1a
              413d 001b  ld   $1b
              413e 001b  ld   $1b
              413f 001b  ld   $1b
              4140 002b  ld   $2b
              4141 001f  ld   $1f
              4142 002a  ld   $2a
              4143 001b  ld   $1b
              4144 002b  ld   $2b
              4145 001a  ld   $1a
              4146 002f  ld   $2f
              4147 001b  ld   $1b
              4148 002b  ld   $2b
              4149 001b  ld   $1b
              414a 002b  ld   $2b
              414b 001f  ld   $1f
              414c 002b  ld   $2b
              414d 001a  ld   $1a
              414e 002f  ld   $2f
              414f 001b  ld   $1b
              4150 002a  ld   $2a
              4151 001b  ld   $1b
              4152 001a  ld   $1a
              4153 001b  ld   $1b
              4154 001a  ld   $1a
              4155 001b  ld   $1b
              4156 0016  ld   $16
              4157 001a  ld   $1a
              4158 0016  ld   $16
              4159 001a  ld   $1a
              415a 001b  ld   $1b
              415b 0006  ld   $06
              415c 001b  ld   $1b
              415d 001a  ld   $1a
              415e 0016  ld   $16
              415f 000a  ld   $0a
              4160 0016  ld   $16
              4161 001a  ld   $1a
              4162 0016  ld   $16
              4163 0006  ld   $06
              4164 0015  ld   $15
              4165 001a  ld   $1a
              4166 0016  ld   $16
              4167 0016  ld   $16
              4168 0016  ld   $16
              4169 0009  ld   $09
              416a 0016  ld   $16
              416b 0006  ld   $06
              416c 0015  ld   $15
              416d 0006  ld   $06
              416e 0015  ld   $15
              416f 0016  ld   $16
              4170 0005  ld   $05
              4171 0006  ld   $06
              4172 0005  ld   $05
              4173 0016  ld   $16
              4174 0005  ld   $05
              4175 0015  ld   $15
              4176 0005  ld   $05
              4177 0005  ld   $05
              4178 0005  ld   $05
              4179 0005  ld   $05
              417a 0001  ld   $01
              417b 0015  ld   $15
              417c 0005  ld   $05
              417d 0015  ld   $15
              417e 0001  ld   $01
              417f 0015  ld   $15
              4180 0005  ld   $05
              4181 0015  ld   $15
              4182 0001  ld   $01
              4183 0015  ld   $15
              4184 0001  ld   $01
              4185 0005  ld   $05
              4186 0015  ld   $15
              4187 0001  ld   $01
              4188 0015  ld   $15
              4189 0005  ld   $05
              418a 0016  ld   $16
              418b 0005  ld   $05
              418c 0011  ld   $11
              418d 0000  ld   $00
              418e 0000  ld   $00
              418f 0000  ld   $00
              * 19 times
              41a0 0200  nop
              41a1 0200  nop
              41a2 0200  nop
              * 91 times
              41fb fe00  bra  ac          ;Trampoline for page $4100 lookups
              41fc fcfd  bra  $41fd
              41fd 1403  ld   $03,y
              41fe e07b  jmp  y,$7b
              41ff 1519  ld   [$19],y
jupiter59:    4200 0000  ld   $00
              4201 0000  ld   $00
              4202 0000  ld   $00
              * 18 times
              4212 0005  ld   $05
              4213 001b  ld   $1b
              4214 001a  ld   $1a
              4215 001b  ld   $1b
              4216 001b  ld   $1b
              4217 001b  ld   $1b
              4218 001b  ld   $1b
              4219 002b  ld   $2b
              421a 001a  ld   $1a
              421b 001b  ld   $1b
              421c 001b  ld   $1b
              421d 002b  ld   $2b
              421e 001b  ld   $1b
              421f 001f  ld   $1f
              4220 001b  ld   $1b
              4221 001a  ld   $1a
              4222 001b  ld   $1b
              4223 001b  ld   $1b
              4224 001b  ld   $1b
              * 5 times
              4227 001a  ld   $1a
              4228 002b  ld   $2b
              4229 001b  ld   $1b
              422a 001b  ld   $1b
              422b 002b  ld   $2b
              422c 001b  ld   $1b
              422d 001a  ld   $1a
              422e 001b  ld   $1b
              422f 001b  ld   $1b
              4230 001b  ld   $1b
              4231 001a  ld   $1a
              4232 001b  ld   $1b
              4233 001b  ld   $1b
              4234 001f  ld   $1f
              4235 002b  ld   $2b
              4236 001a  ld   $1a
              4237 001b  ld   $1b
              4238 002b  ld   $2b
              4239 001b  ld   $1b
              423a 001f  ld   $1f
              423b 001b  ld   $1b
              423c 002b  ld   $2b
              423d 001b  ld   $1b
              423e 002b  ld   $2b
              423f 001a  ld   $1a
              4240 001a  ld   $1a
              4241 001b  ld   $1b
              4242 001b  ld   $1b
              4243 002b  ld   $2b
              4244 001b  ld   $1b
              4245 001b  ld   $1b
              4246 002b  ld   $2b
              4247 001b  ld   $1b
              4248 002b  ld   $2b
              4249 001e  ld   $1e
              424a 002b  ld   $2b
              424b 001a  ld   $1a
              424c 002b  ld   $2b
              424d 001b  ld   $1b
              424e 002b  ld   $2b
              424f 001a  ld   $1a
              4250 001b  ld   $1b
              4251 001a  ld   $1a
              4252 001b  ld   $1b
              4253 001a  ld   $1a
              4254 001a  ld   $1a
              4255 0016  ld   $16
              4256 001a  ld   $1a
              4257 0016  ld   $16
              4258 001b  ld   $1b
              4259 0016  ld   $16
              425a 001a  ld   $1a
              425b 0016  ld   $16
              425c 001a  ld   $1a
              425d 0007  ld   $07
              425e 001a  ld   $1a
              425f 0016  ld   $16
              4260 001a  ld   $1a
              4261 0006  ld   $06
              4262 001a  ld   $1a
              4263 0016  ld   $16
              4264 001a  ld   $1a
              4265 0006  ld   $06
              4266 0016  ld   $16
              4267 000a  ld   $0a
              4268 0016  ld   $16
              4269 0006  ld   $06
              426a 0016  ld   $16
              426b 0015  ld   $15
              426c 0006  ld   $06
              426d 0015  ld   $15
              426e 0006  ld   $06
              426f 0005  ld   $05
              4270 0005  ld   $05
              4271 0016  ld   $16
              4272 0005  ld   $05
              4273 0015  ld   $15
              4274 0005  ld   $05
              4275 0002  ld   $02
              4276 0015  ld   $15
              4277 0005  ld   $05
              4278 0001  ld   $01
              4279 0015  ld   $15
              427a 0005  ld   $05
              427b 0001  ld   $01
              427c 0005  ld   $05
              427d 0001  ld   $01
              427e 0005  ld   $05
              427f 0005  ld   $05
              4280 0005  ld   $05
              4281 0011  ld   $11
              4282 0005  ld   $05
              4283 0005  ld   $05
              4284 0001  ld   $01
              4285 0005  ld   $05
              4286 0001  ld   $01
              4287 0005  ld   $05
              4288 0001  ld   $01
              4289 0015  ld   $15
              428a 0005  ld   $05
              428b 0015  ld   $15
              428c 0005  ld   $05
              428d 0000  ld   $00
              428e 0000  ld   $00
              428f 0000  ld   $00
              * 19 times
              42a0 0200  nop
              42a1 0200  nop
              42a2 0200  nop
              * 91 times
              42fb fe00  bra  ac          ;Trampoline for page $4200 lookups
              42fc fcfd  bra  $42fd
              42fd 1403  ld   $03,y
              42fe e07b  jmp  y,$7b
              42ff 1519  ld   [$19],y
jupiter60:    4300 0000  ld   $00
              4301 0000  ld   $00
              4302 0000  ld   $00
              * 18 times
              4312 0015  ld   $15
              4313 0017  ld   $17
              4314 001a  ld   $1a
              4315 0017  ld   $17
              4316 001a  ld   $1a
              4317 001b  ld   $1b
              4318 001b  ld   $1b
              4319 001b  ld   $1b
              * 5 times
              431c 001a  ld   $1a
              431d 001b  ld   $1b
              431e 001b  ld   $1b
              431f 002b  ld   $2b
              4320 001b  ld   $1b
              4321 001b  ld   $1b
              4322 001b  ld   $1b
              4323 002b  ld   $2b
              4324 001b  ld   $1b
              4325 001b  ld   $1b
              4326 001b  ld   $1b
              * 15 times
              4333 002b  ld   $2b
              4334 001b  ld   $1b
              4335 001b  ld   $1b
              4336 001b  ld   $1b
              4337 002f  ld   $2f
              4338 001b  ld   $1b
              4339 001b  ld   $1b
              433a 002a  ld   $2a
              433b 001b  ld   $1b
              433c 001b  ld   $1b
              433d 001a  ld   $1a
              433e 001b  ld   $1b
              433f 001b  ld   $1b
              4340 001b  ld   $1b
              4341 001a  ld   $1a
              4342 001b  ld   $1b
              4343 001a  ld   $1a
              4344 001a  ld   $1a
              4345 001a  ld   $1a
              4346 001b  ld   $1b
              4347 001a  ld   $1a
              4348 001b  ld   $1b
              4349 001b  ld   $1b
              434a 002b  ld   $2b
              434b 001a  ld   $1a
              434c 001b  ld   $1b
              434d 001a  ld   $1a
              434e 001b  ld   $1b
              434f 001a  ld   $1a
              4350 001b  ld   $1b
              4351 001a  ld   $1a
              4352 0016  ld   $16
              4353 0016  ld   $16
              4354 001a  ld   $1a
              4355 0016  ld   $16
              4356 001a  ld   $1a
              4357 001a  ld   $1a
              4358 001a  ld   $1a
              4359 001b  ld   $1b
              435a 0016  ld   $16
              435b 001a  ld   $1a
              435c 0016  ld   $16
              435d 001a  ld   $1a
              435e 0016  ld   $16
              435f 000a  ld   $0a
              4360 0016  ld   $16
              4361 0016  ld   $16
              4362 0006  ld   $06
              4363 0005  ld   $05
              4364 0016  ld   $16
              4365 0005  ld   $05
              4366 0016  ld   $16
              4367 0005  ld   $05
              4368 0016  ld   $16
              4369 0019  ld   $19
              436a 0006  ld   $06
              436b 0006  ld   $06
              436c 0015  ld   $15
              436d 0006  ld   $06
              436e 0015  ld   $15
              436f 0006  ld   $06
              4370 0015  ld   $15
              4371 0006  ld   $06
              4372 0015  ld   $15
              4373 0006  ld   $06
              4374 0005  ld   $05
              4375 0005  ld   $05
              4376 0001  ld   $01
              4377 0005  ld   $05
              4378 0005  ld   $05
              4379 0001  ld   $01
              437a 0005  ld   $05
              437b 0001  ld   $01
              437c 0015  ld   $15
              437d 0001  ld   $01
              437e 0015  ld   $15
              437f 0001  ld   $01
              4380 0005  ld   $05
              4381 0001  ld   $01
              4382 0005  ld   $05
              4383 0011  ld   $11
              4384 0005  ld   $05
              4385 0011  ld   $11
              4386 0005  ld   $05
              4387 0001  ld   $01
              4388 0005  ld   $05
              4389 0005  ld   $05
              438a 0001  ld   $01
              438b 0015  ld   $15
              438c 0000  ld   $00
              438d 0000  ld   $00
              438e 0000  ld   $00
              * 20 times
              43a0 0200  nop
              43a1 0200  nop
              43a2 0200  nop
              * 91 times
              43fb fe00  bra  ac          ;Trampoline for page $4300 lookups
              43fc fcfd  bra  $43fd
              43fd 1403  ld   $03,y
              43fe e07b  jmp  y,$7b
              43ff 1519  ld   [$19],y
jupiter61:    4400 0000  ld   $00
              4401 0000  ld   $00
              4402 0000  ld   $00
              * 18 times
              4412 0005  ld   $05
              4413 0016  ld   $16
              4414 001b  ld   $1b
              4415 001b  ld   $1b
              4416 001b  ld   $1b
              4417 001a  ld   $1a
              4418 001b  ld   $1b
              4419 001b  ld   $1b
              441a 002b  ld   $2b
              441b 001b  ld   $1b
              441c 001b  ld   $1b
              441d 001b  ld   $1b
              * 10 times
              4425 001a  ld   $1a
              4426 001b  ld   $1b
              4427 002b  ld   $2b
              4428 001b  ld   $1b
              4429 001b  ld   $1b
              442a 001b  ld   $1b
              442b 001a  ld   $1a
              442c 001b  ld   $1b
              442d 001a  ld   $1a
              442e 0017  ld   $17
              442f 001a  ld   $1a
              4430 001b  ld   $1b
              4431 001b  ld   $1b
              4432 001b  ld   $1b
              * 7 times
              4437 001a  ld   $1a
              4438 001b  ld   $1b
              4439 001b  ld   $1b
              443a 001b  ld   $1b
              * 5 times
              443d 001a  ld   $1a
              443e 001b  ld   $1b
              443f 001a  ld   $1a
              4440 0006  ld   $06
              4441 001a  ld   $1a
              4442 0016  ld   $16
              4443 001a  ld   $1a
              4444 0017  ld   $17
              4445 001a  ld   $1a
              4446 0016  ld   $16
              4447 001b  ld   $1b
              4448 001a  ld   $1a
              4449 0016  ld   $16
              444a 001a  ld   $1a
              444b 001a  ld   $1a
              444c 001b  ld   $1b
              444d 0016  ld   $16
              444e 001a  ld   $1a
              444f 0016  ld   $16
              4450 0016  ld   $16
              4451 001a  ld   $1a
              4452 0016  ld   $16
              4453 000a  ld   $0a
              4454 0016  ld   $16
              4455 0007  ld   $07
              4456 001a  ld   $1a
              4457 0017  ld   $17
              4458 0006  ld   $06
              4459 0016  ld   $16
              445a 000a  ld   $0a
              445b 0016  ld   $16
              445c 000a  ld   $0a
              445d 0016  ld   $16
              445e 001a  ld   $1a
              445f 0016  ld   $16
              4460 0006  ld   $06
              4461 0016  ld   $16
              4462 0006  ld   $06
              4463 0016  ld   $16
              4464 0005  ld   $05
              4465 0016  ld   $16
              4466 0006  ld   $06
              4467 0016  ld   $16
              4468 0005  ld   $05
              4469 0006  ld   $06
              446a 0005  ld   $05
              446b 0016  ld   $16
              446c 0005  ld   $05
              446d 0016  ld   $16
              446e 0005  ld   $05
              446f 0005  ld   $05
              4470 0002  ld   $02
              4471 0005  ld   $05
              4472 0001  ld   $01
              4473 0005  ld   $05
              4474 0012  ld   $12
              4475 0005  ld   $05
              4476 0011  ld   $11
              4477 0005  ld   $05
              4478 0001  ld   $01
              4479 0005  ld   $05
              447a 0001  ld   $01
              447b 0005  ld   $05
              447c 0001  ld   $01
              447d 0005  ld   $05
              447e 0001  ld   $01
              447f 0001  ld   $01
              4480 0015  ld   $15
              4481 0001  ld   $01
              4482 0005  ld   $05
              4483 0001  ld   $01
              4484 0005  ld   $05
              4485 0000  ld   $00
              4486 0005  ld   $05
              4487 0011  ld   $11
              4488 0005  ld   $05
              4489 0011  ld   $11
              448a 0005  ld   $05
              448b 0001  ld   $01
              448c 0005  ld   $05
              448d 0000  ld   $00
              448e 0000  ld   $00
              448f 0000  ld   $00
              * 19 times
              44a0 0200  nop
              44a1 0200  nop
              44a2 0200  nop
              * 91 times
              44fb fe00  bra  ac          ;Trampoline for page $4400 lookups
              44fc fcfd  bra  $44fd
              44fd 1403  ld   $03,y
              44fe e07b  jmp  y,$7b
              44ff 1519  ld   [$19],y
jupiter62:    4500 0000  ld   $00
              4501 0000  ld   $00
              4502 0000  ld   $00
              * 18 times
              4512 0015  ld   $15
              4513 0007  ld   $07
              4514 001a  ld   $1a
              4515 0017  ld   $17
              4516 0016  ld   $16
              4517 001b  ld   $1b
              4518 001a  ld   $1a
              4519 001b  ld   $1b
              451a 001a  ld   $1a
              451b 001a  ld   $1a
              451c 001b  ld   $1b
              451d 001a  ld   $1a
              451e 001b  ld   $1b
              451f 001a  ld   $1a
              4520 001b  ld   $1b
              4521 001a  ld   $1a
              4522 001b  ld   $1b
              4523 0016  ld   $16
              4524 001b  ld   $1b
              4525 0016  ld   $16
              4526 001b  ld   $1b
              4527 0016  ld   $16
              4528 001b  ld   $1b
              4529 001b  ld   $1b
              452a 001a  ld   $1a
              452b 0017  ld   $17
              452c 001b  ld   $1b
              452d 0016  ld   $16
              452e 001b  ld   $1b
              452f 001b  ld   $1b
              4530 0017  ld   $17
              4531 001a  ld   $1a
              4532 0017  ld   $17
              4533 001a  ld   $1a
              4534 001b  ld   $1b
              4535 001a  ld   $1a
              4536 001b  ld   $1b
              4537 0016  ld   $16
              4538 001b  ld   $1b
              4539 001a  ld   $1a
              453a 001b  ld   $1b
              453b 001a  ld   $1a
              453c 001b  ld   $1b
              453d 0017  ld   $17
              453e 001b  ld   $1b
              453f 0006  ld   $06
              4540 001b  ld   $1b
              4541 0017  ld   $17
              4542 0006  ld   $06
              4543 001b  ld   $1b
              4544 0006  ld   $06
              4545 0016  ld   $16
              4546 0017  ld   $17
              4547 001a  ld   $1a
              4548 0007  ld   $07
              4549 001a  ld   $1a
              454a 0006  ld   $06
              454b 0016  ld   $16
              454c 0006  ld   $06
              454d 0006  ld   $06
              454e 0006  ld   $06
              454f 0016  ld   $16
              4550 0006  ld   $06
              4551 0006  ld   $06
              4552 0016  ld   $16
              4553 0006  ld   $06
              4554 0016  ld   $16
              4555 001a  ld   $1a
              4556 0016  ld   $16
              4557 0006  ld   $06
              4558 001a  ld   $1a
              4559 0006  ld   $06
              455a 0016  ld   $16
              455b 0006  ld   $06
              455c 0016  ld   $16
              455d 0006  ld   $06
              455e 0016  ld   $16
              455f 0006  ld   $06
              4560 0016  ld   $16
              4561 0009  ld   $09
              4562 0016  ld   $16
              4563 0005  ld   $05
              4564 0006  ld   $06
              4565 0005  ld   $05
              4566 0005  ld   $05
              4567 0005  ld   $05
              4568 0016  ld   $16
              4569 0005  ld   $05
              456a 0016  ld   $16
              456b 0005  ld   $05
              456c 0006  ld   $06
              456d 0005  ld   $05
              456e 0016  ld   $16
              456f 0005  ld   $05
              4570 0015  ld   $15
              4571 0005  ld   $05
              4572 0006  ld   $06
              4573 0001  ld   $01
              4574 0005  ld   $05
              4575 0005  ld   $05
              4576 0001  ld   $01
              4577 0005  ld   $05
              4578 0001  ld   $01
              4579 0005  ld   $05
              457a 0001  ld   $01
              457b 0000  ld   $00
              457c 0005  ld   $05
              457d 0001  ld   $01
              457e 0004  ld   $04
              457f 0001  ld   $01
              4580 0001  ld   $01
              4581 0005  ld   $05
              4582 0000  ld   $00
              4583 0005  ld   $05
              4584 0001  ld   $01
              4585 0005  ld   $05
              4586 0001  ld   $01
              4587 0005  ld   $05
              4588 0001  ld   $01
              4589 0005  ld   $05
              458a 0001  ld   $01
              458b 0015  ld   $15
              458c 0000  ld   $00
              458d 0000  ld   $00
              458e 0000  ld   $00
              * 20 times
              45a0 0200  nop
              45a1 0200  nop
              45a2 0200  nop
              * 91 times
              45fb fe00  bra  ac          ;Trampoline for page $4500 lookups
              45fc fcfd  bra  $45fd
              45fd 1403  ld   $03,y
              45fe e07b  jmp  y,$7b
              45ff 1519  ld   [$19],y
jupiter63:    4600 0000  ld   $00
              4601 0000  ld   $00
              4602 0000  ld   $00
              * 18 times
              4612 0001  ld   $01
              4613 001a  ld   $1a
              4614 0016  ld   $16
              4615 0007  ld   $07
              4616 001a  ld   $1a
              4617 0006  ld   $06
              4618 001b  ld   $1b
              4619 0017  ld   $17
              461a 001b  ld   $1b
              461b 0016  ld   $16
              461c 0007  ld   $07
              461d 001a  ld   $1a
              461e 0007  ld   $07
              461f 0016  ld   $16
              4620 001b  ld   $1b
              4621 0017  ld   $17
              4622 001a  ld   $1a
              4623 0017  ld   $17
              4624 000a  ld   $0a
              4625 0017  ld   $17
              4626 000a  ld   $0a
              4627 0017  ld   $17
              4628 000a  ld   $0a
              4629 0017  ld   $17
              462a 0017  ld   $17
              462b 000a  ld   $0a
              462c 0017  ld   $17
              462d 000a  ld   $0a
              462e 0017  ld   $17
              462f 0006  ld   $06
              4630 001a  ld   $1a
              4631 0017  ld   $17
              4632 000a  ld   $0a
              4633 001b  ld   $1b
              4634 0016  ld   $16
              4635 001b  ld   $1b
              4636 0016  ld   $16
              4637 000a  ld   $0a
              4638 0006  ld   $06
              4639 000b  ld   $0b
              463a 0016  ld   $16
              463b 001b  ld   $1b
              463c 0017  ld   $17
              463d 001a  ld   $1a
              463e 0006  ld   $06
              463f 0017  ld   $17
              4640 000a  ld   $0a
              4641 0016  ld   $16
              4642 000a  ld   $0a
              4643 0016  ld   $16
              4644 0006  ld   $06
              4645 000a  ld   $0a
              4646 0006  ld   $06
              4647 0016  ld   $16
              4648 0006  ld   $06
              4649 0016  ld   $16
              464a 001a  ld   $1a
              464b 0007  ld   $07
              464c 001a  ld   $1a
              464d 0016  ld   $16
              464e 001a  ld   $1a
              464f 0007  ld   $07
              4650 0016  ld   $16
              4651 001a  ld   $1a
              4652 0006  ld   $06
              4653 0016  ld   $16
              4654 0006  ld   $06
              4655 0006  ld   $06
              4656 0006  ld   $06
              4657 0016  ld   $16
              4658 0006  ld   $06
              4659 0016  ld   $16
              465a 0006  ld   $06
              465b 0006  ld   $06
              465c 0016  ld   $16
              465d 0006  ld   $06
              465e 0006  ld   $06
              465f 0005  ld   $05
              4660 0016  ld   $16
              4661 0006  ld   $06
              4662 0005  ld   $05
              4663 0006  ld   $06
              4664 0005  ld   $05
              4665 0016  ld   $16
              4666 0006  ld   $06
              4667 0015  ld   $15
              4668 0006  ld   $06
              4669 0005  ld   $05
              466a 0016  ld   $16
              466b 0005  ld   $05
              466c 0015  ld   $15
              466d 0006  ld   $06
              466e 0015  ld   $15
              466f 0005  ld   $05
              4670 0006  ld   $06
              4671 0001  ld   $01
              4672 0005  ld   $05
              4673 0015  ld   $15
              4674 0005  ld   $05
              4675 0011  ld   $11
              4676 0005  ld   $05
              4677 0001  ld   $01
              4678 0015  ld   $15
              4679 0001  ld   $01
              467a 0005  ld   $05
              467b 0001  ld   $01
              467c 0004  ld   $04
              467d 0001  ld   $01
              467e 0005  ld   $05
              467f 0001  ld   $01
              4680 0005  ld   $05
              4681 0001  ld   $01
              4682 0005  ld   $05
              4683 0001  ld   $01
              4684 0001  ld   $01
              4685 0004  ld   $04
              4686 0001  ld   $01
              4687 0000  ld   $00
              4688 0005  ld   $05
              4689 0001  ld   $01
              468a 0005  ld   $05
              468b 0000  ld   $00
              468c 0005  ld   $05
              468d 0000  ld   $00
              468e 0000  ld   $00
              468f 0000  ld   $00
              * 19 times
              46a0 0200  nop
              46a1 0200  nop
              46a2 0200  nop
              * 91 times
              46fb fe00  bra  ac          ;Trampoline for page $4600 lookups
              46fc fcfd  bra  $46fd
              46fd 1403  ld   $03,y
              46fe e07b  jmp  y,$7b
              46ff 1519  ld   [$19],y
jupiter64:    4700 0000  ld   $00
              4701 0000  ld   $00
              4702 0000  ld   $00
              * 18 times
              4712 0005  ld   $05
              4713 0016  ld   $16
              4714 0006  ld   $06
              4715 001b  ld   $1b
              4716 0016  ld   $16
              4717 001b  ld   $1b
              4718 0006  ld   $06
              4719 001a  ld   $1a
              471a 0007  ld   $07
              471b 001a  ld   $1a
              471c 0007  ld   $07
              471d 0006  ld   $06
              471e 001a  ld   $1a
              471f 001b  ld   $1b
              4720 0017  ld   $17
              4721 001b  ld   $1b
              4722 001b  ld   $1b
              4723 0016  ld   $16
              4724 001b  ld   $1b
              4725 0006  ld   $06
              4726 001b  ld   $1b
              4727 0006  ld   $06
              4728 001b  ld   $1b
              4729 001a  ld   $1a
              472a 001b  ld   $1b
              472b 0016  ld   $16
              472c 001b  ld   $1b
              472d 0007  ld   $07
              472e 001a  ld   $1a
              472f 001b  ld   $1b
              4730 0006  ld   $06
              4731 001b  ld   $1b
              4732 0006  ld   $06
              4733 0017  ld   $17
              4734 000a  ld   $0a
              4735 0006  ld   $06
              4736 001b  ld   $1b
              4737 0017  ld   $17
              4738 0016  ld   $16
              4739 001a  ld   $1a
              473a 0007  ld   $07
              473b 0016  ld   $16
              473c 000a  ld   $0a
              473d 0016  ld   $16
              473e 000b  ld   $0b
              473f 0016  ld   $16
              4740 000a  ld   $0a
              4741 0017  ld   $17
              4742 0007  ld   $07
              4743 001a  ld   $1a
              4744 0006  ld   $06
              4745 0017  ld   $17
              4746 0006  ld   $06
              4747 001a  ld   $1a
              4748 0017  ld   $17
              4749 000a  ld   $0a
              474a 0017  ld   $17
              474b 0016  ld   $16
              474c 001a  ld   $1a
              474d 0007  ld   $07
              474e 0016  ld   $16
              474f 001a  ld   $1a
              4750 0006  ld   $06
              4751 0016  ld   $16
              4752 0006  ld   $06
              4753 0016  ld   $16
              4754 0006  ld   $06
              4755 0016  ld   $16
              4756 0006  ld   $06
              4757 0006  ld   $06
              4758 0005  ld   $05
              4759 0006  ld   $06
              475a 0006  ld   $06
              475b 0005  ld   $05
              475c 0006  ld   $06
              475d 0015  ld   $15
              475e 0006  ld   $06
              475f 0016  ld   $16
              4760 0006  ld   $06
              4761 0015  ld   $15
              4762 0016  ld   $16
              4763 0006  ld   $06
              4764 0015  ld   $15
              4765 0006  ld   $06
              4766 0005  ld   $05
              4767 0006  ld   $06
              4768 0015  ld   $15
              4769 0006  ld   $06
              476a 0005  ld   $05
              476b 0006  ld   $06
              476c 0005  ld   $05
              476d 0001  ld   $01
              476e 0005  ld   $05
              476f 0002  ld   $02
              4770 0015  ld   $15
              4771 0005  ld   $05
              4772 0012  ld   $12
              4773 0005  ld   $05
              4774 0001  ld   $01
              4775 0005  ld   $05
              4776 0005  ld   $05
              4777 0005  ld   $05
              4778 0001  ld   $01
              4779 0005  ld   $05
              477a 0001  ld   $01
              477b 0015  ld   $15
              477c 0001  ld   $01
              477d 0015  ld   $15
              477e 0001  ld   $01
              477f 0005  ld   $05
              4780 0010  ld   $10
              4781 0005  ld   $05
              4782 0001  ld   $01
              4783 0004  ld   $04
              4784 0001  ld   $01
              4785 0005  ld   $05
              4786 0011  ld   $11
              4787 0005  ld   $05
              4788 0001  ld   $01
              4789 0005  ld   $05
              478a 0000  ld   $00
              478b 0001  ld   $01
              478c 0010  ld   $10
              478d 0000  ld   $00
              478e 0000  ld   $00
              478f 0000  ld   $00
              * 19 times
              47a0 0200  nop
              47a1 0200  nop
              47a2 0200  nop
              * 91 times
              47fb fe00  bra  ac          ;Trampoline for page $4700 lookups
              47fc fcfd  bra  $47fd
              47fd 1403  ld   $03,y
              47fe e07b  jmp  y,$7b
              47ff 1519  ld   [$19],y
jupiter65:    4800 0000  ld   $00
              4801 0000  ld   $00
              4802 0000  ld   $00
              * 18 times
              4812 0001  ld   $01
              4813 001a  ld   $1a
              4814 0016  ld   $16
              4815 0007  ld   $07
              4816 0006  ld   $06
              4817 0006  ld   $06
              4818 0017  ld   $17
              4819 000a  ld   $0a
              481a 0017  ld   $17
              481b 0006  ld   $06
              481c 001b  ld   $1b
              481d 001a  ld   $1a
              481e 0017  ld   $17
              481f 001b  ld   $1b
              4820 000a  ld   $0a
              4821 0017  ld   $17
              4822 001a  ld   $1a
              4823 000b  ld   $0b
              4824 0017  ld   $17
              4825 001a  ld   $1a
              4826 001b  ld   $1b
              4827 001b  ld   $1b
              4828 001a  ld   $1a
              4829 0007  ld   $07
              482a 001b  ld   $1b
              482b 0007  ld   $07
              482c 001b  ld   $1b
              482d 001a  ld   $1a
              482e 001b  ld   $1b
              482f 001b  ld   $1b
              4830 0016  ld   $16
              4831 001b  ld   $1b
              4832 0006  ld   $06
              4833 001b  ld   $1b
              4834 0016  ld   $16
              4835 001b  ld   $1b
              4836 0006  ld   $06
              4837 001a  ld   $1a
              4838 000b  ld   $0b
              4839 0016  ld   $16
              483a 001b  ld   $1b
              483b 0016  ld   $16
              483c 001b  ld   $1b
              483d 0006  ld   $06
              483e 001b  ld   $1b
              483f 0016  ld   $16
              4840 000b  ld   $0b
              4841 0016  ld   $16
              4842 001a  ld   $1a
              4843 0007  ld   $07
              4844 0016  ld   $16
              4845 001b  ld   $1b
              4846 0016  ld   $16
              4847 000a  ld   $0a
              4848 0017  ld   $17
              4849 001a  ld   $1a
              484a 0006  ld   $06
              484b 001a  ld   $1a
              484c 0007  ld   $07
              484d 0016  ld   $16
              484e 001a  ld   $1a
              484f 0006  ld   $06
              4850 0016  ld   $16
              4851 000a  ld   $0a
              4852 0016  ld   $16
              4853 0006  ld   $06
              4854 0006  ld   $06
              4855 0006  ld   $06
              4856 0016  ld   $16
              4857 0006  ld   $06
              4858 0006  ld   $06
              4859 0016  ld   $16
              485a 0005  ld   $05
              485b 0016  ld   $16
              485c 0006  ld   $06
              485d 0006  ld   $06
              485e 0006  ld   $06
              485f 0015  ld   $15
              4860 0006  ld   $06
              4861 0006  ld   $06
              4862 0006  ld   $06
              4863 0015  ld   $15
              4864 0006  ld   $06
              4865 0006  ld   $06
              4866 0015  ld   $15
              4867 0006  ld   $06
              4868 0005  ld   $05
              4869 0006  ld   $06
              486a 0011  ld   $11
              486b 0005  ld   $05
              486c 0016  ld   $16
              486d 0005  ld   $05
              486e 0016  ld   $16
              486f 0005  ld   $05
              4870 0001  ld   $01
              4871 0005  ld   $05
              4872 0005  ld   $05
              4873 0005  ld   $05
              4874 0002  ld   $02
              4875 0005  ld   $05
              4876 0001  ld   $01
              4877 0015  ld   $15
              4878 0001  ld   $01
              4879 0005  ld   $05
              487a 0001  ld   $01
              487b 0005  ld   $05
              487c 0005  ld   $05
              487d 0001  ld   $01
              487e 0015  ld   $15
              487f 0005  ld   $05
              4880 0005  ld   $05
              4881 0001  ld   $01
              4882 0015  ld   $15
              4883 0001  ld   $01
              4884 0005  ld   $05
              4885 0000  ld   $00
              4886 0005  ld   $05
              4887 0001  ld   $01
              4888 0015  ld   $15
              4889 0001  ld   $01
              488a 0005  ld   $05
              488b 0015  ld   $15
              488c 0005  ld   $05
              488d 0000  ld   $00
              488e 0000  ld   $00
              488f 0000  ld   $00
              * 19 times
              48a0 0200  nop
              48a1 0200  nop
              48a2 0200  nop
              * 91 times
              48fb fe00  bra  ac          ;Trampoline for page $4800 lookups
              48fc fcfd  bra  $48fd
              48fd 1403  ld   $03,y
              48fe e07b  jmp  y,$7b
              48ff 1519  ld   [$19],y
jupiter66:    4900 0000  ld   $00
              4901 0000  ld   $00
              4902 0000  ld   $00
              * 18 times
              4912 0001  ld   $01
              4913 0016  ld   $16
              4914 001a  ld   $1a
              4915 0006  ld   $06
              4916 001a  ld   $1a
              4917 001b  ld   $1b
              4918 0016  ld   $16
              4919 0007  ld   $07
              491a 001a  ld   $1a
              491b 0006  ld   $06
              491c 0017  ld   $17
              491d 000b  ld   $0b
              491e 001a  ld   $1a
              491f 001b  ld   $1b
              4920 0017  ld   $17
              4921 001b  ld   $1b
              4922 001b  ld   $1b
              4923 001b  ld   $1b
              4924 001b  ld   $1b
              4925 0017  ld   $17
              4926 000a  ld   $0a
              4927 0017  ld   $17
              4928 001b  ld   $1b
              4929 001b  ld   $1b
              492a 001a  ld   $1a
              492b 001b  ld   $1b
              492c 001b  ld   $1b
              492d 0017  ld   $17
              492e 000a  ld   $0a
              492f 0017  ld   $17
              4930 001b  ld   $1b
              4931 0006  ld   $06
              4932 001b  ld   $1b
              4933 0016  ld   $16
              4934 000b  ld   $0b
              4935 0016  ld   $16
              4936 001b  ld   $1b
              4937 001b  ld   $1b
              4938 0017  ld   $17
              4939 001a  ld   $1a
              493a 000b  ld   $0b
              493b 001a  ld   $1a
              493c 0017  ld   $17
              493d 001a  ld   $1a
              493e 0017  ld   $17
              493f 000a  ld   $0a
              4940 0016  ld   $16
              4941 000b  ld   $0b
              4942 0016  ld   $16
              4943 001a  ld   $1a
              4944 001b  ld   $1b
              4945 001a  ld   $1a
              4946 0007  ld   $07
              4947 001a  ld   $1a
              4948 0016  ld   $16
              4949 000b  ld   $0b
              494a 0016  ld   $16
              494b 0006  ld   $06
              494c 001a  ld   $1a
              494d 0006  ld   $06
              494e 0006  ld   $06
              494f 0017  ld   $17
              4950 000a  ld   $0a
              4951 0016  ld   $16
              4952 0006  ld   $06
              4953 0016  ld   $16
              4954 0006  ld   $06
              4955 0006  ld   $06
              4956 0015  ld   $15
              4957 0006  ld   $06
              4958 0005  ld   $05
              4959 0006  ld   $06
              495a 0016  ld   $16
              495b 0006  ld   $06
              495c 0015  ld   $15
              495d 0006  ld   $06
              495e 0016  ld   $16
              495f 0006  ld   $06
              4960 0015  ld   $15
              4961 0006  ld   $06
              4962 0016  ld   $16
              4963 0006  ld   $06
              4964 0016  ld   $16
              4965 0015  ld   $15
              4966 0006  ld   $06
              4967 0016  ld   $16
              4968 0005  ld   $05
              4969 0016  ld   $16
              496a 0005  ld   $05
              496b 0006  ld   $06
              496c 0005  ld   $05
              496d 0015  ld   $15
              496e 0005  ld   $05
              496f 0002  ld   $02
              4970 0005  ld   $05
              4971 0016  ld   $16
              4972 0001  ld   $01
              4973 0005  ld   $05
              4974 0005  ld   $05
              4975 0001  ld   $01
              4976 0005  ld   $05
              4977 0001  ld   $01
              4978 0005  ld   $05
              4979 0011  ld   $11
              497a 0005  ld   $05
              497b 0015  ld   $15
              497c 0001  ld   $01
              497d 0005  ld   $05
              497e 0005  ld   $05
              497f 0011  ld   $11
              4980 0005  ld   $05
              4981 0015  ld   $15
              4982 0005  ld   $05
              4983 0005  ld   $05
              4984 0011  ld   $11
              4985 0005  ld   $05
              4986 0005  ld   $05
              4987 0001  ld   $01
              4988 0004  ld   $04
              4989 0001  ld   $01
              498a 0015  ld   $15
              498b 0001  ld   $01
              498c 0000  ld   $00
              498d 0000  ld   $00
              498e 0000  ld   $00
              * 20 times
              49a0 0200  nop
              49a1 0200  nop
              49a2 0200  nop
              * 91 times
              49fb fe00  bra  ac          ;Trampoline for page $4900 lookups
              49fc fcfd  bra  $49fd
              49fd 1403  ld   $03,y
              49fe e07b  jmp  y,$7b
              49ff 1519  ld   [$19],y
jupiter67:    4a00 0000  ld   $00
              4a01 0000  ld   $00
              4a02 0000  ld   $00
              * 19 times
              4a13 0016  ld   $16
              4a14 001a  ld   $1a
              4a15 0017  ld   $17
              4a16 0007  ld   $07
              4a17 0016  ld   $16
              4a18 000a  ld   $0a
              4a19 0017  ld   $17
              4a1a 001a  ld   $1a
              4a1b 0007  ld   $07
              4a1c 001a  ld   $1a
              4a1d 0016  ld   $16
              4a1e 001b  ld   $1b
              4a1f 0007  ld   $07
              4a20 001b  ld   $1b
              4a21 001b  ld   $1b
              4a22 001a  ld   $1a
              4a23 0017  ld   $17
              4a24 001a  ld   $1a
              4a25 001b  ld   $1b
              4a26 001b  ld   $1b
              4a27 001b  ld   $1b
              * 6 times
              4a2b 0017  ld   $17
              4a2c 001a  ld   $1a
              4a2d 001b  ld   $1b
              4a2e 0017  ld   $17
              4a2f 001a  ld   $1a
              4a30 000b  ld   $0b
              4a31 0016  ld   $16
              4a32 001b  ld   $1b
              4a33 001b  ld   $1b
              4a34 0016  ld   $16
              4a35 001b  ld   $1b
              4a36 001b  ld   $1b
              4a37 0016  ld   $16
              4a38 000b  ld   $0b
              4a39 0016  ld   $16
              4a3a 001b  ld   $1b
              4a3b 0017  ld   $17
              4a3c 000a  ld   $0a
              4a3d 001b  ld   $1b
              4a3e 000a  ld   $0a
              4a3f 0017  ld   $17
              4a40 001b  ld   $1b
              4a41 0016  ld   $16
              4a42 001b  ld   $1b
              4a43 001a  ld   $1a
              4a44 0007  ld   $07
              4a45 001a  ld   $1a
              4a46 0017  ld   $17
              4a47 000a  ld   $0a
              4a48 0017  ld   $17
              4a49 001a  ld   $1a
              4a4a 0016  ld   $16
              4a4b 001b  ld   $1b
              4a4c 0016  ld   $16
              4a4d 0016  ld   $16
              4a4e 0006  ld   $06
              4a4f 0016  ld   $16
              4a50 0006  ld   $06
              4a51 0016  ld   $16
              4a52 0006  ld   $06
              4a53 0016  ld   $16
              4a54 0006  ld   $06
              4a55 0016  ld   $16
              4a56 0006  ld   $06
              4a57 0006  ld   $06
              4a58 0016  ld   $16
              4a59 0006  ld   $06
              4a5a 0016  ld   $16
              4a5b 0006  ld   $06
              4a5c 0016  ld   $16
              4a5d 0006  ld   $06
              4a5e 001a  ld   $1a
              4a5f 0006  ld   $06
              4a60 0006  ld   $06
              4a61 001a  ld   $1a
              4a62 0006  ld   $06
              4a63 0015  ld   $15
              4a64 0006  ld   $06
              4a65 0006  ld   $06
              4a66 0016  ld   $16
              4a67 0006  ld   $06
              4a68 0015  ld   $15
              4a69 0006  ld   $06
              4a6a 0016  ld   $16
              4a6b 0005  ld   $05
              4a6c 0016  ld   $16
              4a6d 0001  ld   $01
              4a6e 0006  ld   $06
              4a6f 0015  ld   $15
              4a70 0005  ld   $05
              4a71 0005  ld   $05
              4a72 0005  ld   $05
              4a73 0001  ld   $01
              4a74 0015  ld   $15
              4a75 0005  ld   $05
              4a76 0001  ld   $01
              4a77 0005  ld   $05
              4a78 0005  ld   $05
              4a79 0001  ld   $01
              4a7a 0005  ld   $05
              4a7b 0005  ld   $05
              4a7c 0015  ld   $15
              4a7d 0005  ld   $05
              4a7e 0015  ld   $15
              4a7f 0005  ld   $05
              4a80 0005  ld   $05
              4a81 0001  ld   $01
              4a82 0015  ld   $15
              4a83 0005  ld   $05
              4a84 0015  ld   $15
              4a85 0005  ld   $05
              4a86 0011  ld   $11
              4a87 0005  ld   $05
              4a88 0015  ld   $15
              4a89 0005  ld   $05
              4a8a 0001  ld   $01
              4a8b 0015  ld   $15
              4a8c 0005  ld   $05
              4a8d 0010  ld   $10
              4a8e 0000  ld   $00
              4a8f 0000  ld   $00
              4a90 0000  ld   $00
              * 18 times
              4aa0 0200  nop
              4aa1 0200  nop
              4aa2 0200  nop
              * 91 times
              4afb fe00  bra  ac          ;Trampoline for page $4a00 lookups
              4afc fcfd  bra  $4afd
              4afd 1403  ld   $03,y
              4afe e07b  jmp  y,$7b
              4aff 1519  ld   [$19],y
jupiter68:    4b00 0000  ld   $00
              4b01 0000  ld   $00
              4b02 0000  ld   $00
              * 19 times
              4b13 0005  ld   $05
              4b14 0006  ld   $06
              4b15 0006  ld   $06
              4b16 0006  ld   $06
              4b17 0006  ld   $06
              4b18 0016  ld   $16
              4b19 0006  ld   $06
              4b1a 0007  ld   $07
              4b1b 0016  ld   $16
              4b1c 0007  ld   $07
              4b1d 001a  ld   $1a
              4b1e 0006  ld   $06
              4b1f 001a  ld   $1a
              4b20 0017  ld   $17
              4b21 0006  ld   $06
              4b22 0017  ld   $17
              4b23 000a  ld   $0a
              4b24 0007  ld   $07
              4b25 0016  ld   $16
              4b26 001b  ld   $1b
              4b27 0016  ld   $16
              4b28 0006  ld   $06
              4b29 0006  ld   $06
              4b2a 0016  ld   $16
              4b2b 0006  ld   $06
              4b2c 0017  ld   $17
              4b2d 000a  ld   $0a
              4b2e 0016  ld   $16
              4b2f 0007  ld   $07
              4b30 0016  ld   $16
              4b31 0006  ld   $06
              4b32 0006  ld   $06
              4b33 0006  ld   $06
              4b34 0006  ld   $06
              4b35 0016  ld   $16
              4b36 0006  ld   $06
              4b37 0016  ld   $16
              4b38 000a  ld   $0a
              4b39 0016  ld   $16
              4b3a 000a  ld   $0a
              4b3b 0016  ld   $16
              4b3c 0006  ld   $06
              4b3d 0016  ld   $16
              4b3e 0006  ld   $06
              4b3f 0006  ld   $06
              4b40 0006  ld   $06
              4b41 0006  ld   $06
              4b42 0016  ld   $16
              4b43 0006  ld   $06
              4b44 001b  ld   $1b
              4b45 0016  ld   $16
              4b46 0006  ld   $06
              4b47 0016  ld   $16
              4b48 0006  ld   $06
              4b49 0006  ld   $06
              4b4a 0006  ld   $06
              * 6 times
              4b4e 0016  ld   $16
              4b4f 0006  ld   $06
              4b50 0006  ld   $06
              4b51 0006  ld   $06
              * 5 times
              4b54 0005  ld   $05
              4b55 0016  ld   $16
              4b56 0006  ld   $06
              4b57 0016  ld   $16
              4b58 0016  ld   $16
              4b59 0006  ld   $06
              4b5a 0016  ld   $16
              4b5b 0005  ld   $05
              4b5c 0006  ld   $06
              4b5d 0016  ld   $16
              4b5e 0005  ld   $05
              4b5f 0016  ld   $16
              4b60 0005  ld   $05
              4b61 0005  ld   $05
              4b62 0016  ld   $16
              4b63 0005  ld   $05
              4b64 0006  ld   $06
              4b65 0015  ld   $15
              4b66 0005  ld   $05
              4b67 0006  ld   $06
              4b68 0015  ld   $15
              4b69 0006  ld   $06
              4b6a 0015  ld   $15
              4b6b 0006  ld   $06
              4b6c 0005  ld   $05
              4b6d 0015  ld   $15
              4b6e 0006  ld   $06
              4b6f 0001  ld   $01
              4b70 0015  ld   $15
              4b71 0001  ld   $01
              4b72 0005  ld   $05
              4b73 0001  ld   $01
              4b74 0005  ld   $05
              4b75 0001  ld   $01
              4b76 0015  ld   $15
              4b77 0001  ld   $01
              4b78 0005  ld   $05
              4b79 0015  ld   $15
              4b7a 0005  ld   $05
              4b7b 0015  ld   $15
              4b7c 0005  ld   $05
              4b7d 0005  ld   $05
              4b7e 0001  ld   $01
              4b7f 0015  ld   $15
              4b80 0005  ld   $05
              4b81 0015  ld   $15
              4b82 0005  ld   $05
              4b83 0015  ld   $15
              4b84 0001  ld   $01
              4b85 0005  ld   $05
              4b86 0005  ld   $05
              4b87 0011  ld   $11
              4b88 0005  ld   $05
              4b89 0001  ld   $01
              4b8a 0015  ld   $15
              4b8b 0000  ld   $00
              4b8c 0000  ld   $00
              4b8d 0000  ld   $00
              * 21 times
              4ba0 0200  nop
              4ba1 0200  nop
              4ba2 0200  nop
              * 91 times
              4bfb fe00  bra  ac          ;Trampoline for page $4b00 lookups
              4bfc fcfd  bra  $4bfd
              4bfd 1403  ld   $03,y
              4bfe e07b  jmp  y,$7b
              4bff 1519  ld   [$19],y
jupiter69:    4c00 0000  ld   $00
              4c01 0000  ld   $00
              4c02 0000  ld   $00
              * 19 times
              4c13 0011  ld   $11
              4c14 0005  ld   $05
              4c15 0002  ld   $02
              4c16 0005  ld   $05
              4c17 0002  ld   $02
              4c18 0006  ld   $06
              4c19 0002  ld   $02
              4c1a 0006  ld   $06
              4c1b 0006  ld   $06
              4c1c 0006  ld   $06
              4c1d 0006  ld   $06
              4c1e 0016  ld   $16
              4c1f 0006  ld   $06
              4c20 0006  ld   $06
              4c21 0016  ld   $16
              4c22 0006  ld   $06
              4c23 0006  ld   $06
              4c24 0006  ld   $06
              * 6 times
              4c28 0016  ld   $16
              4c29 0006  ld   $06
              4c2a 0006  ld   $06
              4c2b 0006  ld   $06
              * 6 times
              4c2f 0005  ld   $05
              4c30 0006  ld   $06
              4c31 0006  ld   $06
              4c32 0005  ld   $05
              4c33 0006  ld   $06
              4c34 0006  ld   $06
              4c35 0005  ld   $05
              4c36 0006  ld   $06
              4c37 0001  ld   $01
              4c38 0006  ld   $06
              4c39 0006  ld   $06
              4c3a 0001  ld   $01
              4c3b 0006  ld   $06
              4c3c 0006  ld   $06
              4c3d 0006  ld   $06
              4c3e 0006  ld   $06
              4c3f 0016  ld   $16
              4c40 0005  ld   $05
              4c41 0006  ld   $06
              4c42 0006  ld   $06
              4c43 0006  ld   $06
              4c44 0001  ld   $01
              4c45 0006  ld   $06
              4c46 0005  ld   $05
              4c47 0006  ld   $06
              4c48 0001  ld   $01
              4c49 0005  ld   $05
              4c4a 0002  ld   $02
              4c4b 0005  ld   $05
              4c4c 0001  ld   $01
              4c4d 0005  ld   $05
              4c4e 0001  ld   $01
              4c4f 0005  ld   $05
              4c50 0001  ld   $01
              4c51 0001  ld   $01
              4c52 0001  ld   $01
              4c53 0005  ld   $05
              4c54 0001  ld   $01
              4c55 0006  ld   $06
              4c56 0001  ld   $01
              4c57 0005  ld   $05
              4c58 0005  ld   $05
              4c59 0005  ld   $05
              4c5a 0006  ld   $06
              4c5b 0005  ld   $05
              4c5c 0006  ld   $06
              4c5d 0005  ld   $05
              4c5e 0001  ld   $01
              4c5f 0005  ld   $05
              4c60 0001  ld   $01
              4c61 0005  ld   $05
              4c62 0001  ld   $01
              4c63 0005  ld   $05
              4c64 0001  ld   $01
              4c65 0001  ld   $01
              4c66 0005  ld   $05
              4c67 0001  ld   $01
              4c68 0001  ld   $01
              4c69 0005  ld   $05
              4c6a 0001  ld   $01
              4c6b 0001  ld   $01
              4c6c 0001  ld   $01
              * 5 times
              4c6f 0005  ld   $05
              4c70 0001  ld   $01
              4c71 0001  ld   $01
              4c72 0001  ld   $01
              * 5 times
              4c75 0005  ld   $05
              4c76 0001  ld   $01
              4c77 0005  ld   $05
              4c78 0001  ld   $01
              4c79 0001  ld   $01
              4c7a 0011  ld   $11
              4c7b 0005  ld   $05
              4c7c 0015  ld   $15
              4c7d 0001  ld   $01
              4c7e 0015  ld   $15
              4c7f 0005  ld   $05
              4c80 0015  ld   $15
              4c81 0005  ld   $05
              4c82 0015  ld   $15
              4c83 0005  ld   $05
              4c84 0005  ld   $05
              4c85 0015  ld   $15
              4c86 0015  ld   $15
              4c87 0005  ld   $05
              4c88 0001  ld   $01
              4c89 0004  ld   $04
              4c8a 0001  ld   $01
              4c8b 0005  ld   $05
              4c8c 0001  ld   $01
              4c8d 0000  ld   $00
              4c8e 0000  ld   $00
              4c8f 0000  ld   $00
              * 19 times
              4ca0 0200  nop
              4ca1 0200  nop
              4ca2 0200  nop
              * 91 times
              4cfb fe00  bra  ac          ;Trampoline for page $4c00 lookups
              4cfc fcfd  bra  $4cfd
              4cfd 1403  ld   $03,y
              4cfe e07b  jmp  y,$7b
              4cff 1519  ld   [$19],y
jupiter70:    4d00 0000  ld   $00
              4d01 0000  ld   $00
              4d02 0000  ld   $00
              * 18 times
              4d12 0001  ld   $01
              4d13 0001  ld   $01
              4d14 0001  ld   $01
              4d15 0006  ld   $06
              4d16 0002  ld   $02
              4d17 0015  ld   $15
              4d18 0002  ld   $02
              4d19 0005  ld   $05
              4d1a 0002  ld   $02
              4d1b 0005  ld   $05
              4d1c 0016  ld   $16
              4d1d 0006  ld   $06
              4d1e 0002  ld   $02
              4d1f 0006  ld   $06
              4d20 0006  ld   $06
              4d21 0006  ld   $06
              4d22 0006  ld   $06
              4d23 0002  ld   $02
              4d24 0005  ld   $05
              4d25 0002  ld   $02
              4d26 0001  ld   $01
              4d27 0006  ld   $06
              4d28 0001  ld   $01
              4d29 0002  ld   $02
              4d2a 0001  ld   $01
              4d2b 0001  ld   $01
              4d2c 0001  ld   $01
              * 5 times
              4d2f 0002  ld   $02
              4d30 0001  ld   $01
              4d31 0001  ld   $01
              4d32 0001  ld   $01
              * 7 times
              4d37 0005  ld   $05
              4d38 0001  ld   $01
              4d39 0001  ld   $01
              4d3a 0001  ld   $01
              * 12 times
              4d44 0005  ld   $05
              4d45 0001  ld   $01
              4d46 0001  ld   $01
              4d47 0001  ld   $01
              4d48 0006  ld   $06
              4d49 0011  ld   $11
              4d4a 0001  ld   $01
              4d4b 0001  ld   $01
              4d4c 0002  ld   $02
              4d4d 0001  ld   $01
              4d4e 0006  ld   $06
              4d4f 0001  ld   $01
              4d50 0006  ld   $06
              4d51 0005  ld   $05
              4d52 0012  ld   $12
              4d53 0005  ld   $05
              4d54 0002  ld   $02
              4d55 0005  ld   $05
              4d56 0016  ld   $16
              4d57 0006  ld   $06
              4d58 0016  ld   $16
              4d59 0006  ld   $06
              4d5a 0015  ld   $15
              4d5b 0006  ld   $06
              4d5c 0001  ld   $01
              4d5d 0006  ld   $06
              4d5e 0001  ld   $01
              4d5f 0006  ld   $06
              4d60 0005  ld   $05
              4d61 0002  ld   $02
              4d62 0005  ld   $05
              4d63 0001  ld   $01
              4d64 0005  ld   $05
              4d65 0001  ld   $01
              4d66 0001  ld   $01
              4d67 0005  ld   $05
              4d68 0001  ld   $01
              4d69 0001  ld   $01
              4d6a 0005  ld   $05
              4d6b 0001  ld   $01
              4d6c 0005  ld   $05
              4d6d 0001  ld   $01
              4d6e 0005  ld   $05
              4d6f 0001  ld   $01
              4d70 0001  ld   $01
              4d71 0005  ld   $05
              4d72 0001  ld   $01
              4d73 0005  ld   $05
              4d74 0001  ld   $01
              4d75 0001  ld   $01
              4d76 0000  ld   $00
              4d77 0001  ld   $01
              4d78 0000  ld   $00
              4d79 0005  ld   $05
              4d7a 0005  ld   $05
              4d7b 0015  ld   $15
              4d7c 0001  ld   $01
              4d7d 0005  ld   $05
              4d7e 0015  ld   $15
              4d7f 0001  ld   $01
              4d80 0015  ld   $15
              4d81 0001  ld   $01
              4d82 0005  ld   $05
              4d83 0011  ld   $11
              4d84 0005  ld   $05
              4d85 0001  ld   $01
              4d86 0005  ld   $05
              4d87 0001  ld   $01
              4d88 0000  ld   $00
              4d89 0001  ld   $01
              4d8a 0000  ld   $00
              4d8b 0000  ld   $00
              4d8c 0000  ld   $00
              * 22 times
              4da0 0200  nop
              4da1 0200  nop
              4da2 0200  nop
              * 91 times
              4dfb fe00  bra  ac          ;Trampoline for page $4d00 lookups
              4dfc fcfd  bra  $4dfd
              4dfd 1403  ld   $03,y
              4dfe e07b  jmp  y,$7b
              4dff 1519  ld   [$19],y
jupiter71:    4e00 0000  ld   $00
              4e01 0000  ld   $00
              4e02 0000  ld   $00
              * 20 times
              4e14 0015  ld   $15
              4e15 0001  ld   $01
              4e16 0005  ld   $05
              4e17 0002  ld   $02
              4e18 0006  ld   $06
              4e19 0001  ld   $01
              4e1a 0016  ld   $16
              4e1b 0006  ld   $06
              4e1c 0002  ld   $02
              4e1d 0006  ld   $06
              4e1e 0006  ld   $06
              4e1f 0016  ld   $16
              4e20 0006  ld   $06
              4e21 0016  ld   $16
              4e22 0006  ld   $06
              4e23 0016  ld   $16
              4e24 0006  ld   $06
              4e25 0001  ld   $01
              4e26 0006  ld   $06
              4e27 0006  ld   $06
              4e28 0016  ld   $16
              4e29 0006  ld   $06
              4e2a 0016  ld   $16
              4e2b 0006  ld   $06
              4e2c 0006  ld   $06
              4e2d 0016  ld   $16
              4e2e 0006  ld   $06
              4e2f 0002  ld   $02
              4e30 0005  ld   $05
              4e31 0006  ld   $06
              4e32 0001  ld   $01
              4e33 0006  ld   $06
              4e34 0012  ld   $12
              4e35 0006  ld   $06
              4e36 0001  ld   $01
              4e37 0002  ld   $02
              4e38 0001  ld   $01
              4e39 0001  ld   $01
              4e3a 0005  ld   $05
              4e3b 0001  ld   $01
              4e3c 0006  ld   $06
              4e3d 0005  ld   $05
              4e3e 0006  ld   $06
              4e3f 0005  ld   $05
              4e40 0002  ld   $02
              4e41 0005  ld   $05
              4e42 0006  ld   $06
              4e43 0016  ld   $16
              4e44 0006  ld   $06
              4e45 0016  ld   $16
              4e46 0006  ld   $06
              4e47 0016  ld   $16
              4e48 0016  ld   $16
              4e49 0006  ld   $06
              4e4a 0006  ld   $06
              4e4b 0016  ld   $16
              4e4c 0006  ld   $06
              4e4d 0016  ld   $16
              4e4e 0006  ld   $06
              4e4f 0011  ld   $11
              4e50 0006  ld   $06
              4e51 0016  ld   $16
              4e52 0006  ld   $06
              4e53 0016  ld   $16
              4e54 0006  ld   $06
              4e55 0016  ld   $16
              4e56 0005  ld   $05
              4e57 0002  ld   $02
              4e58 0005  ld   $05
              4e59 0001  ld   $01
              4e5a 0006  ld   $06
              4e5b 0011  ld   $11
              4e5c 0006  ld   $06
              4e5d 0005  ld   $05
              4e5e 0016  ld   $16
              4e5f 0005  ld   $05
              4e60 0006  ld   $06
              4e61 0005  ld   $05
              4e62 0015  ld   $15
              4e63 0002  ld   $02
              4e64 0005  ld   $05
              4e65 0006  ld   $06
              4e66 0005  ld   $05
              4e67 0012  ld   $12
              4e68 0005  ld   $05
              4e69 0005  ld   $05
              4e6a 0001  ld   $01
              4e6b 0015  ld   $15
              4e6c 0001  ld   $01
              4e6d 0001  ld   $01
              4e6e 0000  ld   $00
              4e6f 0001  ld   $01
              4e70 0001  ld   $01
              4e71 0000  ld   $00
              4e72 0001  ld   $01
              4e73 0000  ld   $00
              4e74 0001  ld   $01
              4e75 0000  ld   $00
              4e76 0005  ld   $05
              4e77 0000  ld   $00
              4e78 0001  ld   $01
              4e79 0000  ld   $00
              4e7a 0011  ld   $11
              4e7b 0001  ld   $01
              4e7c 0005  ld   $05
              4e7d 0015  ld   $15
              4e7e 0005  ld   $05
              4e7f 0005  ld   $05
              4e80 0001  ld   $01
              4e81 0015  ld   $15
              4e82 0001  ld   $01
              4e83 0005  ld   $05
              4e84 0001  ld   $01
              4e85 0005  ld   $05
              4e86 0001  ld   $01
              4e87 0000  ld   $00
              4e88 0005  ld   $05
              4e89 0001  ld   $01
              4e8a 0005  ld   $05
              4e8b 0000  ld   $00
              4e8c 0000  ld   $00
              4e8d 0000  ld   $00
              * 21 times
              4ea0 0200  nop
              4ea1 0200  nop
              4ea2 0200  nop
              * 91 times
              4efb fe00  bra  ac          ;Trampoline for page $4e00 lookups
              4efc fcfd  bra  $4efd
              4efd 1403  ld   $03,y
              4efe e07b  jmp  y,$7b
              4eff 1519  ld   [$19],y
jupiter72:    4f00 0000  ld   $00
              4f01 0000  ld   $00
              4f02 0000  ld   $00
              * 20 times
              4f14 0005  ld   $05
              4f15 0002  ld   $02
              4f16 0001  ld   $01
              4f17 0005  ld   $05
              4f18 0002  ld   $02
              4f19 0006  ld   $06
              4f1a 0005  ld   $05
              4f1b 0002  ld   $02
              4f1c 0016  ld   $16
              4f1d 0006  ld   $06
              4f1e 0002  ld   $02
              4f1f 0016  ld   $16
              4f20 0016  ld   $16
              4f21 0017  ld   $17
              4f22 001a  ld   $1a
              4f23 0017  ld   $17
              4f24 001a  ld   $1a
              4f25 001b  ld   $1b
              4f26 0016  ld   $16
              4f27 001b  ld   $1b
              4f28 001b  ld   $1b
              4f29 001b  ld   $1b
              4f2a 0016  ld   $16
              4f2b 0017  ld   $17
              4f2c 0016  ld   $16
              4f2d 0016  ld   $16
              4f2e 0016  ld   $16
              * 6 times
              4f32 001b  ld   $1b
              4f33 0016  ld   $16
              4f34 001a  ld   $1a
              4f35 0016  ld   $16
              4f36 0016  ld   $16
              4f37 001a  ld   $1a
              4f38 0016  ld   $16
              4f39 0016  ld   $16
              4f3a 0016  ld   $16
              * 8 times
              4f40 001a  ld   $1a
              4f41 0016  ld   $16
              4f42 0016  ld   $16
              4f43 0016  ld   $16
              * 5 times
              4f46 001a  ld   $1a
              4f47 0016  ld   $16
              4f48 0006  ld   $06
              4f49 001a  ld   $1a
              4f4a 0016  ld   $16
              4f4b 0016  ld   $16
              4f4c 001a  ld   $1a
              4f4d 0006  ld   $06
              4f4e 0016  ld   $16
              4f4f 0006  ld   $06
              4f50 0006  ld   $06
              4f51 0005  ld   $05
              4f52 0006  ld   $06
              4f53 0001  ld   $01
              4f54 0006  ld   $06
              4f55 0001  ld   $01
              4f56 0002  ld   $02
              4f57 0005  ld   $05
              4f58 0001  ld   $01
              4f59 0005  ld   $05
              4f5a 0001  ld   $01
              4f5b 0005  ld   $05
              4f5c 0001  ld   $01
              4f5d 0005  ld   $05
              4f5e 0001  ld   $01
              4f5f 0001  ld   $01
              4f60 0001  ld   $01
              4f61 0001  ld   $01
              4f62 0006  ld   $06
              4f63 0005  ld   $05
              4f64 0001  ld   $01
              4f65 0001  ld   $01
              4f66 0005  ld   $05
              4f67 0001  ld   $01
              4f68 0005  ld   $05
              4f69 0001  ld   $01
              4f6a 0001  ld   $01
              4f6b 0001  ld   $01
              4f6c 0001  ld   $01
              4f6d 0005  ld   $05
              4f6e 0001  ld   $01
              4f6f 0005  ld   $05
              4f70 0000  ld   $00
              4f71 0001  ld   $01
              4f72 0001  ld   $01
              4f73 0000  ld   $00
              4f74 0001  ld   $01
              4f75 0010  ld   $10
              4f76 0001  ld   $01
              4f77 0000  ld   $00
              4f78 0001  ld   $01
              4f79 0004  ld   $04
              4f7a 0001  ld   $01
              4f7b 0005  ld   $05
              4f7c 0011  ld   $11
              4f7d 0001  ld   $01
              4f7e 0001  ld   $01
              4f7f 0000  ld   $00
              4f80 0005  ld   $05
              4f81 0001  ld   $01
              4f82 0000  ld   $00
              4f83 0001  ld   $01
              4f84 0000  ld   $00
              4f85 0005  ld   $05
              4f86 0000  ld   $00
              4f87 0011  ld   $11
              4f88 0001  ld   $01
              4f89 0000  ld   $00
              4f8a 0001  ld   $01
              4f8b 0000  ld   $00
              4f8c 0000  ld   $00
              4f8d 0000  ld   $00
              * 21 times
              4fa0 0200  nop
              4fa1 0200  nop
              4fa2 0200  nop
              * 91 times
              4ffb fe00  bra  ac          ;Trampoline for page $4f00 lookups
              4ffc fcfd  bra  $4ffd
              4ffd 1403  ld   $03,y
              4ffe e07b  jmp  y,$7b
              4fff 1519  ld   [$19],y
jupiter73:    5000 0000  ld   $00
              5001 0000  ld   $00
              5002 0000  ld   $00
              * 20 times
              5014 0001  ld   $01
              5015 0015  ld   $15
              5016 0006  ld   $06
              5017 0016  ld   $16
              5018 0002  ld   $02
              5019 0005  ld   $05
              501a 0016  ld   $16
              501b 0006  ld   $06
              501c 0006  ld   $06
              501d 0006  ld   $06
              501e 0016  ld   $16
              501f 001b  ld   $1b
              5020 001a  ld   $1a
              5021 001b  ld   $1b
              5022 001b  ld   $1b
              5023 001b  ld   $1b
              * 6 times
              5027 002b  ld   $2b
              5028 001b  ld   $1b
              5029 001b  ld   $1b
              502a 001a  ld   $1a
              502b 001b  ld   $1b
              502c 001b  ld   $1b
              502d 001b  ld   $1b
              * 18 times
              503d 0006  ld   $06
              503e 0006  ld   $06
              503f 001b  ld   $1b
              5040 0006  ld   $06
              5041 0006  ld   $06
              5042 0016  ld   $16
              5043 000b  ld   $0b
              5044 001b  ld   $1b
              5045 000a  ld   $0a
              5046 0006  ld   $06
              5047 0006  ld   $06
              5048 0006  ld   $06
              5049 0006  ld   $06
              504a 0016  ld   $16
              504b 0006  ld   $06
              504c 0006  ld   $06
              504d 0006  ld   $06
              * 5 times
              5050 0011  ld   $11
              5051 0006  ld   $06
              5052 0001  ld   $01
              5053 0006  ld   $06
              5054 0006  ld   $06
              5055 0005  ld   $05
              5056 0006  ld   $06
              5057 0005  ld   $05
              5058 0016  ld   $16
              5059 0006  ld   $06
              505a 0016  ld   $16
              505b 0006  ld   $06
              505c 0006  ld   $06
              505d 0001  ld   $01
              505e 0006  ld   $06
              505f 0006  ld   $06
              5060 0016  ld   $16
              5061 0006  ld   $06
              5062 0005  ld   $05
              5063 0001  ld   $01
              5064 0005  ld   $05
              5065 0006  ld   $06
              5066 0001  ld   $01
              5067 0005  ld   $05
              5068 0001  ld   $01
              5069 0005  ld   $05
              506a 0015  ld   $15
              506b 0001  ld   $01
              506c 0005  ld   $05
              506d 0001  ld   $01
              506e 0005  ld   $05
              506f 0011  ld   $11
              5070 0001  ld   $01
              5071 0005  ld   $05
              5072 0015  ld   $15
              5073 0001  ld   $01
              5074 0004  ld   $04
              5075 0001  ld   $01
              5076 0005  ld   $05
              5077 0000  ld   $00
              5078 0001  ld   $01
              5079 0000  ld   $00
              507a 0001  ld   $01
              507b 0000  ld   $00
              507c 0005  ld   $05
              507d 0000  ld   $00
              507e 0005  ld   $05
              507f 0000  ld   $00
              5080 0001  ld   $01
              5081 0004  ld   $04
              5082 0001  ld   $01
              5083 0005  ld   $05
              5084 0001  ld   $01
              5085 0001  ld   $01
              5086 0005  ld   $05
              5087 0000  ld   $00
              5088 0005  ld   $05
              5089 0001  ld   $01
              508a 0015  ld   $15
              508b 0000  ld   $00
              508c 0000  ld   $00
              508d 0000  ld   $00
              * 21 times
              50a0 0200  nop
              50a1 0200  nop
              50a2 0200  nop
              * 91 times
              50fb fe00  bra  ac          ;Trampoline for page $5000 lookups
              50fc fcfd  bra  $50fd
              50fd 1403  ld   $03,y
              50fe e07b  jmp  y,$7b
              50ff 1519  ld   [$19],y
jupiter74:    5100 0000  ld   $00
              5101 0000  ld   $00
              5102 0000  ld   $00
              * 20 times
              5114 0001  ld   $01
              5115 0006  ld   $06
              5116 0016  ld   $16
              5117 0002  ld   $02
              5118 0016  ld   $16
              5119 0006  ld   $06
              511a 0001  ld   $01
              511b 0006  ld   $06
              511c 0016  ld   $16
              511d 002b  ld   $2b
              511e 001f  ld   $1f
              511f 001b  ld   $1b
              5120 002b  ld   $2b
              5121 002f  ld   $2f
              5122 002b  ld   $2b
              5123 002b  ld   $2b
              5124 001b  ld   $1b
              5125 001a  ld   $1a
              5126 001b  ld   $1b
              5127 001b  ld   $1b
              5128 001b  ld   $1b
              5129 001a  ld   $1a
              512a 001b  ld   $1b
              512b 002f  ld   $2f
              512c 001b  ld   $1b
              512d 002b  ld   $2b
              512e 001b  ld   $1b
              512f 001b  ld   $1b
              5130 001b  ld   $1b
              5131 0016  ld   $16
              5132 0006  ld   $06
              5133 0006  ld   $06
              5134 0016  ld   $16
              5135 0006  ld   $06
              5136 0006  ld   $06
              5137 0016  ld   $16
              5138 0006  ld   $06
              5139 001a  ld   $1a
              513a 0006  ld   $06
              513b 0016  ld   $16
              513c 0006  ld   $06
              513d 001a  ld   $1a
              513e 0006  ld   $06
              513f 0016  ld   $16
              5140 0006  ld   $06
              5141 001a  ld   $1a
              5142 0007  ld   $07
              5143 0016  ld   $16
              5144 0006  ld   $06
              5145 0006  ld   $06
              5146 0016  ld   $16
              5147 0006  ld   $06
              5148 0016  ld   $16
              5149 0006  ld   $06
              514a 0016  ld   $16
              514b 0006  ld   $06
              514c 0016  ld   $16
              514d 0005  ld   $05
              514e 0016  ld   $16
              514f 0006  ld   $06
              5150 0006  ld   $06
              5151 0016  ld   $16
              5152 0016  ld   $16
              5153 0016  ld   $16
              5154 0006  ld   $06
              5155 0016  ld   $16
              5156 0016  ld   $16
              5157 0016  ld   $16
              * 9 times
              515e 0006  ld   $06
              515f 0015  ld   $15
              5160 0006  ld   $06
              5161 0016  ld   $16
              5162 0006  ld   $06
              5163 0016  ld   $16
              5164 0015  ld   $15
              5165 0016  ld   $16
              5166 0006  ld   $06
              5167 0016  ld   $16
              5168 0016  ld   $16
              5169 0006  ld   $06
              516a 0016  ld   $16
              516b 0005  ld   $05
              516c 0015  ld   $15
              516d 0001  ld   $01
              516e 0015  ld   $15
              516f 0005  ld   $05
              5170 0005  ld   $05
              5171 0015  ld   $15
              5172 0001  ld   $01
              5173 0005  ld   $05
              5174 0015  ld   $15
              5175 0001  ld   $01
              5176 0005  ld   $05
              5177 0011  ld   $11
              5178 0005  ld   $05
              5179 0001  ld   $01
              517a 0004  ld   $04
              517b 0001  ld   $01
              517c 0000  ld   $00
              517d 0001  ld   $01
              517e 0001  ld   $01
              517f 0005  ld   $05
              5180 0000  ld   $00
              5181 0001  ld   $01
              5182 0000  ld   $00
              5183 0005  ld   $05
              5184 0000  ld   $00
              5185 0005  ld   $05
              5186 0000  ld   $00
              5187 0001  ld   $01
              5188 0005  ld   $05
              5189 0000  ld   $00
              518a 0001  ld   $01
              518b 0000  ld   $00
              518c 0000  ld   $00
              518d 0000  ld   $00
              * 21 times
              51a0 0200  nop
              51a1 0200  nop
              51a2 0200  nop
              * 91 times
              51fb fe00  bra  ac          ;Trampoline for page $5100 lookups
              51fc fcfd  bra  $51fd
              51fd 1403  ld   $03,y
              51fe e07b  jmp  y,$7b
              51ff 1519  ld   [$19],y
jupiter75:    5200 0000  ld   $00
              5201 0000  ld   $00
              5202 0000  ld   $00
              * 21 times
              5215 0015  ld   $15
              5216 0006  ld   $06
              5217 0006  ld   $06
              5218 0016  ld   $16
              5219 0006  ld   $06
              521a 0005  ld   $05
              521b 0016  ld   $16
              521c 001e  ld   $1e
              521d 002b  ld   $2b
              521e 002b  ld   $2b
              521f 002b  ld   $2b
              5220 001b  ld   $1b
              5221 002b  ld   $2b
              5222 001f  ld   $1f
              5223 002b  ld   $2b
              5224 001a  ld   $1a
              5225 001b  ld   $1b
              5226 002b  ld   $2b
              5227 001a  ld   $1a
              5228 0016  ld   $16
              5229 001b  ld   $1b
              522a 002b  ld   $2b
              522b 001b  ld   $1b
              522c 001b  ld   $1b
              522d 001a  ld   $1a
              522e 0016  ld   $16
              522f 0016  ld   $16
              5230 0006  ld   $06
              5231 0006  ld   $06
              5232 0006  ld   $06
              5233 001b  ld   $1b
              5234 0016  ld   $16
              5235 001b  ld   $1b
              5236 001b  ld   $1b
              5237 000b  ld   $0b
              5238 001b  ld   $1b
              5239 0017  ld   $17
              523a 000a  ld   $0a
              523b 0017  ld   $17
              523c 001a  ld   $1a
              523d 001b  ld   $1b
              523e 0007  ld   $07
              523f 001a  ld   $1a
              5240 0017  ld   $17
              5241 0006  ld   $06
              5242 001a  ld   $1a
              5243 0006  ld   $06
              5244 0016  ld   $16
              5245 0006  ld   $06
              5246 0006  ld   $06
              5247 0016  ld   $16
              5248 0006  ld   $06
              5249 0016  ld   $16
              524a 0006  ld   $06
              524b 0006  ld   $06
              524c 0006  ld   $06
              524d 0016  ld   $16
              524e 0016  ld   $16
              524f 0006  ld   $06
              5250 0016  ld   $16
              5251 0006  ld   $06
              5252 0016  ld   $16
              5253 0006  ld   $06
              5254 0016  ld   $16
              5255 0006  ld   $06
              5256 0006  ld   $06
              5257 0006  ld   $06
              5258 001a  ld   $1a
              5259 0016  ld   $16
              525a 000a  ld   $0a
              525b 0005  ld   $05
              525c 001a  ld   $1a
              525d 0006  ld   $06
              525e 0005  ld   $05
              525f 0016  ld   $16
              5260 0005  ld   $05
              5261 0016  ld   $16
              5262 0005  ld   $05
              5263 0015  ld   $15
              5264 0006  ld   $06
              5265 0006  ld   $06
              5266 0015  ld   $15
              5267 0015  ld   $15
              5268 0006  ld   $06
              5269 0015  ld   $15
              526a 0015  ld   $15
              526b 0006  ld   $06
              526c 0005  ld   $05
              526d 0016  ld   $16
              526e 0005  ld   $05
              526f 0016  ld   $16
              5270 0001  ld   $01
              5271 0006  ld   $06
              5272 0015  ld   $15
              5273 0005  ld   $05
              5274 0001  ld   $01
              5275 0015  ld   $15
              5276 0005  ld   $05
              5277 0005  ld   $05
              5278 0005  ld   $05
              5279 0000  ld   $00
              527a 0001  ld   $01
              527b 0001  ld   $01
              527c 0005  ld   $05
              527d 0000  ld   $00
              527e 0005  ld   $05
              527f 0000  ld   $00
              5280 0001  ld   $01
              5281 0001  ld   $01
              5282 0005  ld   $05
              5283 0000  ld   $00
              5284 0001  ld   $01
              5285 0000  ld   $00
              5286 0001  ld   $01
              5287 0000  ld   $00
              5288 0001  ld   $01
              5289 0005  ld   $05
              528a 0000  ld   $00
              528b 0000  ld   $00
              528c 0000  ld   $00
              * 22 times
              52a0 0200  nop
              52a1 0200  nop
              52a2 0200  nop
              * 91 times
              52fb fe00  bra  ac          ;Trampoline for page $5200 lookups
              52fc fcfd  bra  $52fd
              52fd 1403  ld   $03,y
              52fe e07b  jmp  y,$7b
              52ff 1519  ld   [$19],y
jupiter76:    5300 0000  ld   $00
              5301 0000  ld   $00
              5302 0000  ld   $00
              * 20 times
              5314 0001  ld   $01
              5315 0015  ld   $15
              5316 001a  ld   $1a
              5317 0016  ld   $16
              5318 0016  ld   $16
              5319 0006  ld   $06
              531a 0016  ld   $16
              531b 0016  ld   $16
              531c 001b  ld   $1b
              531d 002f  ld   $2f
              531e 002b  ld   $2b
              531f 002f  ld   $2f
              5320 002b  ld   $2b
              5321 002f  ld   $2f
              5322 002b  ld   $2b
              5323 002b  ld   $2b
              5324 001b  ld   $1b
              5325 002b  ld   $2b
              5326 001b  ld   $1b
              5327 002b  ld   $2b
              5328 002b  ld   $2b
              5329 002b  ld   $2b
              532a 001b  ld   $1b
              532b 0016  ld   $16
              532c 0005  ld   $05
              532d 0006  ld   $06
              532e 0006  ld   $06
              532f 0006  ld   $06
              * 5 times
              5332 0016  ld   $16
              5333 0006  ld   $06
              5334 0017  ld   $17
              5335 000a  ld   $0a
              5336 0017  ld   $17
              5337 0016  ld   $16
              5338 001b  ld   $1b
              5339 001a  ld   $1a
              533a 001b  ld   $1b
              533b 001b  ld   $1b
              533c 001b  ld   $1b
              533d 0017  ld   $17
              533e 001a  ld   $1a
              533f 001b  ld   $1b
              5340 001a  ld   $1a
              5341 0006  ld   $06
              5342 0016  ld   $16
              5343 000a  ld   $0a
              5344 0016  ld   $16
              5345 0006  ld   $06
              5346 0016  ld   $16
              5347 0006  ld   $06
              5348 0016  ld   $16
              5349 0006  ld   $06
              534a 0016  ld   $16
              534b 0005  ld   $05
              534c 0016  ld   $16
              534d 0006  ld   $06
              534e 0006  ld   $06
              534f 0016  ld   $16
              5350 0006  ld   $06
              5351 0006  ld   $06
              5352 0016  ld   $16
              5353 000a  ld   $0a
              5354 0016  ld   $16
              5355 0016  ld   $16
              5356 0016  ld   $16
              5357 0006  ld   $06
              5358 0016  ld   $16
              5359 0006  ld   $06
              535a 0016  ld   $16
              535b 0016  ld   $16
              535c 0006  ld   $06
              535d 0016  ld   $16
              535e 0006  ld   $06
              535f 0015  ld   $15
              5360 0006  ld   $06
              5361 0015  ld   $15
              5362 0006  ld   $06
              5363 0016  ld   $16
              5364 0005  ld   $05
              5365 0015  ld   $15
              5366 0006  ld   $06
              5367 0005  ld   $05
              5368 0016  ld   $16
              5369 0005  ld   $05
              536a 0002  ld   $02
              536b 0005  ld   $05
              536c 0001  ld   $01
              536d 0005  ld   $05
              536e 0001  ld   $01
              536f 0001  ld   $01
              5370 0005  ld   $05
              5371 0001  ld   $01
              5372 0001  ld   $01
              5373 0005  ld   $05
              5374 0001  ld   $01
              5375 0005  ld   $05
              5376 0001  ld   $01
              5377 0011  ld   $11
              5378 0001  ld   $01
              5379 0005  ld   $05
              537a 0000  ld   $00
              537b 0005  ld   $05
              537c 0001  ld   $01
              537d 0001  ld   $01
              537e 0004  ld   $04
              537f 0001  ld   $01
              5380 0004  ld   $04
              5381 0001  ld   $01
              5382 0000  ld   $00
              5383 0001  ld   $01
              5384 0004  ld   $04
              5385 0001  ld   $01
              5386 0005  ld   $05
              5387 0000  ld   $00
              5388 0001  ld   $01
              5389 0000  ld   $00
              538a 0000  ld   $00
              538b 0000  ld   $00
              * 23 times
              53a0 0200  nop
              53a1 0200  nop
              53a2 0200  nop
              * 91 times
              53fb fe00  bra  ac          ;Trampoline for page $5300 lookups
              53fc fcfd  bra  $53fd
              53fd 1403  ld   $03,y
              53fe e07b  jmp  y,$7b
              53ff 1519  ld   [$19],y
jupiter77:    5400 0000  ld   $00
              5401 0000  ld   $00
              5402 0000  ld   $00
              * 21 times
              5415 0005  ld   $05
              5416 001b  ld   $1b
              5417 0016  ld   $16
              5418 0006  ld   $06
              5419 0005  ld   $05
              541a 001b  ld   $1b
              541b 001f  ld   $1f
              541c 002b  ld   $2b
              541d 002b  ld   $2b
              541e 002f  ld   $2f
              541f 001b  ld   $1b
              5420 002b  ld   $2b
              5421 001a  ld   $1a
              5422 002b  ld   $2b
              5423 001b  ld   $1b
              5424 002b  ld   $2b
              5425 002b  ld   $2b
              5426 001b  ld   $1b
              5427 002f  ld   $2f
              5428 001b  ld   $1b
              5429 001b  ld   $1b
              542a 0006  ld   $06
              542b 0006  ld   $06
              542c 0006  ld   $06
              542d 0016  ld   $16
              542e 0006  ld   $06
              542f 0016  ld   $16
              5430 0006  ld   $06
              5431 0016  ld   $16
              5432 0006  ld   $06
              5433 0006  ld   $06
              5434 0006  ld   $06
              * 6 times
              5438 0016  ld   $16
              5439 0007  ld   $07
              543a 0016  ld   $16
              543b 0017  ld   $17
              543c 001a  ld   $1a
              543d 001b  ld   $1b
              543e 0016  ld   $16
              543f 0016  ld   $16
              5440 0007  ld   $07
              5441 001a  ld   $1a
              5442 0007  ld   $07
              5443 0016  ld   $16
              5444 0006  ld   $06
              5445 0006  ld   $06
              5446 0016  ld   $16
              5447 001a  ld   $1a
              5448 0006  ld   $06
              5449 0016  ld   $16
              544a 0006  ld   $06
              544b 0006  ld   $06
              544c 0016  ld   $16
              544d 0006  ld   $06
              544e 0016  ld   $16
              544f 0006  ld   $06
              5450 001a  ld   $1a
              5451 0005  ld   $05
              5452 0016  ld   $16
              5453 0006  ld   $06
              5454 0016  ld   $16
              5455 0005  ld   $05
              5456 0006  ld   $06
              5457 0005  ld   $05
              5458 0016  ld   $16
              5459 0005  ld   $05
              545a 001a  ld   $1a
              545b 0006  ld   $06
              545c 0015  ld   $15
              545d 0006  ld   $06
              545e 0005  ld   $05
              545f 0006  ld   $06
              5460 0015  ld   $15
              5461 0006  ld   $06
              5462 0016  ld   $16
              5463 0005  ld   $05
              5464 0006  ld   $06
              5465 0015  ld   $15
              5466 0006  ld   $06
              5467 0015  ld   $15
              5468 0005  ld   $05
              5469 0006  ld   $06
              546a 0015  ld   $15
              546b 0001  ld   $01
              546c 0005  ld   $05
              546d 0011  ld   $11
              546e 0005  ld   $05
              546f 0005  ld   $05
              5470 0001  ld   $01
              5471 0005  ld   $05
              5472 0005  ld   $05
              5473 0001  ld   $01
              5474 0005  ld   $05
              5475 0001  ld   $01
              5476 0001  ld   $01
              5477 0005  ld   $05
              5478 0000  ld   $00
              5479 0005  ld   $05
              547a 0001  ld   $01
              547b 0004  ld   $04
              547c 0011  ld   $11
              547d 0005  ld   $05
              547e 0011  ld   $11
              547f 0001  ld   $01
              5480 0001  ld   $01
              5481 0005  ld   $05
              5482 0001  ld   $01
              5483 0005  ld   $05
              5484 0001  ld   $01
              5485 0001  ld   $01
              5486 0000  ld   $00
              5487 0001  ld   $01
              5488 0005  ld   $05
              5489 0001  ld   $01
              548a 0000  ld   $00
              548b 0000  ld   $00
              548c 0000  ld   $00
              * 22 times
              54a0 0200  nop
              54a1 0200  nop
              54a2 0200  nop
              * 91 times
              54fb fe00  bra  ac          ;Trampoline for page $5400 lookups
              54fc fcfd  bra  $54fd
              54fd 1403  ld   $03,y
              54fe e07b  jmp  y,$7b
              54ff 1519  ld   [$19],y
jupiter78:    5500 0000  ld   $00
              5501 0000  ld   $00
              5502 0000  ld   $00
              * 21 times
              5515 0011  ld   $11
              5516 0016  ld   $16
              5517 0006  ld   $06
              5518 0006  ld   $06
              5519 0016  ld   $16
              551a 001b  ld   $1b
              551b 002b  ld   $2b
              551c 002b  ld   $2b
              551d 001f  ld   $1f
              551e 002a  ld   $2a
              551f 002b  ld   $2b
              5520 001f  ld   $1f
              5521 002b  ld   $2b
              5522 001b  ld   $1b
              5523 002f  ld   $2f
              5524 001a  ld   $1a
              5525 001b  ld   $1b
              5526 001a  ld   $1a
              5527 001b  ld   $1b
              5528 0016  ld   $16
              5529 0005  ld   $05
              552a 0006  ld   $06
              552b 0016  ld   $16
              552c 0017  ld   $17
              552d 001b  ld   $1b
              552e 001b  ld   $1b
              552f 001b  ld   $1b
              * 6 times
              5533 0016  ld   $16
              5534 0006  ld   $06
              5535 0016  ld   $16
              5536 0006  ld   $06
              5537 0002  ld   $02
              5538 0006  ld   $06
              5539 0002  ld   $02
              553a 0006  ld   $06
              553b 0006  ld   $06
              553c 0006  ld   $06
              553d 0017  ld   $17
              553e 0006  ld   $06
              553f 0006  ld   $06
              5540 0006  ld   $06
              5541 0016  ld   $16
              5542 0006  ld   $06
              5543 0006  ld   $06
              5544 0006  ld   $06
              5545 0016  ld   $16
              5546 0006  ld   $06
              5547 0006  ld   $06
              5548 0016  ld   $16
              5549 0006  ld   $06
              554a 0016  ld   $16
              554b 0006  ld   $06
              554c 0006  ld   $06
              554d 0016  ld   $16
              554e 0006  ld   $06
              554f 0006  ld   $06
              5550 0016  ld   $16
              5551 0006  ld   $06
              5552 001a  ld   $1a
              5553 0006  ld   $06
              5554 0016  ld   $16
              5555 000a  ld   $0a
              5556 0016  ld   $16
              5557 0016  ld   $16
              5558 001a  ld   $1a
              5559 0006  ld   $06
              555a 0016  ld   $16
              555b 0005  ld   $05
              555c 0016  ld   $16
              555d 0006  ld   $06
              555e 0016  ld   $16
              555f 0005  ld   $05
              5560 0006  ld   $06
              5561 0006  ld   $06
              5562 0015  ld   $15
              5563 0006  ld   $06
              5564 0015  ld   $15
              5565 0002  ld   $02
              5566 0005  ld   $05
              5567 0016  ld   $16
              5568 0001  ld   $01
              5569 0005  ld   $05
              556a 0016  ld   $16
              556b 0001  ld   $01
              556c 0006  ld   $06
              556d 0005  ld   $05
              556e 0001  ld   $01
              556f 0005  ld   $05
              5570 0001  ld   $01
              5571 0015  ld   $15
              5572 0001  ld   $01
              5573 0005  ld   $05
              5574 0011  ld   $11
              5575 0005  ld   $05
              5576 0004  ld   $04
              5577 0001  ld   $01
              5578 0005  ld   $05
              5579 0001  ld   $01
              557a 0005  ld   $05
              557b 0001  ld   $01
              557c 0005  ld   $05
              557d 0001  ld   $01
              557e 0005  ld   $05
              557f 0004  ld   $04
              5580 0011  ld   $11
              5581 0000  ld   $00
              5582 0005  ld   $05
              5583 0000  ld   $00
              5584 0000  ld   $00
              5585 0005  ld   $05
              5586 0000  ld   $00
              5587 0015  ld   $15
              5588 0001  ld   $01
              5589 0000  ld   $00
              558a 0000  ld   $00
              558b 0000  ld   $00
              * 23 times
              55a0 0200  nop
              55a1 0200  nop
              55a2 0200  nop
              * 91 times
              55fb fe00  bra  ac          ;Trampoline for page $5500 lookups
              55fc fcfd  bra  $55fd
              55fd 1403  ld   $03,y
              55fe e07b  jmp  y,$7b
              55ff 1519  ld   [$19],y
jupiter79:    5600 0000  ld   $00
              5601 0000  ld   $00
              5602 0000  ld   $00
              * 22 times
              5616 0016  ld   $16
              5617 001a  ld   $1a
              5618 001a  ld   $1a
              5619 002b  ld   $2b
              561a 002b  ld   $2b
              561b 002b  ld   $2b
              561c 001a  ld   $1a
              561d 002b  ld   $2b
              561e 001b  ld   $1b
              561f 002b  ld   $2b
              5620 002b  ld   $2b
              5621 001b  ld   $1b
              5622 002b  ld   $2b
              5623 001b  ld   $1b
              5624 002b  ld   $2b
              5625 001b  ld   $1b
              5626 001b  ld   $1b
              5627 0016  ld   $16
              5628 0006  ld   $06
              5629 0006  ld   $06
              562a 0017  ld   $17
              562b 001b  ld   $1b
              562c 001b  ld   $1b
              562d 001b  ld   $1b
              * 11 times
              5636 0017  ld   $17
              5637 0006  ld   $06
              5638 0005  ld   $05
              5639 0002  ld   $02
              563a 0005  ld   $05
              563b 0002  ld   $02
              563c 0016  ld   $16
              563d 0006  ld   $06
              563e 0006  ld   $06
              563f 001b  ld   $1b
              5640 0016  ld   $16
              5641 001b  ld   $1b
              5642 0006  ld   $06
              5643 001a  ld   $1a
              5644 0017  ld   $17
              5645 001a  ld   $1a
              5646 001b  ld   $1b
              5647 001a  ld   $1a
              5648 0016  ld   $16
              5649 000a  ld   $0a
              564a 0016  ld   $16
              564b 001a  ld   $1a
              564c 0016  ld   $16
              564d 001a  ld   $1a
              564e 0016  ld   $16
              564f 001a  ld   $1a
              5650 0006  ld   $06
              5651 001a  ld   $1a
              5652 0016  ld   $16
              5653 0006  ld   $06
              5654 001a  ld   $1a
              5655 0016  ld   $16
              5656 0006  ld   $06
              5657 0016  ld   $16
              5658 0006  ld   $06
              5659 0015  ld   $15
              565a 0006  ld   $06
              565b 0016  ld   $16
              565c 0006  ld   $06
              565d 0005  ld   $05
              565e 0006  ld   $06
              565f 0005  ld   $05
              5660 0016  ld   $16
              5661 0005  ld   $05
              5662 0001  ld   $01
              5663 0005  ld   $05
              5664 0002  ld   $02
              5665 0005  ld   $05
              5666 0005  ld   $05
              5667 0001  ld   $01
              5668 0005  ld   $05
              5669 0001  ld   $01
              566a 0005  ld   $05
              566b 0001  ld   $01
              566c 0005  ld   $05
              566d 0001  ld   $01
              566e 0005  ld   $05
              566f 0001  ld   $01
              5670 0005  ld   $05
              5671 0001  ld   $01
              5672 0005  ld   $05
              5673 0001  ld   $01
              5674 0005  ld   $05
              5675 0001  ld   $01
              5676 0001  ld   $01
              5677 0001  ld   $01
              5678 0005  ld   $05
              5679 0001  ld   $01
              567a 0000  ld   $00
              567b 0005  ld   $05
              567c 0001  ld   $01
              567d 0004  ld   $04
              567e 0001  ld   $01
              567f 0001  ld   $01
              5680 0004  ld   $04
              5681 0001  ld   $01
              5682 0000  ld   $00
              5683 0001  ld   $01
              5684 0001  ld   $01
              5685 0001  ld   $01
              5686 0001  ld   $01
              5687 0000  ld   $00
              5688 0005  ld   $05
              5689 0000  ld   $00
              568a 0000  ld   $00
              568b 0000  ld   $00
              * 23 times
              56a0 0200  nop
              56a1 0200  nop
              56a2 0200  nop
              * 91 times
              56fb fe00  bra  ac          ;Trampoline for page $5600 lookups
              56fc fcfd  bra  $56fd
              56fd 1403  ld   $03,y
              56fe e07b  jmp  y,$7b
              56ff 1519  ld   [$19],y
jupiter80:    5700 0000  ld   $00
              5701 0000  ld   $00
              5702 0000  ld   $00
              * 22 times
              5716 0015  ld   $15
              5717 001b  ld   $1b
              5718 001b  ld   $1b
              5719 001a  ld   $1a
              571a 001b  ld   $1b
              571b 002b  ld   $2b
              571c 001b  ld   $1b
              571d 001a  ld   $1a
              571e 001a  ld   $1a
              571f 002b  ld   $2b
              5720 001a  ld   $1a
              5721 002f  ld   $2f
              5722 001b  ld   $1b
              5723 001b  ld   $1b
              5724 001b  ld   $1b
              * 5 times
              5727 0006  ld   $06
              5728 0006  ld   $06
              5729 001b  ld   $1b
              572a 001b  ld   $1b
              572b 000b  ld   $0b
              572c 001b  ld   $1b
              572d 0016  ld   $16
              572e 0017  ld   $17
              572f 0006  ld   $06
              5730 0016  ld   $16
              5731 0017  ld   $17
              5732 001a  ld   $1a
              5733 0017  ld   $17
              5734 001b  ld   $1b
              5735 0017  ld   $17
              5736 001b  ld   $1b
              5737 001a  ld   $1a
              5738 0007  ld   $07
              5739 0006  ld   $06
              573a 0002  ld   $02
              573b 0001  ld   $01
              573c 0006  ld   $06
              573d 0006  ld   $06
              573e 0016  ld   $16
              573f 0006  ld   $06
              5740 0006  ld   $06
              5741 0006  ld   $06
              5742 001a  ld   $1a
              5743 0017  ld   $17
              5744 001a  ld   $1a
              5745 0006  ld   $06
              5746 0016  ld   $16
              5747 0006  ld   $06
              5748 0016  ld   $16
              5749 0006  ld   $06
              574a 0016  ld   $16
              574b 0006  ld   $06
              574c 0006  ld   $06
              574d 0016  ld   $16
              574e 0006  ld   $06
              574f 0016  ld   $16
              5750 0016  ld   $16
              5751 0006  ld   $06
              5752 0016  ld   $16
              5753 0006  ld   $06
              5754 0016  ld   $16
              5755 0005  ld   $05
              5756 0006  ld   $06
              5757 0005  ld   $05
              5758 0006  ld   $06
              5759 0005  ld   $05
              575a 0005  ld   $05
              575b 0005  ld   $05
              575c 0005  ld   $05
              575d 0015  ld   $15
              575e 0005  ld   $05
              575f 0001  ld   $01
              5760 0005  ld   $05
              5761 0002  ld   $02
              5762 0005  ld   $05
              5763 0015  ld   $15
              5764 0001  ld   $01
              5765 0005  ld   $05
              5766 0002  ld   $02
              5767 0015  ld   $15
              5768 0005  ld   $05
              5769 0001  ld   $01
              576a 0005  ld   $05
              576b 0001  ld   $01
              576c 0001  ld   $01
              576d 0005  ld   $05
              576e 0011  ld   $11
              576f 0001  ld   $01
              5770 0001  ld   $01
              5771 0004  ld   $04
              5772 0001  ld   $01
              5773 0001  ld   $01
              5774 0000  ld   $00
              5775 0005  ld   $05
              5776 0011  ld   $11
              5777 0004  ld   $04
              5778 0001  ld   $01
              5779 0000  ld   $00
              577a 0005  ld   $05
              577b 0001  ld   $01
              577c 0000  ld   $00
              577d 0001  ld   $01
              577e 0000  ld   $00
              577f 0001  ld   $01
              5780 0001  ld   $01
              5781 0001  ld   $01
              5782 0004  ld   $04
              5783 0001  ld   $01
              5784 0004  ld   $04
              5785 0001  ld   $01
              5786 0004  ld   $04
              5787 0001  ld   $01
              5788 0000  ld   $00
              5789 0000  ld   $00
              578a 0000  ld   $00
              * 24 times
              57a0 0200  nop
              57a1 0200  nop
              57a2 0200  nop
              * 91 times
              57fb fe00  bra  ac          ;Trampoline for page $5700 lookups
              57fc fcfd  bra  $57fd
              57fd 1403  ld   $03,y
              57fe e07b  jmp  y,$7b
              57ff 1519  ld   [$19],y
jupiter81:    5800 0000  ld   $00
              5801 0000  ld   $00
              5802 0000  ld   $00
              * 23 times
              5817 001a  ld   $1a
              5818 001a  ld   $1a
              5819 001b  ld   $1b
              581a 001a  ld   $1a
              581b 001b  ld   $1b
              581c 001a  ld   $1a
              581d 001b  ld   $1b
              581e 002b  ld   $2b
              581f 001b  ld   $1b
              5820 001a  ld   $1a
              5821 001b  ld   $1b
              5822 001b  ld   $1b
              5823 001a  ld   $1a
              5824 001b  ld   $1b
              5825 001a  ld   $1a
              5826 0006  ld   $06
              5827 0017  ld   $17
              5828 0017  ld   $17
              5829 000a  ld   $0a
              582a 0017  ld   $17
              582b 0006  ld   $06
              582c 001b  ld   $1b
              582d 001b  ld   $1b
              582e 001b  ld   $1b
              * 8 times
              5834 001a  ld   $1a
              5835 001b  ld   $1b
              5836 001a  ld   $1a
              5837 0017  ld   $17
              5838 001b  ld   $1b
              5839 001a  ld   $1a
              583a 0016  ld   $16
              583b 0006  ld   $06
              583c 0002  ld   $02
              583d 0005  ld   $05
              583e 0002  ld   $02
              583f 0006  ld   $06
              5840 0006  ld   $06
              5841 0006  ld   $06
              * 6 times
              5845 0016  ld   $16
              5846 0006  ld   $06
              5847 0006  ld   $06
              5848 0016  ld   $16
              5849 0006  ld   $06
              584a 0006  ld   $06
              584b 0002  ld   $02
              584c 0005  ld   $05
              584d 0006  ld   $06
              584e 0005  ld   $05
              584f 0006  ld   $06
              5850 0005  ld   $05
              5851 0006  ld   $06
              5852 0005  ld   $05
              5853 0001  ld   $01
              5854 0005  ld   $05
              5855 0001  ld   $01
              5856 0001  ld   $01
              5857 0005  ld   $05
              5858 0012  ld   $12
              5859 0005  ld   $05
              585a 0016  ld   $16
              585b 0002  ld   $02
              585c 0005  ld   $05
              585d 0002  ld   $02
              585e 0005  ld   $05
              585f 0006  ld   $06
              5860 0001  ld   $01
              5861 0005  ld   $05
              5862 0002  ld   $02
              5863 0005  ld   $05
              5864 0016  ld   $16
              5865 0001  ld   $01
              5866 0005  ld   $05
              5867 0005  ld   $05
              5868 0012  ld   $12
              5869 0005  ld   $05
              586a 0001  ld   $01
              586b 0001  ld   $01
              586c 0005  ld   $05
              586d 0001  ld   $01
              586e 0001  ld   $01
              586f 0004  ld   $04
              5870 0001  ld   $01
              5871 0001  ld   $01
              5872 0000  ld   $00
              5873 0005  ld   $05
              5874 0001  ld   $01
              5875 0005  ld   $05
              5876 0001  ld   $01
              5877 0000  ld   $00
              5878 0001  ld   $01
              5879 0015  ld   $15
              587a 0001  ld   $01
              587b 0000  ld   $00
              587c 0001  ld   $01
              587d 0004  ld   $04
              587e 0001  ld   $01
              587f 0005  ld   $05
              5880 0000  ld   $00
              5881 0001  ld   $01
              5882 0000  ld   $00
              5883 0001  ld   $01
              5884 0001  ld   $01
              5885 0000  ld   $00
              5886 0001  ld   $01
              5887 0015  ld   $15
              5888 0001  ld   $01
              5889 0000  ld   $00
              588a 0000  ld   $00
              588b 0000  ld   $00
              * 23 times
              58a0 0200  nop
              58a1 0200  nop
              58a2 0200  nop
              * 91 times
              58fb fe00  bra  ac          ;Trampoline for page $5800 lookups
              58fc fcfd  bra  $58fd
              58fd 1403  ld   $03,y
              58fe e07b  jmp  y,$7b
              58ff 1519  ld   [$19],y
jupiter82:    5900 0000  ld   $00
              5901 0000  ld   $00
              5902 0000  ld   $00
              * 23 times
              5917 0015  ld   $15
              5918 0016  ld   $16
              5919 001a  ld   $1a
              591a 001a  ld   $1a
              591b 001a  ld   $1a
              591c 001a  ld   $1a
              591d 001b  ld   $1b
              591e 001b  ld   $1b
              591f 001a  ld   $1a
              5920 002b  ld   $2b
              5921 001b  ld   $1b
              5922 001a  ld   $1a
              5923 0017  ld   $17
              5924 001b  ld   $1b
              5925 001b  ld   $1b
              5926 0006  ld   $06
              5927 001a  ld   $1a
              5928 0007  ld   $07
              5929 001a  ld   $1a
              592a 000b  ld   $0b
              592b 001b  ld   $1b
              592c 001b  ld   $1b
              592d 001b  ld   $1b
              * 14 times
              5939 0017  ld   $17
              593a 001b  ld   $1b
              593b 0016  ld   $16
              593c 0006  ld   $06
              593d 0006  ld   $06
              593e 0006  ld   $06
              593f 0001  ld   $01
              5940 0015  ld   $15
              5941 0002  ld   $02
              5942 0005  ld   $05
              5943 0006  ld   $06
              5944 0016  ld   $16
              5945 0006  ld   $06
              5946 0016  ld   $16
              5947 0006  ld   $06
              5948 0005  ld   $05
              5949 0001  ld   $01
              594a 0005  ld   $05
              594b 0001  ld   $01
              594c 0001  ld   $01
              594d 0001  ld   $01
              * 8 times
              5953 0006  ld   $06
              5954 0001  ld   $01
              5955 0006  ld   $06
              5956 0001  ld   $01
              5957 0005  ld   $05
              5958 0005  ld   $05
              5959 0001  ld   $01
              595a 0005  ld   $05
              595b 0005  ld   $05
              595c 0001  ld   $01
              595d 0005  ld   $05
              595e 0012  ld   $12
              595f 0005  ld   $05
              5960 0015  ld   $15
              5961 0006  ld   $06
              5962 0015  ld   $15
              5963 0006  ld   $06
              5964 0011  ld   $11
              5965 0005  ld   $05
              5966 0001  ld   $01
              5967 0005  ld   $05
              5968 0005  ld   $05
              5969 0001  ld   $01
              596a 0005  ld   $05
              596b 0015  ld   $15
              596c 0001  ld   $01
              596d 0005  ld   $05
              596e 0001  ld   $01
              596f 0001  ld   $01
              5970 0005  ld   $05
              5971 0000  ld   $00
              5972 0001  ld   $01
              5973 0000  ld   $00
              5974 0001  ld   $01
              5975 0000  ld   $00
              5976 0001  ld   $01
              5977 0004  ld   $04
              5978 0001  ld   $01
              5979 0000  ld   $00
              597a 0001  ld   $01
              597b 0000  ld   $00
              597c 0001  ld   $01
              597d 0001  ld   $01
              597e 0000  ld   $00
              597f 0001  ld   $01
              5980 0000  ld   $00
              5981 0005  ld   $05
              5982 0001  ld   $01
              5983 0000  ld   $00
              5984 0005  ld   $05
              5985 0001  ld   $01
              5986 0000  ld   $00
              5987 0000  ld   $00
              5988 0000  ld   $00
              * 26 times
              59a0 0200  nop
              59a1 0200  nop
              59a2 0200  nop
              * 91 times
              59fb fe00  bra  ac          ;Trampoline for page $5900 lookups
              59fc fcfd  bra  $59fd
              59fd 1403  ld   $03,y
              59fe e07b  jmp  y,$7b
              59ff 1519  ld   [$19],y
jupiter83:    5a00 0000  ld   $00
              5a01 0000  ld   $00
              5a02 0000  ld   $00
              * 23 times
              5a17 0001  ld   $01
              5a18 001a  ld   $1a
              5a19 002b  ld   $2b
              5a1a 001a  ld   $1a
              5a1b 0017  ld   $17
              5a1c 002a  ld   $2a
              5a1d 0017  ld   $17
              5a1e 001a  ld   $1a
              5a1f 002b  ld   $2b
              5a20 001b  ld   $1b
              5a21 001b  ld   $1b
              5a22 001a  ld   $1a
              5a23 001b  ld   $1b
              5a24 001a  ld   $1a
              5a25 001b  ld   $1b
              5a26 0006  ld   $06
              5a27 0007  ld   $07
              5a28 001a  ld   $1a
              5a29 0007  ld   $07
              5a2a 0017  ld   $17
              5a2b 001a  ld   $1a
              5a2c 0007  ld   $07
              5a2d 001b  ld   $1b
              5a2e 0016  ld   $16
              5a2f 0007  ld   $07
              5a30 0016  ld   $16
              5a31 0016  ld   $16
              5a32 001b  ld   $1b
              5a33 001a  ld   $1a
              5a34 001b  ld   $1b
              5a35 001b  ld   $1b
              5a36 001b  ld   $1b
              * 5 times
              5a39 001a  ld   $1a
              5a3a 0007  ld   $07
              5a3b 001b  ld   $1b
              5a3c 0017  ld   $17
              5a3d 0016  ld   $16
              5a3e 0006  ld   $06
              5a3f 0006  ld   $06
              5a40 0006  ld   $06
              5a41 0005  ld   $05
              5a42 0002  ld   $02
              5a43 0001  ld   $01
              5a44 0005  ld   $05
              5a45 0006  ld   $06
              5a46 0001  ld   $01
              5a47 0005  ld   $05
              5a48 0001  ld   $01
              5a49 0002  ld   $02
              5a4a 0005  ld   $05
              5a4b 0002  ld   $02
              5a4c 0005  ld   $05
              5a4d 0006  ld   $06
              5a4e 0001  ld   $01
              5a4f 0006  ld   $06
              5a50 0001  ld   $01
              5a51 0005  ld   $05
              5a52 0006  ld   $06
              5a53 0001  ld   $01
              5a54 0015  ld   $15
              5a55 0001  ld   $01
              5a56 0005  ld   $05
              5a57 0002  ld   $02
              5a58 0001  ld   $01
              5a59 0006  ld   $06
              5a5a 0001  ld   $01
              5a5b 0006  ld   $06
              5a5c 0011  ld   $11
              5a5d 0006  ld   $06
              5a5e 0005  ld   $05
              5a5f 0015  ld   $15
              5a60 0002  ld   $02
              5a61 0015  ld   $15
              5a62 0006  ld   $06
              5a63 0015  ld   $15
              5a64 0006  ld   $06
              5a65 0015  ld   $15
              5a66 0006  ld   $06
              5a67 0011  ld   $11
              5a68 0006  ld   $06
              5a69 0015  ld   $15
              5a6a 0016  ld   $16
              5a6b 0005  ld   $05
              5a6c 0005  ld   $05
              5a6d 0005  ld   $05
              5a6e 0015  ld   $15
              5a6f 0005  ld   $05
              5a70 0011  ld   $11
              5a71 0005  ld   $05
              5a72 0001  ld   $01
              5a73 0015  ld   $15
              5a74 0001  ld   $01
              5a75 0005  ld   $05
              5a76 0000  ld   $00
              5a77 0001  ld   $01
              5a78 0000  ld   $00
              5a79 0001  ld   $01
              5a7a 0000  ld   $00
              5a7b 0005  ld   $05
              5a7c 0001  ld   $01
              5a7d 0004  ld   $04
              5a7e 0011  ld   $11
              5a7f 0005  ld   $05
              5a80 0011  ld   $11
              5a81 0004  ld   $04
              5a82 0011  ld   $11
              5a83 0005  ld   $05
              5a84 0011  ld   $11
              5a85 0005  ld   $05
              5a86 0015  ld   $15
              5a87 0001  ld   $01
              5a88 0000  ld   $00
              5a89 0000  ld   $00
              5a8a 0000  ld   $00
              * 24 times
              5aa0 0200  nop
              5aa1 0200  nop
              5aa2 0200  nop
              * 91 times
              5afb fe00  bra  ac          ;Trampoline for page $5a00 lookups
              5afc fcfd  bra  $5afd
              5afd 1403  ld   $03,y
              5afe e07b  jmp  y,$7b
              5aff 1519  ld   [$19],y
jupiter84:    5b00 0000  ld   $00
              5b01 0000  ld   $00
              5b02 0000  ld   $00
              * 24 times
              5b18 0016  ld   $16
              5b19 002a  ld   $2a
              5b1a 001b  ld   $1b
              5b1b 002b  ld   $2b
              5b1c 001a  ld   $1a
              5b1d 001b  ld   $1b
              5b1e 001a  ld   $1a
              5b1f 001b  ld   $1b
              5b20 001a  ld   $1a
              5b21 002b  ld   $2b
              5b22 0016  ld   $16
              5b23 001b  ld   $1b
              5b24 0016  ld   $16
              5b25 0017  ld   $17
              5b26 0016  ld   $16
              5b27 001b  ld   $1b
              5b28 0017  ld   $17
              5b29 000a  ld   $0a
              5b2a 001b  ld   $1b
              5b2b 0017  ld   $17
              5b2c 000a  ld   $0a
              5b2d 0016  ld   $16
              5b2e 0007  ld   $07
              5b2f 0016  ld   $16
              5b30 0006  ld   $06
              5b31 0007  ld   $07
              5b32 0006  ld   $06
              5b33 0016  ld   $16
              5b34 001b  ld   $1b
              5b35 001a  ld   $1a
              5b36 001b  ld   $1b
              5b37 001b  ld   $1b
              5b38 001b  ld   $1b
              5b39 001b  ld   $1b
              5b3a 0016  ld   $16
              5b3b 000a  ld   $0a
              5b3c 0017  ld   $17
              5b3d 001b  ld   $1b
              5b3e 0016  ld   $16
              5b3f 0016  ld   $16
              5b40 0006  ld   $06
              5b41 0006  ld   $06
              5b42 0006  ld   $06
              5b43 0005  ld   $05
              5b44 0006  ld   $06
              5b45 0011  ld   $11
              5b46 0006  ld   $06
              5b47 0006  ld   $06
              5b48 0015  ld   $15
              5b49 0006  ld   $06
              5b4a 0005  ld   $05
              5b4b 0016  ld   $16
              5b4c 0012  ld   $12
              5b4d 0005  ld   $05
              5b4e 0016  ld   $16
              5b4f 0005  ld   $05
              5b50 0006  ld   $06
              5b51 0015  ld   $15
              5b52 0006  ld   $06
              5b53 0015  ld   $15
              5b54 0006  ld   $06
              5b55 0006  ld   $06
              5b56 0005  ld   $05
              5b57 0016  ld   $16
              5b58 0005  ld   $05
              5b59 0016  ld   $16
              5b5a 0005  ld   $05
              5b5b 0016  ld   $16
              5b5c 0006  ld   $06
              5b5d 0016  ld   $16
              5b5e 0016  ld   $16
              5b5f 0006  ld   $06
              5b60 0015  ld   $15
              5b61 0006  ld   $06
              5b62 0015  ld   $15
              5b63 0016  ld   $16
              5b64 0005  ld   $05
              5b65 0016  ld   $16
              5b66 0001  ld   $01
              5b67 0005  ld   $05
              5b68 0016  ld   $16
              5b69 0005  ld   $05
              5b6a 0016  ld   $16
              5b6b 0001  ld   $01
              5b6c 0015  ld   $15
              5b6d 0005  ld   $05
              5b6e 0001  ld   $01
              5b6f 0005  ld   $05
              5b70 0005  ld   $05
              5b71 0011  ld   $11
              5b72 0005  ld   $05
              5b73 0005  ld   $05
              5b74 0001  ld   $01
              5b75 0015  ld   $15
              5b76 0001  ld   $01
              5b77 0005  ld   $05
              5b78 0005  ld   $05
              5b79 0015  ld   $15
              5b7a 0001  ld   $01
              5b7b 0005  ld   $05
              5b7c 0011  ld   $11
              5b7d 0005  ld   $05
              5b7e 0005  ld   $05
              5b7f 0015  ld   $15
              5b80 0005  ld   $05
              5b81 0001  ld   $01
              5b82 0015  ld   $15
              5b83 0005  ld   $05
              5b84 0015  ld   $15
              5b85 0001  ld   $01
              5b86 0015  ld   $15
              5b87 0000  ld   $00
              5b88 0000  ld   $00
              5b89 0000  ld   $00
              * 25 times
              5ba0 0200  nop
              5ba1 0200  nop
              5ba2 0200  nop
              * 91 times
              5bfb fe00  bra  ac          ;Trampoline for page $5b00 lookups
              5bfc fcfd  bra  $5bfd
              5bfd 1403  ld   $03,y
              5bfe e07b  jmp  y,$7b
              5bff 1519  ld   [$19],y
jupiter85:    5c00 0000  ld   $00
              5c01 0000  ld   $00
              5c02 0000  ld   $00
              * 24 times
              5c18 0001  ld   $01
              5c19 002a  ld   $2a
              5c1a 002b  ld   $2b
              5c1b 002e  ld   $2e
              5c1c 001b  ld   $1b
              5c1d 002a  ld   $2a
              5c1e 002b  ld   $2b
              5c1f 002b  ld   $2b
              5c20 002b  ld   $2b
              5c21 001b  ld   $1b
              5c22 002b  ld   $2b
              5c23 001b  ld   $1b
              5c24 0016  ld   $16
              5c25 0006  ld   $06
              5c26 000a  ld   $0a
              5c27 0017  ld   $17
              5c28 000a  ld   $0a
              5c29 0017  ld   $17
              5c2a 0006  ld   $06
              5c2b 001b  ld   $1b
              5c2c 0016  ld   $16
              5c2d 0007  ld   $07
              5c2e 0007  ld   $07
              5c2f 0006  ld   $06
              5c30 0007  ld   $07
              5c31 0016  ld   $16
              5c32 0006  ld   $06
              5c33 0006  ld   $06
              5c34 0006  ld   $06
              5c35 0016  ld   $16
              5c36 001b  ld   $1b
              5c37 001b  ld   $1b
              5c38 0016  ld   $16
              5c39 001b  ld   $1b
              5c3a 001b  ld   $1b
              5c3b 0017  ld   $17
              5c3c 000a  ld   $0a
              5c3d 0006  ld   $06
              5c3e 0017  ld   $17
              5c3f 001b  ld   $1b
              5c40 0016  ld   $16
              5c41 0016  ld   $16
              5c42 0016  ld   $16
              * 5 times
              5c45 0006  ld   $06
              5c46 0016  ld   $16
              5c47 0016  ld   $16
              5c48 0006  ld   $06
              5c49 0016  ld   $16
              5c4a 0016  ld   $16
              5c4b 0005  ld   $05
              5c4c 0016  ld   $16
              5c4d 0006  ld   $06
              5c4e 0016  ld   $16
              5c4f 0006  ld   $06
              5c50 0011  ld   $11
              5c51 0006  ld   $06
              5c52 0016  ld   $16
              5c53 0006  ld   $06
              5c54 0016  ld   $16
              5c55 0016  ld   $16
              5c56 0006  ld   $06
              5c57 0016  ld   $16
              5c58 0006  ld   $06
              5c59 0015  ld   $15
              5c5a 0006  ld   $06
              5c5b 0016  ld   $16
              5c5c 0015  ld   $15
              5c5d 0006  ld   $06
              5c5e 0015  ld   $15
              5c5f 0006  ld   $06
              5c60 0015  ld   $15
              5c61 0006  ld   $06
              5c62 0015  ld   $15
              5c63 0006  ld   $06
              5c64 0016  ld   $16
              5c65 0005  ld   $05
              5c66 0016  ld   $16
              5c67 0005  ld   $05
              5c68 0016  ld   $16
              5c69 0005  ld   $05
              5c6a 0015  ld   $15
              5c6b 0006  ld   $06
              5c6c 0001  ld   $01
              5c6d 0015  ld   $15
              5c6e 0005  ld   $05
              5c6f 0011  ld   $11
              5c70 0005  ld   $05
              5c71 0005  ld   $05
              5c72 0001  ld   $01
              5c73 0015  ld   $15
              5c74 0000  ld   $00
              5c75 0005  ld   $05
              5c76 0001  ld   $01
              5c77 0015  ld   $15
              5c78 0001  ld   $01
              5c79 0000  ld   $00
              5c7a 0005  ld   $05
              5c7b 0011  ld   $11
              5c7c 0005  ld   $05
              5c7d 0015  ld   $15
              5c7e 0005  ld   $05
              5c7f 0015  ld   $15
              5c80 0005  ld   $05
              5c81 0015  ld   $15
              5c82 0005  ld   $05
              5c83 0015  ld   $15
              5c84 0015  ld   $15
              5c85 0005  ld   $05
              5c86 0000  ld   $00
              5c87 0000  ld   $00
              5c88 0000  ld   $00
              * 26 times
              5ca0 0200  nop
              5ca1 0200  nop
              5ca2 0200  nop
              * 91 times
              5cfb fe00  bra  ac          ;Trampoline for page $5c00 lookups
              5cfc fcfd  bra  $5cfd
              5cfd 1403  ld   $03,y
              5cfe e07b  jmp  y,$7b
              5cff 1519  ld   [$19],y
jupiter86:    5d00 0000  ld   $00
              5d01 0000  ld   $00
              5d02 0000  ld   $00
              * 25 times
              5d19 0015  ld   $15
              5d1a 002f  ld   $2f
              5d1b 002a  ld   $2a
              5d1c 002b  ld   $2b
              5d1d 001b  ld   $1b
              5d1e 002f  ld   $2f
              5d1f 002a  ld   $2a
              5d20 002f  ld   $2f
              5d21 002b  ld   $2b
              5d22 001a  ld   $1a
              5d23 002b  ld   $2b
              5d24 001a  ld   $1a
              5d25 0006  ld   $06
              5d26 0016  ld   $16
              5d27 0006  ld   $06
              5d28 0017  ld   $17
              5d29 001b  ld   $1b
              5d2a 0007  ld   $07
              5d2b 0016  ld   $16
              5d2c 0007  ld   $07
              5d2d 0006  ld   $06
              5d2e 0016  ld   $16
              5d2f 0006  ld   $06
              5d30 0006  ld   $06
              5d31 0007  ld   $07
              5d32 0006  ld   $06
              5d33 0002  ld   $02
              5d34 0006  ld   $06
              5d35 0002  ld   $02
              5d36 0017  ld   $17
              5d37 001b  ld   $1b
              5d38 001b  ld   $1b
              5d39 001a  ld   $1a
              5d3a 0017  ld   $17
              5d3b 001a  ld   $1a
              5d3c 0017  ld   $17
              5d3d 0006  ld   $06
              5d3e 0006  ld   $06
              5d3f 0006  ld   $06
              5d40 0016  ld   $16
              5d41 0006  ld   $06
              5d42 001b  ld   $1b
              5d43 001a  ld   $1a
              5d44 001b  ld   $1b
              5d45 001a  ld   $1a
              5d46 0016  ld   $16
              5d47 0006  ld   $06
              5d48 0016  ld   $16
              5d49 0016  ld   $16
              5d4a 0006  ld   $06
              5d4b 0016  ld   $16
              5d4c 0016  ld   $16
              5d4d 0016  ld   $16
              5d4e 0006  ld   $06
              5d4f 0016  ld   $16
              5d50 001a  ld   $1a
              5d51 0016  ld   $16
              5d52 0016  ld   $16
              5d53 0016  ld   $16
              5d54 001a  ld   $1a
              5d55 0016  ld   $16
              5d56 0016  ld   $16
              5d57 0016  ld   $16
              * 6 times
              5d5b 0006  ld   $06
              5d5c 001a  ld   $1a
              5d5d 0005  ld   $05
              5d5e 0016  ld   $16
              5d5f 0016  ld   $16
              5d60 0005  ld   $05
              5d61 0016  ld   $16
              5d62 0005  ld   $05
              5d63 0015  ld   $15
              5d64 0005  ld   $05
              5d65 0015  ld   $15
              5d66 0005  ld   $05
              5d67 0016  ld   $16
              5d68 0001  ld   $01
              5d69 0005  ld   $05
              5d6a 0015  ld   $15
              5d6b 0005  ld   $05
              5d6c 0016  ld   $16
              5d6d 0005  ld   $05
              5d6e 0005  ld   $05
              5d6f 0015  ld   $15
              5d70 0005  ld   $05
              5d71 0011  ld   $11
              5d72 0005  ld   $05
              5d73 0005  ld   $05
              5d74 0015  ld   $15
              5d75 0005  ld   $05
              5d76 0015  ld   $15
              5d77 0005  ld   $05
              5d78 0005  ld   $05
              5d79 0015  ld   $15
              5d7a 0005  ld   $05
              5d7b 0005  ld   $05
              5d7c 0015  ld   $15
              5d7d 0005  ld   $05
              5d7e 0011  ld   $11
              5d7f 0005  ld   $05
              5d80 0015  ld   $15
              5d81 0015  ld   $15
              5d82 0005  ld   $05
              5d83 0015  ld   $15
              5d84 0015  ld   $15
              5d85 0005  ld   $05
              5d86 0000  ld   $00
              5d87 0000  ld   $00
              5d88 0000  ld   $00
              * 26 times
              5da0 0200  nop
              5da1 0200  nop
              5da2 0200  nop
              * 91 times
              5dfb fe00  bra  ac          ;Trampoline for page $5d00 lookups
              5dfc fcfd  bra  $5dfd
              5dfd 1403  ld   $03,y
              5dfe e07b  jmp  y,$7b
              5dff 1519  ld   [$19],y
jupiter87:    5e00 0000  ld   $00
              5e01 0000  ld   $00
              5e02 0000  ld   $00
              * 26 times
              5e1a 001a  ld   $1a
              5e1b 002a  ld   $2a
              5e1c 002b  ld   $2b
              5e1d 002a  ld   $2a
              5e1e 002e  ld   $2e
              5e1f 002b  ld   $2b
              5e20 002b  ld   $2b
              5e21 002b  ld   $2b
              5e22 002f  ld   $2f
              5e23 001b  ld   $1b
              5e24 002b  ld   $2b
              5e25 001b  ld   $1b
              5e26 0016  ld   $16
              5e27 0016  ld   $16
              5e28 0007  ld   $07
              5e29 001a  ld   $1a
              5e2a 0017  ld   $17
              5e2b 000a  ld   $0a
              5e2c 0016  ld   $16
              5e2d 0007  ld   $07
              5e2e 0006  ld   $06
              5e2f 0006  ld   $06
              5e30 0006  ld   $06
              5e31 0002  ld   $02
              5e32 0006  ld   $06
              5e33 0002  ld   $02
              5e34 0006  ld   $06
              5e35 0006  ld   $06
              5e36 0006  ld   $06
              5e37 001a  ld   $1a
              5e38 001b  ld   $1b
              5e39 0017  ld   $17
              5e3a 000a  ld   $0a
              5e3b 0007  ld   $07
              5e3c 001a  ld   $1a
              5e3d 001b  ld   $1b
              5e3e 0016  ld   $16
              5e3f 0016  ld   $16
              5e40 0006  ld   $06
              5e41 0006  ld   $06
              5e42 0016  ld   $16
              5e43 0016  ld   $16
              5e44 0017  ld   $17
              5e45 0006  ld   $06
              5e46 001a  ld   $1a
              5e47 0016  ld   $16
              5e48 000a  ld   $0a
              5e49 0016  ld   $16
              5e4a 001a  ld   $1a
              5e4b 0016  ld   $16
              5e4c 000a  ld   $0a
              5e4d 0016  ld   $16
              5e4e 001a  ld   $1a
              5e4f 0016  ld   $16
              5e50 0006  ld   $06
              5e51 0016  ld   $16
              5e52 001a  ld   $1a
              5e53 0006  ld   $06
              5e54 0016  ld   $16
              5e55 001a  ld   $1a
              5e56 0016  ld   $16
              5e57 001a  ld   $1a
              5e58 001a  ld   $1a
              5e59 001a  ld   $1a
              5e5a 0015  ld   $15
              5e5b 001a  ld   $1a
              5e5c 0015  ld   $15
              5e5d 0016  ld   $16
              5e5e 0006  ld   $06
              5e5f 0015  ld   $15
              5e60 001a  ld   $1a
              5e61 0016  ld   $16
              5e62 0005  ld   $05
              5e63 0016  ld   $16
              5e64 0016  ld   $16
              5e65 0016  ld   $16
              5e66 0005  ld   $05
              5e67 0016  ld   $16
              5e68 0005  ld   $05
              5e69 0015  ld   $15
              5e6a 0002  ld   $02
              5e6b 0005  ld   $05
              5e6c 0015  ld   $15
              5e6d 0005  ld   $05
              5e6e 0005  ld   $05
              5e6f 0011  ld   $11
              5e70 0005  ld   $05
              5e71 0005  ld   $05
              5e72 0005  ld   $05
              5e73 0011  ld   $11
              5e74 0005  ld   $05
              5e75 0001  ld   $01
              5e76 0015  ld   $15
              5e77 0001  ld   $01
              5e78 0015  ld   $15
              5e79 0001  ld   $01
              5e7a 0015  ld   $15
              5e7b 0015  ld   $15
              5e7c 0005  ld   $05
              5e7d 0015  ld   $15
              5e7e 0005  ld   $05
              5e7f 0015  ld   $15
              5e80 0011  ld   $11
              5e81 0005  ld   $05
              5e82 0015  ld   $15
              5e83 0005  ld   $05
              5e84 0015  ld   $15
              5e85 0000  ld   $00
              5e86 0000  ld   $00
              5e87 0000  ld   $00
              * 27 times
              5ea0 0200  nop
              5ea1 0200  nop
              5ea2 0200  nop
              * 91 times
              5efb fe00  bra  ac          ;Trampoline for page $5e00 lookups
              5efc fcfd  bra  $5efd
              5efd 1403  ld   $03,y
              5efe e07b  jmp  y,$7b
              5eff 1519  ld   [$19],y
jupiter88:    5f00 0000  ld   $00
              5f01 0000  ld   $00
              5f02 0000  ld   $00
              * 26 times
              5f1a 0015  ld   $15
              5f1b 002b  ld   $2b
              5f1c 001e  ld   $1e
              5f1d 002b  ld   $2b
              5f1e 001b  ld   $1b
              5f1f 002a  ld   $2a
              5f20 001f  ld   $1f
              5f21 002a  ld   $2a
              5f22 002b  ld   $2b
              5f23 002f  ld   $2f
              5f24 002b  ld   $2b
              5f25 002b  ld   $2b
              5f26 001b  ld   $1b
              5f27 001b  ld   $1b
              5f28 0016  ld   $16
              5f29 0006  ld   $06
              5f2a 0006  ld   $06
              5f2b 0017  ld   $17
              5f2c 0006  ld   $06
              5f2d 0016  ld   $16
              5f2e 0017  ld   $17
              5f2f 0006  ld   $06
              5f30 0006  ld   $06
              5f31 0006  ld   $06
              5f32 0016  ld   $16
              5f33 0007  ld   $07
              5f34 0006  ld   $06
              5f35 0017  ld   $17
              5f36 0006  ld   $06
              5f37 0017  ld   $17
              5f38 001b  ld   $1b
              5f39 0016  ld   $16
              5f3a 0006  ld   $06
              5f3b 0016  ld   $16
              5f3c 0006  ld   $06
              5f3d 001b  ld   $1b
              5f3e 001a  ld   $1a
              5f3f 0007  ld   $07
              5f40 0016  ld   $16
              5f41 0016  ld   $16
              5f42 0006  ld   $06
              5f43 0006  ld   $06
              5f44 0016  ld   $16
              5f45 0006  ld   $06
              5f46 0016  ld   $16
              5f47 001a  ld   $1a
              5f48 0016  ld   $16
              5f49 001a  ld   $1a
              5f4a 0016  ld   $16
              5f4b 001a  ld   $1a
              5f4c 0016  ld   $16
              5f4d 0016  ld   $16
              5f4e 0006  ld   $06
              5f4f 001a  ld   $1a
              5f50 0016  ld   $16
              5f51 001a  ld   $1a
              5f52 0016  ld   $16
              5f53 0016  ld   $16
              5f54 001a  ld   $1a
              5f55 0015  ld   $15
              5f56 0006  ld   $06
              5f57 0015  ld   $15
              5f58 0016  ld   $16
              5f59 0016  ld   $16
              5f5a 000a  ld   $0a
              5f5b 0016  ld   $16
              5f5c 0016  ld   $16
              5f5d 0019  ld   $19
              5f5e 0016  ld   $16
              5f5f 0015  ld   $15
              5f60 0006  ld   $06
              5f61 0015  ld   $15
              5f62 0016  ld   $16
              5f63 0015  ld   $15
              5f64 0005  ld   $05
              5f65 0016  ld   $16
              5f66 0015  ld   $15
              5f67 0005  ld   $05
              5f68 0016  ld   $16
              5f69 0005  ld   $05
              5f6a 0015  ld   $15
              5f6b 0005  ld   $05
              5f6c 0001  ld   $01
              5f6d 0005  ld   $05
              5f6e 0001  ld   $01
              5f6f 0005  ld   $05
              5f70 0011  ld   $11
              5f71 0005  ld   $05
              5f72 0011  ld   $11
              5f73 0005  ld   $05
              5f74 0015  ld   $15
              5f75 0005  ld   $05
              5f76 0015  ld   $15
              5f77 0005  ld   $05
              5f78 0005  ld   $05
              5f79 0015  ld   $15
              5f7a 0005  ld   $05
              5f7b 0001  ld   $01
              5f7c 0015  ld   $15
              5f7d 0001  ld   $01
              5f7e 0015  ld   $15
              5f7f 0005  ld   $05
              5f80 0005  ld   $05
              5f81 0015  ld   $15
              5f82 0001  ld   $01
              5f83 0015  ld   $15
              5f84 0001  ld   $01
              5f85 0000  ld   $00
              5f86 0000  ld   $00
              5f87 0000  ld   $00
              * 27 times
              5fa0 0200  nop
              5fa1 0200  nop
              5fa2 0200  nop
              * 91 times
              5ffb fe00  bra  ac          ;Trampoline for page $5f00 lookups
              5ffc fcfd  bra  $5ffd
              5ffd 1403  ld   $03,y
              5ffe e07b  jmp  y,$7b
              5fff 1519  ld   [$19],y
jupiter89:    6000 0000  ld   $00
              6001 0000  ld   $00
              6002 0000  ld   $00
              * 26 times
              601a 0001  ld   $01
              601b 0015  ld   $15
              601c 002b  ld   $2b
              601d 002a  ld   $2a
              601e 001a  ld   $1a
              601f 002b  ld   $2b
              6020 002a  ld   $2a
              6021 001f  ld   $1f
              6022 002a  ld   $2a
              6023 001b  ld   $1b
              6024 002a  ld   $2a
              6025 001b  ld   $1b
              6026 002b  ld   $2b
              6027 001a  ld   $1a
              6028 001b  ld   $1b
              6029 001b  ld   $1b
              602a 0016  ld   $16
              602b 0006  ld   $06
              602c 0007  ld   $07
              602d 0006  ld   $06
              602e 0006  ld   $06
              602f 0016  ld   $16
              6030 0007  ld   $07
              6031 0017  ld   $17
              6032 0007  ld   $07
              6033 0006  ld   $06
              6034 0016  ld   $16
              6035 0006  ld   $06
              6036 0017  ld   $17
              6037 0006  ld   $06
              6038 0006  ld   $06
              6039 0006  ld   $06
              603a 001b  ld   $1b
              603b 0006  ld   $06
              603c 001a  ld   $1a
              603d 0017  ld   $17
              603e 001a  ld   $1a
              603f 001b  ld   $1b
              6040 001a  ld   $1a
              6041 001b  ld   $1b
              6042 001a  ld   $1a
              6043 001a  ld   $1a
              6044 001a  ld   $1a
              6045 001a  ld   $1a
              6046 0016  ld   $16
              6047 001a  ld   $1a
              6048 001b  ld   $1b
              6049 001a  ld   $1a
              604a 001a  ld   $1a
              604b 0016  ld   $16
              604c 001a  ld   $1a
              604d 001a  ld   $1a
              604e 001a  ld   $1a
              * 5 times
              6051 0016  ld   $16
              6052 001a  ld   $1a
              6053 001a  ld   $1a
              6054 0016  ld   $16
              6055 001a  ld   $1a
              6056 001a  ld   $1a
              6057 0016  ld   $16
              6058 001a  ld   $1a
              6059 001a  ld   $1a
              605a 0015  ld   $15
              605b 0016  ld   $16
              605c 0019  ld   $19
              605d 0016  ld   $16
              605e 001a  ld   $1a
              605f 0016  ld   $16
              6060 0019  ld   $19
              6061 0016  ld   $16
              6062 0015  ld   $15
              6063 001a  ld   $1a
              6064 0015  ld   $15
              6065 0006  ld   $06
              6066 0015  ld   $15
              6067 0016  ld   $16
              6068 0015  ld   $15
              6069 0015  ld   $15
              606a 0006  ld   $06
              606b 0015  ld   $15
              606c 0005  ld   $05
              606d 0015  ld   $15
              606e 0011  ld   $11
              606f 0005  ld   $05
              6070 0001  ld   $01
              6071 0015  ld   $15
              6072 0005  ld   $05
              6073 0015  ld   $15
              6074 0005  ld   $05
              6075 0015  ld   $15
              6076 0001  ld   $01
              6077 0015  ld   $15
              6078 0005  ld   $05
              6079 0011  ld   $11
              607a 0005  ld   $05
              607b 0015  ld   $15
              607c 0005  ld   $05
              607d 0015  ld   $15
              607e 0005  ld   $05
              607f 0016  ld   $16
              6080 0015  ld   $15
              6081 0005  ld   $05
              6082 0015  ld   $15
              6083 0015  ld   $15
              6084 0000  ld   $00
              6085 0000  ld   $00
              6086 0000  ld   $00
              * 28 times
              60a0 0200  nop
              60a1 0200  nop
              60a2 0200  nop
              * 91 times
              60fb fe00  bra  ac          ;Trampoline for page $6000 lookups
              60fc fcfd  bra  $60fd
              60fd 1403  ld   $03,y
              60fe e07b  jmp  y,$7b
              60ff 1519  ld   [$19],y
jupiter90:    6100 0000  ld   $00
              6101 0000  ld   $00
              6102 0000  ld   $00
              * 27 times
              611b 0005  ld   $05
              611c 001a  ld   $1a
              611d 002a  ld   $2a
              611e 001b  ld   $1b
              611f 001a  ld   $1a
              6120 001a  ld   $1a
              6121 002b  ld   $2b
              6122 001a  ld   $1a
              6123 002b  ld   $2b
              6124 001e  ld   $1e
              6125 002b  ld   $2b
              6126 002a  ld   $2a
              6127 001b  ld   $1b
              6128 002b  ld   $2b
              6129 001a  ld   $1a
              612a 001b  ld   $1b
              612b 001a  ld   $1a
              612c 0016  ld   $16
              612d 0006  ld   $06
              612e 0006  ld   $06
              612f 0006  ld   $06
              * 6 times
              6133 0016  ld   $16
              6134 0006  ld   $06
              6135 0006  ld   $06
              6136 0006  ld   $06
              6137 0016  ld   $16
              6138 000a  ld   $0a
              6139 0017  ld   $17
              613a 001a  ld   $1a
              613b 001b  ld   $1b
              613c 001b  ld   $1b
              613d 001a  ld   $1a
              613e 0016  ld   $16
              613f 001a  ld   $1a
              6140 001b  ld   $1b
              6141 001a  ld   $1a
              6142 0027  ld   $27
              6143 001a  ld   $1a
              6144 001b  ld   $1b
              6145 001a  ld   $1a
              6146 001a  ld   $1a
              6147 002a  ld   $2a
              6148 001a  ld   $1a
              6149 002a  ld   $2a
              614a 001b  ld   $1b
              614b 001a  ld   $1a
              614c 001a  ld   $1a
              614d 0016  ld   $16
              614e 002a  ld   $2a
              614f 0016  ld   $16
              6150 001a  ld   $1a
              6151 001a  ld   $1a
              6152 0016  ld   $16
              6153 001a  ld   $1a
              6154 001a  ld   $1a
              6155 0016  ld   $16
              6156 001a  ld   $1a
              6157 0016  ld   $16
              6158 0019  ld   $19
              6159 0016  ld   $16
              615a 001a  ld   $1a
              615b 0016  ld   $16
              615c 001a  ld   $1a
              615d 0016  ld   $16
              615e 0015  ld   $15
              615f 001a  ld   $1a
              6160 0015  ld   $15
              6161 0016  ld   $16
              6162 0019  ld   $19
              6163 0016  ld   $16
              6164 0015  ld   $15
              6165 001a  ld   $1a
              6166 0015  ld   $15
              6167 0006  ld   $06
              6168 0015  ld   $15
              6169 0016  ld   $16
              616a 0015  ld   $15
              616b 0015  ld   $15
              616c 0002  ld   $02
              616d 0005  ld   $05
              616e 0005  ld   $05
              616f 0011  ld   $11
              6170 0005  ld   $05
              6171 0005  ld   $05
              6172 0015  ld   $15
              6173 0005  ld   $05
              6174 0015  ld   $15
              6175 0005  ld   $05
              6176 0015  ld   $15
              6177 0005  ld   $05
              6178 0015  ld   $15
              6179 0005  ld   $05
              617a 0015  ld   $15
              617b 0005  ld   $05
              617c 0015  ld   $15
              617d 0001  ld   $01
              617e 0015  ld   $15
              617f 0005  ld   $05
              6180 0001  ld   $01
              6181 0015  ld   $15
              6182 0005  ld   $05
              6183 0000  ld   $00
              6184 0000  ld   $00
              6185 0000  ld   $00
              * 29 times
              61a0 0200  nop
              61a1 0200  nop
              61a2 0200  nop
              * 91 times
              61fb fe00  bra  ac          ;Trampoline for page $6100 lookups
              61fc fcfd  bra  $61fd
              61fd 1403  ld   $03,y
              61fe e07b  jmp  y,$7b
              61ff 1519  ld   [$19],y
jupiter91:    6200 0000  ld   $00
              6201 0000  ld   $00
              6202 0000  ld   $00
              * 28 times
              621c 0011  ld   $11
              621d 001a  ld   $1a
              621e 0016  ld   $16
              621f 001a  ld   $1a
              6220 0016  ld   $16
              6221 001a  ld   $1a
              6222 001a  ld   $1a
              6223 001b  ld   $1b
              6224 001a  ld   $1a
              6225 002b  ld   $2b
              6226 001a  ld   $1a
              6227 002b  ld   $2b
              6228 001a  ld   $1a
              6229 001b  ld   $1b
              622a 001a  ld   $1a
              622b 001b  ld   $1b
              622c 001b  ld   $1b
              622d 001b  ld   $1b
              622e 0016  ld   $16
              622f 001b  ld   $1b
              6230 0006  ld   $06
              6231 0016  ld   $16
              6232 001a  ld   $1a
              6233 0006  ld   $06
              6234 0016  ld   $16
              6235 000a  ld   $0a
              6236 0016  ld   $16
              6237 001b  ld   $1b
              6238 0016  ld   $16
              6239 001b  ld   $1b
              623a 001a  ld   $1a
              623b 001b  ld   $1b
              623c 0016  ld   $16
              623d 001b  ld   $1b
              623e 001a  ld   $1a
              623f 001a  ld   $1a
              6240 0017  ld   $17
              6241 002a  ld   $2a
              6242 001a  ld   $1a
              6243 002b  ld   $2b
              6244 001a  ld   $1a
              6245 002a  ld   $2a
              6246 001a  ld   $1a
              6247 002b  ld   $2b
              6248 0016  ld   $16
              6249 002a  ld   $2a
              624a 001a  ld   $1a
              624b 0026  ld   $26
              624c 001a  ld   $1a
              624d 0026  ld   $26
              624e 001a  ld   $1a
              624f 001a  ld   $1a
              6250 0016  ld   $16
              6251 002a  ld   $2a
              6252 0016  ld   $16
              6253 0019  ld   $19
              6254 0016  ld   $16
              6255 0026  ld   $26
              6256 001a  ld   $1a
              6257 0016  ld   $16
              6258 001a  ld   $1a
              6259 0016  ld   $16
              625a 0016  ld   $16
              625b 0019  ld   $19
              625c 0016  ld   $16
              625d 0015  ld   $15
              625e 001a  ld   $1a
              625f 0016  ld   $16
              6260 0015  ld   $15
              6261 0006  ld   $06
              6262 0015  ld   $15
              6263 0016  ld   $16
              6264 0015  ld   $15
              6265 0016  ld   $16
              6266 0015  ld   $15
              6267 0015  ld   $15
              6268 0015  ld   $15
              6269 0005  ld   $05
              626a 0015  ld   $15
              626b 0005  ld   $05
              626c 0015  ld   $15
              626d 0015  ld   $15
              626e 0001  ld   $01
              626f 0015  ld   $15
              6270 0005  ld   $05
              6271 0011  ld   $11
              6272 0005  ld   $05
              6273 0015  ld   $15
              6274 0001  ld   $01
              6275 0015  ld   $15
              6276 0001  ld   $01
              6277 0015  ld   $15
              6278 0001  ld   $01
              6279 0005  ld   $05
              627a 0015  ld   $15
              627b 0001  ld   $01
              627c 0015  ld   $15
              627d 0005  ld   $05
              627e 0005  ld   $05
              627f 0011  ld   $11
              6280 0005  ld   $05
              6281 0005  ld   $05
              6282 0011  ld   $11
              6283 0000  ld   $00
              6284 0000  ld   $00
              6285 0000  ld   $00
              * 29 times
              62a0 0200  nop
              62a1 0200  nop
              62a2 0200  nop
              * 91 times
              62fb fe00  bra  ac          ;Trampoline for page $6200 lookups
              62fc fcfd  bra  $62fd
              62fd 1403  ld   $03,y
              62fe e07b  jmp  y,$7b
              62ff 1519  ld   [$19],y
jupiter92:    6300 0000  ld   $00
              6301 0000  ld   $00
              6302 0000  ld   $00
              * 29 times
              631d 0015  ld   $15
              631e 0016  ld   $16
              631f 001a  ld   $1a
              6320 0016  ld   $16
              6321 0016  ld   $16
              6322 001b  ld   $1b
              6323 0016  ld   $16
              6324 001a  ld   $1a
              6325 0016  ld   $16
              6326 001b  ld   $1b
              6327 001a  ld   $1a
              6328 001a  ld   $1a
              6329 0016  ld   $16
              632a 001b  ld   $1b
              632b 0016  ld   $16
              632c 001a  ld   $1a
              632d 001a  ld   $1a
              632e 001b  ld   $1b
              632f 001a  ld   $1a
              6330 001b  ld   $1b
              6331 001b  ld   $1b
              6332 0016  ld   $16
              6333 001b  ld   $1b
              6334 001b  ld   $1b
              6335 0016  ld   $16
              6336 001b  ld   $1b
              6337 001a  ld   $1a
              6338 001b  ld   $1b
              6339 001a  ld   $1a
              633a 0027  ld   $27
              633b 001a  ld   $1a
              633c 002a  ld   $2a
              633d 001b  ld   $1b
              633e 002a  ld   $2a
              633f 0016  ld   $16
              6340 002a  ld   $2a
              6341 001b  ld   $1b
              6342 001a  ld   $1a
              6343 0016  ld   $16
              6344 002a  ld   $2a
              6345 001b  ld   $1b
              6346 0016  ld   $16
              6347 002a  ld   $2a
              6348 001a  ld   $1a
              6349 001a  ld   $1a
              634a 0026  ld   $26
              634b 001a  ld   $1a
              634c 001a  ld   $1a
              634d 001a  ld   $1a
              634e 0026  ld   $26
              634f 001a  ld   $1a
              6350 001a  ld   $1a
              6351 0016  ld   $16
              6352 001a  ld   $1a
              6353 0016  ld   $16
              6354 001a  ld   $1a
              6355 0016  ld   $16
              6356 0019  ld   $19
              6357 0016  ld   $16
              6358 0015  ld   $15
              6359 001a  ld   $1a
              635a 0015  ld   $15
              635b 0016  ld   $16
              635c 001a  ld   $1a
              635d 0015  ld   $15
              635e 0016  ld   $16
              635f 0015  ld   $15
              6360 001a  ld   $1a
              6361 0015  ld   $15
              6362 0015  ld   $15
              6363 0006  ld   $06
              6364 0015  ld   $15
              6365 0005  ld   $05
              6366 0015  ld   $15
              6367 0006  ld   $06
              6368 0015  ld   $15
              6369 0005  ld   $05
              636a 0012  ld   $12
              636b 0005  ld   $05
              636c 0015  ld   $15
              636d 0005  ld   $05
              636e 0005  ld   $05
              636f 0001  ld   $01
              6370 0005  ld   $05
              6371 0015  ld   $15
              6372 0001  ld   $01
              6373 0005  ld   $05
              6374 0015  ld   $15
              6375 0005  ld   $05
              6376 0005  ld   $05
              6377 0005  ld   $05
              6378 0011  ld   $11
              6379 0005  ld   $05
              637a 0001  ld   $01
              637b 0005  ld   $05
              637c 0011  ld   $11
              637d 0005  ld   $05
              637e 0001  ld   $01
              637f 0005  ld   $05
              6380 0011  ld   $11
              6381 0000  ld   $00
              6382 0000  ld   $00
              6383 0000  ld   $00
              * 31 times
              63a0 0200  nop
              63a1 0200  nop
              63a2 0200  nop
              * 91 times
              63fb fe00  bra  ac          ;Trampoline for page $6300 lookups
              63fc fcfd  bra  $63fd
              63fd 1403  ld   $03,y
              63fe e07b  jmp  y,$7b
              63ff 1519  ld   [$19],y
jupiter93:    6400 0000  ld   $00
              6401 0000  ld   $00
              6402 0000  ld   $00
              * 30 times
              641e 0016  ld   $16
              641f 0016  ld   $16
              6420 001a  ld   $1a
              6421 0016  ld   $16
              6422 001a  ld   $1a
              6423 0016  ld   $16
              6424 002a  ld   $2a
              6425 001a  ld   $1a
              6426 001a  ld   $1a
              6427 0026  ld   $26
              6428 001b  ld   $1b
              6429 001a  ld   $1a
              642a 001a  ld   $1a
              642b 001a  ld   $1a
              642c 0017  ld   $17
              642d 001a  ld   $1a
              642e 0016  ld   $16
              642f 001b  ld   $1b
              6430 0016  ld   $16
              6431 001a  ld   $1a
              6432 001b  ld   $1b
              6433 001a  ld   $1a
              6434 001b  ld   $1b
              6435 001a  ld   $1a
              6436 001b  ld   $1b
              6437 001a  ld   $1a
              6438 0027  ld   $27
              6439 001a  ld   $1a
              643a 001a  ld   $1a
              643b 002a  ld   $2a
              643c 001b  ld   $1b
              643d 0016  ld   $16
              643e 002b  ld   $2b
              643f 001a  ld   $1a
              6440 001b  ld   $1b
              6441 002a  ld   $2a
              6442 001a  ld   $1a
              6443 002b  ld   $2b
              6444 0016  ld   $16
              6445 002a  ld   $2a
              6446 001a  ld   $1a
              6447 0016  ld   $16
              6448 002a  ld   $2a
              6449 0016  ld   $16
              644a 001a  ld   $1a
              644b 002a  ld   $2a
              644c 0016  ld   $16
              644d 002a  ld   $2a
              644e 0016  ld   $16
              644f 001a  ld   $1a
              6450 0015  ld   $15
              6451 001a  ld   $1a
              6452 0016  ld   $16
              6453 002a  ld   $2a
              6454 0016  ld   $16
              6455 0019  ld   $19
              6456 0016  ld   $16
              6457 001a  ld   $1a
              6458 0026  ld   $26
              6459 001a  ld   $1a
              645a 0016  ld   $16
              645b 0019  ld   $19
              645c 0016  ld   $16
              645d 0015  ld   $15
              645e 001a  ld   $1a
              645f 0015  ld   $15
              6460 0016  ld   $16
              6461 0015  ld   $15
              6462 0016  ld   $16
              6463 0019  ld   $19
              6464 0016  ld   $16
              6465 0015  ld   $15
              6466 0016  ld   $16
              6467 0015  ld   $15
              6468 0015  ld   $15
              6469 0015  ld   $15
              646a 0005  ld   $05
              646b 0015  ld   $15
              646c 0005  ld   $05
              646d 0001  ld   $01
              646e 0015  ld   $15
              646f 0015  ld   $15
              6470 0001  ld   $01
              6471 0005  ld   $05
              6472 0011  ld   $11
              6473 0005  ld   $05
              6474 0001  ld   $01
              6475 0005  ld   $05
              6476 0011  ld   $11
              6477 0005  ld   $05
              6478 0001  ld   $01
              6479 0005  ld   $05
              647a 0011  ld   $11
              647b 0005  ld   $05
              647c 0001  ld   $01
              647d 0005  ld   $05
              647e 0001  ld   $01
              647f 0005  ld   $05
              6480 0005  ld   $05
              6481 0000  ld   $00
              6482 0000  ld   $00
              6483 0000  ld   $00
              * 31 times
              64a0 0200  nop
              64a1 0200  nop
              64a2 0200  nop
              * 91 times
              64fb fe00  bra  ac          ;Trampoline for page $6400 lookups
              64fc fcfd  bra  $64fd
              64fd 1403  ld   $03,y
              64fe e07b  jmp  y,$7b
              64ff 1519  ld   [$19],y
jupiter94:    6500 0000  ld   $00
              6501 0000  ld   $00
              6502 0000  ld   $00
              * 30 times
              651e 0005  ld   $05
              651f 0016  ld   $16
              6520 001a  ld   $1a
              6521 0016  ld   $16
              6522 001a  ld   $1a
              6523 001a  ld   $1a
              6524 0016  ld   $16
              6525 001a  ld   $1a
              6526 0016  ld   $16
              6527 001a  ld   $1a
              6528 001a  ld   $1a
              6529 0016  ld   $16
              652a 001b  ld   $1b
              652b 0016  ld   $16
              652c 001a  ld   $1a
              652d 001a  ld   $1a
              652e 001a  ld   $1a
              652f 002b  ld   $2b
              6530 001a  ld   $1a
              6531 0017  ld   $17
              6532 002a  ld   $2a
              6533 001a  ld   $1a
              6534 0027  ld   $27
              6535 001a  ld   $1a
              6536 002b  ld   $2b
              6537 001a  ld   $1a
              6538 001a  ld   $1a
              6539 002b  ld   $2b
              653a 0016  ld   $16
              653b 001b  ld   $1b
              653c 002a  ld   $2a
              653d 001a  ld   $1a
              653e 001a  ld   $1a
              653f 0026  ld   $26
              6540 001a  ld   $1a
              6541 0016  ld   $16
              6542 002a  ld   $2a
              6543 001a  ld   $1a
              6544 001a  ld   $1a
              6545 0016  ld   $16
              6546 002b  ld   $2b
              6547 001a  ld   $1a
              6548 001a  ld   $1a
              6549 002a  ld   $2a
              654a 0017  ld   $17
              654b 001a  ld   $1a
              654c 001a  ld   $1a
              654d 0016  ld   $16
              654e 001a  ld   $1a
              654f 0026  ld   $26
              6550 001a  ld   $1a
              6551 001a  ld   $1a
              6552 0015  ld   $15
              6553 001a  ld   $1a
              6554 0016  ld   $16
              6555 001a  ld   $1a
              6556 0015  ld   $15
              6557 001a  ld   $1a
              6558 0015  ld   $15
              6559 0016  ld   $16
              655a 0019  ld   $19
              655b 0016  ld   $16
              655c 0016  ld   $16
              655d 0019  ld   $19
              655e 0006  ld   $06
              655f 0015  ld   $15
              6560 0019  ld   $19
              6561 0016  ld   $16
              6562 0005  ld   $05
              6563 0015  ld   $15
              6564 0016  ld   $16
              6565 0005  ld   $05
              6566 0015  ld   $15
              6567 0006  ld   $06
              6568 0015  ld   $15
              6569 0005  ld   $05
              656a 0015  ld   $15
              656b 0005  ld   $05
              656c 0015  ld   $15
              656d 0015  ld   $15
              656e 0005  ld   $05
              656f 0005  ld   $05
              6570 0001  ld   $01
              6571 0005  ld   $05
              6572 0005  ld   $05
              6573 0015  ld   $15
              6574 0001  ld   $01
              6575 0015  ld   $15
              6576 0005  ld   $05
              6577 0001  ld   $01
              6578 0015  ld   $15
              6579 0001  ld   $01
              657a 0005  ld   $05
              657b 0001  ld   $01
              657c 0015  ld   $15
              657d 0000  ld   $00
              657e 0015  ld   $15
              657f 0000  ld   $00
              6580 0000  ld   $00
              6581 0000  ld   $00
              * 33 times
              65a0 0200  nop
              65a1 0200  nop
              65a2 0200  nop
              * 91 times
              65fb fe00  bra  ac          ;Trampoline for page $6500 lookups
              65fc fcfd  bra  $65fd
              65fd 1403  ld   $03,y
              65fe e07b  jmp  y,$7b
              65ff 1519  ld   [$19],y
jupiter95:    6600 0000  ld   $00
              6601 0000  ld   $00
              6602 0000  ld   $00
              * 32 times
              6620 0016  ld   $16
              6621 001a  ld   $1a
              6622 0016  ld   $16
              6623 0016  ld   $16
              6624 001b  ld   $1b
              6625 0016  ld   $16
              6626 001a  ld   $1a
              6627 0016  ld   $16
              6628 001a  ld   $1a
              6629 0016  ld   $16
              662a 001a  ld   $1a
              662b 001b  ld   $1b
              662c 002a  ld   $2a
              662d 001b  ld   $1b
              662e 002a  ld   $2a
              662f 0016  ld   $16
              6630 001a  ld   $1a
              6631 001a  ld   $1a
              6632 0016  ld   $16
              6633 001b  ld   $1b
              6634 001a  ld   $1a
              6635 0016  ld   $16
              6636 001a  ld   $1a
              6637 001a  ld   $1a
              6638 0027  ld   $27
              6639 001a  ld   $1a
              663a 001a  ld   $1a
              663b 002a  ld   $2a
              663c 0016  ld   $16
              663d 001b  ld   $1b
              663e 0026  ld   $26
              663f 001a  ld   $1a
              6640 002b  ld   $2b
              6641 001a  ld   $1a
              6642 001a  ld   $1a
              6643 0017  ld   $17
              6644 001a  ld   $1a
              6645 002a  ld   $2a
              6646 001a  ld   $1a
              6647 0016  ld   $16
              6648 002a  ld   $2a
              6649 0016  ld   $16
              664a 001a  ld   $1a
              664b 002a  ld   $2a
              664c 0016  ld   $16
              664d 002a  ld   $2a
              664e 0016  ld   $16
              664f 001a  ld   $1a
              6650 0016  ld   $16
              6651 001a  ld   $1a
              6652 0026  ld   $26
              6653 001a  ld   $1a
              6654 0015  ld   $15
              6655 002a  ld   $2a
              6656 0016  ld   $16
              6657 0016  ld   $16
              6658 0019  ld   $19
              6659 0016  ld   $16
              665a 0016  ld   $16
              665b 0019  ld   $19
              665c 0016  ld   $16
              665d 0015  ld   $15
              665e 0016  ld   $16
              665f 0015  ld   $15
              6660 0016  ld   $16
              6661 0005  ld   $05
              6662 0016  ld   $16
              6663 0015  ld   $15
              6664 0015  ld   $15
              6665 0006  ld   $06
              6666 0015  ld   $15
              6667 0015  ld   $15
              6668 0005  ld   $05
              6669 0015  ld   $15
              666a 0015  ld   $15
              666b 0011  ld   $11
              666c 0015  ld   $15
              666d 0005  ld   $05
              666e 0011  ld   $11
              666f 0015  ld   $15
              6670 0005  ld   $05
              6671 0010  ld   $10
              6672 0001  ld   $01
              6673 0005  ld   $05
              6674 0015  ld   $15
              6675 0001  ld   $01
              6676 0005  ld   $05
              6677 0015  ld   $15
              6678 0001  ld   $01
              6679 0005  ld   $05
              667a 0010  ld   $10
              667b 0005  ld   $05
              667c 0001  ld   $01
              667d 0005  ld   $05
              667e 0001  ld   $01
              667f 0015  ld   $15
              6680 0000  ld   $00
              6681 0000  ld   $00
              6682 0000  ld   $00
              * 32 times
              66a0 0200  nop
              66a1 0200  nop
              66a2 0200  nop
              * 91 times
              66fb fe00  bra  ac          ;Trampoline for page $6600 lookups
              66fc fcfd  bra  $66fd
              66fd 1403  ld   $03,y
              66fe e07b  jmp  y,$7b
              66ff 1519  ld   [$19],y
jupiter96:    6700 0000  ld   $00
              6701 0000  ld   $00
              6702 0000  ld   $00
              * 32 times
              6720 0015  ld   $15
              6721 0016  ld   $16
              6722 001a  ld   $1a
              6723 0016  ld   $16
              6724 0016  ld   $16
              6725 001a  ld   $1a
              6726 0016  ld   $16
              6727 0016  ld   $16
              6728 0016  ld   $16
              6729 001a  ld   $1a
              672a 0016  ld   $16
              672b 001a  ld   $1a
              672c 002b  ld   $2b
              672d 001a  ld   $1a
              672e 002b  ld   $2b
              672f 001a  ld   $1a
              6730 002a  ld   $2a
              6731 0016  ld   $16
              6732 001a  ld   $1a
              6733 0016  ld   $16
              6734 001a  ld   $1a
              6735 001a  ld   $1a
              6736 0016  ld   $16
              6737 001b  ld   $1b
              6738 001a  ld   $1a
              6739 0016  ld   $16
              673a 001a  ld   $1a
              673b 0016  ld   $16
              673c 001a  ld   $1a
              673d 002a  ld   $2a
              673e 0016  ld   $16
              673f 001a  ld   $1a
              6740 0016  ld   $16
              6741 001a  ld   $1a
              6742 0026  ld   $26
              6743 001a  ld   $1a
              6744 0016  ld   $16
              6745 001a  ld   $1a
              6746 0016  ld   $16
              6747 002a  ld   $2a
              6748 0016  ld   $16
              6749 001a  ld   $1a
              674a 0016  ld   $16
              674b 0016  ld   $16
              674c 001a  ld   $1a
              674d 0015  ld   $15
              674e 001a  ld   $1a
              674f 0015  ld   $15
              6750 0016  ld   $16
              6751 0016  ld   $16
              6752 0019  ld   $19
              6753 0016  ld   $16
              6754 0016  ld   $16
              6755 0015  ld   $15
              6756 001a  ld   $1a
              6757 0015  ld   $15
              6758 0016  ld   $16
              6759 0015  ld   $15
              675a 0015  ld   $15
              675b 0016  ld   $16
              675c 0015  ld   $15
              675d 0006  ld   $06
              675e 0015  ld   $15
              675f 0015  ld   $15
              6760 0005  ld   $05
              6761 0015  ld   $15
              6762 0015  ld   $15
              6763 0006  ld   $06
              6764 0015  ld   $15
              6765 0005  ld   $05
              6766 0015  ld   $15
              6767 0005  ld   $05
              6768 0015  ld   $15
              6769 0002  ld   $02
              676a 0005  ld   $05
              676b 0015  ld   $15
              676c 0015  ld   $15
              676d 0016  ld   $16
              676e 0015  ld   $15
              676f 0005  ld   $05
              6770 0001  ld   $01
              6771 0005  ld   $05
              6772 0015  ld   $15
              6773 0001  ld   $01
              6774 0005  ld   $05
              6775 0004  ld   $04
              6776 0001  ld   $01
              6777 0005  ld   $05
              6778 0004  ld   $04
              6779 0001  ld   $01
              677a 0005  ld   $05
              677b 0011  ld   $11
              677c 0005  ld   $05
              677d 0011  ld   $11
              677e 0000  ld   $00
              677f 0000  ld   $00
              6780 0000  ld   $00
              * 34 times
              67a0 0200  nop
              67a1 0200  nop
              67a2 0200  nop
              * 91 times
              67fb fe00  bra  ac          ;Trampoline for page $6700 lookups
              67fc fcfd  bra  $67fd
              67fd 1403  ld   $03,y
              67fe e07b  jmp  y,$7b
              67ff 1519  ld   [$19],y
jupiter97:    6800 0000  ld   $00
              6801 0000  ld   $00
              6802 0000  ld   $00
              * 33 times
              6821 0015  ld   $15
              6822 0016  ld   $16
              6823 001a  ld   $1a
              6824 0016  ld   $16
              6825 001a  ld   $1a
              6826 0016  ld   $16
              6827 001a  ld   $1a
              6828 001a  ld   $1a
              6829 0016  ld   $16
              682a 001a  ld   $1a
              682b 0016  ld   $16
              682c 002a  ld   $2a
              682d 001a  ld   $1a
              682e 002b  ld   $2b
              682f 002a  ld   $2a
              6830 001b  ld   $1b
              6831 001a  ld   $1a
              6832 0016  ld   $16
              6833 001a  ld   $1a
              6834 0016  ld   $16
              6835 0016  ld   $16
              6836 001a  ld   $1a
              6837 0016  ld   $16
              6838 001a  ld   $1a
              6839 0016  ld   $16
              683a 001a  ld   $1a
              683b 0016  ld   $16
              683c 001a  ld   $1a
              683d 0016  ld   $16
              683e 001a  ld   $1a
              683f 0016  ld   $16
              6840 001a  ld   $1a
              6841 0016  ld   $16
              6842 001a  ld   $1a
              6843 0016  ld   $16
              6844 001a  ld   $1a
              6845 0016  ld   $16
              6846 001a  ld   $1a
              6847 0016  ld   $16
              6848 001a  ld   $1a
              6849 0016  ld   $16
              684a 001a  ld   $1a
              684b 0016  ld   $16
              684c 0015  ld   $15
              684d 001a  ld   $1a
              684e 0016  ld   $16
              684f 0016  ld   $16
              6850 001a  ld   $1a
              6851 0015  ld   $15
              6852 0016  ld   $16
              6853 0016  ld   $16
              6854 0019  ld   $19
              6855 0016  ld   $16
              6856 0015  ld   $15
              6857 0016  ld   $16
              6858 0019  ld   $19
              6859 0016  ld   $16
              685a 0005  ld   $05
              685b 0016  ld   $16
              685c 0005  ld   $05
              685d 0015  ld   $15
              685e 0015  ld   $15
              685f 0006  ld   $06
              6860 0015  ld   $15
              6861 0006  ld   $06
              6862 0015  ld   $15
              6863 0005  ld   $05
              6864 0015  ld   $15
              6865 0005  ld   $05
              6866 0015  ld   $15
              6867 0005  ld   $05
              6868 0005  ld   $05
              6869 0015  ld   $15
              686a 0015  ld   $15
              686b 0015  ld   $15
              686c 0015  ld   $15
              686d 0005  ld   $05
              686e 0015  ld   $15
              686f 0005  ld   $05
              6870 0015  ld   $15
              6871 0001  ld   $01
              6872 0005  ld   $05
              6873 0011  ld   $11
              6874 0005  ld   $05
              6875 0011  ld   $11
              6876 0005  ld   $05
              6877 0011  ld   $11
              6878 0005  ld   $05
              6879 0011  ld   $11
              687a 0005  ld   $05
              687b 0005  ld   $05
              687c 0001  ld   $01
              687d 0004  ld   $04
              687e 0000  ld   $00
              687f 0000  ld   $00
              6880 0000  ld   $00
              * 34 times
              68a0 0200  nop
              68a1 0200  nop
              68a2 0200  nop
              * 91 times
              68fb fe00  bra  ac          ;Trampoline for page $6800 lookups
              68fc fcfd  bra  $68fd
              68fd 1403  ld   $03,y
              68fe e07b  jmp  y,$7b
              68ff 1519  ld   [$19],y
jupiter98:    6900 0000  ld   $00
              6901 0000  ld   $00
              6902 0000  ld   $00
              * 34 times
              6922 0015  ld   $15
              6923 001a  ld   $1a
              6924 001a  ld   $1a
              6925 0016  ld   $16
              6926 001a  ld   $1a
              6927 0017  ld   $17
              6928 001a  ld   $1a
              6929 0016  ld   $16
              692a 001a  ld   $1a
              692b 0016  ld   $16
              692c 001a  ld   $1a
              692d 001a  ld   $1a
              692e 002a  ld   $2a
              692f 001b  ld   $1b
              6930 001a  ld   $1a
              6931 002a  ld   $2a
              6932 0016  ld   $16
              6933 001a  ld   $1a
              6934 0016  ld   $16
              6935 001a  ld   $1a
              6936 001a  ld   $1a
              6937 0016  ld   $16
              6938 002a  ld   $2a
              6939 001a  ld   $1a
              693a 0016  ld   $16
              693b 002a  ld   $2a
              693c 0016  ld   $16
              693d 001a  ld   $1a
              693e 0016  ld   $16
              693f 002a  ld   $2a
              6940 0016  ld   $16
              6941 001a  ld   $1a
              6942 0016  ld   $16
              6943 001a  ld   $1a
              6944 0016  ld   $16
              6945 0016  ld   $16
              6946 001a  ld   $1a
              6947 0016  ld   $16
              6948 0016  ld   $16
              6949 0019  ld   $19
              694a 0016  ld   $16
              694b 0019  ld   $19
              694c 0016  ld   $16
              694d 0016  ld   $16
              694e 0019  ld   $19
              694f 0016  ld   $16
              6950 0015  ld   $15
              6951 001a  ld   $1a
              6952 0016  ld   $16
              6953 0015  ld   $15
              6954 0016  ld   $16
              6955 0016  ld   $16
              6956 0015  ld   $15
              6957 0006  ld   $06
              6958 0016  ld   $16
              6959 0015  ld   $15
              695a 0016  ld   $16
              695b 0005  ld   $05
              695c 0016  ld   $16
              695d 0015  ld   $15
              695e 0006  ld   $06
              695f 0015  ld   $15
              6960 0015  ld   $15
              6961 0005  ld   $05
              6962 0015  ld   $15
              6963 0016  ld   $16
              6964 0005  ld   $05
              6965 0015  ld   $15
              6966 0005  ld   $05
              6967 0011  ld   $11
              6968 0005  ld   $05
              6969 0015  ld   $15
              696a 0005  ld   $05
              696b 0012  ld   $12
              696c 0005  ld   $05
              696d 0015  ld   $15
              696e 0001  ld   $01
              696f 0015  ld   $15
              6970 0001  ld   $01
              6971 0015  ld   $15
              6972 0005  ld   $05
              6973 0005  ld   $05
              6974 0005  ld   $05
              6975 0015  ld   $15
              6976 0001  ld   $01
              6977 0015  ld   $15
              6978 0005  ld   $05
              6979 0005  ld   $05
              697a 0001  ld   $01
              697b 0015  ld   $15
              697c 0004  ld   $04
              697d 0000  ld   $00
              697e 0000  ld   $00
              697f 0000  ld   $00
              * 35 times
              69a0 0200  nop
              69a1 0200  nop
              69a2 0200  nop
              * 91 times
              69fb fe00  bra  ac          ;Trampoline for page $6900 lookups
              69fc fcfd  bra  $69fd
              69fd 1403  ld   $03,y
              69fe e07b  jmp  y,$7b
              69ff 1519  ld   [$19],y
jupiter99:    6a00 0000  ld   $00
              6a01 0000  ld   $00
              6a02 0000  ld   $00
              * 35 times
              6a23 0015  ld   $15
              6a24 0016  ld   $16
              6a25 001a  ld   $1a
              6a26 0016  ld   $16
              6a27 001a  ld   $1a
              6a28 001a  ld   $1a
              6a29 0016  ld   $16
              6a2a 001b  ld   $1b
              6a2b 001a  ld   $1a
              6a2c 0026  ld   $26
              6a2d 001b  ld   $1b
              6a2e 0016  ld   $16
              6a2f 002a  ld   $2a
              6a30 0016  ld   $16
              6a31 001b  ld   $1b
              6a32 001a  ld   $1a
              6a33 0016  ld   $16
              6a34 001a  ld   $1a
              6a35 0017  ld   $17
              6a36 001a  ld   $1a
              6a37 001a  ld   $1a
              6a38 0016  ld   $16
              6a39 001b  ld   $1b
              6a3a 0016  ld   $16
              6a3b 001a  ld   $1a
              6a3c 001b  ld   $1b
              6a3d 0016  ld   $16
              6a3e 001a  ld   $1a
              6a3f 001a  ld   $1a
              6a40 0016  ld   $16
              6a41 001a  ld   $1a
              6a42 0016  ld   $16
              6a43 0016  ld   $16
              6a44 001a  ld   $1a
              6a45 0016  ld   $16
              6a46 0016  ld   $16
              6a47 0015  ld   $15
              6a48 001a  ld   $1a
              6a49 0016  ld   $16
              6a4a 0016  ld   $16
              6a4b 0016  ld   $16
              6a4c 0016  ld   $16
              6a4d 001a  ld   $1a
              6a4e 0016  ld   $16
              6a4f 0016  ld   $16
              6a50 001a  ld   $1a
              6a51 0015  ld   $15
              6a52 0016  ld   $16
              6a53 0019  ld   $19
              6a54 0016  ld   $16
              6a55 0005  ld   $05
              6a56 001a  ld   $1a
              6a57 0016  ld   $16
              6a58 0015  ld   $15
              6a59 0006  ld   $06
              6a5a 0019  ld   $19
              6a5b 0016  ld   $16
              6a5c 0015  ld   $15
              6a5d 0006  ld   $06
              6a5e 0015  ld   $15
              6a5f 0015  ld   $15
              6a60 0005  ld   $05
              6a61 0015  ld   $15
              6a62 0005  ld   $05
              6a63 0005  ld   $05
              6a64 0015  ld   $15
              6a65 0001  ld   $01
              6a66 0015  ld   $15
              6a67 0005  ld   $05
              6a68 0015  ld   $15
              6a69 0001  ld   $01
              6a6a 0015  ld   $15
              6a6b 0005  ld   $05
              6a6c 0015  ld   $15
              6a6d 0005  ld   $05
              6a6e 0011  ld   $11
              6a6f 0005  ld   $05
              6a70 0015  ld   $15
              6a71 0005  ld   $05
              6a72 0001  ld   $01
              6a73 0014  ld   $14
              6a74 0001  ld   $01
              6a75 0005  ld   $05
              6a76 0001  ld   $01
              6a77 0005  ld   $05
              6a78 0011  ld   $11
              6a79 0005  ld   $05
              6a7a 0015  ld   $15
              6a7b 0001  ld   $01
              6a7c 0000  ld   $00
              6a7d 0000  ld   $00
              6a7e 0000  ld   $00
              * 36 times
              6aa0 0200  nop
              6aa1 0200  nop
              6aa2 0200  nop
              * 91 times
              6afb fe00  bra  ac          ;Trampoline for page $6a00 lookups
              6afc fcfd  bra  $6afd
              6afd 1403  ld   $03,y
              6afe e07b  jmp  y,$7b
              6aff 1519  ld   [$19],y
jupiter100:   6b00 0000  ld   $00
              6b01 0000  ld   $00
              6b02 0000  ld   $00
              * 36 times
              6b24 0005  ld   $05
              6b25 0016  ld   $16
              6b26 001a  ld   $1a
              6b27 0016  ld   $16
              6b28 0016  ld   $16
              6b29 001a  ld   $1a
              6b2a 0016  ld   $16
              6b2b 001a  ld   $1a
              6b2c 0016  ld   $16
              6b2d 001a  ld   $1a
              6b2e 001a  ld   $1a
              6b2f 0016  ld   $16
              6b30 001a  ld   $1a
              6b31 0016  ld   $16
              6b32 0016  ld   $16
              6b33 001a  ld   $1a
              6b34 0016  ld   $16
              6b35 0016  ld   $16
              6b36 001a  ld   $1a
              6b37 0016  ld   $16
              6b38 001a  ld   $1a
              6b39 0016  ld   $16
              6b3a 001a  ld   $1a
              6b3b 0016  ld   $16
              6b3c 0016  ld   $16
              6b3d 001a  ld   $1a
              6b3e 0016  ld   $16
              6b3f 0016  ld   $16
              6b40 0016  ld   $16
              6b41 001a  ld   $1a
              6b42 0016  ld   $16
              6b43 001a  ld   $1a
              6b44 0016  ld   $16
              6b45 0015  ld   $15
              6b46 001a  ld   $1a
              6b47 0016  ld   $16
              6b48 0016  ld   $16
              6b49 0019  ld   $19
              6b4a 0016  ld   $16
              6b4b 0019  ld   $19
              6b4c 0016  ld   $16
              6b4d 0015  ld   $15
              6b4e 001a  ld   $1a
              6b4f 0015  ld   $15
              6b50 0016  ld   $16
              6b51 0005  ld   $05
              6b52 0016  ld   $16
              6b53 0005  ld   $05
              6b54 0016  ld   $16
              6b55 0015  ld   $15
              6b56 0006  ld   $06
              6b57 0015  ld   $15
              6b58 0005  ld   $05
              6b59 0016  ld   $16
              6b5a 0005  ld   $05
              6b5b 0015  ld   $15
              6b5c 0005  ld   $05
              6b5d 0015  ld   $15
              6b5e 0005  ld   $05
              6b5f 0001  ld   $01
              6b60 0005  ld   $05
              6b61 0011  ld   $11
              6b62 0005  ld   $05
              6b63 0015  ld   $15
              6b64 0001  ld   $01
              6b65 0015  ld   $15
              6b66 0005  ld   $05
              6b67 0001  ld   $01
              6b68 0005  ld   $05
              6b69 0005  ld   $05
              6b6a 0001  ld   $01
              6b6b 0015  ld   $15
              6b6c 0001  ld   $01
              6b6d 0015  ld   $15
              6b6e 0005  ld   $05
              6b6f 0005  ld   $05
              6b70 0001  ld   $01
              6b71 0015  ld   $15
              6b72 0001  ld   $01
              6b73 0005  ld   $05
              6b74 0001  ld   $01
              6b75 0015  ld   $15
              6b76 0004  ld   $04
              6b77 0001  ld   $01
              6b78 0005  ld   $05
              6b79 0011  ld   $11
              6b7a 0004  ld   $04
              6b7b 0000  ld   $00
              6b7c 0000  ld   $00
              6b7d 0000  ld   $00
              * 37 times
              6ba0 0200  nop
              6ba1 0200  nop
              6ba2 0200  nop
              * 91 times
              6bfb fe00  bra  ac          ;Trampoline for page $6b00 lookups
              6bfc fcfd  bra  $6bfd
              6bfd 1403  ld   $03,y
              6bfe e07b  jmp  y,$7b
              6bff 1519  ld   [$19],y
jupiter101:   6c00 0000  ld   $00
              6c01 0000  ld   $00
              6c02 0000  ld   $00
              * 38 times
              6c26 001a  ld   $1a
              6c27 0016  ld   $16
              6c28 001a  ld   $1a
              6c29 0006  ld   $06
              6c2a 0015  ld   $15
              6c2b 0006  ld   $06
              6c2c 0016  ld   $16
              6c2d 0016  ld   $16
              6c2e 001a  ld   $1a
              6c2f 0016  ld   $16
              6c30 001a  ld   $1a
              6c31 0016  ld   $16
              6c32 0015  ld   $15
              6c33 0006  ld   $06
              6c34 0016  ld   $16
              6c35 0005  ld   $05
              6c36 0016  ld   $16
              6c37 0005  ld   $05
              6c38 0016  ld   $16
              6c39 0006  ld   $06
              6c3a 0015  ld   $15
              6c3b 0016  ld   $16
              6c3c 001a  ld   $1a
              6c3d 0016  ld   $16
              6c3e 0016  ld   $16
              6c3f 001a  ld   $1a
              6c40 0016  ld   $16
              6c41 0016  ld   $16
              6c42 001a  ld   $1a
              6c43 0016  ld   $16
              6c44 0016  ld   $16
              6c45 001a  ld   $1a
              6c46 0005  ld   $05
              6c47 0016  ld   $16
              6c48 0005  ld   $05
              6c49 0016  ld   $16
              6c4a 0005  ld   $05
              6c4b 0016  ld   $16
              6c4c 0005  ld   $05
              6c4d 0016  ld   $16
              6c4e 0016  ld   $16
              6c4f 0005  ld   $05
              6c50 001a  ld   $1a
              6c51 0015  ld   $15
              6c52 0016  ld   $16
              6c53 0015  ld   $15
              6c54 0016  ld   $16
              6c55 0005  ld   $05
              6c56 0015  ld   $15
              6c57 0005  ld   $05
              6c58 0015  ld   $15
              6c59 0005  ld   $05
              6c5a 0001  ld   $01
              6c5b 0015  ld   $15
              6c5c 0005  ld   $05
              6c5d 0001  ld   $01
              6c5e 0005  ld   $05
              6c5f 0015  ld   $15
              6c60 0001  ld   $01
              6c61 0015  ld   $15
              6c62 0005  ld   $05
              6c63 0011  ld   $11
              6c64 0005  ld   $05
              6c65 0005  ld   $05
              6c66 0015  ld   $15
              6c67 0005  ld   $05
              6c68 0011  ld   $11
              6c69 0005  ld   $05
              6c6a 0011  ld   $11
              6c6b 0005  ld   $05
              6c6c 0005  ld   $05
              6c6d 0001  ld   $01
              6c6e 0015  ld   $15
              6c6f 0011  ld   $11
              6c70 0015  ld   $15
              6c71 0004  ld   $04
              6c72 0005  ld   $05
              6c73 0011  ld   $11
              6c74 0005  ld   $05
              6c75 0005  ld   $05
              6c76 0011  ld   $11
              6c77 0005  ld   $05
              6c78 0015  ld   $15
              6c79 0000  ld   $00
              6c7a 0000  ld   $00
              6c7b 0000  ld   $00
              * 39 times
              6ca0 0200  nop
              6ca1 0200  nop
              6ca2 0200  nop
              * 91 times
              6cfb fe00  bra  ac          ;Trampoline for page $6c00 lookups
              6cfc fcfd  bra  $6cfd
              6cfd 1403  ld   $03,y
              6cfe e07b  jmp  y,$7b
              6cff 1519  ld   [$19],y
jupiter102:   6d00 0000  ld   $00
              6d01 0000  ld   $00
              6d02 0000  ld   $00
              * 38 times
              6d26 0001  ld   $01
              6d27 0015  ld   $15
              6d28 0016  ld   $16
              6d29 0005  ld   $05
              6d2a 0016  ld   $16
              6d2b 0016  ld   $16
              6d2c 0005  ld   $05
              6d2d 0016  ld   $16
              6d2e 0016  ld   $16
              6d2f 001a  ld   $1a
              6d30 0016  ld   $16
              6d31 001a  ld   $1a
              6d32 0006  ld   $06
              6d33 0015  ld   $15
              6d34 0006  ld   $06
              6d35 0015  ld   $15
              6d36 0006  ld   $06
              6d37 0016  ld   $16
              6d38 0006  ld   $06
              6d39 0015  ld   $15
              6d3a 0006  ld   $06
              6d3b 0005  ld   $05
              6d3c 0016  ld   $16
              6d3d 0015  ld   $15
              6d3e 001a  ld   $1a
              6d3f 0016  ld   $16
              6d40 0016  ld   $16
              6d41 0019  ld   $19
              6d42 0006  ld   $06
              6d43 0015  ld   $15
              6d44 0016  ld   $16
              6d45 0005  ld   $05
              6d46 0016  ld   $16
              6d47 0005  ld   $05
              6d48 0016  ld   $16
              6d49 0005  ld   $05
              6d4a 0016  ld   $16
              6d4b 0005  ld   $05
              6d4c 0016  ld   $16
              6d4d 0005  ld   $05
              6d4e 001a  ld   $1a
              6d4f 0016  ld   $16
              6d50 0016  ld   $16
              6d51 0009  ld   $09
              6d52 0016  ld   $16
              6d53 0005  ld   $05
              6d54 0005  ld   $05
              6d55 0016  ld   $16
              6d56 0001  ld   $01
              6d57 0005  ld   $05
              6d58 0016  ld   $16
              6d59 0005  ld   $05
              6d5a 0015  ld   $15
              6d5b 0002  ld   $02
              6d5c 0005  ld   $05
              6d5d 0015  ld   $15
              6d5e 0001  ld   $01
              6d5f 0005  ld   $05
              6d60 0015  ld   $15
              6d61 0001  ld   $01
              6d62 0005  ld   $05
              6d63 0005  ld   $05
              6d64 0001  ld   $01
              6d65 0011  ld   $11
              6d66 0005  ld   $05
              6d67 0001  ld   $01
              6d68 0005  ld   $05
              6d69 0001  ld   $01
              6d6a 0004  ld   $04
              6d6b 0001  ld   $01
              6d6c 0015  ld   $15
              6d6d 0005  ld   $05
              6d6e 0015  ld   $15
              6d6f 0005  ld   $05
              6d70 0001  ld   $01
              6d71 0005  ld   $05
              6d72 0001  ld   $01
              6d73 0005  ld   $05
              6d74 0000  ld   $00
              6d75 0005  ld   $05
              6d76 0001  ld   $01
              6d77 0015  ld   $15
              6d78 0000  ld   $00
              6d79 0000  ld   $00
              6d7a 0000  ld   $00
              * 40 times
              6da0 0200  nop
              6da1 0200  nop
              6da2 0200  nop
              * 91 times
              6dfb fe00  bra  ac          ;Trampoline for page $6d00 lookups
              6dfc fcfd  bra  $6dfd
              6dfd 1403  ld   $03,y
              6dfe e07b  jmp  y,$7b
              6dff 1519  ld   [$19],y
jupiter103:   6e00 0000  ld   $00
              6e01 0000  ld   $00
              6e02 0000  ld   $00
              * 40 times
              6e28 0005  ld   $05
              6e29 0019  ld   $19
              6e2a 0016  ld   $16
              6e2b 0005  ld   $05
              6e2c 001a  ld   $1a
              6e2d 0015  ld   $15
              6e2e 0006  ld   $06
              6e2f 0015  ld   $15
              6e30 0006  ld   $06
              6e31 0019  ld   $19
              6e32 0016  ld   $16
              6e33 0016  ld   $16
              6e34 0005  ld   $05
              6e35 0015  ld   $15
              6e36 0006  ld   $06
              6e37 0005  ld   $05
              6e38 001a  ld   $1a
              6e39 001a  ld   $1a
              6e3a 001a  ld   $1a
              6e3b 0016  ld   $16
              6e3c 0005  ld   $05
              6e3d 0006  ld   $06
              6e3e 0016  ld   $16
              6e3f 0005  ld   $05
              6e40 0016  ld   $16
              6e41 0006  ld   $06
              6e42 0015  ld   $15
              6e43 0006  ld   $06
              6e44 0016  ld   $16
              6e45 0005  ld   $05
              6e46 0005  ld   $05
              6e47 0016  ld   $16
              6e48 0005  ld   $05
              6e49 0016  ld   $16
              6e4a 0016  ld   $16
              6e4b 0015  ld   $15
              6e4c 0006  ld   $06
              6e4d 0015  ld   $15
              6e4e 0006  ld   $06
              6e4f 0015  ld   $15
              6e50 0005  ld   $05
              6e51 0016  ld   $16
              6e52 0005  ld   $05
              6e53 0016  ld   $16
              6e54 0005  ld   $05
              6e55 0005  ld   $05
              6e56 0015  ld   $15
              6e57 0001  ld   $01
              6e58 0005  ld   $05
              6e59 0001  ld   $01
              6e5a 0005  ld   $05
              6e5b 0015  ld   $15
              6e5c 0005  ld   $05
              6e5d 0001  ld   $01
              6e5e 0015  ld   $15
              6e5f 0001  ld   $01
              6e60 0005  ld   $05
              6e61 0001  ld   $01
              6e62 0005  ld   $05
              6e63 0011  ld   $11
              6e64 0005  ld   $05
              6e65 0005  ld   $05
              6e66 0001  ld   $01
              6e67 0015  ld   $15
              6e68 0000  ld   $00
              6e69 0015  ld   $15
              6e6a 0001  ld   $01
              6e6b 0005  ld   $05
              6e6c 0001  ld   $01
              6e6d 0005  ld   $05
              6e6e 0001  ld   $01
              6e6f 0001  ld   $01
              6e70 0005  ld   $05
              6e71 0011  ld   $11
              6e72 0005  ld   $05
              6e73 0011  ld   $11
              6e74 0005  ld   $05
              6e75 0011  ld   $11
              6e76 0004  ld   $04
              6e77 0000  ld   $00
              6e78 0000  ld   $00
              6e79 0000  ld   $00
              * 41 times
              6ea0 0200  nop
              6ea1 0200  nop
              6ea2 0200  nop
              * 91 times
              6efb fe00  bra  ac          ;Trampoline for page $6e00 lookups
              6efc fcfd  bra  $6efd
              6efd 1403  ld   $03,y
              6efe e07b  jmp  y,$7b
              6eff 1519  ld   [$19],y
jupiter104:   6f00 0000  ld   $00
              6f01 0000  ld   $00
              6f02 0000  ld   $00
              * 41 times
              6f29 0015  ld   $15
              6f2a 0016  ld   $16
              6f2b 0015  ld   $15
              6f2c 0006  ld   $06
              6f2d 0006  ld   $06
              6f2e 0015  ld   $15
              6f2f 0016  ld   $16
              6f30 0006  ld   $06
              6f31 0016  ld   $16
              6f32 0015  ld   $15
              6f33 0006  ld   $06
              6f34 0005  ld   $05
              6f35 0006  ld   $06
              6f36 0015  ld   $15
              6f37 0016  ld   $16
              6f38 0005  ld   $05
              6f39 0016  ld   $16
              6f3a 0016  ld   $16
              6f3b 0005  ld   $05
              6f3c 0005  ld   $05
              6f3d 0016  ld   $16
              6f3e 0005  ld   $05
              6f3f 0006  ld   $06
              6f40 0015  ld   $15
              6f41 0006  ld   $06
              6f42 0015  ld   $15
              6f43 0006  ld   $06
              6f44 0015  ld   $15
              6f45 0006  ld   $06
              6f46 0016  ld   $16
              6f47 0005  ld   $05
              6f48 0015  ld   $15
              6f49 0006  ld   $06
              6f4a 0015  ld   $15
              6f4b 0006  ld   $06
              6f4c 0015  ld   $15
              6f4d 0005  ld   $05
              6f4e 0005  ld   $05
              6f4f 0016  ld   $16
              6f50 0005  ld   $05
              6f51 0005  ld   $05
              6f52 0005  ld   $05
              6f53 0005  ld   $05
              6f54 0011  ld   $11
              6f55 0006  ld   $06
              6f56 0005  ld   $05
              6f57 0005  ld   $05
              6f58 0015  ld   $15
              6f59 0005  ld   $05
              6f5a 0015  ld   $15
              6f5b 0006  ld   $06
              6f5c 0015  ld   $15
              6f5d 0001  ld   $01
              6f5e 0005  ld   $05
              6f5f 0001  ld   $01
              6f60 0015  ld   $15
              6f61 0001  ld   $01
              6f62 0005  ld   $05
              6f63 0001  ld   $01
              6f64 0004  ld   $04
              6f65 0011  ld   $11
              6f66 0004  ld   $04
              6f67 0005  ld   $05
              6f68 0011  ld   $11
              6f69 0005  ld   $05
              6f6a 0000  ld   $00
              6f6b 0005  ld   $05
              6f6c 0014  ld   $14
              6f6d 0001  ld   $01
              6f6e 0005  ld   $05
              6f6f 0014  ld   $14
              6f70 0005  ld   $05
              6f71 0000  ld   $00
              6f72 0005  ld   $05
              6f73 0000  ld   $00
              6f74 0005  ld   $05
              6f75 0000  ld   $00
              6f76 0001  ld   $01
              6f77 0000  ld   $00
              6f78 0000  ld   $00
              6f79 0000  ld   $00
              * 41 times
              6fa0 0200  nop
              6fa1 0200  nop
              6fa2 0200  nop
              * 91 times
              6ffb fe00  bra  ac          ;Trampoline for page $6f00 lookups
              6ffc fcfd  bra  $6ffd
              6ffd 1403  ld   $03,y
              6ffe e07b  jmp  y,$7b
              6fff 1519  ld   [$19],y
jupiter105:   7000 0000  ld   $00
              7001 0000  ld   $00
              7002 0000  ld   $00
              * 42 times
              702a 0001  ld   $01
              702b 0015  ld   $15
              702c 001a  ld   $1a
              702d 0015  ld   $15
              702e 0016  ld   $16
              702f 000a  ld   $0a
              7030 0015  ld   $15
              7031 0016  ld   $16
              7032 000a  ld   $0a
              7033 0015  ld   $15
              7034 0016  ld   $16
              7035 0016  ld   $16
              7036 0015  ld   $15
              7037 0006  ld   $06
              7038 0016  ld   $16
              7039 0005  ld   $05
              703a 0016  ld   $16
              703b 0006  ld   $06
              703c 0015  ld   $15
              703d 0006  ld   $06
              703e 0005  ld   $05
              703f 0015  ld   $15
              7040 0006  ld   $06
              7041 0015  ld   $15
              7042 0006  ld   $06
              7043 0015  ld   $15
              7044 0016  ld   $16
              7045 0005  ld   $05
              7046 0015  ld   $15
              7047 0006  ld   $06
              7048 0015  ld   $15
              7049 0006  ld   $06
              704a 0015  ld   $15
              704b 0005  ld   $05
              704c 0016  ld   $16
              704d 0005  ld   $05
              704e 0016  ld   $16
              704f 0005  ld   $05
              7050 0001  ld   $01
              7051 0016  ld   $16
              7052 0005  ld   $05
              7053 0016  ld   $16
              7054 0005  ld   $05
              7055 0015  ld   $15
              7056 0001  ld   $01
              7057 0005  ld   $05
              7058 0001  ld   $01
              7059 0005  ld   $05
              705a 0001  ld   $01
              705b 0005  ld   $05
              705c 0001  ld   $01
              705d 0015  ld   $15
              705e 0001  ld   $01
              705f 0005  ld   $05
              7060 0001  ld   $01
              7061 0005  ld   $05
              7062 0011  ld   $11
              7063 0005  ld   $05
              7064 0001  ld   $01
              7065 0005  ld   $05
              7066 0001  ld   $01
              7067 0005  ld   $05
              7068 0000  ld   $00
              7069 0005  ld   $05
              706a 0005  ld   $05
              706b 0001  ld   $01
              706c 0005  ld   $05
              706d 0000  ld   $00
              706e 0005  ld   $05
              706f 0001  ld   $01
              7070 0001  ld   $01
              7071 0005  ld   $05
              7072 0000  ld   $00
              7073 0005  ld   $05
              7074 0011  ld   $11
              7075 0000  ld   $00
              7076 0000  ld   $00
              7077 0000  ld   $00
              * 43 times
              70a0 0200  nop
              70a1 0200  nop
              70a2 0200  nop
              * 91 times
              70fb fe00  bra  ac          ;Trampoline for page $7000 lookups
              70fc fcfd  bra  $70fd
              70fd 1403  ld   $03,y
              70fe e07b  jmp  y,$7b
              70ff 1519  ld   [$19],y
jupiter106:   7100 0000  ld   $00
              7101 0000  ld   $00
              7102 0000  ld   $00
              * 44 times
              712c 0005  ld   $05
              712d 0015  ld   $15
              712e 0015  ld   $15
              712f 0016  ld   $16
              7130 0005  ld   $05
              7131 0016  ld   $16
              7132 0016  ld   $16
              7133 001a  ld   $1a
              7134 0005  ld   $05
              7135 0016  ld   $16
              7136 0005  ld   $05
              7137 0015  ld   $15
              7138 0006  ld   $06
              7139 0015  ld   $15
              713a 0005  ld   $05
              713b 0015  ld   $15
              713c 0016  ld   $16
              713d 0005  ld   $05
              713e 0016  ld   $16
              713f 0005  ld   $05
              7140 0015  ld   $15
              7141 0006  ld   $06
              7142 0015  ld   $15
              7143 0006  ld   $06
              7144 0015  ld   $15
              7145 0006  ld   $06
              7146 0015  ld   $15
              7147 0016  ld   $16
              7148 0005  ld   $05
              7149 0015  ld   $15
              714a 0006  ld   $06
              714b 0015  ld   $15
              714c 0005  ld   $05
              714d 0015  ld   $15
              714e 0005  ld   $05
              714f 0015  ld   $15
              7150 0005  ld   $05
              7151 0015  ld   $15
              7152 0001  ld   $01
              7153 0005  ld   $05
              7154 0001  ld   $01
              7155 0005  ld   $05
              7156 0015  ld   $15
              7157 0005  ld   $05
              7158 0015  ld   $15
              7159 0001  ld   $01
              715a 0015  ld   $15
              715b 0005  ld   $05
              715c 0005  ld   $05
              715d 0005  ld   $05
              715e 0001  ld   $01
              715f 0015  ld   $15
              7160 0000  ld   $00
              7161 0005  ld   $05
              7162 0000  ld   $00
              7163 0005  ld   $05
              7164 0015  ld   $15
              7165 0001  ld   $01
              7166 0015  ld   $15
              7167 0001  ld   $01
              7168 0015  ld   $15
              7169 0005  ld   $05
              716a 0010  ld   $10
              716b 0005  ld   $05
              716c 0015  ld   $15
              716d 0001  ld   $01
              716e 0015  ld   $15
              716f 0004  ld   $04
              7170 0015  ld   $15
              7171 0001  ld   $01
              7172 0015  ld   $15
              7173 0000  ld   $00
              7174 0000  ld   $00
              7175 0000  ld   $00
              * 45 times
              71a0 0200  nop
              71a1 0200  nop
              71a2 0200  nop
              * 91 times
              71fb fe00  bra  ac          ;Trampoline for page $7100 lookups
              71fc fcfd  bra  $71fd
              71fd 1403  ld   $03,y
              71fe e07b  jmp  y,$7b
              71ff 1519  ld   [$19],y
jupiter107:   7200 0000  ld   $00
              7201 0000  ld   $00
              7202 0000  ld   $00
              * 45 times
              722d 0005  ld   $05
              722e 0005  ld   $05
              722f 0015  ld   $15
              7230 0016  ld   $16
              7231 0015  ld   $15
              7232 0005  ld   $05
              7233 0016  ld   $16
              7234 0015  ld   $15
              7235 001a  ld   $1a
              7236 0015  ld   $15
              7237 0016  ld   $16
              7238 0019  ld   $19
              7239 0006  ld   $06
              723a 0016  ld   $16
              723b 0015  ld   $15
              723c 0006  ld   $06
              723d 0015  ld   $15
              723e 0016  ld   $16
              723f 0005  ld   $05
              7240 0016  ld   $16
              7241 0015  ld   $15
              7242 0006  ld   $06
              7243 0015  ld   $15
              7244 0006  ld   $06
              7245 0015  ld   $15
              7246 0016  ld   $16
              7247 0005  ld   $05
              7248 0016  ld   $16
              7249 0015  ld   $15
              724a 0016  ld   $16
              724b 0015  ld   $15
              724c 0016  ld   $16
              724d 0005  ld   $05
              724e 0016  ld   $16
              724f 0005  ld   $05
              7250 0012  ld   $12
              7251 0005  ld   $05
              7252 0016  ld   $16
              7253 0015  ld   $15
              7254 0005  ld   $05
              7255 0015  ld   $15
              7256 0005  ld   $05
              7257 0011  ld   $11
              7258 0005  ld   $05
              7259 0015  ld   $15
              725a 0005  ld   $05
              725b 0015  ld   $15
              725c 0001  ld   $01
              725d 0015  ld   $15
              725e 0001  ld   $01
              725f 0005  ld   $05
              7260 0015  ld   $15
              7261 0005  ld   $05
              7262 0015  ld   $15
              7263 0001  ld   $01
              7264 0015  ld   $15
              7265 0005  ld   $05
              7266 0005  ld   $05
              7267 0015  ld   $15
              7268 0005  ld   $05
              7269 0001  ld   $01
              726a 0005  ld   $05
              726b 0001  ld   $01
              726c 0015  ld   $15
              726d 0005  ld   $05
              726e 0015  ld   $15
              726f 0001  ld   $01
              7270 0015  ld   $15
              7271 0004  ld   $04
              7272 0000  ld   $00
              7273 0000  ld   $00
              7274 0000  ld   $00
              * 46 times
              72a0 0200  nop
              72a1 0200  nop
              72a2 0200  nop
              * 91 times
              72fb fe00  bra  ac          ;Trampoline for page $7200 lookups
              72fc fcfd  bra  $72fd
              72fd 1403  ld   $03,y
              72fe e07b  jmp  y,$7b
              72ff 1519  ld   [$19],y
jupiter108:   7300 0000  ld   $00
              7301 0000  ld   $00
              7302 0000  ld   $00
              * 46 times
              732e 0001  ld   $01
              732f 0015  ld   $15
              7330 0015  ld   $15
              7331 001a  ld   $1a
              7332 0016  ld   $16
              7333 0015  ld   $15
              7334 000a  ld   $0a
              7335 0015  ld   $15
              7336 0016  ld   $16
              7337 0006  ld   $06
              7338 0015  ld   $15
              7339 001a  ld   $1a
              733a 0015  ld   $15
              733b 001a  ld   $1a
              733c 0016  ld   $16
              733d 0015  ld   $15
              733e 001a  ld   $1a
              733f 0015  ld   $15
              7340 001a  ld   $1a
              7341 0015  ld   $15
              7342 001a  ld   $1a
              7343 0015  ld   $15
              7344 0016  ld   $16
              7345 0019  ld   $19
              7346 0005  ld   $05
              7347 0016  ld   $16
              7348 0015  ld   $15
              7349 0005  ld   $05
              734a 0016  ld   $16
              734b 0005  ld   $05
              734c 0015  ld   $15
              734d 0016  ld   $16
              734e 0015  ld   $15
              734f 0015  ld   $15
              7350 0015  ld   $15
              7351 0005  ld   $05
              7352 0015  ld   $15
              7353 0015  ld   $15
              7354 0006  ld   $06
              7355 0015  ld   $15
              7356 0005  ld   $05
              7357 0016  ld   $16
              7358 0005  ld   $05
              7359 0015  ld   $15
              735a 0001  ld   $01
              735b 0015  ld   $15
              735c 0005  ld   $05
              735d 0015  ld   $15
              735e 0005  ld   $05
              735f 0015  ld   $15
              7360 0001  ld   $01
              7361 0005  ld   $05
              7362 0015  ld   $15
              7363 0005  ld   $05
              7364 0001  ld   $01
              7365 0015  ld   $15
              7366 0000  ld   $00
              7367 0005  ld   $05
              7368 0011  ld   $11
              7369 0005  ld   $05
              736a 0015  ld   $15
              736b 0005  ld   $05
              736c 0011  ld   $11
              736d 0015  ld   $15
              736e 0015  ld   $15
              736f 0001  ld   $01
              7370 0000  ld   $00
              7371 0000  ld   $00
              7372 0000  ld   $00
              * 48 times
              73a0 0200  nop
              73a1 0200  nop
              73a2 0200  nop
              * 91 times
              73fb fe00  bra  ac          ;Trampoline for page $7300 lookups
              73fc fcfd  bra  $73fd
              73fd 1403  ld   $03,y
              73fe e07b  jmp  y,$7b
              73ff 1519  ld   [$19],y
jupiter109:   7400 0000  ld   $00
              7401 0000  ld   $00
              7402 0000  ld   $00
              * 49 times
              7431 0005  ld   $05
              7432 0019  ld   $19
              7433 0016  ld   $16
              7434 0015  ld   $15
              7435 0006  ld   $06
              7436 0015  ld   $15
              7437 001a  ld   $1a
              7438 0015  ld   $15
              7439 0016  ld   $16
              743a 0016  ld   $16
              743b 0015  ld   $15
              743c 0006  ld   $06
              743d 0015  ld   $15
              743e 0016  ld   $16
              743f 0005  ld   $05
              7440 0016  ld   $16
              7441 0015  ld   $15
              7442 0006  ld   $06
              7443 0019  ld   $19
              7444 0016  ld   $16
              7445 0015  ld   $15
              7446 0016  ld   $16
              7447 0015  ld   $15
              7448 0016  ld   $16
              7449 0015  ld   $15
              744a 0019  ld   $19
              744b 0016  ld   $16
              744c 0015  ld   $15
              744d 0005  ld   $05
              744e 0016  ld   $16
              744f 0005  ld   $05
              7450 0016  ld   $16
              7451 0015  ld   $15
              7452 0006  ld   $06
              7453 0015  ld   $15
              7454 0005  ld   $05
              7455 0015  ld   $15
              7456 0015  ld   $15
              7457 0005  ld   $05
              7458 0015  ld   $15
              7459 0001  ld   $01
              745a 0015  ld   $15
              745b 0005  ld   $05
              745c 0015  ld   $15
              745d 0001  ld   $01
              745e 0015  ld   $15
              745f 0001  ld   $01
              7460 0015  ld   $15
              7461 0015  ld   $15
              7462 0001  ld   $01
              7463 0005  ld   $05
              7464 0011  ld   $11
              7465 0005  ld   $05
              7466 0015  ld   $15
              7467 0001  ld   $01
              7468 0005  ld   $05
              7469 0015  ld   $15
              746a 0001  ld   $01
              746b 0015  ld   $15
              746c 0015  ld   $15
              746d 0005  ld   $05
              746e 0000  ld   $00
              746f 0000  ld   $00
              7470 0000  ld   $00
              * 50 times
              74a0 0200  nop
              74a1 0200  nop
              74a2 0200  nop
              * 91 times
              74fb fe00  bra  ac          ;Trampoline for page $7400 lookups
              74fc fcfd  bra  $74fd
              74fd 1403  ld   $03,y
              74fe e07b  jmp  y,$7b
              74ff 1519  ld   [$19],y
jupiter110:   7500 0000  ld   $00
              7501 0000  ld   $00
              7502 0000  ld   $00
              * 51 times
              7533 0015  ld   $15
              7534 0015  ld   $15
              7535 0016  ld   $16
              7536 0015  ld   $15
              7537 0015  ld   $15
              7538 0016  ld   $16
              7539 0019  ld   $19
              753a 0016  ld   $16
              753b 0019  ld   $19
              753c 0016  ld   $16
              753d 0015  ld   $15
              753e 0006  ld   $06
              753f 0015  ld   $15
              7540 001a  ld   $1a
              7541 0015  ld   $15
              7542 0016  ld   $16
              7543 0015  ld   $15
              7544 0016  ld   $16
              7545 0005  ld   $05
              7546 0015  ld   $15
              7547 0009  ld   $09
              7548 0016  ld   $16
              7549 0015  ld   $15
              754a 0016  ld   $16
              754b 0005  ld   $05
              754c 0015  ld   $15
              754d 0016  ld   $16
              754e 0015  ld   $15
              754f 0005  ld   $05
              7550 0015  ld   $15
              7551 0005  ld   $05
              7552 0015  ld   $15
              7553 0015  ld   $15
              7554 0011  ld   $11
              7555 0005  ld   $05
              7556 0015  ld   $15
              7557 0001  ld   $01
              7558 0015  ld   $15
              7559 0005  ld   $05
              755a 0001  ld   $01
              755b 0015  ld   $15
              755c 0001  ld   $01
              755d 0015  ld   $15
              755e 0005  ld   $05
              755f 0015  ld   $15
              7560 0005  ld   $05
              7561 0001  ld   $01
              7562 0015  ld   $15
              7563 0005  ld   $05
              7564 0000  ld   $00
              7565 0015  ld   $15
              7566 0001  ld   $01
              7567 0005  ld   $05
              7568 0015  ld   $15
              7569 0001  ld   $01
              756a 0005  ld   $05
              756b 0000  ld   $00
              756c 0000  ld   $00
              756d 0000  ld   $00
              * 53 times
              75a0 0200  nop
              75a1 0200  nop
              75a2 0200  nop
              * 91 times
              75fb fe00  bra  ac          ;Trampoline for page $7500 lookups
              75fc fcfd  bra  $75fd
              75fd 1403  ld   $03,y
              75fe e07b  jmp  y,$7b
              75ff 1519  ld   [$19],y
jupiter111:   7600 0000  ld   $00
              7601 0000  ld   $00
              7602 0000  ld   $00
              * 53 times
              7635 0015  ld   $15
              7636 0005  ld   $05
              7637 0016  ld   $16
              7638 0005  ld   $05
              7639 0015  ld   $15
              763a 0005  ld   $05
              763b 0016  ld   $16
              763c 0005  ld   $05
              763d 0015  ld   $15
              763e 0016  ld   $16
              763f 0015  ld   $15
              7640 0015  ld   $15
              7641 0016  ld   $16
              7642 0005  ld   $05
              7643 0015  ld   $15
              7644 0005  ld   $05
              7645 0015  ld   $15
              7646 0016  ld   $16
              7647 0015  ld   $15
              7648 0015  ld   $15
              7649 0015  ld   $15
              764a 0005  ld   $05
              764b 0015  ld   $15
              764c 0015  ld   $15
              764d 0015  ld   $15
              764e 0005  ld   $05
              764f 0015  ld   $15
              7650 0015  ld   $15
              7651 0001  ld   $01
              7652 0015  ld   $15
              7653 0005  ld   $05
              7654 0015  ld   $15
              7655 0001  ld   $01
              7656 0015  ld   $15
              7657 0005  ld   $05
              7658 0005  ld   $05
              7659 0011  ld   $11
              765a 0005  ld   $05
              765b 0015  ld   $15
              765c 0005  ld   $05
              765d 0005  ld   $05
              765e 0011  ld   $11
              765f 0005  ld   $05
              7660 0001  ld   $01
              7661 0015  ld   $15
              7662 0005  ld   $05
              7663 0011  ld   $11
              7664 0005  ld   $05
              7665 0001  ld   $01
              7666 0015  ld   $15
              7667 0014  ld   $14
              7668 0001  ld   $01
              7669 0004  ld   $04
              766a 0010  ld   $10
              766b 0000  ld   $00
              766c 0000  ld   $00
              766d 0000  ld   $00
              * 53 times
              76a0 0200  nop
              76a1 0200  nop
              76a2 0200  nop
              * 91 times
              76fb fe00  bra  ac          ;Trampoline for page $7600 lookups
              76fc fcfd  bra  $76fd
              76fd 1403  ld   $03,y
              76fe e07b  jmp  y,$7b
              76ff 1519  ld   [$19],y
jupiter112:   7700 0000  ld   $00
              7701 0000  ld   $00
              7702 0000  ld   $00
              * 55 times
              7737 0004  ld   $04
              7738 0015  ld   $15
              7739 0005  ld   $05
              773a 0015  ld   $15
              773b 0005  ld   $05
              773c 0015  ld   $15
              773d 0005  ld   $05
              773e 0015  ld   $15
              773f 0005  ld   $05
              7740 0005  ld   $05
              7741 0005  ld   $05
              7742 0015  ld   $15
              7743 0005  ld   $05
              7744 0015  ld   $15
              7745 0005  ld   $05
              7746 0005  ld   $05
              7747 0005  ld   $05
              * 5 times
              774a 0015  ld   $15
              774b 0005  ld   $05
              774c 0005  ld   $05
              774d 0005  ld   $05
              774e 0015  ld   $15
              774f 0005  ld   $05
              7750 0005  ld   $05
              7751 0015  ld   $15
              7752 0005  ld   $05
              7753 0001  ld   $01
              7754 0015  ld   $15
              7755 0005  ld   $05
              7756 0001  ld   $01
              7757 0015  ld   $15
              7758 0001  ld   $01
              7759 0005  ld   $05
              775a 0001  ld   $01
              775b 0005  ld   $05
              775c 0011  ld   $11
              775d 0005  ld   $05
              775e 0005  ld   $05
              775f 0010  ld   $10
              7760 0005  ld   $05
              7761 0000  ld   $00
              7762 0015  ld   $15
              7763 0005  ld   $05
              7764 0011  ld   $11
              7765 0005  ld   $05
              7766 0000  ld   $00
              7767 0001  ld   $01
              7768 0000  ld   $00
              7769 0000  ld   $00
              776a 0000  ld   $00
              * 56 times
              77a0 0200  nop
              77a1 0200  nop
              77a2 0200  nop
              * 91 times
              77fb fe00  bra  ac          ;Trampoline for page $7700 lookups
              77fc fcfd  bra  $77fd
              77fd 1403  ld   $03,y
              77fe e07b  jmp  y,$7b
              77ff 1519  ld   [$19],y
jupiter113:   7800 0000  ld   $00
              7801 0000  ld   $00
              7802 0000  ld   $00
              * 58 times
              783a 0001  ld   $01
              783b 0011  ld   $11
              783c 0005  ld   $05
              783d 0015  ld   $15
              783e 0005  ld   $05
              783f 0015  ld   $15
              7840 0015  ld   $15
              7841 0005  ld   $05
              7842 0015  ld   $15
              7843 0005  ld   $05
              7844 0015  ld   $15
              7845 0005  ld   $05
              7846 0015  ld   $15
              7847 0001  ld   $01
              7848 0015  ld   $15
              7849 0005  ld   $05
              784a 0001  ld   $01
              784b 0015  ld   $15
              784c 0001  ld   $01
              784d 0015  ld   $15
              784e 0001  ld   $01
              784f 0005  ld   $05
              7850 0011  ld   $11
              7851 0001  ld   $01
              7852 0005  ld   $05
              7853 0011  ld   $11
              7854 0005  ld   $05
              7855 0001  ld   $01
              7856 0015  ld   $15
              7857 0001  ld   $01
              7858 0005  ld   $05
              7859 0001  ld   $01
              785a 0015  ld   $15
              785b 0000  ld   $00
              785c 0005  ld   $05
              785d 0010  ld   $10
              785e 0001  ld   $01
              785f 0005  ld   $05
              7860 0011  ld   $11
              7861 0005  ld   $05
              7862 0000  ld   $00
              7863 0015  ld   $15
              7864 0000  ld   $00
              7865 0000  ld   $00
              7866 0000  ld   $00
              * 60 times
              78a0 0200  nop
              78a1 0200  nop
              78a2 0200  nop
              * 91 times
              78fb fe00  bra  ac          ;Trampoline for page $7800 lookups
              78fc fcfd  bra  $78fd
              78fd 1403  ld   $03,y
              78fe e07b  jmp  y,$7b
              78ff 1519  ld   [$19],y
jupiter114:   7900 0000  ld   $00
              7901 0000  ld   $00
              7902 0000  ld   $00
              * 61 times
              793d 0001  ld   $01
              793e 0001  ld   $01
              793f 0001  ld   $01
              7940 0005  ld   $05
              7941 0011  ld   $11
              7942 0005  ld   $05
              7943 0001  ld   $01
              7944 0001  ld   $01
              7945 0015  ld   $15
              7946 0001  ld   $01
              7947 0005  ld   $05
              7948 0001  ld   $01
              7949 0005  ld   $05
              794a 0001  ld   $01
              794b 0005  ld   $05
              794c 0005  ld   $05
              794d 0001  ld   $01
              794e 0005  ld   $05
              794f 0011  ld   $11
              7950 0005  ld   $05
              7951 0001  ld   $01
              7952 0005  ld   $05
              7953 0001  ld   $01
              7954 0005  ld   $05
              7955 0011  ld   $11
              7956 0004  ld   $04
              7957 0001  ld   $01
              7958 0004  ld   $04
              7959 0001  ld   $01
              795a 0004  ld   $04
              795b 0001  ld   $01
              795c 0015  ld   $15
              795d 0005  ld   $05
              795e 0015  ld   $15
              795f 0000  ld   $00
              7960 0005  ld   $05
              7961 0000  ld   $00
              7962 0000  ld   $00
              7963 0000  ld   $00
              * 63 times
              79a0 0200  nop
              79a1 0200  nop
              79a2 0200  nop
              * 91 times
              79fb fe00  bra  ac          ;Trampoline for page $7900 lookups
              79fc fcfd  bra  $79fd
              79fd 1403  ld   $03,y
              79fe e07b  jmp  y,$7b
              79ff 1519  ld   [$19],y
jupiter115:   7a00 0000  ld   $00
              7a01 0000  ld   $00
              7a02 0000  ld   $00
              * 66 times
              7a42 0015  ld   $15
              7a43 0000  ld   $00
              7a44 0015  ld   $15
              7a45 0001  ld   $01
              7a46 0014  ld   $14
              7a47 0001  ld   $01
              7a48 0005  ld   $05
              7a49 0010  ld   $10
              7a4a 0005  ld   $05
              7a4b 0001  ld   $01
              7a4c 0011  ld   $11
              7a4d 0004  ld   $04
              7a4e 0001  ld   $01
              7a4f 0004  ld   $04
              7a50 0011  ld   $11
              7a51 0004  ld   $04
              7a52 0011  ld   $11
              7a53 0004  ld   $04
              7a54 0011  ld   $11
              7a55 0004  ld   $04
              7a56 0001  ld   $01
              7a57 0015  ld   $15
              7a58 0001  ld   $01
              7a59 0014  ld   $14
              7a5a 0001  ld   $01
              7a5b 0015  ld   $15
              7a5c 0000  ld   $00
              7a5d 0000  ld   $00
              7a5e 0001  ld   $01
              7a5f 0000  ld   $00
              7a60 0000  ld   $00
              7a61 0000  ld   $00
              * 65 times
              7aa0 0200  nop
              7aa1 0200  nop
              7aa2 0200  nop
              * 91 times
              7afb fe00  bra  ac          ;Trampoline for page $7a00 lookups
              7afc fcfd  bra  $7afd
              7afd 1403  ld   $03,y
              7afe e07b  jmp  y,$7b
              7aff 1519  ld   [$19],y
jupiter116:   7b00 0000  ld   $00
              7b01 0000  ld   $00
              7b02 0000  ld   $00
              * 72 times
              7b48 0011  ld   $11
              7b49 0004  ld   $04
              7b4a 0001  ld   $01
              7b4b 0014  ld   $14
              7b4c 0001  ld   $01
              7b4d 0005  ld   $05
              7b4e 0010  ld   $10
              7b4f 0001  ld   $01
              7b50 0005  ld   $05
              7b51 0010  ld   $10
              7b52 0005  ld   $05
              7b53 0000  ld   $00
              7b54 0001  ld   $01
              7b55 0000  ld   $00
              7b56 0005  ld   $05
              7b57 0000  ld   $00
              7b58 0000  ld   $00
              7b59 0000  ld   $00
              * 73 times
              7ba0 0200  nop
              7ba1 0200  nop
              7ba2 0200  nop
              * 91 times
              7bfb fe00  bra  ac          ;Trampoline for page $7b00 lookups
              7bfc fcfd  bra  $7bfd
              7bfd 1403  ld   $03,y
              7bfe e07b  jmp  y,$7b
              7bff 1519  ld   [$19],y
jupiter117:   7c00 0000  ld   $00
              7c01 0000  ld   $00
              7c02 0000  ld   $00
              * 83 times
              7c53 0010  ld   $10
              7c54 0000  ld   $00
              7c55 0000  ld   $00
              7c56 0000  ld   $00
              * 76 times
              7ca0 0200  nop
              7ca1 0200  nop
              7ca2 0200  nop
              * 91 times
              7cfb fe00  bra  ac          ;Trampoline for page $7c00 lookups
              7cfc fcfd  bra  $7cfd
              7cfd 1403  ld   $03,y
              7cfe e07b  jmp  y,$7b
              7cff 1519  ld   [$19],y
jupiter118:   7d00 0000  ld   $00
              7d01 0000  ld   $00
              7d02 0000  ld   $00
              * 160 times
              7da0 0200  nop
              7da1 0200  nop
              7da2 0200  nop
              * 91 times
              7dfb fe00  bra  ac          ;Trampoline for page $7d00 lookups
              7dfc fcfd  bra  $7dfd
              7dfd 1403  ld   $03,y
              7dfe e07b  jmp  y,$7b
              7dff 1519  ld   [$19],y
jupiter119:   7e00 0000  ld   $00
              7e01 0000  ld   $00
              7e02 0000  ld   $00
              * 160 times
              7ea0 0200  nop
              7ea1 0200  nop
              7ea2 0200  nop
              * 91 times
              7efb fe00  bra  ac          ;Trampoline for page $7e00 lookups
              7efc fcfd  bra  $7efd
              7efd 1403  ld   $03,y
              7efe e07b  jmp  y,$7b
              7eff 1519  ld   [$19],y
initVcpu:     7f00 1000  ld   $00,x
              7f01 1403  ld   $03,y
              7f02 dcfc  st   $fc,[y,x++] ;0300 DEF
              7f03 dc6d  st   $6d,[y,x++]
              7f04 dc22  st   $22,[y,x++] ;0302 LDW
              7f05 dc24  st   $24,[y,x++] ;0303 'Char'
              7f06 dcf3  st   $f3,[y,x++] ;0304 SUBI
              7f07 dc52  st   $52,[y,x++]
              7f08 dc36  st   $36,[y,x++] ;0306 COND
              7f09 dc56  st   $56,[y,x++] ;0307 GE
              7f0a dc10  st   $10,[y,x++]
              7f0b dcf0  st   $f0,[y,x++] ;0309 ADDI
              7f0c dc32  st   $32,[y,x++]
              7f0d dc2c  st   $2c,[y,x++] ;030b STW
              7f0e dc26  st   $26,[y,x++] ;030c 'i'
              7f0f dc12  st   $12,[y,x++] ;030d LDWI
              7f10 dc00  st   $00,[y,x++]
              7f11 dc05  st   $05,[y,x++]
              7f12 dc98  st   $98,[y,x++] ;0310 BRA
              7f13 dc15  st   $15,[y,x++]
              7f14 dc2c  st   $2c,[y,x++] ;0312 STW
              7f15 dc26  st   $26,[y,x++] ;0313 'i'
              7f16 dc12  st   $12,[y,x++] ;0314 LDWI
              7f17 dc00  st   $00,[y,x++]
              7f18 dc06  st   $06,[y,x++]
              7f19 dc2c  st   $2c,[y,x++] ;0317 STW
              7f1a dc28  st   $28,[y,x++] ;0318 'fontData'
              7f1b dc22  st   $22,[y,x++] ;0319 LDW
              7f1c dc26  st   $26,[y,x++] ;031a 'i'
              7f1d dcc4  st   $c4,[y,x++] ;031b ADDW
              7f1e dc26  st   $26,[y,x++] ;031c 'i'
              7f1f dc2c  st   $2c,[y,x++] ;031d STW
              7f20 dc2a  st   $2a,[y,x++] ;031e 'tmp'
              7f21 dcc4  st   $c4,[y,x++] ;031f ADDW
              7f22 dc2a  st   $2a,[y,x++] ;0320 'tmp'
              7f23 dcc4  st   $c4,[y,x++] ;0321 ADDW
              7f24 dc26  st   $26,[y,x++] ;0322 'i'
              7f25 dcc4  st   $c4,[y,x++] ;0323 ADDW
              7f26 dc28  st   $28,[y,x++] ;0324 'fontData'
              7f27 dc2c  st   $2c,[y,x++] ;0325 STW
              7f28 dc28  st   $28,[y,x++] ;0326 'fontData'
              7f29 dc12  st   $12,[y,x++] ;0327 LDWI
              7f2a dc00  st   $00,[y,x++]
              7f2b dc08  st   $08,[y,x++]
              7f2c dcc4  st   $c4,[y,x++] ;032a ADDW
              7f2d dc2c  st   $2c,[y,x++] ;032b 'Pos'
              7f2e dc2c  st   $2c,[y,x++] ;032c STW
              7f2f dc2e  st   $2e,[y,x++] ;032d 'U'
              7f30 dc5c  st   $5c,[y,x++] ;032e LDI
              7f31 dc05  st   $05,[y,x++]
              7f32 dc2c  st   $2c,[y,x++] ;0330 STW
              7f33 dc26  st   $26,[y,x++] ;0331 'i'
              7f34 dc36  st   $36,[y,x++] ;0332 COND
              7f35 dc4d  st   $4d,[y,x++] ;0333 EQ
              7f36 dc3b  st   $3b,[y,x++]
              7f37 dc22  st   $22,[y,x++] ;0335 LDW
              7f38 dc28  st   $28,[y,x++] ;0336 'fontData'
              7f39 dc78  st   $78,[y,x++] ;0337 LOOKUP
              7f3a dc00  st   $00,[y,x++]
              7f3b dcf6  st   $f6,[y,x++] ;0339 INC
              7f3c dc28  st   $28,[y,x++] ;033a 'fontData'
              7f3d dc98  st   $98,[y,x++] ;033b BRA
              7f3e dc3d  st   $3d,[y,x++]
              7f3f dc5c  st   $5c,[y,x++] ;033d LDI
              7f40 dc00  st   $00,[y,x++]
              7f41 dc2c  st   $2c,[y,x++] ;033f STW
              7f42 dc30  st   $30,[y,x++] ;0340 'bits'
              7f43 dc22  st   $22,[y,x++] ;0341 LDW
              7f44 dc2c  st   $2c,[y,x++] ;0342 'Pos'
              7f45 dc2c  st   $2c,[y,x++] ;0343 STW
              7f46 dc32  st   $32,[y,x++] ;0344 'p'
              7f47 dc22  st   $22,[y,x++] ;0345 LDW
              7f48 dc30  st   $30,[y,x++] ;0346 'bits'
              7f49 dc8a  st   $8a,[y,x++] ;0347 ANDI
              7f4a dc80  st   $80,[y,x++]
              7f4b dc36  st   $36,[y,x++] ;0349 COND
              7f4c dc75  st   $75,[y,x++] ;034a NE
              7f4d dc4e  st   $4e,[y,x++]
              7f4e dc5c  st   $5c,[y,x++] ;034c LDI
              7f4f dc00  st   $00,[y,x++]
              7f50 dc98  st   $98,[y,x++] ;034e BRA
              7f51 dc50  st   $50,[y,x++]
              7f52 dc22  st   $22,[y,x++] ;0350 LDW
              7f53 dc34  st   $34,[y,x++] ;0351 'Color'
              7f54 dc9b  st   $9b,[y,x++] ;0352 POKE
              7f55 dc32  st   $32,[y,x++] ;0353 'p'
              7f56 dc22  st   $22,[y,x++] ;0354 LDW
              7f57 dc30  st   $30,[y,x++] ;0355 'bits'
              7f58 dcc4  st   $c4,[y,x++] ;0356 ADDW
              7f59 dc30  st   $30,[y,x++] ;0357 'bits'
              7f5a dc2c  st   $2c,[y,x++] ;0358 STW
              7f5b dc30  st   $30,[y,x++] ;0359 'bits'
              7f5c dcf6  st   $f6,[y,x++] ;035a INC
              7f5d dc33  st   $33,[y,x++] ;035b 'p'+1
              7f5e dc22  st   $22,[y,x++] ;035c LDW
              7f5f dc32  st   $32,[y,x++] ;035d 'p'
              7f60 dcb0  st   $b0,[y,x++] ;035e SUBW
              7f61 dc2e  st   $2e,[y,x++] ;035f 'U'
              7f62 dc36  st   $36,[y,x++] ;0360 COND
              7f63 dc53  st   $53,[y,x++] ;0361 LT
              7f64 dc43  st   $43,[y,x++]
              7f65 dcf6  st   $f6,[y,x++] ;0363 INC
              7f66 dc2c  st   $2c,[y,x++] ;0364 'Pos'
              7f67 dc22  st   $22,[y,x++] ;0365 LDW
              7f68 dc26  st   $26,[y,x++] ;0366 'i'
              7f69 dcf3  st   $f3,[y,x++] ;0367 SUBI
              7f6a dc01  st   $01,[y,x++]
              7f6b dc2c  st   $2c,[y,x++] ;0369 STW
              7f6c dc26  st   $26,[y,x++] ;036a 'i'
              7f6d dc36  st   $36,[y,x++] ;036b COND
              7f6e dc56  st   $56,[y,x++] ;036c GE
              7f6f dc30  st   $30,[y,x++]
              7f70 dcff  st   $ff,[y,x++] ;036e RET
              7f71 dc2c  st   $2c,[y,x++] ;036f STW
              7f72 dc36  st   $36,[y,x++] ;0370 'DrawChar'
              7f73 dcfc  st   $fc,[y,x++] ;0371 DEF
              7f74 dc95  st   $95,[y,x++]
              7f75 dc80  st   $80,[y,x++] ;0373 PUSH
              7f76 dc22  st   $22,[y,x++] ;0374 LDW
              7f77 dc38  st   $38,[y,x++] ;0375 'Text'
              7f78 dcd8  st   $d8,[y,x++] ;0376 PEEK
              7f79 dc2c  st   $2c,[y,x++] ;0377 STW
              7f7a dc24  st   $24,[y,x++] ;0378 'Char'
              7f7b dc36  st   $36,[y,x++] ;0379 COND
              7f7c dc4d  st   $4d,[y,x++] ;037a EQ
              7f7d dc93  st   $93,[y,x++]
              7f7e dcf6  st   $f6,[y,x++] ;037c INC
              7f7f dc38  st   $38,[y,x++] ;037d 'Text'
              7f80 dc22  st   $22,[y,x++] ;037e LDW
              7f81 dc24  st   $24,[y,x++] ;037f 'Char'
              7f82 dc94  st   $94,[y,x++] ;0380 XORI
              7f83 dc0a  st   $0a,[y,x++]
              7f84 dc36  st   $36,[y,x++] ;0382 COND
              7f85 dc75  st   $75,[y,x++] ;0383 NE
              7f86 dc8e  st   $8e,[y,x++]
              7f87 dc61  st   $61,[y,x++] ;0385 ST
              7f88 dc2c  st   $2c,[y,x++] ;0386 'Pos'
              7f89 dc12  st   $12,[y,x++] ;0387 LDWI
              7f8a dc00  st   $00,[y,x++]
              7f8b dc08  st   $08,[y,x++]
              7f8c dcc4  st   $c4,[y,x++] ;038a ADDW
              7f8d dc2c  st   $2c,[y,x++] ;038b 'Pos'
              7f8e dc2c  st   $2c,[y,x++] ;038c STW
              7f8f dc2c  st   $2c,[y,x++] ;038d 'Pos'
              7f90 dc98  st   $98,[y,x++] ;038e BRA
              7f91 dc91  st   $91,[y,x++]
              7f92 dc22  st   $22,[y,x++] ;0390 LDW
              7f93 dc36  st   $36,[y,x++] ;0391 'DrawChar'
              7f94 dce4  st   $e4,[y,x++] ;0392 CALL
              7f95 dc98  st   $98,[y,x++] ;0393 BRA
              7f96 dc72  st   $72,[y,x++]
              7f97 dc66  st   $66,[y,x++] ;0395 POP
              7f98 dce4  st   $e4,[y,x++] ;0396 CALL
              7f99 dc2c  st   $2c,[y,x++] ;0397 STW
              7f9a dc3a  st   $3a,[y,x++] ;0398 'DrawText'
              7f9b dcfc  st   $fc,[y,x++] ;0399 DEF
              7f9c dcd2  st   $d2,[y,x++]
              7f9d dc20  st   $20,[y,x++]
              7f9e dc20  st   $20,[y,x++]
              7f9f dc20  st   $20,[y,x++]
              7fa0 dc2a  st   $2a,[y,x++]
              7fa1 dc2a  st   $2a,[y,x++]
              7fa2 dc2a  st   $2a,[y,x++]
              7fa3 dc20  st   $20,[y,x++]
              7fa4 dc47  st   $47,[y,x++]
              7fa5 dc69  st   $69,[y,x++]
              7fa6 dc67  st   $67,[y,x++]
              7fa7 dc61  st   $61,[y,x++]
              7fa8 dc74  st   $74,[y,x++]
              7fa9 dc72  st   $72,[y,x++]
              7faa dc6f  st   $6f,[y,x++]
              7fab dc6e  st   $6e,[y,x++]
              7fac dc20  st   $20,[y,x++]
              7fad dc33  st   $33,[y,x++]
              7fae dc32  st   $32,[y,x++]
              7faf dc4b  st   $4b,[y,x++]
              7fb0 dc20  st   $20,[y,x++]
              7fb1 dc2a  st   $2a,[y,x++]
              7fb2 dc2a  st   $2a,[y,x++]
              7fb3 dc2a  st   $2a,[y,x++]
              7fb4 dc20  st   $20,[y,x++]
              7fb5 dc20  st   $20,[y,x++]
              7fb6 dc20  st   $20,[y,x++]
              7fb7 dc20  st   $20,[y,x++]
              7fb8 dc0a  st   $0a,[y,x++]
              7fb9 dc54  st   $54,[y,x++]
              7fba dc54  st   $54,[y,x++]
              7fbb dc4c  st   $4c,[y,x++]
              7fbc dc20  st   $20,[y,x++]
              7fbd dc63  st   $63,[y,x++]
              7fbe dc6f  st   $6f,[y,x++]
              7fbf dc6c  st   $6c,[y,x++]
              7fc0 dc6f  st   $6f,[y,x++]
              7fc1 dc72  st   $72,[y,x++]
              7fc2 dc20  st   $20,[y,x++]
              7fc3 dc63  st   $63,[y,x++]
              7fc4 dc6f  st   $6f,[y,x++]
              7fc5 dc6d  st   $6d,[y,x++]
              7fc6 dc70  st   $70,[y,x++]
              7fc7 dc75  st   $75,[y,x++]
              7fc8 dc74  st   $74,[y,x++]
              7fc9 dc65  st   $65,[y,x++]
              7fca dc72  st   $72,[y,x++]
              7fcb dc20  st   $20,[y,x++]
              7fcc dc52  st   $52,[y,x++]
              7fcd dc4f  st   $4f,[y,x++]
              7fce dc4d  st   $4d,[y,x++]
              7fcf dc20  st   $20,[y,x++]
              7fd0 dc76  st   $76,[y,x++]
              7fd1 dc30  st   $30,[y,x++]
              7fd2 dc20  st   $20,[y,x++]
              7fd3 dc20  st   $20,[y,x++]
              7fd4 dc0a  st   $0a,[y,x++]
              7fd5 dc00  st   $00,[y,x++]
              7fd6 dc2c  st   $2c,[y,x++] ;03d4 STW
              7fd7 dc3c  st   $3c,[y,x++] ;03d5 'Welcome'
              7fd8 dcff  st   $ff,[y,x++] ;03d6 RET
              7fd9 1000  ld   $00,x
              7fda 1404  ld   $04,y
              7fdb dcfc  st   $fc,[y,x++] ;0400 DEF
              7fdc dc26  st   $26,[y,x++]
              7fdd dc22  st   $22,[y,x++] ;0402 LDW
              7fde dc2c  st   $2c,[y,x++] ;0403 'Pos'
              7fdf dc2c  st   $2c,[y,x++] ;0404 STW
              7fe0 dc32  st   $32,[y,x++] ;0405 'p'
              7fe1 dc22  st   $22,[y,x++] ;0406 LDW
              7fe2 dc32  st   $32,[y,x++] ;0407 'p'
              7fe3 dc2c  st   $2c,[y,x++] ;0408 STW
              7fe4 dc22  st   $22,[y,x++]
              7fe5 dc12  st   $12,[y,x++] ;040a LDWI
              7fe6 dc31  st   $31,[y,x++]
              7fe7 dc04  st   $04,[y,x++]
              7fe8 dcac  st   $ac,[y,x++] ;040d SYS
              7fe9 dcfb  st   $fb,[y,x++]
              7fea dc12  st   $12,[y,x++] ;040f LDWI
              7feb dc00  st   $00,[y,x++]
              7fec dc08  st   $08,[y,x++]
              7fed dcc4  st   $c4,[y,x++] ;0412 ADDW
              7fee dc32  st   $32,[y,x++] ;0413 'p'
              7fef dc2c  st   $2c,[y,x++] ;0414 STW
              7ff0 dc32  st   $32,[y,x++] ;0415 'p'
              7ff1 dc36  st   $36,[y,x++] ;0416 COND
              7ff2 dc50  st   $50,[y,x++] ;0417 GT
              7ff3 dc04  st   $04,[y,x++]
              7ff4 dc12  st   $12,[y,x++] ;0419 LDWI
              7ff5 dc01  st   $01,[y,x++]
              7ff6 dc98  st   $98,[y,x++]
              7ff7 dcc4  st   $c4,[y,x++] ;041c ADDW
              7ff8 dc32  st   $32,[y,x++] ;041d 'p'
              7ff9 dc2c  st   $2c,[y,x++] ;041e STW
              7ffa dc32  st   $32,[y,x++] ;041f 'p'
              7ffb dc8a  st   $8a,[y,x++] ;0420 ANDI
              7ffc dcff  st   $ff,[y,x++]
              7ffd dc94  st   $94,[y,x++] ;0422 XORI
              7ffe dca0  st   $a0,[y,x++]
              7fff dc36  st   $36,[y,x++] ;0424 COND
              8000 dc75  st   $75,[y,x++] ;0425 NE
              8001 dc04  st   $04,[y,x++]
              8002 dcff  st   $ff,[y,x++] ;0427 RET
              8003 dc2c  st   $2c,[y,x++] ;0428 STW
              8004 dc3e  st   $3e,[y,x++] ;0429 'Clear'
              8005 dcfc  st   $fc,[y,x++] ;042a DEF
              8006 dcec  st   $ec,[y,x++]
              8007 dc22  st   $22,[y,x++] ;042c LDW
              8008 dc40  st   $40,[y,x++] ;042d 'ShiftControl'
              8009 dcd8  st   $d8,[y,x++] ;042e PEEK
              800a dc2c  st   $2c,[y,x++] ;042f STW
              800b dc42  st   $42,[y,x++] ;0430 'X'
              800c dc94  st   $94,[y,x++] ;0431 XORI
              800d dc78  st   $78,[y,x++]
              800e dc36  st   $36,[y,x++] ;0433 COND
              800f dc75  st   $75,[y,x++] ;0434 NE
              8010 dc38  st   $38,[y,x++]
              8011 dc5c  st   $5c,[y,x++] ;0436 LDI
              8012 dc0a  st   $0a,[y,x++]
              8013 dc2c  st   $2c,[y,x++] ;0438 STW
              8014 dc44  st   $44,[y,x++] ;0439 'BallA'
              8015 dc1b  st   $1b,[y,x++] ;043a LD
              8016 dc0d  st   $0d,[y,x++]
              8017 dcc4  st   $c4,[y,x++] ;043c ADDW
              8018 dc46  st   $46,[y,x++] ;043d 'Rnd0'
              8019 dcc4  st   $c4,[y,x++] ;043e ADDW
              801a dc48  st   $48,[y,x++] ;043f 'Rnd1'
              801b dc2c  st   $2c,[y,x++] ;0440 STW
              801c dc46  st   $46,[y,x++] ;0441 'Rnd0'
              801d dcc4  st   $c4,[y,x++] ;0442 ADDW
              801e dc48  st   $48,[y,x++] ;0443 'Rnd1'
              801f dc36  st   $36,[y,x++] ;0444 COND
              8020 dc56  st   $56,[y,x++] ;0445 GE
              8021 dc47  st   $47,[y,x++]
              8022 dc94  st   $94,[y,x++] ;0447 XORI
              8023 dc2b  st   $2b,[y,x++]
              8024 dcc4  st   $c4,[y,x++] ;0449 ADDW
              8025 dc48  st   $48,[y,x++] ;044a 'Rnd1'
              8026 dcc4  st   $c4,[y,x++] ;044b ADDW
              8027 dc48  st   $48,[y,x++] ;044c 'Rnd1'
              8028 dc2c  st   $2c,[y,x++] ;044d STW
              8029 dc48  st   $48,[y,x++] ;044e 'Rnd1'
              802a dc22  st   $22,[y,x++] ;044f LDW
              802b dc4a  st   $4a,[y,x++] ;0450 'Height'
              802c dcf3  st   $f3,[y,x++] ;0451 SUBI
              802d dc58  st   $58,[y,x++]
              802e dc36  st   $36,[y,x++] ;0453 COND
              802f dc59  st   $59,[y,x++] ;0454 LE
              8030 dc61  st   $61,[y,x++]
              8031 dc22  st   $22,[y,x++] ;0456 LDW
              8032 dc46  st   $46,[y,x++] ;0457 'Rnd0'
              8033 dc8a  st   $8a,[y,x++] ;0458 ANDI
              8034 dc10  st   $10,[y,x++]
              8035 dc36  st   $36,[y,x++] ;045a COND
              8036 dc4d  st   $4d,[y,x++] ;045b EQ
              8037 dc61  st   $61,[y,x++]
              8038 dc22  st   $22,[y,x++] ;045d LDW
              8039 dc4a  st   $4a,[y,x++] ;045e 'Height'
              803a dcf3  st   $f3,[y,x++] ;045f SUBI
              803b dc01  st   $01,[y,x++]
              803c dc2c  st   $2c,[y,x++] ;0461 STW
              803d dc4a  st   $4a,[y,x++] ;0462 'Height'
              803e dc22  st   $22,[y,x++] ;0463 LDW
              803f dc4a  st   $4a,[y,x++] ;0464 'Height'
              8040 dcf3  st   $f3,[y,x++] ;0465 SUBI
              8041 dc76  st   $76,[y,x++]
              8042 dc36  st   $36,[y,x++] ;0467 COND
              8043 dc56  st   $56,[y,x++] ;0468 GE
              8044 dc71  st   $71,[y,x++]
              8045 dc22  st   $22,[y,x++] ;046a LDW
              8046 dc46  st   $46,[y,x++] ;046b 'Rnd0'
              8047 dc8a  st   $8a,[y,x++] ;046c ANDI
              8048 dc80  st   $80,[y,x++]
              8049 dc36  st   $36,[y,x++] ;046e COND
              804a dc4d  st   $4d,[y,x++] ;046f EQ
              804b dc71  st   $71,[y,x++]
              804c dcf6  st   $f6,[y,x++] ;0471 INC
              804d dc4a  st   $4a,[y,x++] ;0472 'Height'
              804e dc22  st   $22,[y,x++] ;0473 LDW
              804f dc4a  st   $4a,[y,x++] ;0474 'Height'
              8050 dcf0  st   $f0,[y,x++] ;0475 ADDI
              8051 dc08  st   $08,[y,x++]
              8052 dc61  st   $61,[y,x++] ;0477 ST
              8053 dc4d  st   $4d,[y,x++] ;0478 'J'+1
              8054 dc22  st   $22,[y,x++] ;0479 LDW
              8055 dc42  st   $42,[y,x++] ;047a 'X'
              8056 dcf0  st   $f0,[y,x++] ;047b ADDI
              8057 dca0  st   $a0,[y,x++]
              8058 dc8a  st   $8a,[y,x++] ;047d ANDI
              8059 dcff  st   $ff,[y,x++]
              805a dc2c  st   $2c,[y,x++] ;047f STW
              805b dc32  st   $32,[y,x++] ;0480 'p'
              805c dc12  st   $12,[y,x++] ;0481 LDWI
              805d dc00  st   $00,[y,x++]
              805e dc10  st   $10,[y,x++]
              805f dcc4  st   $c4,[y,x++] ;0484 ADDW
              8060 dc32  st   $32,[y,x++] ;0485 'p'
              8061 dc2c  st   $2c,[y,x++] ;0486 STW
              8062 dc4e  st   $4e,[y,x++] ;0487 'V'
              8063 dc5c  st   $5c,[y,x++] ;0488 LDI
              8064 dc01  st   $01,[y,x++]
              8065 dc9b  st   $9b,[y,x++] ;048a POKE
              8066 dc4e  st   $4e,[y,x++] ;048b 'V'
              8067 dcf6  st   $f6,[y,x++] ;048c INC
              8068 dc4f  st   $4f,[y,x++] ;048d 'V'+1
              8069 dc22  st   $22,[y,x++] ;048e LDW
              806a dc4e  st   $4e,[y,x++] ;048f 'V'
              806b dc36  st   $36,[y,x++] ;0490 COND
              806c dc50  st   $50,[y,x++] ;0491 GT
              806d dc86  st   $86,[y,x++]
              806e dc22  st   $22,[y,x++] ;0493 LDW
              806f dc46  st   $46,[y,x++] ;0494 'Rnd0'
              8070 dc8a  st   $8a,[y,x++] ;0495 ANDI
              8071 dc7f  st   $7f,[y,x++]
              8072 dc2c  st   $2c,[y,x++] ;0497 STW
              8073 dc50  st   $50,[y,x++] ;0498 'A'
              8074 dc22  st   $22,[y,x++] ;0499 LDW
              8075 dc50  st   $50,[y,x++] ;049a 'A'
              8076 dcf3  st   $f3,[y,x++] ;049b SUBI
              8077 dc6f  st   $6f,[y,x++]
              8078 dc36  st   $36,[y,x++] ;049d COND
              8079 dc59  st   $59,[y,x++] ;049e LE
              807a dca2  st   $a2,[y,x++]
              807b dc5c  st   $5c,[y,x++] ;04a0 LDI
              807c dc77  st   $77,[y,x++]
              807d dc2c  st   $2c,[y,x++] ;04a2 STW
              807e dc50  st   $50,[y,x++] ;04a3 'A'
              807f dc22  st   $22,[y,x++] ;04a4 LDW
              8080 dc50  st   $50,[y,x++] ;04a5 'A'
              8081 dcf0  st   $f0,[y,x++] ;04a6 ADDI
              8082 dc10  st   $10,[y,x++]
              8083 dc61  st   $61,[y,x++] ;04a8 ST
              8084 dc51  st   $51,[y,x++] ;04a9 'A'+1
              8085 dc22  st   $22,[y,x++] ;04aa LDW
              8086 dc32  st   $32,[y,x++] ;04ab 'p'
              8087 dc61  st   $61,[y,x++] ;04ac ST
              8088 dc50  st   $50,[y,x++] ;04ad 'A'
              8089 dc5c  st   $5c,[y,x++] ;04ae LDI
              808a dc02  st   $02,[y,x++]
              808b dc9b  st   $9b,[y,x++] ;04b0 POKE
              808c dc50  st   $50,[y,x++] ;04b1 'A'
              808d dc22  st   $22,[y,x++] ;04b2 LDW
              808e dc4c  st   $4c,[y,x++] ;04b3 'J'
              808f dcc4  st   $c4,[y,x++] ;04b4 ADDW
              8090 dc32  st   $32,[y,x++] ;04b5 'p'
              8091 dc2c  st   $2c,[y,x++] ;04b6 STW
              8092 dc4e  st   $4e,[y,x++] ;04b7 'V'
              8093 dc5c  st   $5c,[y,x++] ;04b8 LDI
              8094 dc3f  st   $3f,[y,x++]
              8095 dc9b  st   $9b,[y,x++] ;04ba POKE
              8096 dc4e  st   $4e,[y,x++] ;04bb 'V'
              8097 dcf6  st   $f6,[y,x++] ;04bc INC
              8098 dc4f  st   $4f,[y,x++] ;04bd 'V'+1
              8099 dc22  st   $22,[y,x++] ;04be LDW
              809a dc42  st   $42,[y,x++] ;04bf 'X'
              809b dc8a  st   $8a,[y,x++] ;04c0 ANDI
              809c dc08  st   $08,[y,x++]
              809d dcc4  st   $c4,[y,x++] ;04c2 ADDW
              809e dc4a  st   $4a,[y,x++] ;04c3 'Height'
              809f dc2c  st   $2c,[y,x++] ;04c4 STW
              80a0 dc52  st   $52,[y,x++] ;04c5 'C'
              80a1 dc22  st   $22,[y,x++] ;04c6 LDW
              80a2 dc52  st   $52,[y,x++] ;04c7 'C'
              80a3 dcf0  st   $f0,[y,x++] ;04c8 ADDI
              80a4 dc01  st   $01,[y,x++]
              80a5 dc2c  st   $2c,[y,x++] ;04ca STW
              80a6 dc52  st   $52,[y,x++] ;04cb 'C'
              80a7 dc8a  st   $8a,[y,x++] ;04cc ANDI
              80a8 dc08  st   $08,[y,x++]
              80a9 dc36  st   $36,[y,x++] ;04ce COND
              80aa dc75  st   $75,[y,x++] ;04cf NE
              80ab dcd3  st   $d3,[y,x++]
              80ac dc5c  st   $5c,[y,x++] ;04d1 LDI
              80ad dc2a  st   $2a,[y,x++]
              80ae dc98  st   $98,[y,x++] ;04d3 BRA
              80af dcd5  st   $d5,[y,x++]
              80b0 dc5c  st   $5c,[y,x++] ;04d5 LDI
              80b1 dc20  st   $20,[y,x++]
              80b2 dc9b  st   $9b,[y,x++] ;04d7 POKE
              80b3 dc4e  st   $4e,[y,x++] ;04d8 'V'
              80b4 dcf6  st   $f6,[y,x++] ;04d9 INC
              80b5 dc4f  st   $4f,[y,x++] ;04da 'V'+1
              80b6 dc22  st   $22,[y,x++] ;04db LDW
              80b7 dc4e  st   $4e,[y,x++] ;04dc 'V'
              80b8 dc36  st   $36,[y,x++] ;04dd COND
              80b9 dc50  st   $50,[y,x++] ;04de GT
              80ba dcc4  st   $c4,[y,x++]
              80bb dc1b  st   $1b,[y,x++] ;04e0 LD
              80bc dc08  st   $08,[y,x++]
              80bd dc36  st   $36,[y,x++] ;04e2 COND
              80be dc75  st   $75,[y,x++] ;04e3 NE
              80bf dcde  st   $de,[y,x++]
              80c0 dc22  st   $22,[y,x++] ;04e5 LDW
              80c1 dc42  st   $42,[y,x++] ;04e6 'X'
              80c2 dcf0  st   $f0,[y,x++] ;04e7 ADDI
              80c3 dc01  st   $01,[y,x++]
              80c4 dc9b  st   $9b,[y,x++] ;04e9 POKE
              80c5 dc40  st   $40,[y,x++] ;04ea 'ShiftControl'
              80c6 dc22  st   $22,[y,x++] ;04eb LDW
              80c7 dc54  st   $54,[y,x++] ;04ec 'RunPart2'
              80c8 dce4  st   $e4,[y,x++] ;04ed CALL
              80c9 dc2c  st   $2c,[y,x++] ;04ee STW
              80ca dc56  st   $56,[y,x++] ;04ef 'RunPart1'
              80cb dcf6  st   $f6,[y,x++] ;04f0 INC
              80cc dc1d  st   $1d,[y,x++]
              80cd dcff  st   $ff,[y,x++] ;04f2 RET
              80ce 1000  ld   $00,x
              80cf 1405  ld   $05,y
              80d0 dcfc  st   $fc,[y,x++] ;0500 DEF
              80d1 dc4c  st   $4c,[y,x++]
              80d2 dc22  st   $22,[y,x++] ;0502 LDW
              80d3 dc58  st   $58,[y,x++] ;0503 'OldPixel'
              80d4 dc9b  st   $9b,[y,x++] ;0504 POKE
              80d5 dc5a  st   $5a,[y,x++] ;0505 'Ball'
              80d6 dc22  st   $22,[y,x++] ;0506 LDW
              80d7 dc44  st   $44,[y,x++] ;0507 'BallA'
              80d8 dcc4  st   $c4,[y,x++] ;0508 ADDW
              80d9 dc5c  st   $5c,[y,x++] ;0509 'BallV'
              80da dc2c  st   $2c,[y,x++] ;050a STW
              80db dc5c  st   $5c,[y,x++] ;050b 'BallV'
              80dc dc22  st   $22,[y,x++] ;050c LDW
              80dd dc5e  st   $5e,[y,x++] ;050d 'BallY'
              80de dcc4  st   $c4,[y,x++] ;050e ADDW
              80df dc5c  st   $5c,[y,x++] ;050f 'BallV'
              80e0 dc2c  st   $2c,[y,x++] ;0510 STW
              80e1 dc5e  st   $5e,[y,x++] ;0511 'BallY'
              80e2 dc22  st   $22,[y,x++] ;0512 LDW
              80e3 dc42  st   $42,[y,x++] ;0513 'X'
              80e4 dcf0  st   $f0,[y,x++] ;0514 ADDI
              80e5 dc32  st   $32,[y,x++]
              80e6 dc2c  st   $2c,[y,x++] ;0516 STW
              80e7 dc5a  st   $5a,[y,x++] ;0517 'Ball'
              80e8 dc1b  st   $1b,[y,x++] ;0518 LD
              80e9 dc5f  st   $5f,[y,x++] ;0519 'BallY'+1
              80ea dc61  st   $61,[y,x++] ;051a ST
              80eb dc5b  st   $5b,[y,x++] ;051b 'Ball'+1
              80ec dc22  st   $22,[y,x++] ;051c LDW
              80ed dc5a  st   $5a,[y,x++] ;051d 'Ball'
              80ee dcd8  st   $d8,[y,x++] ;051e PEEK
              80ef dc2c  st   $2c,[y,x++] ;051f STW
              80f0 dc58  st   $58,[y,x++] ;0520 'OldPixel'
              80f1 dc22  st   $22,[y,x++] ;0521 LDW
              80f2 dc58  st   $58,[y,x++] ;0522 'OldPixel'
              80f3 dcf3  st   $f3,[y,x++] ;0523 SUBI
              80f4 dc02  st   $02,[y,x++]
              80f5 dc36  st   $36,[y,x++] ;0525 COND
              80f6 dc59  st   $59,[y,x++] ;0526 LE
              80f7 dc45  st   $45,[y,x++]
              80f8 dc5c  st   $5c,[y,x++] ;0528 LDI
              80f9 dc00  st   $00,[y,x++]
              80fa dcb0  st   $b0,[y,x++] ;052a SUBW
              80fb dc5c  st   $5c,[y,x++] ;052b 'BallV'
              80fc dc2c  st   $2c,[y,x++] ;052c STW
              80fd dc5c  st   $5c,[y,x++] ;052d 'BallV'
              80fe dc22  st   $22,[y,x++] ;052e LDW
              80ff dc5e  st   $5e,[y,x++] ;052f 'BallY'
              8100 dcc4  st   $c4,[y,x++] ;0530 ADDW
              8101 dc5c  st   $5c,[y,x++] ;0531 'BallV'
              8102 dc2c  st   $2c,[y,x++] ;0532 STW
              8103 dc5e  st   $5e,[y,x++] ;0533 'BallY'
              8104 dc5c  st   $5c,[y,x++] ;0534 LDI
              8105 dc0a  st   $0a,[y,x++]
              8106 dc61  st   $61,[y,x++] ;0536 ST
              8107 dc12  st   $12,[y,x++]
              8108 dc22  st   $22,[y,x++] ;0538 LDW
              8109 dc42  st   $42,[y,x++] ;0539 'X'
              810a dcf0  st   $f0,[y,x++] ;053a ADDI
              810b dc32  st   $32,[y,x++]
              810c dc2c  st   $2c,[y,x++] ;053c STW
              810d dc5a  st   $5a,[y,x++] ;053d 'Ball'
              810e dc1b  st   $1b,[y,x++] ;053e LD
              810f dc5f  st   $5f,[y,x++] ;053f 'BallY'+1
              8110 dc61  st   $61,[y,x++] ;0540 ST
              8111 dc5b  st   $5b,[y,x++] ;0541 'Ball'+1
              8112 dc22  st   $22,[y,x++] ;0542 LDW
              8113 dc5a  st   $5a,[y,x++] ;0543 'Ball'
              8114 dcd8  st   $d8,[y,x++] ;0544 PEEK
              8115 dc2c  st   $2c,[y,x++] ;0545 STW
              8116 dc58  st   $58,[y,x++] ;0546 'OldPixel'
              8117 dc5c  st   $5c,[y,x++] ;0547 LDI
              8118 dc3f  st   $3f,[y,x++]
              8119 dc9b  st   $9b,[y,x++] ;0549 POKE
              811a dc5a  st   $5a,[y,x++] ;054a 'Ball'
              811b dc22  st   $22,[y,x++] ;054b LDW
              811c dc56  st   $56,[y,x++] ;054c 'RunPart1'
              811d dce4  st   $e4,[y,x++] ;054d CALL
              811e dc2c  st   $2c,[y,x++] ;054e STW
              811f dc54  st   $54,[y,x++] ;054f 'RunPart2'
              8120 dcfc  st   $fc,[y,x++] ;0550 DEF
              8121 dc86  st   $86,[y,x++]
              8122 dc12  st   $12,[y,x++] ;0552 LDWI
              8123 dc00  st   $00,[y,x++]
              8124 dc07  st   $07,[y,x++]
              8125 dc2c  st   $2c,[y,x++] ;0555 STW
              8126 dc32  st   $32,[y,x++] ;0556 'p'
              8127 dc22  st   $22,[y,x++] ;0557 LDW
              8128 dc2c  st   $2c,[y,x++] ;0558 'Pos'
              8129 dc2c  st   $2c,[y,x++] ;0559 STW
              812a dc60  st   $60,[y,x++] ;055a 'q'
              812b dc22  st   $22,[y,x++] ;055b LDW
              812c dc32  st   $32,[y,x++] ;055c 'p'
              812d dc78  st   $78,[y,x++] ;055d LOOKUP
              812e dc00  st   $00,[y,x++]
              812f dc9b  st   $9b,[y,x++] ;055f POKE
              8130 dc60  st   $60,[y,x++] ;0560 'q'
              8131 dc12  st   $12,[y,x++] ;0561 LDWI
              8132 dc00  st   $00,[y,x++]
              8133 dc01  st   $01,[y,x++]
              8134 dcc4  st   $c4,[y,x++] ;0564 ADDW
              8135 dc32  st   $32,[y,x++] ;0565 'p'
              8136 dc2c  st   $2c,[y,x++] ;0566 STW
              8137 dc32  st   $32,[y,x++] ;0567 'p'
              8138 dc12  st   $12,[y,x++] ;0568 LDWI
              8139 dc00  st   $00,[y,x++]
              813a dc01  st   $01,[y,x++]
              813b dcc4  st   $c4,[y,x++] ;056b ADDW
              813c dc60  st   $60,[y,x++] ;056c 'q'
              813d dc2c  st   $2c,[y,x++] ;056d STW
              813e dc60  st   $60,[y,x++] ;056e 'q'
              813f dc36  st   $36,[y,x++] ;056f COND
              8140 dc50  st   $50,[y,x++] ;0570 GT
              8141 dc59  st   $59,[y,x++]
              8142 dc12  st   $12,[y,x++] ;0572 LDWI
              8143 dc01  st   $01,[y,x++]
              8144 dc88  st   $88,[y,x++]
              8145 dcc4  st   $c4,[y,x++] ;0575 ADDW
              8146 dc32  st   $32,[y,x++] ;0576 'p'
              8147 dc2c  st   $2c,[y,x++] ;0577 STW
              8148 dc32  st   $32,[y,x++] ;0578 'p'
              8149 dc12  st   $12,[y,x++] ;0579 LDWI
              814a dc01  st   $01,[y,x++]
              814b dc88  st   $88,[y,x++]
              814c dcc4  st   $c4,[y,x++] ;057c ADDW
              814d dc60  st   $60,[y,x++] ;057d 'q'
              814e dc2c  st   $2c,[y,x++] ;057e STW
              814f dc60  st   $60,[y,x++] ;057f 'q'
              8150 dc8a  st   $8a,[y,x++] ;0580 ANDI
              8151 dcff  st   $ff,[y,x++]
              8152 dc94  st   $94,[y,x++] ;0582 XORI
              8153 dca0  st   $a0,[y,x++]
              8154 dc36  st   $36,[y,x++] ;0584 COND
              8155 dc75  st   $75,[y,x++] ;0585 NE
              8156 dc59  st   $59,[y,x++]
              8157 dcff  st   $ff,[y,x++] ;0587 RET
              8158 dc2c  st   $2c,[y,x++] ;0588 STW
              8159 dc62  st   $62,[y,x++] ;0589 'DrawJupiter'
              815a dc12  st   $12,[y,x++] ;058a LDWI
              815b dc00  st   $00,[y,x++]
              815c dc08  st   $08,[y,x++]
              815d dc2c  st   $2c,[y,x++] ;058d STW
              815e dc2c  st   $2c,[y,x++] ;058e 'Pos'
              815f dc22  st   $22,[y,x++] ;058f LDW
              8160 dc62  st   $62,[y,x++] ;0590 'DrawJupiter'
              8161 dce4  st   $e4,[y,x++] ;0591 CALL
              8162 dc5c  st   $5c,[y,x++] ;0592 LDI
              8163 dc78  st   $78,[y,x++]
              8164 dc61  st   $61,[y,x++] ;0594 ST
              8165 dc12  st   $12,[y,x++]
              8166 dc5c  st   $5c,[y,x++] ;0596 LDI
              8167 dc08  st   $08,[y,x++]
              8168 dc2c  st   $2c,[y,x++] ;0598 STW
              8169 dc34  st   $34,[y,x++] ;0599 'Color'
              816a dc12  st   $12,[y,x++] ;059a LDWI
              816b dc00  st   $00,[y,x++]
              816c dc08  st   $08,[y,x++]
              816d dc2c  st   $2c,[y,x++] ;059d STW
              816e dc2c  st   $2c,[y,x++] ;059e 'Pos'
              816f dc22  st   $22,[y,x++] ;059f LDW
              8170 dc3c  st   $3c,[y,x++] ;05a0 'Welcome'
              8171 dc2c  st   $2c,[y,x++] ;05a1 STW
              8172 dc38  st   $38,[y,x++] ;05a2 'Text'
              8173 dc22  st   $22,[y,x++] ;05a3 LDW
              8174 dc3a  st   $3a,[y,x++] ;05a4 'DrawText'
              8175 dce4  st   $e4,[y,x++] ;05a5 CALL
              8176 dc1b  st   $1b,[y,x++] ;05a6 LD
              8177 dc12  st   $12,[y,x++]
              8178 dc36  st   $36,[y,x++] ;05a8 COND
              8179 dc75  st   $75,[y,x++] ;05a9 NE
              817a dca4  st   $a4,[y,x++]
              817b dc5c  st   $5c,[y,x++] ;05ab LDI
              817c dc3e  st   $3e,[y,x++]
              817d dc2c  st   $2c,[y,x++] ;05ad STW
              817e dc34  st   $34,[y,x++] ;05ae 'Color'
              817f dc12  st   $12,[y,x++] ;05af LDWI
              8180 dc00  st   $00,[y,x++]
              8181 dc08  st   $08,[y,x++]
              8182 dc2c  st   $2c,[y,x++] ;05b2 STW
              8183 dc2c  st   $2c,[y,x++] ;05b3 'Pos'
              8184 dc22  st   $22,[y,x++] ;05b4 LDW
              8185 dc3c  st   $3c,[y,x++] ;05b5 'Welcome'
              8186 dc2c  st   $2c,[y,x++] ;05b6 STW
              8187 dc38  st   $38,[y,x++] ;05b7 'Text'
              8188 dc22  st   $22,[y,x++] ;05b8 LDW
              8189 dc3a  st   $3a,[y,x++] ;05b9 'DrawText'
              818a dce4  st   $e4,[y,x++] ;05ba CALL
              818b dc22  st   $22,[y,x++] ;05bb LDW
              818c dc34  st   $34,[y,x++] ;05bc 'Color'
              818d dc94  st   $94,[y,x++] ;05bd XORI
              818e dc08  st   $08,[y,x++]
              818f dc36  st   $36,[y,x++] ;05bf COND
              8190 dc4d  st   $4d,[y,x++] ;05c0 EQ
              8191 dcc8  st   $c8,[y,x++]
              8192 dc22  st   $22,[y,x++] ;05c2 LDW
              8193 dc34  st   $34,[y,x++] ;05c3 'Color'
              8194 dcf3  st   $f3,[y,x++] ;05c4 SUBI
              8195 dc09  st   $09,[y,x++]
              8196 dc2c  st   $2c,[y,x++] ;05c6 STW
              8197 dc34  st   $34,[y,x++] ;05c7 'Color'
              8198 dc98  st   $98,[y,x++] ;05c8 BRA
              8199 dcad  st   $ad,[y,x++]
              819a dc1b  st   $1b,[y,x++] ;05ca LD
              819b dc0d  st   $0d,[y,x++]
              819c dcf0  st   $f0,[y,x++] ;05cc ADDI
              819d dc96  st   $96,[y,x++]
              819e dc8a  st   $8a,[y,x++] ;05ce ANDI
              819f dcff  st   $ff,[y,x++]
              81a0 dc2c  st   $2c,[y,x++] ;05d0 STW
              81a1 dc2a  st   $2a,[y,x++] ;05d1 'tmp'
              81a2 dc1b  st   $1b,[y,x++] ;05d2 LD
              81a3 dc0d  st   $0d,[y,x++]
              81a4 dcb0  st   $b0,[y,x++] ;05d4 SUBW
              81a5 dc2a  st   $2a,[y,x++] ;05d5 'tmp'
              81a6 dc36  st   $36,[y,x++] ;05d6 COND
              81a7 dc75  st   $75,[y,x++] ;05d7 NE
              81a8 dcd0  st   $d0,[y,x++]
              81a9 dc22  st   $22,[y,x++] ;05d9 LDW
              81aa dc3e  st   $3e,[y,x++] ;05da 'Clear'
              81ab dce4  st   $e4,[y,x++] ;05db CALL
              81ac dc12  st   $12,[y,x++] ;05dc LDWI
              81ad dc11  st   $11,[y,x++]
              81ae dc01  st   $01,[y,x++]
              81af dc2c  st   $2c,[y,x++] ;05df STW
              81b0 dc40  st   $40,[y,x++] ;05e0 'ShiftControl'
              81b1 dc5c  st   $5c,[y,x++] ;05e1 LDI
              81b2 dc00  st   $00,[y,x++]
              81b3 dc2c  st   $2c,[y,x++] ;05e3 STW
              81b4 dc48  st   $48,[y,x++] ;05e4 'Rnd1'
              81b5 dc2c  st   $2c,[y,x++] ;05e5 STW
              81b6 dc46  st   $46,[y,x++] ;05e6 'Rnd0'
              81b7 dc2c  st   $2c,[y,x++] ;05e7 STW
              81b8 dc4c  st   $4c,[y,x++] ;05e8 'J'
              81b9 dc2c  st   $2c,[y,x++] ;05e9 STW
              81ba dc44  st   $44,[y,x++] ;05ea 'BallA'
              81bb dc2c  st   $2c,[y,x++] ;05eb STW
              81bc dc5c  st   $5c,[y,x++] ;05ec 'BallV'
              81bd dc2c  st   $2c,[y,x++] ;05ed STW
              81be dc58  st   $58,[y,x++] ;05ee 'OldPixel'
              81bf dc12  st   $12,[y,x++] ;05ef LDWI
              81c0 dc00  st   $00,[y,x++]
              81c1 dc18  st   $18,[y,x++]
              81c2 dc2c  st   $2c,[y,x++] ;05f2 STW
              81c3 dc5e  st   $5e,[y,x++] ;05f3 'BallY'
              81c4 dc2c  st   $2c,[y,x++] ;05f4 STW
              81c5 dc5a  st   $5a,[y,x++] ;05f5 'Ball'
              81c6 dc5c  st   $5c,[y,x++] ;05f6 LDI
              81c7 dc5a  st   $5a,[y,x++]
              81c8 dc2c  st   $2c,[y,x++] ;05f8 STW
              81c9 dc4a  st   $4a,[y,x++] ;05f9 'Height'
              81ca dc22  st   $22,[y,x++] ;05fa LDW
              81cb dc56  st   $56,[y,x++] ;05fb 'RunPart1'
              81cc dce4  st   $e4,[y,x++] ;05fc CALL
              81cd 00fe  ld   $fe
              81ce c218  st   [$18]
              81cf 0003  ld   $03
              81d0 c219  st   [$19]
              81d1 0000  ld   $00
              81d2 c21e  st   [$1e]
              81d3 c21f  st   [$1f]
              81d4 c21c  st   [$1c]
              81d5 0004  ld   $04
              81d6 c21d  st   [$1d]
              81d7 150f  ld   [$0f],y
              81d8 e10e  jmp  y,[$0e]
              81d9 0200  nop
              81da
