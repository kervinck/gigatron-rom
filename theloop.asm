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
              001e d222  st   [$22],x
              001f d623  st   [$23],y
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
              002a 0122  ld   [$22]
              002b 8001  adda $01
              002c ec20  bne  .initEnt0
              002d d222  st   [$22],x
              002e 0123  ld   [$23]
              002f 8001  adda $01
              0030 ec20  bne  .initEnt0
              0031 d623  st   [$23],y
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
.loop:        004f c222  st   [$22]
              0050 8200  adda ac
              0051 9200  adda ac,x
              0052 0122  ld   [$22]
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
              0431 0200  nop
              0432 0200  nop
              * 208 times
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
              0702 1000  ld   $00,x
              0703 1403  ld   $03,y
              0704 dcfc  st   $fc,[y,x++] ;0300 DEF
              0705 dc6d  st   $6d,[y,x++]
              0706 dc22  st   $22,[y,x++] ;0302 LDW
              0707 dc22  st   $22,[y,x++] ;0303 'Char'
              0708 dcf3  st   $f3,[y,x++] ;0304 SUBI
              0709 dc52  st   $52,[y,x++]
              070a dc36  st   $36,[y,x++] ;0306 COND
              070b dc56  st   $56,[y,x++] ;0307 GE
              070c dc10  st   $10,[y,x++]
              070d dcf0  st   $f0,[y,x++] ;0309 ADDI
              070e dc32  st   $32,[y,x++]
              070f dc2c  st   $2c,[y,x++] ;030b STW
              0710 dc24  st   $24,[y,x++] ;030c 'i'
              0711 dc12  st   $12,[y,x++] ;030d LDWI
              0712 dc00  st   $00,[y,x++]
              0713 dc05  st   $05,[y,x++]
              0714 dc98  st   $98,[y,x++] ;0310 BRA
              0715 dc15  st   $15,[y,x++]
              0716 dc2c  st   $2c,[y,x++] ;0312 STW
              0717 dc24  st   $24,[y,x++] ;0313 'i'
              0718 dc12  st   $12,[y,x++] ;0314 LDWI
              0719 dc00  st   $00,[y,x++]
              071a dc06  st   $06,[y,x++]
              071b dc2c  st   $2c,[y,x++] ;0317 STW
              071c dc26  st   $26,[y,x++] ;0318 'fontData'
              071d dc22  st   $22,[y,x++] ;0319 LDW
              071e dc24  st   $24,[y,x++] ;031a 'i'
              071f dcc4  st   $c4,[y,x++] ;031b ADDW
              0720 dc24  st   $24,[y,x++] ;031c 'i'
              0721 dc2c  st   $2c,[y,x++] ;031d STW
              0722 dc28  st   $28,[y,x++] ;031e 'tmp'
              0723 dcc4  st   $c4,[y,x++] ;031f ADDW
              0724 dc28  st   $28,[y,x++] ;0320 'tmp'
              0725 dcc4  st   $c4,[y,x++] ;0321 ADDW
              0726 dc24  st   $24,[y,x++] ;0322 'i'
              0727 dcc4  st   $c4,[y,x++] ;0323 ADDW
              0728 dc26  st   $26,[y,x++] ;0324 'fontData'
              0729 dc2c  st   $2c,[y,x++] ;0325 STW
              072a dc26  st   $26,[y,x++] ;0326 'fontData'
              072b dc12  st   $12,[y,x++] ;0327 LDWI
              072c dc00  st   $00,[y,x++]
              072d dc08  st   $08,[y,x++]
              072e dcc4  st   $c4,[y,x++] ;032a ADDW
              072f dc2a  st   $2a,[y,x++] ;032b 'Pos'
              0730 dc2c  st   $2c,[y,x++] ;032c STW
              0731 dc2c  st   $2c,[y,x++] ;032d 'U'
              0732 dc5c  st   $5c,[y,x++] ;032e LDI
              0733 dc05  st   $05,[y,x++]
              0734 dc2c  st   $2c,[y,x++] ;0330 STW
              0735 dc24  st   $24,[y,x++] ;0331 'i'
              0736 dc36  st   $36,[y,x++] ;0332 COND
              0737 dc4d  st   $4d,[y,x++] ;0333 EQ
              0738 dc3b  st   $3b,[y,x++]
              0739 dc22  st   $22,[y,x++] ;0335 LDW
              073a dc26  st   $26,[y,x++] ;0336 'fontData'
              073b dc78  st   $78,[y,x++] ;0337 LOOKUP
              073c dc00  st   $00,[y,x++]
              073d dcf6  st   $f6,[y,x++] ;0339 INC
              073e dc26  st   $26,[y,x++] ;033a 'fontData'
              073f dc98  st   $98,[y,x++] ;033b BRA
              0740 dc3d  st   $3d,[y,x++]
              0741 dc5c  st   $5c,[y,x++] ;033d LDI
              0742 dc00  st   $00,[y,x++]
              0743 dc2c  st   $2c,[y,x++] ;033f STW
              0744 dc2e  st   $2e,[y,x++] ;0340 'bits'
              0745 dc22  st   $22,[y,x++] ;0341 LDW
              0746 dc2a  st   $2a,[y,x++] ;0342 'Pos'
              0747 dc2c  st   $2c,[y,x++] ;0343 STW
              0748 dc30  st   $30,[y,x++] ;0344 'p'
              0749 dc22  st   $22,[y,x++] ;0345 LDW
              074a dc2e  st   $2e,[y,x++] ;0346 'bits'
              074b dc8a  st   $8a,[y,x++] ;0347 ANDI
              074c dc80  st   $80,[y,x++]
              074d dc36  st   $36,[y,x++] ;0349 COND
              074e dc75  st   $75,[y,x++] ;034a NE
              074f dc4e  st   $4e,[y,x++]
              0750 dc5c  st   $5c,[y,x++] ;034c LDI
              0751 dc00  st   $00,[y,x++]
              0752 dc98  st   $98,[y,x++] ;034e BRA
              0753 dc50  st   $50,[y,x++]
              0754 dc22  st   $22,[y,x++] ;0350 LDW
              0755 dc32  st   $32,[y,x++] ;0351 'Color'
              0756 dc9b  st   $9b,[y,x++] ;0352 POKE
              0757 dc30  st   $30,[y,x++] ;0353 'p'
              0758 dc22  st   $22,[y,x++] ;0354 LDW
              0759 dc2e  st   $2e,[y,x++] ;0355 'bits'
              075a dcc4  st   $c4,[y,x++] ;0356 ADDW
              075b dc2e  st   $2e,[y,x++] ;0357 'bits'
              075c dc2c  st   $2c,[y,x++] ;0358 STW
              075d dc2e  st   $2e,[y,x++] ;0359 'bits'
              075e dcf6  st   $f6,[y,x++] ;035a INC
              075f dc31  st   $31,[y,x++] ;035b 'p'+1
              0760 dc22  st   $22,[y,x++] ;035c LDW
              0761 dc30  st   $30,[y,x++] ;035d 'p'
              0762 dcb0  st   $b0,[y,x++] ;035e SUBW
              0763 dc2c  st   $2c,[y,x++] ;035f 'U'
              0764 dc36  st   $36,[y,x++] ;0360 COND
              0765 dc53  st   $53,[y,x++] ;0361 LT
              0766 dc43  st   $43,[y,x++]
              0767 dcf6  st   $f6,[y,x++] ;0363 INC
              0768 dc2a  st   $2a,[y,x++] ;0364 'Pos'
              0769 dc22  st   $22,[y,x++] ;0365 LDW
              076a dc24  st   $24,[y,x++] ;0366 'i'
              076b dcf3  st   $f3,[y,x++] ;0367 SUBI
              076c dc01  st   $01,[y,x++]
              076d dc2c  st   $2c,[y,x++] ;0369 STW
              076e dc24  st   $24,[y,x++] ;036a 'i'
              076f dc36  st   $36,[y,x++] ;036b COND
              0770 dc56  st   $56,[y,x++] ;036c GE
              0771 dc30  st   $30,[y,x++]
              0772 dcff  st   $ff,[y,x++] ;036e RET
              0773 dc2c  st   $2c,[y,x++] ;036f STW
              0774 dc34  st   $34,[y,x++] ;0370 'DrawChar'
              0775 dcfc  st   $fc,[y,x++] ;0371 DEF
              0776 dc95  st   $95,[y,x++]
              0777 dc80  st   $80,[y,x++] ;0373 PUSH
              0778 dc22  st   $22,[y,x++] ;0374 LDW
              0779 dc36  st   $36,[y,x++] ;0375 'Text'
              077a dcd8  st   $d8,[y,x++] ;0376 PEEK
              077b dc2c  st   $2c,[y,x++] ;0377 STW
              077c dc22  st   $22,[y,x++] ;0378 'Char'
              077d dc36  st   $36,[y,x++] ;0379 COND
              077e dc4d  st   $4d,[y,x++] ;037a EQ
              077f dc93  st   $93,[y,x++]
              0780 dcf6  st   $f6,[y,x++] ;037c INC
              0781 dc36  st   $36,[y,x++] ;037d 'Text'
              0782 dc22  st   $22,[y,x++] ;037e LDW
              0783 dc22  st   $22,[y,x++] ;037f 'Char'
              0784 dc94  st   $94,[y,x++] ;0380 XORI
              0785 dc0a  st   $0a,[y,x++]
              0786 dc36  st   $36,[y,x++] ;0382 COND
              0787 dc75  st   $75,[y,x++] ;0383 NE
              0788 dc8e  st   $8e,[y,x++]
              0789 dc61  st   $61,[y,x++] ;0385 ST
              078a dc2a  st   $2a,[y,x++] ;0386 'Pos'
              078b dc12  st   $12,[y,x++] ;0387 LDWI
              078c dc00  st   $00,[y,x++]
              078d dc08  st   $08,[y,x++]
              078e dcc4  st   $c4,[y,x++] ;038a ADDW
              078f dc2a  st   $2a,[y,x++] ;038b 'Pos'
              0790 dc2c  st   $2c,[y,x++] ;038c STW
              0791 dc2a  st   $2a,[y,x++] ;038d 'Pos'
              0792 dc98  st   $98,[y,x++] ;038e BRA
              0793 dc91  st   $91,[y,x++]
              0794 dc22  st   $22,[y,x++] ;0390 LDW
              0795 dc34  st   $34,[y,x++] ;0391 'DrawChar'
              0796 dce4  st   $e4,[y,x++] ;0392 CALL
              0797 dc98  st   $98,[y,x++] ;0393 BRA
              0798 dc72  st   $72,[y,x++]
              0799 dc66  st   $66,[y,x++] ;0395 POP
              079a dce4  st   $e4,[y,x++] ;0396 CALL
              079b dc2c  st   $2c,[y,x++] ;0397 STW
              079c dc38  st   $38,[y,x++] ;0398 'DrawText'
              079d dcfc  st   $fc,[y,x++] ;0399 DEF
              079e dcd2  st   $d2,[y,x++]
              079f dc20  st   $20,[y,x++]
              07a0 dc20  st   $20,[y,x++]
              07a1 dc20  st   $20,[y,x++]
              07a2 dc2a  st   $2a,[y,x++]
              07a3 dc2a  st   $2a,[y,x++]
              07a4 dc2a  st   $2a,[y,x++]
              07a5 dc20  st   $20,[y,x++]
              07a6 dc47  st   $47,[y,x++]
              07a7 dc69  st   $69,[y,x++]
              07a8 dc67  st   $67,[y,x++]
              07a9 dc61  st   $61,[y,x++]
              07aa dc74  st   $74,[y,x++]
              07ab dc72  st   $72,[y,x++]
              07ac dc6f  st   $6f,[y,x++]
              07ad dc6e  st   $6e,[y,x++]
              07ae dc20  st   $20,[y,x++]
              07af dc33  st   $33,[y,x++]
              07b0 dc32  st   $32,[y,x++]
              07b1 dc4b  st   $4b,[y,x++]
              07b2 dc20  st   $20,[y,x++]
              07b3 dc2a  st   $2a,[y,x++]
              07b4 dc2a  st   $2a,[y,x++]
              07b5 dc2a  st   $2a,[y,x++]
              07b6 dc20  st   $20,[y,x++]
              07b7 dc20  st   $20,[y,x++]
              07b8 dc20  st   $20,[y,x++]
              07b9 dc20  st   $20,[y,x++]
              07ba dc0a  st   $0a,[y,x++]
              07bb dc54  st   $54,[y,x++]
              07bc dc54  st   $54,[y,x++]
              07bd dc4c  st   $4c,[y,x++]
              07be dc20  st   $20,[y,x++]
              07bf dc63  st   $63,[y,x++]
              07c0 dc6f  st   $6f,[y,x++]
              07c1 dc6c  st   $6c,[y,x++]
              07c2 dc6f  st   $6f,[y,x++]
              07c3 dc72  st   $72,[y,x++]
              07c4 dc20  st   $20,[y,x++]
              07c5 dc63  st   $63,[y,x++]
              07c6 dc6f  st   $6f,[y,x++]
              07c7 dc6d  st   $6d,[y,x++]
              07c8 dc70  st   $70,[y,x++]
              07c9 dc75  st   $75,[y,x++]
              07ca dc74  st   $74,[y,x++]
              07cb dc65  st   $65,[y,x++]
              07cc dc72  st   $72,[y,x++]
              07cd dc20  st   $20,[y,x++]
              07ce dc52  st   $52,[y,x++]
              07cf dc4f  st   $4f,[y,x++]
              07d0 dc4d  st   $4d,[y,x++]
              07d1 dc20  st   $20,[y,x++]
              07d2 dc76  st   $76,[y,x++]
              07d3 dc30  st   $30,[y,x++]
              07d4 dc20  st   $20,[y,x++]
              07d5 dc20  st   $20,[y,x++]
              07d6 dc0a  st   $0a,[y,x++]
              07d7 dc00  st   $00,[y,x++]
              07d8 dc2c  st   $2c,[y,x++] ;03d4 STW
              07d9 dc3a  st   $3a,[y,x++] ;03d5 'Welcome'
              07da dc12  st   $12,[y,x++] ;03d6 LDWI
              07db dc00  st   $00,[y,x++]
              07dc dc04  st   $04,[y,x++]
              07dd dce4  st   $e4,[y,x++] ;03d9 CALL
              07de 1000  ld   $00,x
              07df 1404  ld   $04,y
              07e0 dcfc  st   $fc,[y,x++] ;0400 DEF
              07e1 dc1e  st   $1e,[y,x++]
              07e2 dc22  st   $22,[y,x++] ;0402 LDW
              07e3 dc2a  st   $2a,[y,x++] ;0403 'Pos'
              07e4 dc2c  st   $2c,[y,x++] ;0404 STW
              07e5 dc30  st   $30,[y,x++] ;0405 'p'
              07e6 dc5c  st   $5c,[y,x++] ;0406 LDI
              07e7 dc00  st   $00,[y,x++]
              07e8 dc9b  st   $9b,[y,x++] ;0408 POKE
              07e9 dc30  st   $30,[y,x++] ;0409 'p'
              07ea dcf6  st   $f6,[y,x++] ;040a INC
              07eb dc31  st   $31,[y,x++] ;040b 'p'+1
              07ec dc22  st   $22,[y,x++] ;040c LDW
              07ed dc30  st   $30,[y,x++] ;040d 'p'
              07ee dc36  st   $36,[y,x++] ;040e COND
              07ef dc50  st   $50,[y,x++] ;040f GT
              07f0 dc04  st   $04,[y,x++]
              07f1 dc12  st   $12,[y,x++] ;0411 LDWI
              07f2 dc01  st   $01,[y,x++]
              07f3 dc98  st   $98,[y,x++]
              07f4 dcc4  st   $c4,[y,x++] ;0414 ADDW
              07f5 dc30  st   $30,[y,x++] ;0415 'p'
              07f6 dc2c  st   $2c,[y,x++] ;0416 STW
              07f7 dc30  st   $30,[y,x++] ;0417 'p'
              07f8 dc8a  st   $8a,[y,x++] ;0418 ANDI
              07f9 dcff  st   $ff,[y,x++]
              07fa dc94  st   $94,[y,x++] ;041a XORI
              07fb dca0  st   $a0,[y,x++]
              07fc dc36  st   $36,[y,x++] ;041c COND
              07fd dc75  st   $75,[y,x++] ;041d NE
              07fe dc04  st   $04,[y,x++]
              07ff dcff  st   $ff,[y,x++] ;041f RET
              0800 dc2c  st   $2c,[y,x++] ;0420 STW
              0801 dc3c  st   $3c,[y,x++] ;0421 'Clear'
              0802 dcfc  st   $fc,[y,x++] ;0422 DEF
              0803 dce4  st   $e4,[y,x++]
              0804 dc22  st   $22,[y,x++] ;0424 LDW
              0805 dc3e  st   $3e,[y,x++] ;0425 'ShiftControl'
              0806 dcd8  st   $d8,[y,x++] ;0426 PEEK
              0807 dc2c  st   $2c,[y,x++] ;0427 STW
              0808 dc40  st   $40,[y,x++] ;0428 'X'
              0809 dc94  st   $94,[y,x++] ;0429 XORI
              080a dc78  st   $78,[y,x++]
              080b dc36  st   $36,[y,x++] ;042b COND
              080c dc75  st   $75,[y,x++] ;042c NE
              080d dc30  st   $30,[y,x++]
              080e dc5c  st   $5c,[y,x++] ;042e LDI
              080f dc0a  st   $0a,[y,x++]
              0810 dc2c  st   $2c,[y,x++] ;0430 STW
              0811 dc42  st   $42,[y,x++] ;0431 'BallA'
              0812 dc1b  st   $1b,[y,x++] ;0432 LD
              0813 dc0d  st   $0d,[y,x++]
              0814 dcc4  st   $c4,[y,x++] ;0434 ADDW
              0815 dc44  st   $44,[y,x++] ;0435 'Rnd0'
              0816 dcc4  st   $c4,[y,x++] ;0436 ADDW
              0817 dc46  st   $46,[y,x++] ;0437 'Rnd1'
              0818 dc2c  st   $2c,[y,x++] ;0438 STW
              0819 dc44  st   $44,[y,x++] ;0439 'Rnd0'
              081a dcc4  st   $c4,[y,x++] ;043a ADDW
              081b dc46  st   $46,[y,x++] ;043b 'Rnd1'
              081c dc36  st   $36,[y,x++] ;043c COND
              081d dc56  st   $56,[y,x++] ;043d GE
              081e dc3f  st   $3f,[y,x++]
              081f dc94  st   $94,[y,x++] ;043f XORI
              0820 dc2b  st   $2b,[y,x++]
              0821 dcc4  st   $c4,[y,x++] ;0441 ADDW
              0822 dc46  st   $46,[y,x++] ;0442 'Rnd1'
              0823 dcc4  st   $c4,[y,x++] ;0443 ADDW
              0824 dc46  st   $46,[y,x++] ;0444 'Rnd1'
              0825 dc2c  st   $2c,[y,x++] ;0445 STW
              0826 dc46  st   $46,[y,x++] ;0446 'Rnd1'
              0827 dc22  st   $22,[y,x++] ;0447 LDW
              0828 dc48  st   $48,[y,x++] ;0448 'Height'
              0829 dcf3  st   $f3,[y,x++] ;0449 SUBI
              082a dc58  st   $58,[y,x++]
              082b dc36  st   $36,[y,x++] ;044b COND
              082c dc59  st   $59,[y,x++] ;044c LE
              082d dc59  st   $59,[y,x++]
              082e dc22  st   $22,[y,x++] ;044e LDW
              082f dc44  st   $44,[y,x++] ;044f 'Rnd0'
              0830 dc8a  st   $8a,[y,x++] ;0450 ANDI
              0831 dc10  st   $10,[y,x++]
              0832 dc36  st   $36,[y,x++] ;0452 COND
              0833 dc4d  st   $4d,[y,x++] ;0453 EQ
              0834 dc59  st   $59,[y,x++]
              0835 dc22  st   $22,[y,x++] ;0455 LDW
              0836 dc48  st   $48,[y,x++] ;0456 'Height'
              0837 dcf3  st   $f3,[y,x++] ;0457 SUBI
              0838 dc01  st   $01,[y,x++]
              0839 dc2c  st   $2c,[y,x++] ;0459 STW
              083a dc48  st   $48,[y,x++] ;045a 'Height'
              083b dc22  st   $22,[y,x++] ;045b LDW
              083c dc48  st   $48,[y,x++] ;045c 'Height'
              083d dcf3  st   $f3,[y,x++] ;045d SUBI
              083e dc76  st   $76,[y,x++]
              083f dc36  st   $36,[y,x++] ;045f COND
              0840 dc56  st   $56,[y,x++] ;0460 GE
              0841 dc69  st   $69,[y,x++]
              0842 dc22  st   $22,[y,x++] ;0462 LDW
              0843 dc44  st   $44,[y,x++] ;0463 'Rnd0'
              0844 dc8a  st   $8a,[y,x++] ;0464 ANDI
              0845 dc80  st   $80,[y,x++]
              0846 dc36  st   $36,[y,x++] ;0466 COND
              0847 dc4d  st   $4d,[y,x++] ;0467 EQ
              0848 dc69  st   $69,[y,x++]
              0849 dcf6  st   $f6,[y,x++] ;0469 INC
              084a dc48  st   $48,[y,x++] ;046a 'Height'
              084b dc22  st   $22,[y,x++] ;046b LDW
              084c dc48  st   $48,[y,x++] ;046c 'Height'
              084d dcf0  st   $f0,[y,x++] ;046d ADDI
              084e dc08  st   $08,[y,x++]
              084f dc61  st   $61,[y,x++] ;046f ST
              0850 dc4b  st   $4b,[y,x++] ;0470 'J'+1
              0851 dc22  st   $22,[y,x++] ;0471 LDW
              0852 dc40  st   $40,[y,x++] ;0472 'X'
              0853 dcf0  st   $f0,[y,x++] ;0473 ADDI
              0854 dca0  st   $a0,[y,x++]
              0855 dc8a  st   $8a,[y,x++] ;0475 ANDI
              0856 dcff  st   $ff,[y,x++]
              0857 dc2c  st   $2c,[y,x++] ;0477 STW
              0858 dc30  st   $30,[y,x++] ;0478 'p'
              0859 dc12  st   $12,[y,x++] ;0479 LDWI
              085a dc00  st   $00,[y,x++]
              085b dc10  st   $10,[y,x++]
              085c dcc4  st   $c4,[y,x++] ;047c ADDW
              085d dc30  st   $30,[y,x++] ;047d 'p'
              085e dc2c  st   $2c,[y,x++] ;047e STW
              085f dc4c  st   $4c,[y,x++] ;047f 'V'
              0860 dc5c  st   $5c,[y,x++] ;0480 LDI
              0861 dc01  st   $01,[y,x++]
              0862 dc9b  st   $9b,[y,x++] ;0482 POKE
              0863 dc4c  st   $4c,[y,x++] ;0483 'V'
              0864 dcf6  st   $f6,[y,x++] ;0484 INC
              0865 dc4d  st   $4d,[y,x++] ;0485 'V'+1
              0866 dc22  st   $22,[y,x++] ;0486 LDW
              0867 dc4c  st   $4c,[y,x++] ;0487 'V'
              0868 dc36  st   $36,[y,x++] ;0488 COND
              0869 dc50  st   $50,[y,x++] ;0489 GT
              086a dc7e  st   $7e,[y,x++]
              086b dc22  st   $22,[y,x++] ;048b LDW
              086c dc44  st   $44,[y,x++] ;048c 'Rnd0'
              086d dc8a  st   $8a,[y,x++] ;048d ANDI
              086e dc7f  st   $7f,[y,x++]
              086f dc2c  st   $2c,[y,x++] ;048f STW
              0870 dc4e  st   $4e,[y,x++] ;0490 'A'
              0871 dc22  st   $22,[y,x++] ;0491 LDW
              0872 dc4e  st   $4e,[y,x++] ;0492 'A'
              0873 dcf3  st   $f3,[y,x++] ;0493 SUBI
              0874 dc6f  st   $6f,[y,x++]
              0875 dc36  st   $36,[y,x++] ;0495 COND
              0876 dc59  st   $59,[y,x++] ;0496 LE
              0877 dc9a  st   $9a,[y,x++]
              0878 dc5c  st   $5c,[y,x++] ;0498 LDI
              0879 dc77  st   $77,[y,x++]
              087a dc2c  st   $2c,[y,x++] ;049a STW
              087b dc4e  st   $4e,[y,x++] ;049b 'A'
              087c dc22  st   $22,[y,x++] ;049c LDW
              087d dc4e  st   $4e,[y,x++] ;049d 'A'
              087e dcf0  st   $f0,[y,x++] ;049e ADDI
              087f dc10  st   $10,[y,x++]
              0880 dc61  st   $61,[y,x++] ;04a0 ST
              0881 dc4f  st   $4f,[y,x++] ;04a1 'A'+1
              0882 dc22  st   $22,[y,x++] ;04a2 LDW
              0883 dc30  st   $30,[y,x++] ;04a3 'p'
              0884 dc61  st   $61,[y,x++] ;04a4 ST
              0885 dc4e  st   $4e,[y,x++] ;04a5 'A'
              0886 dc5c  st   $5c,[y,x++] ;04a6 LDI
              0887 dc02  st   $02,[y,x++]
              0888 dc9b  st   $9b,[y,x++] ;04a8 POKE
              0889 dc4e  st   $4e,[y,x++] ;04a9 'A'
              088a dc22  st   $22,[y,x++] ;04aa LDW
              088b dc4a  st   $4a,[y,x++] ;04ab 'J'
              088c dcc4  st   $c4,[y,x++] ;04ac ADDW
              088d dc30  st   $30,[y,x++] ;04ad 'p'
              088e dc2c  st   $2c,[y,x++] ;04ae STW
              088f dc4c  st   $4c,[y,x++] ;04af 'V'
              0890 dc5c  st   $5c,[y,x++] ;04b0 LDI
              0891 dc3f  st   $3f,[y,x++]
              0892 dc9b  st   $9b,[y,x++] ;04b2 POKE
              0893 dc4c  st   $4c,[y,x++] ;04b3 'V'
              0894 dcf6  st   $f6,[y,x++] ;04b4 INC
              0895 dc4d  st   $4d,[y,x++] ;04b5 'V'+1
              0896 dc22  st   $22,[y,x++] ;04b6 LDW
              0897 dc40  st   $40,[y,x++] ;04b7 'X'
              0898 dc8a  st   $8a,[y,x++] ;04b8 ANDI
              0899 dc08  st   $08,[y,x++]
              089a dcc4  st   $c4,[y,x++] ;04ba ADDW
              089b dc48  st   $48,[y,x++] ;04bb 'Height'
              089c dc2c  st   $2c,[y,x++] ;04bc STW
              089d dc50  st   $50,[y,x++] ;04bd 'C'
              089e dc22  st   $22,[y,x++] ;04be LDW
              089f dc50  st   $50,[y,x++] ;04bf 'C'
              08a0 dcf0  st   $f0,[y,x++] ;04c0 ADDI
              08a1 dc01  st   $01,[y,x++]
              08a2 dc2c  st   $2c,[y,x++] ;04c2 STW
              08a3 dc50  st   $50,[y,x++] ;04c3 'C'
              08a4 dc8a  st   $8a,[y,x++] ;04c4 ANDI
              08a5 dc08  st   $08,[y,x++]
              08a6 dc36  st   $36,[y,x++] ;04c6 COND
              08a7 dc75  st   $75,[y,x++] ;04c7 NE
              08a8 dccb  st   $cb,[y,x++]
              08a9 dc5c  st   $5c,[y,x++] ;04c9 LDI
              08aa dc2a  st   $2a,[y,x++]
              08ab dc98  st   $98,[y,x++] ;04cb BRA
              08ac dccd  st   $cd,[y,x++]
              08ad dc5c  st   $5c,[y,x++] ;04cd LDI
              08ae dc20  st   $20,[y,x++]
              08af dc9b  st   $9b,[y,x++] ;04cf POKE
              08b0 dc4c  st   $4c,[y,x++] ;04d0 'V'
              08b1 dcf6  st   $f6,[y,x++] ;04d1 INC
              08b2 dc4d  st   $4d,[y,x++] ;04d2 'V'+1
              08b3 dc22  st   $22,[y,x++] ;04d3 LDW
              08b4 dc4c  st   $4c,[y,x++] ;04d4 'V'
              08b5 dc36  st   $36,[y,x++] ;04d5 COND
              08b6 dc50  st   $50,[y,x++] ;04d6 GT
              08b7 dcbc  st   $bc,[y,x++]
              08b8 dc1b  st   $1b,[y,x++] ;04d8 LD
              08b9 dc08  st   $08,[y,x++]
              08ba dc36  st   $36,[y,x++] ;04da COND
              08bb dc75  st   $75,[y,x++] ;04db NE
              08bc dcd6  st   $d6,[y,x++]
              08bd dc22  st   $22,[y,x++] ;04dd LDW
              08be dc40  st   $40,[y,x++] ;04de 'X'
              08bf dcf0  st   $f0,[y,x++] ;04df ADDI
              08c0 dc01  st   $01,[y,x++]
              08c1 dc9b  st   $9b,[y,x++] ;04e1 POKE
              08c2 dc3e  st   $3e,[y,x++] ;04e2 'ShiftControl'
              08c3 dc22  st   $22,[y,x++] ;04e3 LDW
              08c4 dc52  st   $52,[y,x++] ;04e4 'RunPart2'
              08c5 dce4  st   $e4,[y,x++] ;04e5 CALL
              08c6 dc2c  st   $2c,[y,x++] ;04e6 STW
              08c7 dc54  st   $54,[y,x++] ;04e7 'RunPart1'
              08c8 dc12  st   $12,[y,x++] ;04e8 LDWI
              08c9 dc00  st   $00,[y,x++]
              08ca dc05  st   $05,[y,x++]
              08cb dce4  st   $e4,[y,x++] ;04eb CALL
              08cc 1000  ld   $00,x
              08cd 1405  ld   $05,y
              08ce dcfc  st   $fc,[y,x++] ;0500 DEF
              08cf dc4c  st   $4c,[y,x++]
              08d0 dc22  st   $22,[y,x++] ;0502 LDW
              08d1 dc56  st   $56,[y,x++] ;0503 'OldPixel'
              08d2 dc9b  st   $9b,[y,x++] ;0504 POKE
              08d3 dc58  st   $58,[y,x++] ;0505 'Ball'
              08d4 dc22  st   $22,[y,x++] ;0506 LDW
              08d5 dc42  st   $42,[y,x++] ;0507 'BallA'
              08d6 dcc4  st   $c4,[y,x++] ;0508 ADDW
              08d7 dc5a  st   $5a,[y,x++] ;0509 'BallV'
              08d8 dc2c  st   $2c,[y,x++] ;050a STW
              08d9 dc5a  st   $5a,[y,x++] ;050b 'BallV'
              08da dc22  st   $22,[y,x++] ;050c LDW
              08db dc5c  st   $5c,[y,x++] ;050d 'BallY'
              08dc dcc4  st   $c4,[y,x++] ;050e ADDW
              08dd dc5a  st   $5a,[y,x++] ;050f 'BallV'
              08de dc2c  st   $2c,[y,x++] ;0510 STW
              08df dc5c  st   $5c,[y,x++] ;0511 'BallY'
              08e0 dc22  st   $22,[y,x++] ;0512 LDW
              08e1 dc40  st   $40,[y,x++] ;0513 'X'
              08e2 dcf0  st   $f0,[y,x++] ;0514 ADDI
              08e3 dc32  st   $32,[y,x++]
              08e4 dc2c  st   $2c,[y,x++] ;0516 STW
              08e5 dc58  st   $58,[y,x++] ;0517 'Ball'
              08e6 dc1b  st   $1b,[y,x++] ;0518 LD
              08e7 dc5d  st   $5d,[y,x++] ;0519 'BallY'+1
              08e8 dc61  st   $61,[y,x++] ;051a ST
              08e9 dc59  st   $59,[y,x++] ;051b 'Ball'+1
              08ea dc22  st   $22,[y,x++] ;051c LDW
              08eb dc58  st   $58,[y,x++] ;051d 'Ball'
              08ec dcd8  st   $d8,[y,x++] ;051e PEEK
              08ed dc2c  st   $2c,[y,x++] ;051f STW
              08ee dc56  st   $56,[y,x++] ;0520 'OldPixel'
              08ef dc22  st   $22,[y,x++] ;0521 LDW
              08f0 dc56  st   $56,[y,x++] ;0522 'OldPixel'
              08f1 dcf3  st   $f3,[y,x++] ;0523 SUBI
              08f2 dc02  st   $02,[y,x++]
              08f3 dc36  st   $36,[y,x++] ;0525 COND
              08f4 dc59  st   $59,[y,x++] ;0526 LE
              08f5 dc45  st   $45,[y,x++]
              08f6 dc5c  st   $5c,[y,x++] ;0528 LDI
              08f7 dc00  st   $00,[y,x++]
              08f8 dcb0  st   $b0,[y,x++] ;052a SUBW
              08f9 dc5a  st   $5a,[y,x++] ;052b 'BallV'
              08fa dc2c  st   $2c,[y,x++] ;052c STW
              08fb dc5a  st   $5a,[y,x++] ;052d 'BallV'
              08fc dc22  st   $22,[y,x++] ;052e LDW
              08fd dc5c  st   $5c,[y,x++] ;052f 'BallY'
              08fe dcc4  st   $c4,[y,x++] ;0530 ADDW
              08ff dc5a  st   $5a,[y,x++] ;0531 'BallV'
              0900 dc2c  st   $2c,[y,x++] ;0532 STW
              0901 dc5c  st   $5c,[y,x++] ;0533 'BallY'
              0902 dc5c  st   $5c,[y,x++] ;0534 LDI
              0903 dc0a  st   $0a,[y,x++]
              0904 dc61  st   $61,[y,x++] ;0536 ST
              0905 dc12  st   $12,[y,x++]
              0906 dc22  st   $22,[y,x++] ;0538 LDW
              0907 dc40  st   $40,[y,x++] ;0539 'X'
              0908 dcf0  st   $f0,[y,x++] ;053a ADDI
              0909 dc32  st   $32,[y,x++]
              090a dc2c  st   $2c,[y,x++] ;053c STW
              090b dc58  st   $58,[y,x++] ;053d 'Ball'
              090c dc1b  st   $1b,[y,x++] ;053e LD
              090d dc5d  st   $5d,[y,x++] ;053f 'BallY'+1
              090e dc61  st   $61,[y,x++] ;0540 ST
              090f dc59  st   $59,[y,x++] ;0541 'Ball'+1
              0910 dc22  st   $22,[y,x++] ;0542 LDW
              0911 dc58  st   $58,[y,x++] ;0543 'Ball'
              0912 dcd8  st   $d8,[y,x++] ;0544 PEEK
              0913 dc2c  st   $2c,[y,x++] ;0545 STW
              0914 dc56  st   $56,[y,x++] ;0546 'OldPixel'
              0915 dc5c  st   $5c,[y,x++] ;0547 LDI
              0916 dc3f  st   $3f,[y,x++]
              0917 dc9b  st   $9b,[y,x++] ;0549 POKE
              0918 dc58  st   $58,[y,x++] ;054a 'Ball'
              0919 dc22  st   $22,[y,x++] ;054b LDW
              091a dc54  st   $54,[y,x++] ;054c 'RunPart1'
              091b dce4  st   $e4,[y,x++] ;054d CALL
              091c dc2c  st   $2c,[y,x++] ;054e STW
              091d dc52  st   $52,[y,x++] ;054f 'RunPart2'
              091e dc5c  st   $5c,[y,x++] ;0550 LDI
              091f dcb4  st   $b4,[y,x++]
              0920 dc61  st   $61,[y,x++] ;0552 ST
              0921 dc12  st   $12,[y,x++]
              0922 dc5c  st   $5c,[y,x++] ;0554 LDI
              0923 dc08  st   $08,[y,x++]
              0924 dc2c  st   $2c,[y,x++] ;0556 STW
              0925 dc32  st   $32,[y,x++] ;0557 'Color'
              0926 dc12  st   $12,[y,x++] ;0558 LDWI
              0927 dc00  st   $00,[y,x++]
              0928 dc08  st   $08,[y,x++]
              0929 dc2c  st   $2c,[y,x++] ;055b STW
              092a dc2a  st   $2a,[y,x++] ;055c 'Pos'
              092b dc22  st   $22,[y,x++] ;055d LDW
              092c dc3a  st   $3a,[y,x++] ;055e 'Welcome'
              092d dc2c  st   $2c,[y,x++] ;055f STW
              092e dc36  st   $36,[y,x++] ;0560 'Text'
              092f dc22  st   $22,[y,x++] ;0561 LDW
              0930 dc38  st   $38,[y,x++] ;0562 'DrawText'
              0931 dce4  st   $e4,[y,x++] ;0563 CALL
              0932 dc22  st   $22,[y,x++] ;0564 LDW
              0933 dc3c  st   $3c,[y,x++] ;0565 'Clear'
              0934 dce4  st   $e4,[y,x++] ;0566 CALL
              0935 dc1b  st   $1b,[y,x++] ;0567 LD
              0936 dc12  st   $12,[y,x++]
              0937 dc36  st   $36,[y,x++] ;0569 COND
              0938 dc75  st   $75,[y,x++] ;056a NE
              0939 dc65  st   $65,[y,x++]
              093a dc5c  st   $5c,[y,x++] ;056c LDI
              093b dc3e  st   $3e,[y,x++]
              093c dc2c  st   $2c,[y,x++] ;056e STW
              093d dc32  st   $32,[y,x++] ;056f 'Color'
              093e dc12  st   $12,[y,x++] ;0570 LDWI
              093f dc00  st   $00,[y,x++]
              0940 dc08  st   $08,[y,x++]
              0941 dc2c  st   $2c,[y,x++] ;0573 STW
              0942 dc2a  st   $2a,[y,x++] ;0574 'Pos'
              0943 dc22  st   $22,[y,x++] ;0575 LDW
              0944 dc3a  st   $3a,[y,x++] ;0576 'Welcome'
              0945 dc2c  st   $2c,[y,x++] ;0577 STW
              0946 dc36  st   $36,[y,x++] ;0578 'Text'
              0947 dc22  st   $22,[y,x++] ;0579 LDW
              0948 dc38  st   $38,[y,x++] ;057a 'DrawText'
              0949 dce4  st   $e4,[y,x++] ;057b CALL
              094a dc22  st   $22,[y,x++] ;057c LDW
              094b dc32  st   $32,[y,x++] ;057d 'Color'
              094c dc94  st   $94,[y,x++] ;057e XORI
              094d dc08  st   $08,[y,x++]
              094e dc36  st   $36,[y,x++] ;0580 COND
              094f dc4d  st   $4d,[y,x++] ;0581 EQ
              0950 dc89  st   $89,[y,x++]
              0951 dc22  st   $22,[y,x++] ;0583 LDW
              0952 dc32  st   $32,[y,x++] ;0584 'Color'
              0953 dcf3  st   $f3,[y,x++] ;0585 SUBI
              0954 dc09  st   $09,[y,x++]
              0955 dc2c  st   $2c,[y,x++] ;0587 STW
              0956 dc32  st   $32,[y,x++] ;0588 'Color'
              0957 dc98  st   $98,[y,x++] ;0589 BRA
              0958 dc6e  st   $6e,[y,x++]
              0959 dc12  st   $12,[y,x++] ;058b LDWI
              095a dc11  st   $11,[y,x++]
              095b dc01  st   $01,[y,x++]
              095c dc2c  st   $2c,[y,x++] ;058e STW
              095d dc3e  st   $3e,[y,x++] ;058f 'ShiftControl'
              095e dc5c  st   $5c,[y,x++] ;0590 LDI
              095f dc00  st   $00,[y,x++]
              0960 dc2c  st   $2c,[y,x++] ;0592 STW
              0961 dc46  st   $46,[y,x++] ;0593 'Rnd1'
              0962 dc2c  st   $2c,[y,x++] ;0594 STW
              0963 dc44  st   $44,[y,x++] ;0595 'Rnd0'
              0964 dc2c  st   $2c,[y,x++] ;0596 STW
              0965 dc4a  st   $4a,[y,x++] ;0597 'J'
              0966 dc2c  st   $2c,[y,x++] ;0598 STW
              0967 dc42  st   $42,[y,x++] ;0599 'BallA'
              0968 dc2c  st   $2c,[y,x++] ;059a STW
              0969 dc5a  st   $5a,[y,x++] ;059b 'BallV'
              096a dc2c  st   $2c,[y,x++] ;059c STW
              096b dc56  st   $56,[y,x++] ;059d 'OldPixel'
              096c dc12  st   $12,[y,x++] ;059e LDWI
              096d dc00  st   $00,[y,x++]
              096e dc18  st   $18,[y,x++]
              096f dc2c  st   $2c,[y,x++] ;05a1 STW
              0970 dc5c  st   $5c,[y,x++] ;05a2 'BallY'
              0971 dc2c  st   $2c,[y,x++] ;05a3 STW
              0972 dc58  st   $58,[y,x++] ;05a4 'Ball'
              0973 dc5c  st   $5c,[y,x++] ;05a5 LDI
              0974 dc5a  st   $5a,[y,x++]
              0975 dc2c  st   $2c,[y,x++] ;05a7 STW
              0976 dc48  st   $48,[y,x++] ;05a8 'Height'
              0977 dc22  st   $22,[y,x++] ;05a9 LDW
              0978 dc54  st   $54,[y,x++] ;05aa 'RunPart1'
              0979 dce4  st   $e4,[y,x++] ;05ab CALL
              097a 00fe  ld   $fe
              097b c218  st   [$18]
              097c 0003  ld   $03
              097d c219  st   [$19]
              097e 0000  ld   $00
              097f c21e  st   [$1e]
              0980 c21f  st   [$1f]
              0981 150f  ld   [$0f],y
              0982 e10e  jmp  y,[$0e]
              0983 0200  nop
              0984
