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
              0079 1407  ld   $07,y
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
initVcpu:     0700 1000  ld   $00,x
              0701 1403  ld   $03,y
              0702 dcfc  st   $fc,[y,x++] ;0300 DEF
              0703 dc6d  st   $6d,[y,x++]
              0704 dc22  st   $22,[y,x++] ;0302 LDW
              0705 dc24  st   $24,[y,x++] ;0303 'Char'
              0706 dcf3  st   $f3,[y,x++] ;0304 SUBI
              0707 dc52  st   $52,[y,x++]
              0708 dc36  st   $36,[y,x++] ;0306 COND
              0709 dc56  st   $56,[y,x++] ;0307 GE
              070a dc10  st   $10,[y,x++]
              070b dcf0  st   $f0,[y,x++] ;0309 ADDI
              070c dc32  st   $32,[y,x++]
              070d dc2c  st   $2c,[y,x++] ;030b STW
              070e dc26  st   $26,[y,x++] ;030c 'i'
              070f dc12  st   $12,[y,x++] ;030d LDWI
              0710 dc00  st   $00,[y,x++]
              0711 dc05  st   $05,[y,x++]
              0712 dc98  st   $98,[y,x++] ;0310 BRA
              0713 dc15  st   $15,[y,x++]
              0714 dc2c  st   $2c,[y,x++] ;0312 STW
              0715 dc26  st   $26,[y,x++] ;0313 'i'
              0716 dc12  st   $12,[y,x++] ;0314 LDWI
              0717 dc00  st   $00,[y,x++]
              0718 dc06  st   $06,[y,x++]
              0719 dc2c  st   $2c,[y,x++] ;0317 STW
              071a dc28  st   $28,[y,x++] ;0318 'fontData'
              071b dc22  st   $22,[y,x++] ;0319 LDW
              071c dc26  st   $26,[y,x++] ;031a 'i'
              071d dcc4  st   $c4,[y,x++] ;031b ADDW
              071e dc26  st   $26,[y,x++] ;031c 'i'
              071f dc2c  st   $2c,[y,x++] ;031d STW
              0720 dc2a  st   $2a,[y,x++] ;031e 'tmp'
              0721 dcc4  st   $c4,[y,x++] ;031f ADDW
              0722 dc2a  st   $2a,[y,x++] ;0320 'tmp'
              0723 dcc4  st   $c4,[y,x++] ;0321 ADDW
              0724 dc26  st   $26,[y,x++] ;0322 'i'
              0725 dcc4  st   $c4,[y,x++] ;0323 ADDW
              0726 dc28  st   $28,[y,x++] ;0324 'fontData'
              0727 dc2c  st   $2c,[y,x++] ;0325 STW
              0728 dc28  st   $28,[y,x++] ;0326 'fontData'
              0729 dc12  st   $12,[y,x++] ;0327 LDWI
              072a dc00  st   $00,[y,x++]
              072b dc08  st   $08,[y,x++]
              072c dcc4  st   $c4,[y,x++] ;032a ADDW
              072d dc2c  st   $2c,[y,x++] ;032b 'Pos'
              072e dc2c  st   $2c,[y,x++] ;032c STW
              072f dc2e  st   $2e,[y,x++] ;032d 'U'
              0730 dc5c  st   $5c,[y,x++] ;032e LDI
              0731 dc05  st   $05,[y,x++]
              0732 dc2c  st   $2c,[y,x++] ;0330 STW
              0733 dc26  st   $26,[y,x++] ;0331 'i'
              0734 dc36  st   $36,[y,x++] ;0332 COND
              0735 dc4d  st   $4d,[y,x++] ;0333 EQ
              0736 dc3b  st   $3b,[y,x++]
              0737 dc22  st   $22,[y,x++] ;0335 LDW
              0738 dc28  st   $28,[y,x++] ;0336 'fontData'
              0739 dc78  st   $78,[y,x++] ;0337 LOOKUP
              073a dc00  st   $00,[y,x++]
              073b dcf6  st   $f6,[y,x++] ;0339 INC
              073c dc28  st   $28,[y,x++] ;033a 'fontData'
              073d dc98  st   $98,[y,x++] ;033b BRA
              073e dc3d  st   $3d,[y,x++]
              073f dc5c  st   $5c,[y,x++] ;033d LDI
              0740 dc00  st   $00,[y,x++]
              0741 dc2c  st   $2c,[y,x++] ;033f STW
              0742 dc30  st   $30,[y,x++] ;0340 'bits'
              0743 dc22  st   $22,[y,x++] ;0341 LDW
              0744 dc2c  st   $2c,[y,x++] ;0342 'Pos'
              0745 dc2c  st   $2c,[y,x++] ;0343 STW
              0746 dc32  st   $32,[y,x++] ;0344 'p'
              0747 dc22  st   $22,[y,x++] ;0345 LDW
              0748 dc30  st   $30,[y,x++] ;0346 'bits'
              0749 dc8a  st   $8a,[y,x++] ;0347 ANDI
              074a dc80  st   $80,[y,x++]
              074b dc36  st   $36,[y,x++] ;0349 COND
              074c dc75  st   $75,[y,x++] ;034a NE
              074d dc4e  st   $4e,[y,x++]
              074e dc5c  st   $5c,[y,x++] ;034c LDI
              074f dc00  st   $00,[y,x++]
              0750 dc98  st   $98,[y,x++] ;034e BRA
              0751 dc50  st   $50,[y,x++]
              0752 dc22  st   $22,[y,x++] ;0350 LDW
              0753 dc34  st   $34,[y,x++] ;0351 'Color'
              0754 dc9b  st   $9b,[y,x++] ;0352 POKE
              0755 dc32  st   $32,[y,x++] ;0353 'p'
              0756 dc22  st   $22,[y,x++] ;0354 LDW
              0757 dc30  st   $30,[y,x++] ;0355 'bits'
              0758 dcc4  st   $c4,[y,x++] ;0356 ADDW
              0759 dc30  st   $30,[y,x++] ;0357 'bits'
              075a dc2c  st   $2c,[y,x++] ;0358 STW
              075b dc30  st   $30,[y,x++] ;0359 'bits'
              075c dcf6  st   $f6,[y,x++] ;035a INC
              075d dc33  st   $33,[y,x++] ;035b 'p'+1
              075e dc22  st   $22,[y,x++] ;035c LDW
              075f dc32  st   $32,[y,x++] ;035d 'p'
              0760 dcb0  st   $b0,[y,x++] ;035e SUBW
              0761 dc2e  st   $2e,[y,x++] ;035f 'U'
              0762 dc36  st   $36,[y,x++] ;0360 COND
              0763 dc53  st   $53,[y,x++] ;0361 LT
              0764 dc43  st   $43,[y,x++]
              0765 dcf6  st   $f6,[y,x++] ;0363 INC
              0766 dc2c  st   $2c,[y,x++] ;0364 'Pos'
              0767 dc22  st   $22,[y,x++] ;0365 LDW
              0768 dc26  st   $26,[y,x++] ;0366 'i'
              0769 dcf3  st   $f3,[y,x++] ;0367 SUBI
              076a dc01  st   $01,[y,x++]
              076b dc2c  st   $2c,[y,x++] ;0369 STW
              076c dc26  st   $26,[y,x++] ;036a 'i'
              076d dc36  st   $36,[y,x++] ;036b COND
              076e dc56  st   $56,[y,x++] ;036c GE
              076f dc30  st   $30,[y,x++]
              0770 dcff  st   $ff,[y,x++] ;036e RET
              0771 dc2c  st   $2c,[y,x++] ;036f STW
              0772 dc36  st   $36,[y,x++] ;0370 'DrawChar'
              0773 dcfc  st   $fc,[y,x++] ;0371 DEF
              0774 dc95  st   $95,[y,x++]
              0775 dc80  st   $80,[y,x++] ;0373 PUSH
              0776 dc22  st   $22,[y,x++] ;0374 LDW
              0777 dc38  st   $38,[y,x++] ;0375 'Text'
              0778 dcd8  st   $d8,[y,x++] ;0376 PEEK
              0779 dc2c  st   $2c,[y,x++] ;0377 STW
              077a dc24  st   $24,[y,x++] ;0378 'Char'
              077b dc36  st   $36,[y,x++] ;0379 COND
              077c dc4d  st   $4d,[y,x++] ;037a EQ
              077d dc93  st   $93,[y,x++]
              077e dcf6  st   $f6,[y,x++] ;037c INC
              077f dc38  st   $38,[y,x++] ;037d 'Text'
              0780 dc22  st   $22,[y,x++] ;037e LDW
              0781 dc24  st   $24,[y,x++] ;037f 'Char'
              0782 dc94  st   $94,[y,x++] ;0380 XORI
              0783 dc0a  st   $0a,[y,x++]
              0784 dc36  st   $36,[y,x++] ;0382 COND
              0785 dc75  st   $75,[y,x++] ;0383 NE
              0786 dc8e  st   $8e,[y,x++]
              0787 dc61  st   $61,[y,x++] ;0385 ST
              0788 dc2c  st   $2c,[y,x++] ;0386 'Pos'
              0789 dc12  st   $12,[y,x++] ;0387 LDWI
              078a dc00  st   $00,[y,x++]
              078b dc08  st   $08,[y,x++]
              078c dcc4  st   $c4,[y,x++] ;038a ADDW
              078d dc2c  st   $2c,[y,x++] ;038b 'Pos'
              078e dc2c  st   $2c,[y,x++] ;038c STW
              078f dc2c  st   $2c,[y,x++] ;038d 'Pos'
              0790 dc98  st   $98,[y,x++] ;038e BRA
              0791 dc91  st   $91,[y,x++]
              0792 dc22  st   $22,[y,x++] ;0390 LDW
              0793 dc36  st   $36,[y,x++] ;0391 'DrawChar'
              0794 dce4  st   $e4,[y,x++] ;0392 CALL
              0795 dc98  st   $98,[y,x++] ;0393 BRA
              0796 dc72  st   $72,[y,x++]
              0797 dc66  st   $66,[y,x++] ;0395 POP
              0798 dce4  st   $e4,[y,x++] ;0396 CALL
              0799 dc2c  st   $2c,[y,x++] ;0397 STW
              079a dc3a  st   $3a,[y,x++] ;0398 'DrawText'
              079b dcfc  st   $fc,[y,x++] ;0399 DEF
              079c dcd2  st   $d2,[y,x++]
              079d dc20  st   $20,[y,x++]
              079e dc20  st   $20,[y,x++]
              079f dc20  st   $20,[y,x++]
              07a0 dc2a  st   $2a,[y,x++]
              07a1 dc2a  st   $2a,[y,x++]
              07a2 dc2a  st   $2a,[y,x++]
              07a3 dc20  st   $20,[y,x++]
              07a4 dc47  st   $47,[y,x++]
              07a5 dc69  st   $69,[y,x++]
              07a6 dc67  st   $67,[y,x++]
              07a7 dc61  st   $61,[y,x++]
              07a8 dc74  st   $74,[y,x++]
              07a9 dc72  st   $72,[y,x++]
              07aa dc6f  st   $6f,[y,x++]
              07ab dc6e  st   $6e,[y,x++]
              07ac dc20  st   $20,[y,x++]
              07ad dc33  st   $33,[y,x++]
              07ae dc32  st   $32,[y,x++]
              07af dc4b  st   $4b,[y,x++]
              07b0 dc20  st   $20,[y,x++]
              07b1 dc2a  st   $2a,[y,x++]
              07b2 dc2a  st   $2a,[y,x++]
              07b3 dc2a  st   $2a,[y,x++]
              07b4 dc20  st   $20,[y,x++]
              07b5 dc20  st   $20,[y,x++]
              07b6 dc20  st   $20,[y,x++]
              07b7 dc20  st   $20,[y,x++]
              07b8 dc0a  st   $0a,[y,x++]
              07b9 dc54  st   $54,[y,x++]
              07ba dc54  st   $54,[y,x++]
              07bb dc4c  st   $4c,[y,x++]
              07bc dc20  st   $20,[y,x++]
              07bd dc63  st   $63,[y,x++]
              07be dc6f  st   $6f,[y,x++]
              07bf dc6c  st   $6c,[y,x++]
              07c0 dc6f  st   $6f,[y,x++]
              07c1 dc72  st   $72,[y,x++]
              07c2 dc20  st   $20,[y,x++]
              07c3 dc63  st   $63,[y,x++]
              07c4 dc6f  st   $6f,[y,x++]
              07c5 dc6d  st   $6d,[y,x++]
              07c6 dc70  st   $70,[y,x++]
              07c7 dc75  st   $75,[y,x++]
              07c8 dc74  st   $74,[y,x++]
              07c9 dc65  st   $65,[y,x++]
              07ca dc72  st   $72,[y,x++]
              07cb dc20  st   $20,[y,x++]
              07cc dc52  st   $52,[y,x++]
              07cd dc4f  st   $4f,[y,x++]
              07ce dc4d  st   $4d,[y,x++]
              07cf dc20  st   $20,[y,x++]
              07d0 dc76  st   $76,[y,x++]
              07d1 dc30  st   $30,[y,x++]
              07d2 dc20  st   $20,[y,x++]
              07d3 dc20  st   $20,[y,x++]
              07d4 dc0a  st   $0a,[y,x++]
              07d5 dc00  st   $00,[y,x++]
              07d6 dc2c  st   $2c,[y,x++] ;03d4 STW
              07d7 dc3c  st   $3c,[y,x++] ;03d5 'Welcome'
              07d8 dcff  st   $ff,[y,x++] ;03d6 RET
              07d9 1000  ld   $00,x
              07da 1404  ld   $04,y
              07db dcfc  st   $fc,[y,x++] ;0400 DEF
              07dc dc26  st   $26,[y,x++]
              07dd dc22  st   $22,[y,x++] ;0402 LDW
              07de dc2c  st   $2c,[y,x++] ;0403 'Pos'
              07df dc2c  st   $2c,[y,x++] ;0404 STW
              07e0 dc32  st   $32,[y,x++] ;0405 'p'
              07e1 dc22  st   $22,[y,x++] ;0406 LDW
              07e2 dc32  st   $32,[y,x++] ;0407 'p'
              07e3 dc2c  st   $2c,[y,x++] ;0408 STW
              07e4 dc22  st   $22,[y,x++]
              07e5 dc12  st   $12,[y,x++] ;040a LDWI
              07e6 dc31  st   $31,[y,x++]
              07e7 dc04  st   $04,[y,x++]
              07e8 dcac  st   $ac,[y,x++] ;040d SYS
              07e9 dcfb  st   $fb,[y,x++]
              07ea dc12  st   $12,[y,x++] ;040f LDWI
              07eb dc00  st   $00,[y,x++]
              07ec dc08  st   $08,[y,x++]
              07ed dcc4  st   $c4,[y,x++] ;0412 ADDW
              07ee dc32  st   $32,[y,x++] ;0413 'p'
              07ef dc2c  st   $2c,[y,x++] ;0414 STW
              07f0 dc32  st   $32,[y,x++] ;0415 'p'
              07f1 dc36  st   $36,[y,x++] ;0416 COND
              07f2 dc50  st   $50,[y,x++] ;0417 GT
              07f3 dc04  st   $04,[y,x++]
              07f4 dc12  st   $12,[y,x++] ;0419 LDWI
              07f5 dc01  st   $01,[y,x++]
              07f6 dc98  st   $98,[y,x++]
              07f7 dcc4  st   $c4,[y,x++] ;041c ADDW
              07f8 dc32  st   $32,[y,x++] ;041d 'p'
              07f9 dc2c  st   $2c,[y,x++] ;041e STW
              07fa dc32  st   $32,[y,x++] ;041f 'p'
              07fb dc8a  st   $8a,[y,x++] ;0420 ANDI
              07fc dcff  st   $ff,[y,x++]
              07fd dc94  st   $94,[y,x++] ;0422 XORI
              07fe dca0  st   $a0,[y,x++]
              07ff dc36  st   $36,[y,x++] ;0424 COND
              0800 dc75  st   $75,[y,x++] ;0425 NE
              0801 dc04  st   $04,[y,x++]
              0802 dcff  st   $ff,[y,x++] ;0427 RET
              0803 dc2c  st   $2c,[y,x++] ;0428 STW
              0804 dc3e  st   $3e,[y,x++] ;0429 'Clear'
              0805 dcfc  st   $fc,[y,x++] ;042a DEF
              0806 dcec  st   $ec,[y,x++]
              0807 dc22  st   $22,[y,x++] ;042c LDW
              0808 dc40  st   $40,[y,x++] ;042d 'ShiftControl'
              0809 dcd8  st   $d8,[y,x++] ;042e PEEK
              080a dc2c  st   $2c,[y,x++] ;042f STW
              080b dc42  st   $42,[y,x++] ;0430 'X'
              080c dc94  st   $94,[y,x++] ;0431 XORI
              080d dc78  st   $78,[y,x++]
              080e dc36  st   $36,[y,x++] ;0433 COND
              080f dc75  st   $75,[y,x++] ;0434 NE
              0810 dc38  st   $38,[y,x++]
              0811 dc5c  st   $5c,[y,x++] ;0436 LDI
              0812 dc0a  st   $0a,[y,x++]
              0813 dc2c  st   $2c,[y,x++] ;0438 STW
              0814 dc44  st   $44,[y,x++] ;0439 'BallA'
              0815 dc1b  st   $1b,[y,x++] ;043a LD
              0816 dc0d  st   $0d,[y,x++]
              0817 dcc4  st   $c4,[y,x++] ;043c ADDW
              0818 dc46  st   $46,[y,x++] ;043d 'Rnd0'
              0819 dcc4  st   $c4,[y,x++] ;043e ADDW
              081a dc48  st   $48,[y,x++] ;043f 'Rnd1'
              081b dc2c  st   $2c,[y,x++] ;0440 STW
              081c dc46  st   $46,[y,x++] ;0441 'Rnd0'
              081d dcc4  st   $c4,[y,x++] ;0442 ADDW
              081e dc48  st   $48,[y,x++] ;0443 'Rnd1'
              081f dc36  st   $36,[y,x++] ;0444 COND
              0820 dc56  st   $56,[y,x++] ;0445 GE
              0821 dc47  st   $47,[y,x++]
              0822 dc94  st   $94,[y,x++] ;0447 XORI
              0823 dc2b  st   $2b,[y,x++]
              0824 dcc4  st   $c4,[y,x++] ;0449 ADDW
              0825 dc48  st   $48,[y,x++] ;044a 'Rnd1'
              0826 dcc4  st   $c4,[y,x++] ;044b ADDW
              0827 dc48  st   $48,[y,x++] ;044c 'Rnd1'
              0828 dc2c  st   $2c,[y,x++] ;044d STW
              0829 dc48  st   $48,[y,x++] ;044e 'Rnd1'
              082a dc22  st   $22,[y,x++] ;044f LDW
              082b dc4a  st   $4a,[y,x++] ;0450 'Height'
              082c dcf3  st   $f3,[y,x++] ;0451 SUBI
              082d dc58  st   $58,[y,x++]
              082e dc36  st   $36,[y,x++] ;0453 COND
              082f dc59  st   $59,[y,x++] ;0454 LE
              0830 dc61  st   $61,[y,x++]
              0831 dc22  st   $22,[y,x++] ;0456 LDW
              0832 dc46  st   $46,[y,x++] ;0457 'Rnd0'
              0833 dc8a  st   $8a,[y,x++] ;0458 ANDI
              0834 dc10  st   $10,[y,x++]
              0835 dc36  st   $36,[y,x++] ;045a COND
              0836 dc4d  st   $4d,[y,x++] ;045b EQ
              0837 dc61  st   $61,[y,x++]
              0838 dc22  st   $22,[y,x++] ;045d LDW
              0839 dc4a  st   $4a,[y,x++] ;045e 'Height'
              083a dcf3  st   $f3,[y,x++] ;045f SUBI
              083b dc01  st   $01,[y,x++]
              083c dc2c  st   $2c,[y,x++] ;0461 STW
              083d dc4a  st   $4a,[y,x++] ;0462 'Height'
              083e dc22  st   $22,[y,x++] ;0463 LDW
              083f dc4a  st   $4a,[y,x++] ;0464 'Height'
              0840 dcf3  st   $f3,[y,x++] ;0465 SUBI
              0841 dc76  st   $76,[y,x++]
              0842 dc36  st   $36,[y,x++] ;0467 COND
              0843 dc56  st   $56,[y,x++] ;0468 GE
              0844 dc71  st   $71,[y,x++]
              0845 dc22  st   $22,[y,x++] ;046a LDW
              0846 dc46  st   $46,[y,x++] ;046b 'Rnd0'
              0847 dc8a  st   $8a,[y,x++] ;046c ANDI
              0848 dc80  st   $80,[y,x++]
              0849 dc36  st   $36,[y,x++] ;046e COND
              084a dc4d  st   $4d,[y,x++] ;046f EQ
              084b dc71  st   $71,[y,x++]
              084c dcf6  st   $f6,[y,x++] ;0471 INC
              084d dc4a  st   $4a,[y,x++] ;0472 'Height'
              084e dc22  st   $22,[y,x++] ;0473 LDW
              084f dc4a  st   $4a,[y,x++] ;0474 'Height'
              0850 dcf0  st   $f0,[y,x++] ;0475 ADDI
              0851 dc08  st   $08,[y,x++]
              0852 dc61  st   $61,[y,x++] ;0477 ST
              0853 dc4d  st   $4d,[y,x++] ;0478 'J'+1
              0854 dc22  st   $22,[y,x++] ;0479 LDW
              0855 dc42  st   $42,[y,x++] ;047a 'X'
              0856 dcf0  st   $f0,[y,x++] ;047b ADDI
              0857 dca0  st   $a0,[y,x++]
              0858 dc8a  st   $8a,[y,x++] ;047d ANDI
              0859 dcff  st   $ff,[y,x++]
              085a dc2c  st   $2c,[y,x++] ;047f STW
              085b dc32  st   $32,[y,x++] ;0480 'p'
              085c dc12  st   $12,[y,x++] ;0481 LDWI
              085d dc00  st   $00,[y,x++]
              085e dc10  st   $10,[y,x++]
              085f dcc4  st   $c4,[y,x++] ;0484 ADDW
              0860 dc32  st   $32,[y,x++] ;0485 'p'
              0861 dc2c  st   $2c,[y,x++] ;0486 STW
              0862 dc4e  st   $4e,[y,x++] ;0487 'V'
              0863 dc5c  st   $5c,[y,x++] ;0488 LDI
              0864 dc01  st   $01,[y,x++]
              0865 dc9b  st   $9b,[y,x++] ;048a POKE
              0866 dc4e  st   $4e,[y,x++] ;048b 'V'
              0867 dcf6  st   $f6,[y,x++] ;048c INC
              0868 dc4f  st   $4f,[y,x++] ;048d 'V'+1
              0869 dc22  st   $22,[y,x++] ;048e LDW
              086a dc4e  st   $4e,[y,x++] ;048f 'V'
              086b dc36  st   $36,[y,x++] ;0490 COND
              086c dc50  st   $50,[y,x++] ;0491 GT
              086d dc86  st   $86,[y,x++]
              086e dc22  st   $22,[y,x++] ;0493 LDW
              086f dc46  st   $46,[y,x++] ;0494 'Rnd0'
              0870 dc8a  st   $8a,[y,x++] ;0495 ANDI
              0871 dc7f  st   $7f,[y,x++]
              0872 dc2c  st   $2c,[y,x++] ;0497 STW
              0873 dc50  st   $50,[y,x++] ;0498 'A'
              0874 dc22  st   $22,[y,x++] ;0499 LDW
              0875 dc50  st   $50,[y,x++] ;049a 'A'
              0876 dcf3  st   $f3,[y,x++] ;049b SUBI
              0877 dc6f  st   $6f,[y,x++]
              0878 dc36  st   $36,[y,x++] ;049d COND
              0879 dc59  st   $59,[y,x++] ;049e LE
              087a dca2  st   $a2,[y,x++]
              087b dc5c  st   $5c,[y,x++] ;04a0 LDI
              087c dc77  st   $77,[y,x++]
              087d dc2c  st   $2c,[y,x++] ;04a2 STW
              087e dc50  st   $50,[y,x++] ;04a3 'A'
              087f dc22  st   $22,[y,x++] ;04a4 LDW
              0880 dc50  st   $50,[y,x++] ;04a5 'A'
              0881 dcf0  st   $f0,[y,x++] ;04a6 ADDI
              0882 dc10  st   $10,[y,x++]
              0883 dc61  st   $61,[y,x++] ;04a8 ST
              0884 dc51  st   $51,[y,x++] ;04a9 'A'+1
              0885 dc22  st   $22,[y,x++] ;04aa LDW
              0886 dc32  st   $32,[y,x++] ;04ab 'p'
              0887 dc61  st   $61,[y,x++] ;04ac ST
              0888 dc50  st   $50,[y,x++] ;04ad 'A'
              0889 dc5c  st   $5c,[y,x++] ;04ae LDI
              088a dc02  st   $02,[y,x++]
              088b dc9b  st   $9b,[y,x++] ;04b0 POKE
              088c dc50  st   $50,[y,x++] ;04b1 'A'
              088d dc22  st   $22,[y,x++] ;04b2 LDW
              088e dc4c  st   $4c,[y,x++] ;04b3 'J'
              088f dcc4  st   $c4,[y,x++] ;04b4 ADDW
              0890 dc32  st   $32,[y,x++] ;04b5 'p'
              0891 dc2c  st   $2c,[y,x++] ;04b6 STW
              0892 dc4e  st   $4e,[y,x++] ;04b7 'V'
              0893 dc5c  st   $5c,[y,x++] ;04b8 LDI
              0894 dc3f  st   $3f,[y,x++]
              0895 dc9b  st   $9b,[y,x++] ;04ba POKE
              0896 dc4e  st   $4e,[y,x++] ;04bb 'V'
              0897 dcf6  st   $f6,[y,x++] ;04bc INC
              0898 dc4f  st   $4f,[y,x++] ;04bd 'V'+1
              0899 dc22  st   $22,[y,x++] ;04be LDW
              089a dc42  st   $42,[y,x++] ;04bf 'X'
              089b dc8a  st   $8a,[y,x++] ;04c0 ANDI
              089c dc08  st   $08,[y,x++]
              089d dcc4  st   $c4,[y,x++] ;04c2 ADDW
              089e dc4a  st   $4a,[y,x++] ;04c3 'Height'
              089f dc2c  st   $2c,[y,x++] ;04c4 STW
              08a0 dc52  st   $52,[y,x++] ;04c5 'C'
              08a1 dc22  st   $22,[y,x++] ;04c6 LDW
              08a2 dc52  st   $52,[y,x++] ;04c7 'C'
              08a3 dcf0  st   $f0,[y,x++] ;04c8 ADDI
              08a4 dc01  st   $01,[y,x++]
              08a5 dc2c  st   $2c,[y,x++] ;04ca STW
              08a6 dc52  st   $52,[y,x++] ;04cb 'C'
              08a7 dc8a  st   $8a,[y,x++] ;04cc ANDI
              08a8 dc08  st   $08,[y,x++]
              08a9 dc36  st   $36,[y,x++] ;04ce COND
              08aa dc75  st   $75,[y,x++] ;04cf NE
              08ab dcd3  st   $d3,[y,x++]
              08ac dc5c  st   $5c,[y,x++] ;04d1 LDI
              08ad dc2a  st   $2a,[y,x++]
              08ae dc98  st   $98,[y,x++] ;04d3 BRA
              08af dcd5  st   $d5,[y,x++]
              08b0 dc5c  st   $5c,[y,x++] ;04d5 LDI
              08b1 dc20  st   $20,[y,x++]
              08b2 dc9b  st   $9b,[y,x++] ;04d7 POKE
              08b3 dc4e  st   $4e,[y,x++] ;04d8 'V'
              08b4 dcf6  st   $f6,[y,x++] ;04d9 INC
              08b5 dc4f  st   $4f,[y,x++] ;04da 'V'+1
              08b6 dc22  st   $22,[y,x++] ;04db LDW
              08b7 dc4e  st   $4e,[y,x++] ;04dc 'V'
              08b8 dc36  st   $36,[y,x++] ;04dd COND
              08b9 dc50  st   $50,[y,x++] ;04de GT
              08ba dcc4  st   $c4,[y,x++]
              08bb dc1b  st   $1b,[y,x++] ;04e0 LD
              08bc dc08  st   $08,[y,x++]
              08bd dc36  st   $36,[y,x++] ;04e2 COND
              08be dc75  st   $75,[y,x++] ;04e3 NE
              08bf dcde  st   $de,[y,x++]
              08c0 dc22  st   $22,[y,x++] ;04e5 LDW
              08c1 dc42  st   $42,[y,x++] ;04e6 'X'
              08c2 dcf0  st   $f0,[y,x++] ;04e7 ADDI
              08c3 dc01  st   $01,[y,x++]
              08c4 dc9b  st   $9b,[y,x++] ;04e9 POKE
              08c5 dc40  st   $40,[y,x++] ;04ea 'ShiftControl'
              08c6 dc22  st   $22,[y,x++] ;04eb LDW
              08c7 dc54  st   $54,[y,x++] ;04ec 'RunPart2'
              08c8 dce4  st   $e4,[y,x++] ;04ed CALL
              08c9 dc2c  st   $2c,[y,x++] ;04ee STW
              08ca dc56  st   $56,[y,x++] ;04ef 'RunPart1'
              08cb dcf6  st   $f6,[y,x++] ;04f0 INC
              08cc dc1d  st   $1d,[y,x++]
              08cd dcff  st   $ff,[y,x++] ;04f2 RET
              08ce 1000  ld   $00,x
              08cf 1405  ld   $05,y
              08d0 dcfc  st   $fc,[y,x++] ;0500 DEF
              08d1 dc4c  st   $4c,[y,x++]
              08d2 dc22  st   $22,[y,x++] ;0502 LDW
              08d3 dc58  st   $58,[y,x++] ;0503 'OldPixel'
              08d4 dc9b  st   $9b,[y,x++] ;0504 POKE
              08d5 dc5a  st   $5a,[y,x++] ;0505 'Ball'
              08d6 dc22  st   $22,[y,x++] ;0506 LDW
              08d7 dc44  st   $44,[y,x++] ;0507 'BallA'
              08d8 dcc4  st   $c4,[y,x++] ;0508 ADDW
              08d9 dc5c  st   $5c,[y,x++] ;0509 'BallV'
              08da dc2c  st   $2c,[y,x++] ;050a STW
              08db dc5c  st   $5c,[y,x++] ;050b 'BallV'
              08dc dc22  st   $22,[y,x++] ;050c LDW
              08dd dc5e  st   $5e,[y,x++] ;050d 'BallY'
              08de dcc4  st   $c4,[y,x++] ;050e ADDW
              08df dc5c  st   $5c,[y,x++] ;050f 'BallV'
              08e0 dc2c  st   $2c,[y,x++] ;0510 STW
              08e1 dc5e  st   $5e,[y,x++] ;0511 'BallY'
              08e2 dc22  st   $22,[y,x++] ;0512 LDW
              08e3 dc42  st   $42,[y,x++] ;0513 'X'
              08e4 dcf0  st   $f0,[y,x++] ;0514 ADDI
              08e5 dc32  st   $32,[y,x++]
              08e6 dc2c  st   $2c,[y,x++] ;0516 STW
              08e7 dc5a  st   $5a,[y,x++] ;0517 'Ball'
              08e8 dc1b  st   $1b,[y,x++] ;0518 LD
              08e9 dc5f  st   $5f,[y,x++] ;0519 'BallY'+1
              08ea dc61  st   $61,[y,x++] ;051a ST
              08eb dc5b  st   $5b,[y,x++] ;051b 'Ball'+1
              08ec dc22  st   $22,[y,x++] ;051c LDW
              08ed dc5a  st   $5a,[y,x++] ;051d 'Ball'
              08ee dcd8  st   $d8,[y,x++] ;051e PEEK
              08ef dc2c  st   $2c,[y,x++] ;051f STW
              08f0 dc58  st   $58,[y,x++] ;0520 'OldPixel'
              08f1 dc22  st   $22,[y,x++] ;0521 LDW
              08f2 dc58  st   $58,[y,x++] ;0522 'OldPixel'
              08f3 dcf3  st   $f3,[y,x++] ;0523 SUBI
              08f4 dc02  st   $02,[y,x++]
              08f5 dc36  st   $36,[y,x++] ;0525 COND
              08f6 dc59  st   $59,[y,x++] ;0526 LE
              08f7 dc45  st   $45,[y,x++]
              08f8 dc5c  st   $5c,[y,x++] ;0528 LDI
              08f9 dc00  st   $00,[y,x++]
              08fa dcb0  st   $b0,[y,x++] ;052a SUBW
              08fb dc5c  st   $5c,[y,x++] ;052b 'BallV'
              08fc dc2c  st   $2c,[y,x++] ;052c STW
              08fd dc5c  st   $5c,[y,x++] ;052d 'BallV'
              08fe dc22  st   $22,[y,x++] ;052e LDW
              08ff dc5e  st   $5e,[y,x++] ;052f 'BallY'
              0900 dcc4  st   $c4,[y,x++] ;0530 ADDW
              0901 dc5c  st   $5c,[y,x++] ;0531 'BallV'
              0902 dc2c  st   $2c,[y,x++] ;0532 STW
              0903 dc5e  st   $5e,[y,x++] ;0533 'BallY'
              0904 dc5c  st   $5c,[y,x++] ;0534 LDI
              0905 dc0a  st   $0a,[y,x++]
              0906 dc61  st   $61,[y,x++] ;0536 ST
              0907 dc12  st   $12,[y,x++]
              0908 dc22  st   $22,[y,x++] ;0538 LDW
              0909 dc42  st   $42,[y,x++] ;0539 'X'
              090a dcf0  st   $f0,[y,x++] ;053a ADDI
              090b dc32  st   $32,[y,x++]
              090c dc2c  st   $2c,[y,x++] ;053c STW
              090d dc5a  st   $5a,[y,x++] ;053d 'Ball'
              090e dc1b  st   $1b,[y,x++] ;053e LD
              090f dc5f  st   $5f,[y,x++] ;053f 'BallY'+1
              0910 dc61  st   $61,[y,x++] ;0540 ST
              0911 dc5b  st   $5b,[y,x++] ;0541 'Ball'+1
              0912 dc22  st   $22,[y,x++] ;0542 LDW
              0913 dc5a  st   $5a,[y,x++] ;0543 'Ball'
              0914 dcd8  st   $d8,[y,x++] ;0544 PEEK
              0915 dc2c  st   $2c,[y,x++] ;0545 STW
              0916 dc58  st   $58,[y,x++] ;0546 'OldPixel'
              0917 dc5c  st   $5c,[y,x++] ;0547 LDI
              0918 dc3f  st   $3f,[y,x++]
              0919 dc9b  st   $9b,[y,x++] ;0549 POKE
              091a dc5a  st   $5a,[y,x++] ;054a 'Ball'
              091b dc22  st   $22,[y,x++] ;054b LDW
              091c dc56  st   $56,[y,x++] ;054c 'RunPart1'
              091d dce4  st   $e4,[y,x++] ;054d CALL
              091e dc2c  st   $2c,[y,x++] ;054e STW
              091f dc54  st   $54,[y,x++] ;054f 'RunPart2'
              0920 dc5c  st   $5c,[y,x++] ;0550 LDI
              0921 dc78  st   $78,[y,x++]
              0922 dc61  st   $61,[y,x++] ;0552 ST
              0923 dc12  st   $12,[y,x++]
              0924 dc5c  st   $5c,[y,x++] ;0554 LDI
              0925 dc08  st   $08,[y,x++]
              0926 dc2c  st   $2c,[y,x++] ;0556 STW
              0927 dc34  st   $34,[y,x++] ;0557 'Color'
              0928 dc12  st   $12,[y,x++] ;0558 LDWI
              0929 dc00  st   $00,[y,x++]
              092a dc08  st   $08,[y,x++]
              092b dc2c  st   $2c,[y,x++] ;055b STW
              092c dc2c  st   $2c,[y,x++] ;055c 'Pos'
              092d dc22  st   $22,[y,x++] ;055d LDW
              092e dc3c  st   $3c,[y,x++] ;055e 'Welcome'
              092f dc2c  st   $2c,[y,x++] ;055f STW
              0930 dc38  st   $38,[y,x++] ;0560 'Text'
              0931 dc22  st   $22,[y,x++] ;0561 LDW
              0932 dc3a  st   $3a,[y,x++] ;0562 'DrawText'
              0933 dce4  st   $e4,[y,x++] ;0563 CALL
              0934 dc22  st   $22,[y,x++] ;0564 LDW
              0935 dc3e  st   $3e,[y,x++] ;0565 'Clear'
              0936 dce4  st   $e4,[y,x++] ;0566 CALL
              0937 dc1b  st   $1b,[y,x++] ;0567 LD
              0938 dc12  st   $12,[y,x++]
              0939 dc36  st   $36,[y,x++] ;0569 COND
              093a dc75  st   $75,[y,x++] ;056a NE
              093b dc65  st   $65,[y,x++]
              093c dc5c  st   $5c,[y,x++] ;056c LDI
              093d dc3e  st   $3e,[y,x++]
              093e dc2c  st   $2c,[y,x++] ;056e STW
              093f dc34  st   $34,[y,x++] ;056f 'Color'
              0940 dc12  st   $12,[y,x++] ;0570 LDWI
              0941 dc00  st   $00,[y,x++]
              0942 dc08  st   $08,[y,x++]
              0943 dc2c  st   $2c,[y,x++] ;0573 STW
              0944 dc2c  st   $2c,[y,x++] ;0574 'Pos'
              0945 dc22  st   $22,[y,x++] ;0575 LDW
              0946 dc3c  st   $3c,[y,x++] ;0576 'Welcome'
              0947 dc2c  st   $2c,[y,x++] ;0577 STW
              0948 dc38  st   $38,[y,x++] ;0578 'Text'
              0949 dc22  st   $22,[y,x++] ;0579 LDW
              094a dc3a  st   $3a,[y,x++] ;057a 'DrawText'
              094b dce4  st   $e4,[y,x++] ;057b CALL
              094c dc22  st   $22,[y,x++] ;057c LDW
              094d dc34  st   $34,[y,x++] ;057d 'Color'
              094e dc94  st   $94,[y,x++] ;057e XORI
              094f dc08  st   $08,[y,x++]
              0950 dc36  st   $36,[y,x++] ;0580 COND
              0951 dc4d  st   $4d,[y,x++] ;0581 EQ
              0952 dc89  st   $89,[y,x++]
              0953 dc22  st   $22,[y,x++] ;0583 LDW
              0954 dc34  st   $34,[y,x++] ;0584 'Color'
              0955 dcf3  st   $f3,[y,x++] ;0585 SUBI
              0956 dc09  st   $09,[y,x++]
              0957 dc2c  st   $2c,[y,x++] ;0587 STW
              0958 dc34  st   $34,[y,x++] ;0588 'Color'
              0959 dc98  st   $98,[y,x++] ;0589 BRA
              095a dc6e  st   $6e,[y,x++]
              095b dc12  st   $12,[y,x++] ;058b LDWI
              095c dc11  st   $11,[y,x++]
              095d dc01  st   $01,[y,x++]
              095e dc2c  st   $2c,[y,x++] ;058e STW
              095f dc40  st   $40,[y,x++] ;058f 'ShiftControl'
              0960 dc5c  st   $5c,[y,x++] ;0590 LDI
              0961 dc00  st   $00,[y,x++]
              0962 dc2c  st   $2c,[y,x++] ;0592 STW
              0963 dc48  st   $48,[y,x++] ;0593 'Rnd1'
              0964 dc2c  st   $2c,[y,x++] ;0594 STW
              0965 dc46  st   $46,[y,x++] ;0595 'Rnd0'
              0966 dc2c  st   $2c,[y,x++] ;0596 STW
              0967 dc4c  st   $4c,[y,x++] ;0597 'J'
              0968 dc2c  st   $2c,[y,x++] ;0598 STW
              0969 dc44  st   $44,[y,x++] ;0599 'BallA'
              096a dc2c  st   $2c,[y,x++] ;059a STW
              096b dc5c  st   $5c,[y,x++] ;059b 'BallV'
              096c dc2c  st   $2c,[y,x++] ;059c STW
              096d dc58  st   $58,[y,x++] ;059d 'OldPixel'
              096e dc12  st   $12,[y,x++] ;059e LDWI
              096f dc00  st   $00,[y,x++]
              0970 dc18  st   $18,[y,x++]
              0971 dc2c  st   $2c,[y,x++] ;05a1 STW
              0972 dc5e  st   $5e,[y,x++] ;05a2 'BallY'
              0973 dc2c  st   $2c,[y,x++] ;05a3 STW
              0974 dc5a  st   $5a,[y,x++] ;05a4 'Ball'
              0975 dc5c  st   $5c,[y,x++] ;05a5 LDI
              0976 dc5a  st   $5a,[y,x++]
              0977 dc2c  st   $2c,[y,x++] ;05a7 STW
              0978 dc4a  st   $4a,[y,x++] ;05a8 'Height'
              0979 dc22  st   $22,[y,x++] ;05a9 LDW
              097a dc56  st   $56,[y,x++] ;05aa 'RunPart1'
              097b dce4  st   $e4,[y,x++] ;05ab CALL
              097c 00fe  ld   $fe
              097d c218  st   [$18]
              097e 0003  ld   $03
              097f c219  st   [$19]
              0980 0000  ld   $00
              0981 c21e  st   [$1e]
              0982 c21f  st   [$1f]
              0983 c21c  st   [$1c]
              0984 0004  ld   $04
              0985 c21d  st   [$1d]
              0986 150f  ld   [$0f],y
              0987 e10e  jmp  y,[$0e]
              0988 0200  nop
              0989
