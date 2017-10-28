address
|    encoding
|    |     instruction
|    |     |    operands
|    |     |    |
V    V     V    V
0000 0000  ld   $00
0001 1880  ld   $80,out
0002 18c0  ld   $c0,out
0003 0001  ld   $01
0004 d601  st   [$01],y
0005 00ff  ld   $ff
0006 6900  xora [y,$00]
0007 ca00  st   [y,$00]
0008 c200  st   [$00]
0009 6900  xora [y,$00]
000a ec0a  bne  $0a
000b 00ff  ld   $ff
000c 6900  xora [y,$00]
000d ca00  st   [y,$00]
000e 6100  xora [$00]
000f f013  beq  $13
0010 0101  ld   [$01]
0011 fc04  bra  $04
0012 8200  adda ac
0013 0001  ld   $01
0014 1880  ld   $80,out
0015 18c0  ld   $c0,out
0016 0000  ld   $00
0017 d218  st   [$18],x
0018 d619  st   [$19],y
0019 010e  ld   [$0e]
001a f41d  bge  $1d
001b 8d00  adda [y,x]
001c 80bf  adda $bf
001d c20e  st   [$0e]
001e 010f  ld   [$0f]
001f f422  bge  $22
0020 810e  adda [$0e]
0021 80c1  adda $c1
0022 c20f  st   [$0f]
0023 0118  ld   [$18]
0024 8001  adda $01
0025 ec19  bne  $19
0026 d218  st   [$18],x
0027 0119  ld   [$19]
0028 8001  adda $01
0029 ec19  bne  $19
002a d619  st   [$19],y
002b 0003  ld   $03
002c 1880  ld   $80,out
002d 18c0  ld   $c0,out
002e 0104  ld   [$04]
002f 8105  adda [$05]
0030 805a  adda $5a
0031 ec35  bne  $35
0032 0000  ld   $00
0033 0104  ld   [$04]
0034 8001  adda $01
0035 c204  st   [$04]
0036 60ff  xora $ff
0037 a059  suba $59
0038 c205  st   [$05]
0039 c000  st   $00,[$00]
003a 0001  ld   $01
003b c280  st   [$80]
003c 1401  ld   $01,y
003d 1000  ld   $00,x
003e 0008  ld   $08
003f de00  st   [y,x++]
0040 dc00  st   $00,[y,x++]
0041 8001  adda $01
0042 f440  bge  $40
0043 de00  st   [y,x++]
0044 1402  ld   $02,y
0045 0000  ld   $00
0046 c202  st   [$02]
0047 c218  st   [$18]
0048 8200  adda ac
0049 9200  adda ac,x
004a 0118  ld   [$18]
004b ce00  st   [y,x]
004c 8001  adda $01
004d 6040  xora $40
004e ec47  bne  $47
004f 6040  xora $40
0050 003c  ld   $3c
0051 c210  st   [$10]
0052 0000  ld   $00
0053 c211  st   [$11]
0054 000c  ld   $0c
0055 c212  st   [$12]
0056 1401  ld   $01,y
0057 10fa  ld   $fa,x
0058 dc38  st   $38,[y,x++]
0059 cc06  st   $06,[y,x]
005a 1402  ld   $02,y
005b 10fa  ld   $fa,x
005c dc70  st   $70,[y,x++]
005d cc0c  st   $0c,[y,x]
005e 1403  ld   $03,y
005f 10fa  ld   $fa,x
0060 dc10  st   $10,[y,x++]
0061 cc10  st   $10,[y,x]
0062 1404  ld   $04,y
0063 10fa  ld   $fa,x
0064 dc20  st   $20,[y,x++]
0065 cc13  st   $13,[y,x]
0066 0050  ld   $50
0067 c214  st   [$14]
0068 007f  ld   $7f
0069 c216  st   [$16]
006a 00ff  ld   $ff
006b c215  st   [$15]
006c c217  st   [$17]
006d c213  st   [$13]
006e 0007  ld   $07
006f 1880  ld   $80,out
0070 18c0  ld   $c0,out
0071 0077  ld   $77
0072 c20c  st   [$0c]
0073 0000  ld   $00
0074 c20d  st   [$0d]
0075 1404  ld   $04,y
0076 e000  jmp  y,$00
0077 000f  ld   $0f
0078 1880  ld   $80,out
0079 18c0  ld   $c0,out
007a c207  st   [$07]
007b 1401  ld   $01,y
007c e000  jmp  y,$00
007d 00c0  ld   $c0
007e 0000  ld   $00
007f 0000  ld   $00
0080 0000  ld   $00
* 130 times
0100 c209  st   [$09]
0101 0080  ld   $80
0102 c20a  st   [$0a]
0103 0110  ld   [$10]
0104 ec25  bne  $25
0105 0009  ld   $09
0106 8111  adda [$11]
0107 fe00  bra  ac
0108 fc19  bra  $19
0109 001f  ld   $1f
010a 002f  ld   $2f
010b 0037  ld   $37
010c 0043  ld   $43
010d 0051  ld   $51
010e 0062  ld   $62
010f 0074  ld   $74
0110 0088  ld   $88
0111 0094  ld   $94
0112 00a2  ld   $a2
0113 00b1  ld   $b1
0114 00c2  ld   $c2
0115 00d4  ld   $d4
0116 00e8  ld   $e8
0117 00fc  ld   $fc
0118 001e  ld   $1e
0119 c207  st   [$07]
011a 30f0  anda $f0,x
011b 1402  ld   $02,y
011c 0d00  ld   [y,x]
011d 1200  ld   ac,x
011e 0d00  ld   [y,x]
011f c211  st   [$11]
0120 0107  ld   [$07]
0121 200f  anda $0f
0122 c207  st   [$07]
0123 fc2b  bra  $2b
0124 0112  ld   [$12]
0125 0005  ld   $05
0126 ec26  bne  $26
0127 a001  suba $01
0128 0200  nop
0129 0110  ld   [$10]
012a a001  suba $01
012b c210  st   [$10]
012c 1401  ld   $01,y
012d 0901  ld   [y,$01]
012e 8001  adda $01
012f ca01  st   [y,$01]
0130 0001  ld   $01
0131 ec31  bne  $31
0132 a001  suba $01
0133 0917  ld   [y,$17]
0134 a001  suba $01
0135 ca17  st   [y,$17]
0136 1413  ld   $13,y
0137 104b  ld   $4b,x
0138 dc00  st   $00,[y,x++]
0139 0001  ld   $01
013a c218  st   [$18]
013b 2113  anda [$13]
013c f03f  beq  $3f
013d fc40  bra  $40
013e dc0c  st   $0c,[y,x++]
013f dc04  st   $04,[y,x++]
0140 0118  ld   [$18]
0141 f43a  bge  $3a
0142 8200  adda ac
0143 dc00  st   $00,[y,x++]
0144 1114  ld   [$14],x
0145 1516  ld   [$16],y
0146 cc00  st   $00,[y,x]
0147 0114  ld   [$14]
0148 8115  adda [$15]
0149 f44c  bge  $4c
014a fc4d  bra  $4d
014b 0000  ld   $00
014c 0001  ld   $01
014d c218  st   [$18]
014e 0114  ld   [$14]
014f 8115  adda [$15]
0150 a0a0  suba $a0
0151 e854  blt  $54
0152 fc55  bra  $55
0153 0118  ld   [$18]
0154 0001  ld   $01
0155 ec5a  bne  $5a
0156 0000  ld   $00
0157 a115  suba [$15]
0158 fc5d  bra  $5d
0159 c215  st   [$15]
015a 0200  nop
015b 0200  nop
015c 0200  nop
015d 0114  ld   [$14]
015e 9115  adda [$15],x
015f 0d00  ld   [y,x]
0160 ec66  bne  $66
0161 0000  ld   $00
0162 0114  ld   [$14]
0163 8115  adda [$15]
0164 fc6a  bra  $6a
0165 d214  st   [$14],x
0166 cc00  st   $00,[y,x]
0167 a115  suba [$15]
0168 c215  st   [$15]
0169 1114  ld   [$14],x
016a 0116  ld   [$16]
016b 8117  adda [$17]
016c 207f  anda $7f
016d a013  suba $13
016e f473  bge  $73
016f 0000  ld   $00
0170 a117  suba [$17]
0171 fc76  bra  $76
0172 c217  st   [$17]
0173 0200  nop
0174 0200  nop
0175 0200  nop
0176 0116  ld   [$16]
0177 9517  adda [$17],y
0178 0d00  ld   [y,x]
0179 ec7f  bne  $7f
017a 0000  ld   $00
017b 0116  ld   [$16]
017c 8117  adda [$17]
017d fc83  bra  $83
017e d616  st   [$16],y
017f cc00  st   $00,[y,x]
0180 a117  suba [$17]
0181 c217  st   [$17]
0182 1516  ld   [$16],y
0183 cc3f  st   $3f,[y,x]
0184 0005  ld   $05
0185 ec85  bne  $85
0186 a001  suba $01
0187 0200  nop
0188 0026  ld   $26
0189 c208  st   [$08]
018a 1909  ld   [$09],out
018b 0102  ld   [$02]
018c 2003  anda $03
018d 8001  adda $01
018e 190a  ld   [$0a],out
018f d602  st   [$02],y
0190 007f  ld   $7f
0191 29fe  anda [y,$fe]
0192 89fa  adda [y,$fa]
0193 cafe  st   [y,$fe]
0194 3080  anda $80,x
0195 0500  ld   [x]
0196 89ff  adda [y,$ff]
0197 89fb  adda [y,$fb]
0198 caff  st   [y,$ff]
0199 0200  nop
019a 0200  nop
019b 30fc  anda $fc,x
019c 1402  ld   $02,y
019d 0d00  ld   [y,x]
019e 8103  adda [$03]
019f c203  st   [$03]
01a0 0200  nop
01a1 0200  nop
01a2 0200  nop
01a3 0200  nop
01a4 0106  ld   [$06]
01a5 0200  nop
01a6 1909  ld   [$09],out
01a7 0108  ld   [$08]
01a8 f0d3  beq  $d3
01a9 a001  suba $01
01aa c208  st   [$08]
01ab a020  suba $20
01ac ecb1  bne  $b1
01ad a002  suba $02
01ae 00c0  ld   $c0
01af fcb6  bra  $b6
01b0 c209  st   [$09]
01b1 ecb5  bne  $b5
01b2 0040  ld   $40
01b3 fcb7  bra  $b7
01b4 c209  st   [$09]
01b5 0109  ld   [$09]
01b6 0200  nop
01b7 6040  xora $40
01b8 c20a  st   [$0a]
01b9 0108  ld   [$08]
01ba 6018  xora $18
01bb ecbe  bne  $be
01bc fcbf  bra  $bf
01bd c313  st   in,[$13]
01be 0200  nop
01bf 0108  ld   [$08]
01c0 2003  anda $03
01c1 eccd  bne  $cd
01c2 0103  ld   [$03]
01c3 20f0  anda $f0
01c4 4107  ora  [$07]
01c5 c206  st   [$06]
01c6 c003  st   $03,[$03]
01c7 0046  ld   $46
01c8 ecc8  bne  $c8
01c9 a001  suba $01
01ca 0200  nop
01cb fc8b  bra  $8b
01cc 1909  ld   [$09],out
01cd 0048  ld   $48
01ce ecce  bne  $ce
01cf a001  suba $01
01d0 0200  nop
01d1 fc8b  bra  $8b
01d2 1909  ld   [$09],out
01d3 0000  ld   $00
01d4 c209  st   [$09]
01d5 c20b  st   [$0b]
01d6 0050  ld   $50
01d7 ecd7  bne  $d7
01d8 a001  suba $01
01d9 0200  nop
01da 0102  ld   [$02]
01db 2003  anda $03
01dc 8001  adda $01
01dd 1402  ld   $02,y
01de e0b0  jmp  y,$b0
01df 1880  ld   $80,out
01e0 0000  ld   $00
01e1 0000  ld   $00
01e2 0000  ld   $00
* 32 times
0200 00c8  ld   $c8
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
020b 5d00  ora  [y,x++],out
020c 5d00  ora  [y,x++],out
020d 5d00  ora  [y,x++],out
* 160 times
02ab 18c0  ld   $c0,out
02ac 0102  ld   [$02]
02ad 2003  anda $03
02ae 8001  adda $01
02af 1880  ld   $80,out
02b0 d602  st   [$02],y
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
02c7 18c0  ld   $c0,out
02c8 00d3  ld   $d3
02c9 c20b  st   [$0b]
02ca 1401  ld   $01,y
02cb 0108  ld   [$08]
02cc 9001  adda $01,x
02cd 0109  ld   [$09]
02ce 8d00  adda [y,x]
02cf d209  st   [$09],x
02d0 150a  ld   [$0a],y
02d1 fc0b  bra  $0b
02d2 00c0  ld   $c0
02d3 0103  ld   [$03]
02d4 20f0  anda $f0
02d5 4107  ora  [$07]
02d6 c206  st   [$06]
02d7 c003  st   $03,[$03]
02d8 00de  ld   $de
02d9 c20b  st   [$0b]
02da 1109  ld   [$09],x
02db 150a  ld   [$0a],y
02dc fc0b  bra  $0b
02dd 00c0  ld   $c0
02de 1109  ld   [$09],x
02df 0108  ld   [$08]
02e0 a0ee  suba $ee
02e1 f0e9  beq  $e9
02e2 150a  ld   [$0a],y
02e3 80f0  adda $f0
02e4 c208  st   [$08]
02e5 0000  ld   $00
02e6 c20b  st   [$0b]
02e7 fc0b  bra  $0b
02e8 00c0  ld   $c0
02e9 0200  nop
02ea 0200  nop
02eb 00ef  ld   $ef
02ec c20b  st   [$0b]
02ed fc0b  bra  $0b
02ee 00c0  ld   $c0
02ef 1401  ld   $01,y
02f0 e000  jmp  y,$00
02f1 00c0  ld   $c0
02f2 0108  ld   [$08]
02f3 a0ee  suba $ee
02f4 ecf8  bne  $f8
02f5 80f0  adda $f0
02f6 fcfa  bra  $fa
02f7 00ef  ld   $ef
02f8 c208  st   [$08]
02f9 0000  ld   $00
02fa c20b  st   [$0b]
02fb 1403  ld   $03,y
02fc e000  jmp  y,$00
02fd 0200  nop
02fe 0000  ld   $00
02ff 0000  ld   $00
0300 004e  ld   $4e
0301 ec01  bne  $01
0302 a001  suba $01
0303 0200  nop
0304 1402  ld   $02,y
0305 e0ad  jmp  y,$ad
0306 0102  ld   [$02]
0307 0000  ld   $00
0308 0000  ld   $00
0309 0000  ld   $00
* 249 times
0400 1000  ld   $00,x
0401 1408  ld   $08,y
0402 dc14  st   $14,[y,x++]
0403 dc00  st   $00,[y,x++]
0404 dc14  st   $14,[y,x++]
0405 dc14  st   $14,[y,x++]
0406 dc04  st   $04,[y,x++]
0407 dc10  st   $10,[y,x++]
0408 dc04  st   $04,[y,x++]
0409 dc00  st   $00,[y,x++]
040a dc15  st   $15,[y,x++]
040b dc19  st   $19,[y,x++]
040c dc19  st   $19,[y,x++]
040d dc15  st   $15,[y,x++]
040e dc15  st   $15,[y,x++]
040f dc15  st   $15,[y,x++]
0410 dc19  st   $19,[y,x++]
0411 dc25  st   $25,[y,x++]
0412 dc15  st   $15,[y,x++]
0413 dc19  st   $19,[y,x++]
0414 dc15  st   $15,[y,x++]
0415 dc15  st   $15,[y,x++]
0416 dc19  st   $19,[y,x++]
0417 dc25  st   $25,[y,x++]
0418 dc14  st   $14,[y,x++]
0419 dc15  st   $15,[y,x++]
041a dc14  st   $14,[y,x++]
041b dc01  st   $01,[y,x++]
041c dc14  st   $14,[y,x++]
041d dc04  st   $04,[y,x++]
041e dc14  st   $14,[y,x++]
041f dc04  st   $04,[y,x++]
0420 dc14  st   $14,[y,x++]
0421 dc01  st   $01,[y,x++]
0422 dc14  st   $14,[y,x++]
0423 dc04  st   $04,[y,x++]
0424 dc14  st   $14,[y,x++]
0425 dc04  st   $04,[y,x++]
0426 dc14  st   $14,[y,x++]
0427 dc04  st   $04,[y,x++]
0428 dc04  st   $04,[y,x++]
0429 dc14  st   $14,[y,x++]
042a dc04  st   $04,[y,x++]
042b dc10  st   $10,[y,x++]
042c dc14  st   $14,[y,x++]
042d dc04  st   $04,[y,x++]
042e dc04  st   $04,[y,x++]
042f dc18  st   $18,[y,x++]
0430 dc05  st   $05,[y,x++]
0431 dc14  st   $14,[y,x++]
0432 dc04  st   $04,[y,x++]
0433 dc00  st   $00,[y,x++]
0434 dc14  st   $14,[y,x++]
0435 dc14  st   $14,[y,x++]
0436 dc04  st   $04,[y,x++]
0437 dc14  st   $14,[y,x++]
0438 dc00  st   $00,[y,x++]
0439 dc05  st   $05,[y,x++]
043a dc14  st   $14,[y,x++]
043b dc04  st   $04,[y,x++]
043c dc04  st   $04,[y,x++]
043d dc14  st   $14,[y,x++]
043e dc10  st   $10,[y,x++]
043f dc14  st   $14,[y,x++]
0440 dc00  st   $00,[y,x++]
0441 dc14  st   $14,[y,x++]
0442 dc04  st   $04,[y,x++]
0443 dc14  st   $14,[y,x++]
0444 dc04  st   $04,[y,x++]
0445 dc04  st   $04,[y,x++]
0446 dc14  st   $14,[y,x++]
0447 dc05  st   $05,[y,x++]
0448 dc04  st   $04,[y,x++]
0449 dc01  st   $01,[y,x++]
044a dc14  st   $14,[y,x++]
044b dc04  st   $04,[y,x++]
044c dc14  st   $14,[y,x++]
044d dc00  st   $00,[y,x++]
044e dc10  st   $10,[y,x++]
044f dc04  st   $04,[y,x++]
0450 dc14  st   $14,[y,x++]
0451 dc01  st   $01,[y,x++]
0452 dc14  st   $14,[y,x++]
0453 dc04  st   $04,[y,x++]
0454 dc00  st   $00,[y,x++]
0455 dc14  st   $14,[y,x++]
0456 dc05  st   $05,[y,x++]
0457 dc10  st   $10,[y,x++]
0458 dc04  st   $04,[y,x++]
0459 dc14  st   $14,[y,x++]
045a dc04  st   $04,[y,x++]
045b dc14  st   $14,[y,x++]
045c dc00  st   $00,[y,x++]
045d dc04  st   $04,[y,x++]
045e dc14  st   $14,[y,x++]
045f dc00  st   $00,[y,x++]
0460 dc00  st   $00,[y,x++]
0461 dc14  st   $14,[y,x++]
0462 dc04  st   $04,[y,x++]
0463 dc04  st   $04,[y,x++]
0464 dc14  st   $14,[y,x++]
0465 dc14  st   $14,[y,x++]
0466 dc14  st   $14,[y,x++]
0467 dc14  st   $14,[y,x++]
0468 dc00  st   $00,[y,x++]
0469 dc04  st   $04,[y,x++]
046a dc14  st   $14,[y,x++]
046b dc00  st   $00,[y,x++]
046c dc04  st   $04,[y,x++]
046d dc10  st   $10,[y,x++]
046e dc04  st   $04,[y,x++]
046f dc04  st   $04,[y,x++]
0470 dc10  st   $10,[y,x++]
0471 dc15  st   $15,[y,x++]
0472 dc14  st   $14,[y,x++]
0473 dc04  st   $04,[y,x++]
0474 dc14  st   $14,[y,x++]
0475 dc00  st   $00,[y,x++]
0476 dc15  st   $15,[y,x++]
0477 dc04  st   $04,[y,x++]
0478 dc14  st   $14,[y,x++]
0479 dc05  st   $05,[y,x++]
047a dc14  st   $14,[y,x++]
047b dc15  st   $15,[y,x++]
047c dc19  st   $19,[y,x++]
047d dc19  st   $19,[y,x++]
047e dc14  st   $14,[y,x++]
047f dc04  st   $04,[y,x++]
0480 dc15  st   $15,[y,x++]
0481 dc15  st   $15,[y,x++]
0482 dc14  st   $14,[y,x++]
0483 dc04  st   $04,[y,x++]
0484 dc14  st   $14,[y,x++]
0485 dc04  st   $04,[y,x++]
0486 dc15  st   $15,[y,x++]
0487 dc14  st   $14,[y,x++]
0488 dc14  st   $14,[y,x++]
0489 dc05  st   $05,[y,x++]
048a dc14  st   $14,[y,x++]
048b dc04  st   $04,[y,x++]
048c dc14  st   $14,[y,x++]
048d dc04  st   $04,[y,x++]
048e dc15  st   $15,[y,x++]
048f dc15  st   $15,[y,x++]
0490 dc14  st   $14,[y,x++]
0491 dc00  st   $00,[y,x++]
0492 dc14  st   $14,[y,x++]
0493 dc04  st   $04,[y,x++]
0494 dc10  st   $10,[y,x++]
0495 dc04  st   $04,[y,x++]
0496 dc04  st   $04,[y,x++]
0497 dc14  st   $14,[y,x++]
0498 dc00  st   $00,[y,x++]
0499 dc04  st   $04,[y,x++]
049a dc14  st   $14,[y,x++]
049b dc00  st   $00,[y,x++]
049c dc04  st   $04,[y,x++]
049d dc14  st   $14,[y,x++]
049e dc15  st   $15,[y,x++]
049f dc19  st   $19,[y,x++]
04a0 dc29  st   $29,[y,x++]
04a1 dc15  st   $15,[y,x++]
04a2 dc14  st   $14,[y,x++]
04a3 dc14  st   $14,[y,x++]
04a4 dc04  st   $04,[y,x++]
04a5 dc15  st   $15,[y,x++]
04a6 dc15  st   $15,[y,x++]
04a7 dc29  st   $29,[y,x++]
04a8 dc15  st   $15,[y,x++]
04a9 dc14  st   $14,[y,x++]
04aa dc04  st   $04,[y,x++]
04ab dc15  st   $15,[y,x++]
04ac dc15  st   $15,[y,x++]
04ad dc04  st   $04,[y,x++]
04ae dc14  st   $14,[y,x++]
04af dc04  st   $04,[y,x++]
04b0 dc15  st   $15,[y,x++]
04b1 dc15  st   $15,[y,x++]
04b2 dc04  st   $04,[y,x++]
04b3 dc14  st   $14,[y,x++]
04b4 dc04  st   $04,[y,x++]
04b5 dc05  st   $05,[y,x++]
04b6 dc10  st   $10,[y,x++]
04b7 dc04  st   $04,[y,x++]
04b8 dc10  st   $10,[y,x++]
04b9 dc04  st   $04,[y,x++]
04ba dc04  st   $04,[y,x++]
04bb dc15  st   $15,[y,x++]
04bc dc15  st   $15,[y,x++]
04bd dc14  st   $14,[y,x++]
04be dc14  st   $14,[y,x++]
04bf dc00  st   $00,[y,x++]
04c0 dc14  st   $14,[y,x++]
04c1 dc11  st   $11,[y,x++]
04c2 dc14  st   $14,[y,x++]
04c3 dc15  st   $15,[y,x++]
04c4 dc29  st   $29,[y,x++]
04c5 dc04  st   $04,[y,x++]
04c6 dc04  st   $04,[y,x++]
04c7 dc14  st   $14,[y,x++]
04c8 dc15  st   $15,[y,x++]
04c9 dc29  st   $29,[y,x++]
04ca dc15  st   $15,[y,x++]
04cb dc04  st   $04,[y,x++]
04cc dc14  st   $14,[y,x++]
04cd dc04  st   $04,[y,x++]
04ce dc14  st   $14,[y,x++]
04cf dc14  st   $14,[y,x++]
04d0 dc04  st   $04,[y,x++]
04d1 dc14  st   $14,[y,x++]
04d2 dc04  st   $04,[y,x++]
04d3 dc10  st   $10,[y,x++]
04d4 dc04  st   $04,[y,x++]
04d5 dc14  st   $14,[y,x++]
04d6 dc15  st   $15,[y,x++]
04d7 dc14  st   $14,[y,x++]
04d8 dc04  st   $04,[y,x++]
04d9 dc14  st   $14,[y,x++]
04da dc15  st   $15,[y,x++]
04db dc15  st   $15,[y,x++]
04dc dc29  st   $29,[y,x++]
04dd dc19  st   $19,[y,x++]
04de dc14  st   $14,[y,x++]
04df dc14  st   $14,[y,x++]
04e0 dc04  st   $04,[y,x++]
04e1 dc15  st   $15,[y,x++]
04e2 dc29  st   $29,[y,x++]
04e3 dc19  st   $19,[y,x++]
04e4 dc15  st   $15,[y,x++]
04e5 dc14  st   $14,[y,x++]
04e6 dc04  st   $04,[y,x++]
04e7 dc00  st   $00,[y,x++]
04e8 dc14  st   $14,[y,x++]
04e9 dc14  st   $14,[y,x++]
04ea dc04  st   $04,[y,x++]
04eb dc14  st   $14,[y,x++]
04ec dc04  st   $04,[y,x++]
04ed dc00  st   $00,[y,x++]
04ee dc14  st   $14,[y,x++]
04ef dc15  st   $15,[y,x++]
04f0 dc19  st   $19,[y,x++]
04f1 dc25  st   $25,[y,x++]
04f2 dc05  st   $05,[y,x++]
04f3 dc14  st   $14,[y,x++]
04f4 dc04  st   $04,[y,x++]
04f5 dc14  st   $14,[y,x++]
04f6 dc14  st   $14,[y,x++]
04f7 dc15  st   $15,[y,x++]
04f8 dc14  st   $14,[y,x++]
04f9 dc04  st   $04,[y,x++]
04fa dc14  st   $14,[y,x++]
04fb dc04  st   $04,[y,x++]
04fc dc04  st   $04,[y,x++]
04fd dc14  st   $14,[y,x++]
04fe dc04  st   $04,[y,x++]
04ff dc14  st   $14,[y,x++]
0500 dc04  st   $04,[y,x++]
0501 dc04  st   $04,[y,x++]
0502 1409  ld   $09,y
0503 dc04  st   $04,[y,x++]
0504 dc14  st   $14,[y,x++]
0505 dc04  st   $04,[y,x++]
0506 dc14  st   $14,[y,x++]
0507 dc14  st   $14,[y,x++]
0508 dc14  st   $14,[y,x++]
* 5 times
050b dc15  st   $15,[y,x++]
050c dc3f  st   $3f,[y,x++]
050d dc2a  st   $2a,[y,x++]
050e dc3f  st   $3f,[y,x++]
050f dc3f  st   $3f,[y,x++]
0510 dc3f  st   $3f,[y,x++]
0511 dc2f  st   $2f,[y,x++]
0512 dc3e  st   $3e,[y,x++]
0513 dc2e  st   $2e,[y,x++]
0514 dc3b  st   $3b,[y,x++]
0515 dc3f  st   $3f,[y,x++]
0516 dc3f  st   $3f,[y,x++]
0517 dc2f  st   $2f,[y,x++]
0518 dc3e  st   $3e,[y,x++]
0519 dc19  st   $19,[y,x++]
051a dc3a  st   $3a,[y,x++]
051b dc29  st   $29,[y,x++]
051c dc14  st   $14,[y,x++]
051d dc10  st   $10,[y,x++]
051e dc14  st   $14,[y,x++]
051f dc00  st   $00,[y,x++]
0520 dc14  st   $14,[y,x++]
0521 dc14  st   $14,[y,x++]
0522 dc14  st   $14,[y,x++]
0523 dc10  st   $10,[y,x++]
0524 dc14  st   $14,[y,x++]
0525 dc04  st   $04,[y,x++]
0526 dc14  st   $14,[y,x++]
0527 dc10  st   $10,[y,x++]
0528 dc15  st   $15,[y,x++]
0529 dc14  st   $14,[y,x++]
052a dc10  st   $10,[y,x++]
052b dc04  st   $04,[y,x++]
052c dc14  st   $14,[y,x++]
052d dc14  st   $14,[y,x++]
052e dc15  st   $15,[y,x++]
052f dc14  st   $14,[y,x++]
0530 dc09  st   $09,[y,x++]
0531 dc14  st   $14,[y,x++]
0532 dc14  st   $14,[y,x++]
0533 dc14  st   $14,[y,x++]
0534 dc14  st   $14,[y,x++]
0535 dc05  st   $05,[y,x++]
0536 dc14  st   $14,[y,x++]
0537 dc14  st   $14,[y,x++]
0538 dc10  st   $10,[y,x++]
0539 dc14  st   $14,[y,x++]
053a dc14  st   $14,[y,x++]
053b dc14  st   $14,[y,x++]
053c dc10  st   $10,[y,x++]
053d dc14  st   $14,[y,x++]
053e dc04  st   $04,[y,x++]
053f dc15  st   $15,[y,x++]
0540 dc04  st   $04,[y,x++]
0541 dc04  st   $04,[y,x++]
0542 dc14  st   $14,[y,x++]
0543 dc10  st   $10,[y,x++]
0544 dc14  st   $14,[y,x++]
0545 dc14  st   $14,[y,x++]
0546 dc10  st   $10,[y,x++]
0547 dc10  st   $10,[y,x++]
0548 dc14  st   $14,[y,x++]
0549 dc14  st   $14,[y,x++]
054a dc14  st   $14,[y,x++]
* 7 times
054f dc04  st   $04,[y,x++]
0550 dc14  st   $14,[y,x++]
0551 dc14  st   $14,[y,x++]
0552 dc14  st   $14,[y,x++]
0553 dc10  st   $10,[y,x++]
0554 dc14  st   $14,[y,x++]
0555 dc14  st   $14,[y,x++]
0556 dc14  st   $14,[y,x++]
* 6 times
055a dc04  st   $04,[y,x++]
055b dc14  st   $14,[y,x++]
055c dc14  st   $14,[y,x++]
055d dc14  st   $14,[y,x++]
055e dc15  st   $15,[y,x++]
055f dc14  st   $14,[y,x++]
0560 dc14  st   $14,[y,x++]
0561 dc14  st   $14,[y,x++]
* 5 times
0564 dc26  st   $26,[y,x++]
0565 dc2a  st   $2a,[y,x++]
0566 dc04  st   $04,[y,x++]
0567 dc05  st   $05,[y,x++]
0568 dc14  st   $14,[y,x++]
0569 dc14  st   $14,[y,x++]
056a dc14  st   $14,[y,x++]
056b dc15  st   $15,[y,x++]
056c dc14  st   $14,[y,x++]
056d dc14  st   $14,[y,x++]
056e dc14  st   $14,[y,x++]
* 5 times
0571 dc04  st   $04,[y,x++]
0572 dc14  st   $14,[y,x++]
0573 dc00  st   $00,[y,x++]
0574 dc15  st   $15,[y,x++]
0575 dc14  st   $14,[y,x++]
0576 dc14  st   $14,[y,x++]
0577 dc14  st   $14,[y,x++]
0578 dc14  st   $14,[y,x++]
0579 dc04  st   $04,[y,x++]
057a dc14  st   $14,[y,x++]
057b dc25  st   $25,[y,x++]
057c dc3f  st   $3f,[y,x++]
057d dc3f  st   $3f,[y,x++]
057e dc3a  st   $3a,[y,x++]
057f dc15  st   $15,[y,x++]
0580 dc11  st   $11,[y,x++]
0581 dc3e  st   $3e,[y,x++]
0582 dc3f  st   $3f,[y,x++]
0583 dc29  st   $29,[y,x++]
0584 dc14  st   $14,[y,x++]
0585 dc14  st   $14,[y,x++]
0586 dc15  st   $15,[y,x++]
0587 dc3f  st   $3f,[y,x++]
0588 dc3f  st   $3f,[y,x++]
0589 dc14  st   $14,[y,x++]
058a dc29  st   $29,[y,x++]
058b dc3b  st   $3b,[y,x++]
058c dc14  st   $14,[y,x++]
058d dc14  st   $14,[y,x++]
058e dc14  st   $14,[y,x++]
058f dc15  st   $15,[y,x++]
0590 dc3f  st   $3f,[y,x++]
0591 dc14  st   $14,[y,x++]
0592 dc14  st   $14,[y,x++]
0593 dc14  st   $14,[y,x++]
0594 dc15  st   $15,[y,x++]
0595 dc14  st   $14,[y,x++]
0596 dc14  st   $14,[y,x++]
0597 dc14  st   $14,[y,x++]
* 5 times
059a dc15  st   $15,[y,x++]
059b dc14  st   $14,[y,x++]
059c dc14  st   $14,[y,x++]
059d dc10  st   $10,[y,x++]
059e dc14  st   $14,[y,x++]
059f dc2a  st   $2a,[y,x++]
05a0 dc3f  st   $3f,[y,x++]
05a1 dc2a  st   $2a,[y,x++]
05a2 dc3f  st   $3f,[y,x++]
05a3 dc2b  st   $2b,[y,x++]
05a4 dc14  st   $14,[y,x++]
05a5 dc14  st   $14,[y,x++]
05a6 dc3f  st   $3f,[y,x++]
05a7 dc3f  st   $3f,[y,x++]
05a8 dc2a  st   $2a,[y,x++]
05a9 dc3f  st   $3f,[y,x++]
05aa dc2a  st   $2a,[y,x++]
05ab dc14  st   $14,[y,x++]
05ac dc29  st   $29,[y,x++]
05ad dc3f  st   $3f,[y,x++]
05ae dc10  st   $10,[y,x++]
05af dc14  st   $14,[y,x++]
05b0 dc15  st   $15,[y,x++]
05b1 dc3f  st   $3f,[y,x++]
05b2 dc25  st   $25,[y,x++]
05b3 dc14  st   $14,[y,x++]
05b4 dc14  st   $14,[y,x++]
05b5 dc14  st   $14,[y,x++]
* 9 times
05bc dc25  st   $25,[y,x++]
05bd dc3f  st   $3f,[y,x++]
05be dc19  st   $19,[y,x++]
05bf dc14  st   $14,[y,x++]
05c0 dc14  st   $14,[y,x++]
05c1 dc04  st   $04,[y,x++]
05c2 dc29  st   $29,[y,x++]
05c3 dc3f  st   $3f,[y,x++]
05c4 dc2f  st   $2f,[y,x++]
05c5 dc3e  st   $3e,[y,x++]
05c6 dc15  st   $15,[y,x++]
05c7 dc14  st   $14,[y,x++]
05c8 dc29  st   $29,[y,x++]
05c9 dc3f  st   $3f,[y,x++]
05ca dc2a  st   $2a,[y,x++]
05cb dc3f  st   $3f,[y,x++]
05cc dc2a  st   $2a,[y,x++]
05cd dc14  st   $14,[y,x++]
05ce dc14  st   $14,[y,x++]
05cf dc10  st   $10,[y,x++]
05d0 dc04  st   $04,[y,x++]
05d1 dc11  st   $11,[y,x++]
05d2 dc14  st   $14,[y,x++]
05d3 dc14  st   $14,[y,x++]
05d4 dc14  st   $14,[y,x++]
05d5 dc15  st   $15,[y,x++]
05d6 dc2a  st   $2a,[y,x++]
05d7 dc2f  st   $2f,[y,x++]
05d8 dc14  st   $14,[y,x++]
05d9 dc10  st   $10,[y,x++]
05da dc14  st   $14,[y,x++]
05db dc2a  st   $2a,[y,x++]
05dc dc3f  st   $3f,[y,x++]
05dd dc2a  st   $2a,[y,x++]
05de dc3f  st   $3f,[y,x++]
05df dc2a  st   $2a,[y,x++]
05e0 dc14  st   $14,[y,x++]
05e1 dc14  st   $14,[y,x++]
05e2 dc2a  st   $2a,[y,x++]
05e3 dc3f  st   $3f,[y,x++]
05e4 dc2a  st   $2a,[y,x++]
05e5 dc3f  st   $3f,[y,x++]
05e6 dc15  st   $15,[y,x++]
05e7 dc14  st   $14,[y,x++]
05e8 dc14  st   $14,[y,x++]
05e9 dc14  st   $14,[y,x++]
05ea dc05  st   $05,[y,x++]
05eb dc10  st   $10,[y,x++]
05ec dc14  st   $14,[y,x++]
05ed dc14  st   $14,[y,x++]
05ee dc15  st   $15,[y,x++]
05ef dc2a  st   $2a,[y,x++]
05f0 dc3f  st   $3f,[y,x++]
05f1 dc3a  st   $3a,[y,x++]
05f2 dc2a  st   $2a,[y,x++]
05f3 dc14  st   $14,[y,x++]
05f4 dc14  st   $14,[y,x++]
05f5 dc11  st   $11,[y,x++]
05f6 dc14  st   $14,[y,x++]
05f7 dc2a  st   $2a,[y,x++]
05f8 dc3f  st   $3f,[y,x++]
05f9 dc15  st   $15,[y,x++]
05fa dc14  st   $14,[y,x++]
05fb dc14  st   $14,[y,x++]
05fc dc14  st   $14,[y,x++]
05fd dc10  st   $10,[y,x++]
05fe dc14  st   $14,[y,x++]
05ff dc11  st   $11,[y,x++]
0600 dc14  st   $14,[y,x++]
0601 dc10  st   $10,[y,x++]
0602 dc14  st   $14,[y,x++]
0603 140a  ld   $0a,y
0604 dc04  st   $04,[y,x++]
0605 dc10  st   $10,[y,x++]
0606 dc11  st   $11,[y,x++]
0607 dc14  st   $14,[y,x++]
0608 dc11  st   $11,[y,x++]
0609 dc04  st   $04,[y,x++]
060a dc14  st   $14,[y,x++]
060b dc00  st   $00,[y,x++]
060c dc14  st   $14,[y,x++]
060d dc14  st   $14,[y,x++]
060e dc14  st   $14,[y,x++]
060f dc2a  st   $2a,[y,x++]
0610 dc2e  st   $2e,[y,x++]
0611 dc15  st   $15,[y,x++]
0612 dc14  st   $14,[y,x++]
0613 dc14  st   $14,[y,x++]
0614 dc15  st   $15,[y,x++]
0615 dc14  st   $14,[y,x++]
0616 dc3f  st   $3f,[y,x++]
0617 dc19  st   $19,[y,x++]
0618 dc14  st   $14,[y,x++]
0619 dc14  st   $14,[y,x++]
061a dc15  st   $15,[y,x++]
061b dc2f  st   $2f,[y,x++]
061c dc19  st   $19,[y,x++]
061d dc11  st   $11,[y,x++]
061e dc14  st   $14,[y,x++]
061f dc04  st   $04,[y,x++]
0620 dc15  st   $15,[y,x++]
0621 dc14  st   $14,[y,x++]
0622 dc14  st   $14,[y,x++]
0623 dc14  st   $14,[y,x++]
0624 dc14  st   $14,[y,x++]
0625 dc15  st   $15,[y,x++]
0626 dc15  st   $15,[y,x++]
0627 dc15  st   $15,[y,x++]
0628 dc04  st   $04,[y,x++]
0629 dc14  st   $14,[y,x++]
062a dc04  st   $04,[y,x++]
062b dc11  st   $11,[y,x++]
062c dc15  st   $15,[y,x++]
062d dc11  st   $11,[y,x++]
062e dc04  st   $04,[y,x++]
062f dc14  st   $14,[y,x++]
0630 dc04  st   $04,[y,x++]
0631 dc19  st   $19,[y,x++]
0632 dc14  st   $14,[y,x++]
0633 dc14  st   $14,[y,x++]
0634 dc14  st   $14,[y,x++]
0635 dc14  st   $14,[y,x++]
0636 dc24  st   $24,[y,x++]
0637 dc14  st   $14,[y,x++]
0638 dc14  st   $14,[y,x++]
0639 dc14  st   $14,[y,x++]
063a dc15  st   $15,[y,x++]
063b dc14  st   $14,[y,x++]
063c dc15  st   $15,[y,x++]
063d dc15  st   $15,[y,x++]
063e dc14  st   $14,[y,x++]
063f dc10  st   $10,[y,x++]
0640 dc14  st   $14,[y,x++]
0641 dc14  st   $14,[y,x++]
0642 dc14  st   $14,[y,x++]
0643 dc15  st   $15,[y,x++]
0644 dc25  st   $25,[y,x++]
0645 dc19  st   $19,[y,x++]
0646 dc14  st   $14,[y,x++]
0647 dc14  st   $14,[y,x++]
0648 dc14  st   $14,[y,x++]
0649 dc25  st   $25,[y,x++]
064a dc29  st   $29,[y,x++]
064b dc14  st   $14,[y,x++]
064c dc14  st   $14,[y,x++]
064d dc14  st   $14,[y,x++]
064e dc15  st   $15,[y,x++]
064f dc15  st   $15,[y,x++]
0650 dc15  st   $15,[y,x++]
0651 dc29  st   $29,[y,x++]
0652 dc14  st   $14,[y,x++]
0653 dc14  st   $14,[y,x++]
0654 dc19  st   $19,[y,x++]
0655 dc15  st   $15,[y,x++]
0656 dc14  st   $14,[y,x++]
0657 dc14  st   $14,[y,x++]
0658 dc14  st   $14,[y,x++]
0659 dc14  st   $14,[y,x++]
065a dc15  st   $15,[y,x++]
065b dc25  st   $25,[y,x++]
065c dc14  st   $14,[y,x++]
065d dc14  st   $14,[y,x++]
065e dc14  st   $14,[y,x++]
065f dc15  st   $15,[y,x++]
0660 dc14  st   $14,[y,x++]
0661 dc14  st   $14,[y,x++]
0662 dc15  st   $15,[y,x++]
0663 dc14  st   $14,[y,x++]
0664 dc14  st   $14,[y,x++]
0665 dc2a  st   $2a,[y,x++]
0666 dc3f  st   $3f,[y,x++]
0667 dc15  st   $15,[y,x++]
0668 dc14  st   $14,[y,x++]
0669 dc14  st   $14,[y,x++]
066a dc15  st   $15,[y,x++]
066b dc15  st   $15,[y,x++]
066c dc15  st   $15,[y,x++]
066d dc15  st   $15,[y,x++]
066e dc14  st   $14,[y,x++]
066f dc14  st   $14,[y,x++]
0670 dc15  st   $15,[y,x++]
0671 dc14  st   $14,[y,x++]
0672 dc15  st   $15,[y,x++]
0673 dc14  st   $14,[y,x++]
0674 dc10  st   $10,[y,x++]
0675 dc14  st   $14,[y,x++]
0676 dc14  st   $14,[y,x++]
0677 dc14  st   $14,[y,x++]
0678 dc10  st   $10,[y,x++]
0679 dc14  st   $14,[y,x++]
067a dc10  st   $10,[y,x++]
067b dc15  st   $15,[y,x++]
067c dc3f  st   $3f,[y,x++]
067d dc29  st   $29,[y,x++]
067e dc14  st   $14,[y,x++]
067f dc00  st   $00,[y,x++]
0680 dc14  st   $14,[y,x++]
0681 dc04  st   $04,[y,x++]
0682 dc2a  st   $2a,[y,x++]
0683 dc3f  st   $3f,[y,x++]
0684 dc2b  st   $2b,[y,x++]
0685 dc14  st   $14,[y,x++]
0686 dc14  st   $14,[y,x++]
0687 dc2a  st   $2a,[y,x++]
0688 dc3f  st   $3f,[y,x++]
0689 dc3e  st   $3e,[y,x++]
068a dc14  st   $14,[y,x++]
068b dc2a  st   $2a,[y,x++]
068c dc3f  st   $3f,[y,x++]
068d dc04  st   $04,[y,x++]
068e dc10  st   $10,[y,x++]
068f dc14  st   $14,[y,x++]
0690 dc14  st   $14,[y,x++]
0691 dc3f  st   $3f,[y,x++]
0692 dc14  st   $14,[y,x++]
0693 dc15  st   $15,[y,x++]
0694 dc15  st   $15,[y,x++]
0695 dc15  st   $15,[y,x++]
* 5 times
0698 dc00  st   $00,[y,x++]
0699 dc14  st   $14,[y,x++]
069a dc10  st   $10,[y,x++]
069b dc04  st   $04,[y,x++]
069c dc14  st   $14,[y,x++]
069d dc14  st   $14,[y,x++]
069e dc14  st   $14,[y,x++]
069f dc04  st   $04,[y,x++]
06a0 dc15  st   $15,[y,x++]
06a1 dc15  st   $15,[y,x++]
06a2 dc14  st   $14,[y,x++]
06a3 dc15  st   $15,[y,x++]
06a4 dc3f  st   $3f,[y,x++]
06a5 dc15  st   $15,[y,x++]
06a6 dc14  st   $14,[y,x++]
06a7 dc15  st   $15,[y,x++]
06a8 dc14  st   $14,[y,x++]
06a9 dc14  st   $14,[y,x++]
06aa dc2a  st   $2a,[y,x++]
06ab dc3f  st   $3f,[y,x++]
06ac dc14  st   $14,[y,x++]
06ad dc29  st   $29,[y,x++]
06ae dc2e  st   $2e,[y,x++]
06af dc10  st   $10,[y,x++]
06b0 dc29  st   $29,[y,x++]
06b1 dc3f  st   $3f,[y,x++]
06b2 dc25  st   $25,[y,x++]
06b3 dc14  st   $14,[y,x++]
06b4 dc14  st   $14,[y,x++]
06b5 dc14  st   $14,[y,x++]
06b6 dc14  st   $14,[y,x++]
06b7 dc10  st   $10,[y,x++]
06b8 dc14  st   $14,[y,x++]
06b9 dc00  st   $00,[y,x++]
06ba dc14  st   $14,[y,x++]
06bb dc11  st   $11,[y,x++]
06bc dc15  st   $15,[y,x++]
06bd dc2e  st   $2e,[y,x++]
06be dc3f  st   $3f,[y,x++]
06bf dc19  st   $19,[y,x++]
06c0 dc14  st   $14,[y,x++]
06c1 dc14  st   $14,[y,x++]
06c2 dc15  st   $15,[y,x++]
06c3 dc3f  st   $3f,[y,x++]
06c4 dc19  st   $19,[y,x++]
06c5 dc14  st   $14,[y,x++]
06c6 dc14  st   $14,[y,x++]
06c7 dc10  st   $10,[y,x++]
06c8 dc14  st   $14,[y,x++]
06c9 dc3f  st   $3f,[y,x++]
06ca dc2a  st   $2a,[y,x++]
06cb dc14  st   $14,[y,x++]
06cc dc29  st   $29,[y,x++]
06cd dc3f  st   $3f,[y,x++]
06ce dc14  st   $14,[y,x++]
06cf dc15  st   $15,[y,x++]
06d0 dc15  st   $15,[y,x++]
06d1 dc14  st   $14,[y,x++]
06d2 dc04  st   $04,[y,x++]
06d3 dc14  st   $14,[y,x++]
06d4 dc15  st   $15,[y,x++]
06d5 dc14  st   $14,[y,x++]
06d6 dc2a  st   $2a,[y,x++]
06d7 dc3f  st   $3f,[y,x++]
06d8 dc3a  st   $3a,[y,x++]
06d9 dc14  st   $14,[y,x++]
06da dc14  st   $14,[y,x++]
06db dc04  st   $04,[y,x++]
06dc dc15  st   $15,[y,x++]
06dd dc15  st   $15,[y,x++]
06de dc14  st   $14,[y,x++]
06df dc25  st   $25,[y,x++]
06e0 dc3f  st   $3f,[y,x++]
06e1 dc15  st   $15,[y,x++]
06e2 dc15  st   $15,[y,x++]
06e3 dc3f  st   $3f,[y,x++]
06e4 dc15  st   $15,[y,x++]
06e5 dc14  st   $14,[y,x++]
06e6 dc2a  st   $2a,[y,x++]
06e7 dc3e  st   $3e,[y,x++]
06e8 dc15  st   $15,[y,x++]
06e9 dc14  st   $14,[y,x++]
06ea dc14  st   $14,[y,x++]
06eb dc10  st   $10,[y,x++]
06ec dc14  st   $14,[y,x++]
06ed dc15  st   $15,[y,x++]
06ee dc14  st   $14,[y,x++]
06ef dc29  st   $29,[y,x++]
06f0 dc3f  st   $3f,[y,x++]
06f1 dc15  st   $15,[y,x++]
06f2 dc14  st   $14,[y,x++]
06f3 dc14  st   $14,[y,x++]
06f4 dc04  st   $04,[y,x++]
06f5 dc14  st   $14,[y,x++]
06f6 dc04  st   $04,[y,x++]
06f7 dc2a  st   $2a,[y,x++]
06f8 dc3f  st   $3f,[y,x++]
06f9 dc3f  st   $3f,[y,x++]
06fa dc15  st   $15,[y,x++]
06fb dc10  st   $10,[y,x++]
06fc dc10  st   $10,[y,x++]
06fd dc04  st   $04,[y,x++]
06fe dc15  st   $15,[y,x++]
06ff dc04  st   $04,[y,x++]
0700 dc14  st   $14,[y,x++]
0701 dc14  st   $14,[y,x++]
0702 dc14  st   $14,[y,x++]
0703 dc14  st   $14,[y,x++]
0704 140b  ld   $0b,y
0705 dc14  st   $14,[y,x++]
0706 dc14  st   $14,[y,x++]
0707 dc04  st   $04,[y,x++]
0708 dc14  st   $14,[y,x++]
0709 dc14  st   $14,[y,x++]
070a dc14  st   $14,[y,x++]
* 6 times
070e dc10  st   $10,[y,x++]
070f dc14  st   $14,[y,x++]
0710 dc15  st   $15,[y,x++]
0711 dc3b  st   $3b,[y,x++]
0712 dc14  st   $14,[y,x++]
0713 dc10  st   $10,[y,x++]
0714 dc14  st   $14,[y,x++]
0715 dc14  st   $14,[y,x++]
0716 dc14  st   $14,[y,x++]
0717 dc2a  st   $2a,[y,x++]
0718 dc25  st   $25,[y,x++]
0719 dc10  st   $10,[y,x++]
071a dc14  st   $14,[y,x++]
071b dc14  st   $14,[y,x++]
071c dc3f  st   $3f,[y,x++]
071d dc15  st   $15,[y,x++]
071e dc14  st   $14,[y,x++]
071f dc14  st   $14,[y,x++]
0720 dc14  st   $14,[y,x++]
0721 dc00  st   $00,[y,x++]
0722 dc14  st   $14,[y,x++]
0723 dc14  st   $14,[y,x++]
0724 dc00  st   $00,[y,x++]
0725 dc15  st   $15,[y,x++]
0726 dc0a  st   $0a,[y,x++]
0727 dc0b  st   $0b,[y,x++]
0728 dc16  st   $16,[y,x++]
0729 dc17  st   $17,[y,x++]
072a dc15  st   $15,[y,x++]
072b dc15  st   $15,[y,x++]
072c dc03  st   $03,[y,x++]
072d dc02  st   $02,[y,x++]
072e dc13  st   $13,[y,x++]
072f dc03  st   $03,[y,x++]
0730 dc14  st   $14,[y,x++]
0731 dc18  st   $18,[y,x++]
0732 dc08  st   $08,[y,x++]
0733 dc04  st   $04,[y,x++]
0734 dc14  st   $14,[y,x++]
0735 dc39  st   $39,[y,x++]
0736 dc38  st   $38,[y,x++]
0737 dc29  st   $29,[y,x++]
0738 dc39  st   $39,[y,x++]
0739 dc15  st   $15,[y,x++]
073a dc14  st   $14,[y,x++]
073b dc0e  st   $0e,[y,x++]
073c dc0e  st   $0e,[y,x++]
073d dc0e  st   $0e,[y,x++]
073e dc0a  st   $0a,[y,x++]
073f dc14  st   $14,[y,x++]
0740 dc14  st   $14,[y,x++]
0741 dc04  st   $04,[y,x++]
0742 dc10  st   $10,[y,x++]
0743 dc2a  st   $2a,[y,x++]
0744 dc3f  st   $3f,[y,x++]
0745 dc2f  st   $2f,[y,x++]
0746 dc3f  st   $3f,[y,x++]
0747 dc2a  st   $2a,[y,x++]
0748 dc14  st   $14,[y,x++]
0749 dc3f  st   $3f,[y,x++]
074a dc2f  st   $2f,[y,x++]
074b dc2b  st   $2b,[y,x++]
074c dc3e  st   $3e,[y,x++]
074d dc15  st   $15,[y,x++]
074e dc14  st   $14,[y,x++]
074f dc2a  st   $2a,[y,x++]
0750 dc3f  st   $3f,[y,x++]
0751 dc3f  st   $3f,[y,x++]
0752 dc3f  st   $3f,[y,x++]
0753 dc2a  st   $2a,[y,x++]
0754 dc3f  st   $3f,[y,x++]
0755 dc3f  st   $3f,[y,x++]
0756 dc3f  st   $3f,[y,x++]
0757 dc15  st   $15,[y,x++]
0758 dc14  st   $14,[y,x++]
0759 dc2a  st   $2a,[y,x++]
075a dc3f  st   $3f,[y,x++]
075b dc3f  st   $3f,[y,x++]
075c dc3f  st   $3f,[y,x++]
075d dc3f  st   $3f,[y,x++]
075e dc15  st   $15,[y,x++]
075f dc25  st   $25,[y,x++]
0760 dc2e  st   $2e,[y,x++]
0761 dc14  st   $14,[y,x++]
0762 dc00  st   $00,[y,x++]
0763 dc15  st   $15,[y,x++]
0764 dc3f  st   $3f,[y,x++]
0765 dc15  st   $15,[y,x++]
0766 dc3f  st   $3f,[y,x++]
0767 dc3f  st   $3f,[y,x++]
0768 dc3a  st   $3a,[y,x++]
0769 dc2a  st   $2a,[y,x++]
076a dc15  st   $15,[y,x++]
076b dc3e  st   $3e,[y,x++]
076c dc3e  st   $3e,[y,x++]
076d dc3e  st   $3e,[y,x++]
076e dc3f  st   $3f,[y,x++]
076f dc15  st   $15,[y,x++]
0770 dc15  st   $15,[y,x++]
0771 dc3f  st   $3f,[y,x++]
0772 dc3a  st   $3a,[y,x++]
0773 dc3f  st   $3f,[y,x++]
0774 dc15  st   $15,[y,x++]
0775 dc14  st   $14,[y,x++]
0776 dc04  st   $04,[y,x++]
0777 dc14  st   $14,[y,x++]
0778 dc00  st   $00,[y,x++]
0779 dc04  st   $04,[y,x++]
077a dc14  st   $14,[y,x++]
077b dc14  st   $14,[y,x++]
077c dc2a  st   $2a,[y,x++]
077d dc2e  st   $2e,[y,x++]
077e dc15  st   $15,[y,x++]
077f dc15  st   $15,[y,x++]
0780 dc14  st   $14,[y,x++]
0781 dc14  st   $14,[y,x++]
0782 dc14  st   $14,[y,x++]
0783 dc3a  st   $3a,[y,x++]
0784 dc2a  st   $2a,[y,x++]
0785 dc3f  st   $3f,[y,x++]
0786 dc15  st   $15,[y,x++]
0787 dc14  st   $14,[y,x++]
0788 dc3f  st   $3f,[y,x++]
0789 dc2a  st   $2a,[y,x++]
078a dc2b  st   $2b,[y,x++]
078b dc15  st   $15,[y,x++]
078c dc29  st   $29,[y,x++]
078d dc2a  st   $2a,[y,x++]
078e dc15  st   $15,[y,x++]
078f dc14  st   $14,[y,x++]
0790 dc14  st   $14,[y,x++]
0791 dc29  st   $29,[y,x++]
0792 dc3f  st   $3f,[y,x++]
0793 dc04  st   $04,[y,x++]
0794 dc2a  st   $2a,[y,x++]
0795 dc3e  st   $3e,[y,x++]
0796 dc3e  st   $3e,[y,x++]
0797 dc3f  st   $3f,[y,x++]
0798 dc2a  st   $2a,[y,x++]
0799 dc15  st   $15,[y,x++]
079a dc14  st   $14,[y,x++]
079b dc14  st   $14,[y,x++]
079c dc14  st   $14,[y,x++]
079d dc10  st   $10,[y,x++]
079e dc04  st   $04,[y,x++]
079f dc15  st   $15,[y,x++]
07a0 dc10  st   $10,[y,x++]
07a1 dc14  st   $14,[y,x++]
07a2 dc14  st   $14,[y,x++]
07a3 dc14  st   $14,[y,x++]
07a4 dc29  st   $29,[y,x++]
07a5 dc3f  st   $3f,[y,x++]
07a6 dc15  st   $15,[y,x++]
07a7 dc14  st   $14,[y,x++]
07a8 dc14  st   $14,[y,x++]
07a9 dc14  st   $14,[y,x++]
07aa dc04  st   $04,[y,x++]
07ab dc25  st   $25,[y,x++]
07ac dc2f  st   $2f,[y,x++]
07ad dc14  st   $14,[y,x++]
07ae dc2a  st   $2a,[y,x++]
07af dc2a  st   $2a,[y,x++]
07b0 dc29  st   $29,[y,x++]
07b1 dc3f  st   $3f,[y,x++]
07b2 dc1a  st   $1a,[y,x++]
07b3 dc14  st   $14,[y,x++]
07b4 dc05  st   $05,[y,x++]
07b5 dc04  st   $04,[y,x++]
07b6 dc11  st   $11,[y,x++]
07b7 dc04  st   $04,[y,x++]
07b8 dc14  st   $14,[y,x++]
07b9 dc14  st   $14,[y,x++]
07ba dc14  st   $14,[y,x++]
07bb dc14  st   $14,[y,x++]
07bc dc04  st   $04,[y,x++]
07bd dc14  st   $14,[y,x++]
07be dc14  st   $14,[y,x++]
07bf dc3b  st   $3b,[y,x++]
07c0 dc25  st   $25,[y,x++]
07c1 dc04  st   $04,[y,x++]
07c2 dc10  st   $10,[y,x++]
07c3 dc2a  st   $2a,[y,x++]
07c4 dc3e  st   $3e,[y,x++]
07c5 dc15  st   $15,[y,x++]
07c6 dc15  st   $15,[y,x++]
07c7 dc15  st   $15,[y,x++]
07c8 dc14  st   $14,[y,x++]
07c9 dc15  st   $15,[y,x++]
07ca dc3f  st   $3f,[y,x++]
07cb dc15  st   $15,[y,x++]
07cc dc00  st   $00,[y,x++]
07cd dc15  st   $15,[y,x++]
07ce dc3f  st   $3f,[y,x++]
07cf dc15  st   $15,[y,x++]
07d0 dc2a  st   $2a,[y,x++]
07d1 dc2f  st   $2f,[y,x++]
07d2 dc14  st   $14,[y,x++]
07d3 dc14  st   $14,[y,x++]
07d4 dc3b  st   $3b,[y,x++]
07d5 dc2a  st   $2a,[y,x++]
07d6 dc14  st   $14,[y,x++]
07d7 dc14  st   $14,[y,x++]
07d8 dc15  st   $15,[y,x++]
07d9 dc3f  st   $3f,[y,x++]
07da dc04  st   $04,[y,x++]
07db dc10  st   $10,[y,x++]
07dc dc14  st   $14,[y,x++]
07dd dc14  st   $14,[y,x++]
07de dc14  st   $14,[y,x++]
07df dc14  st   $14,[y,x++]
07e0 dc15  st   $15,[y,x++]
07e1 dc3f  st   $3f,[y,x++]
07e2 dc15  st   $15,[y,x++]
07e3 dc29  st   $29,[y,x++]
07e4 dc2f  st   $2f,[y,x++]
07e5 dc14  st   $14,[y,x++]
07e6 dc14  st   $14,[y,x++]
07e7 dc15  st   $15,[y,x++]
07e8 dc3f  st   $3f,[y,x++]
07e9 dc25  st   $25,[y,x++]
07ea dc3f  st   $3f,[y,x++]
07eb dc1a  st   $1a,[y,x++]
07ec dc14  st   $14,[y,x++]
07ed dc29  st   $29,[y,x++]
07ee dc2f  st   $2f,[y,x++]
07ef dc15  st   $15,[y,x++]
07f0 dc3b  st   $3b,[y,x++]
07f1 dc29  st   $29,[y,x++]
07f2 dc15  st   $15,[y,x++]
07f3 dc19  st   $19,[y,x++]
07f4 dc14  st   $14,[y,x++]
07f5 dc14  st   $14,[y,x++]
07f6 dc10  st   $10,[y,x++]
07f7 dc29  st   $29,[y,x++]
07f8 dc3f  st   $3f,[y,x++]
07f9 dc15  st   $15,[y,x++]
07fa dc3f  st   $3f,[y,x++]
07fb dc19  st   $19,[y,x++]
07fc dc14  st   $14,[y,x++]
07fd dc14  st   $14,[y,x++]
07fe dc14  st   $14,[y,x++]
07ff dc10  st   $10,[y,x++]
0800 dc14  st   $14,[y,x++]
0801 dc00  st   $00,[y,x++]
0802 dc04  st   $04,[y,x++]
0803 dc10  st   $10,[y,x++]
0804 dc15  st   $15,[y,x++]
0805 140c  ld   $0c,y
0806 dc04  st   $04,[y,x++]
0807 dc11  st   $11,[y,x++]
0808 dc14  st   $14,[y,x++]
0809 dc10  st   $10,[y,x++]
080a dc04  st   $04,[y,x++]
080b dc14  st   $14,[y,x++]
080c dc04  st   $04,[y,x++]
080d dc14  st   $14,[y,x++]
080e dc15  st   $15,[y,x++]
080f dc04  st   $04,[y,x++]
0810 dc14  st   $14,[y,x++]
0811 dc25  st   $25,[y,x++]
0812 dc2e  st   $2e,[y,x++]
0813 dc14  st   $14,[y,x++]
0814 dc04  st   $04,[y,x++]
0815 dc14  st   $14,[y,x++]
0816 dc05  st   $05,[y,x++]
0817 dc14  st   $14,[y,x++]
0818 dc2a  st   $2a,[y,x++]
0819 dc29  st   $29,[y,x++]
081a dc10  st   $10,[y,x++]
081b dc14  st   $14,[y,x++]
081c dc14  st   $14,[y,x++]
081d dc2f  st   $2f,[y,x++]
081e dc29  st   $29,[y,x++]
081f dc10  st   $10,[y,x++]
0820 dc11  st   $11,[y,x++]
0821 dc04  st   $04,[y,x++]
0822 dc14  st   $14,[y,x++]
0823 dc10  st   $10,[y,x++]
0824 dc14  st   $14,[y,x++]
0825 dc14  st   $14,[y,x++]
0826 dc06  st   $06,[y,x++]
0827 dc1b  st   $1b,[y,x++]
0828 dc15  st   $15,[y,x++]
0829 dc10  st   $10,[y,x++]
082a dc15  st   $15,[y,x++]
082b dc15  st   $15,[y,x++]
082c dc03  st   $03,[y,x++]
082d dc01  st   $01,[y,x++]
082e dc15  st   $15,[y,x++]
082f dc00  st   $00,[y,x++]
0830 dc03  st   $03,[y,x++]
0831 dc12  st   $12,[y,x++]
0832 dc18  st   $18,[y,x++]
0833 dc18  st   $18,[y,x++]
0834 dc14  st   $14,[y,x++]
0835 dc29  st   $29,[y,x++]
0836 dc29  st   $29,[y,x++]
0837 dc14  st   $14,[y,x++]
0838 dc15  st   $15,[y,x++]
0839 dc28  st   $28,[y,x++]
083a dc39  st   $39,[y,x++]
083b dc14  st   $14,[y,x++]
083c dc1e  st   $1e,[y,x++]
083d dc0a  st   $0a,[y,x++]
083e dc15  st   $15,[y,x++]
083f dc14  st   $14,[y,x++]
0840 dc11  st   $11,[y,x++]
0841 dc14  st   $14,[y,x++]
0842 dc14  st   $14,[y,x++]
0843 dc15  st   $15,[y,x++]
0844 dc3f  st   $3f,[y,x++]
0845 dc29  st   $29,[y,x++]
0846 dc14  st   $14,[y,x++]
0847 dc15  st   $15,[y,x++]
0848 dc15  st   $15,[y,x++]
0849 dc2a  st   $2a,[y,x++]
084a dc2b  st   $2b,[y,x++]
084b dc14  st   $14,[y,x++]
084c dc14  st   $14,[y,x++]
084d dc3e  st   $3e,[y,x++]
084e dc2a  st   $2a,[y,x++]
084f dc15  st   $15,[y,x++]
0850 dc2e  st   $2e,[y,x++]
0851 dc2a  st   $2a,[y,x++]
0852 dc14  st   $14,[y,x++]
0853 dc29  st   $29,[y,x++]
0854 dc3f  st   $3f,[y,x++]
0855 dc15  st   $15,[y,x++]
0856 dc15  st   $15,[y,x++]
0857 dc3a  st   $3a,[y,x++]
0858 dc2a  st   $2a,[y,x++]
0859 dc15  st   $15,[y,x++]
085a dc2b  st   $2b,[y,x++]
085b dc2a  st   $2a,[y,x++]
085c dc14  st   $14,[y,x++]
085d dc14  st   $14,[y,x++]
085e dc3e  st   $3e,[y,x++]
085f dc2a  st   $2a,[y,x++]
0860 dc15  st   $15,[y,x++]
0861 dc3f  st   $3f,[y,x++]
0862 dc14  st   $14,[y,x++]
0863 dc14  st   $14,[y,x++]
0864 dc15  st   $15,[y,x++]
0865 dc3f  st   $3f,[y,x++]
0866 dc14  st   $14,[y,x++]
0867 dc2a  st   $2a,[y,x++]
0868 dc2b  st   $2b,[y,x++]
0869 dc14  st   $14,[y,x++]
086a dc14  st   $14,[y,x++]
086b dc2a  st   $2a,[y,x++]
086c dc2a  st   $2a,[y,x++]
086d dc14  st   $14,[y,x++]
086e dc14  st   $14,[y,x++]
086f dc2e  st   $2e,[y,x++]
0870 dc2a  st   $2a,[y,x++]
0871 dc29  st   $29,[y,x++]
0872 dc3f  st   $3f,[y,x++]
0873 dc2a  st   $2a,[y,x++]
0874 dc15  st   $15,[y,x++]
0875 dc14  st   $14,[y,x++]
0876 dc14  st   $14,[y,x++]
0877 dc04  st   $04,[y,x++]
0878 dc14  st   $14,[y,x++]
0879 dc15  st   $15,[y,x++]
087a dc10  st   $10,[y,x++]
087b dc04  st   $04,[y,x++]
087c dc14  st   $14,[y,x++]
087d dc2a  st   $2a,[y,x++]
087e dc3f  st   $3f,[y,x++]
087f dc3b  st   $3b,[y,x++]
0880 dc3f  st   $3f,[y,x++]
0881 dc3f  st   $3f,[y,x++]
0882 dc19  st   $19,[y,x++]
0883 dc14  st   $14,[y,x++]
0884 dc2e  st   $2e,[y,x++]
0885 dc29  st   $29,[y,x++]
0886 dc2a  st   $2a,[y,x++]
0887 dc2a  st   $2a,[y,x++]
0888 dc2a  st   $2a,[y,x++]
0889 dc2e  st   $2e,[y,x++]
088a dc15  st   $15,[y,x++]
088b dc3e  st   $3e,[y,x++]
088c dc14  st   $14,[y,x++]
088d dc2a  st   $2a,[y,x++]
088e dc3f  st   $3f,[y,x++]
088f dc3f  st   $3f,[y,x++]
0890 dc2e  st   $2e,[y,x++]
0891 dc3f  st   $3f,[y,x++]
0892 dc3f  st   $3f,[y,x++]
0893 dc2f  st   $2f,[y,x++]
0894 dc14  st   $14,[y,x++]
0895 dc15  st   $15,[y,x++]
0896 dc14  st   $14,[y,x++]
0897 dc15  st   $15,[y,x++]
0898 dc3f  st   $3f,[y,x++]
0899 dc19  st   $19,[y,x++]
089a dc14  st   $14,[y,x++]
089b dc10  st   $10,[y,x++]
089c dc14  st   $14,[y,x++]
089d dc14  st   $14,[y,x++]
089e dc14  st   $14,[y,x++]
089f dc10  st   $10,[y,x++]
08a0 dc14  st   $14,[y,x++]
08a1 dc05  st   $05,[y,x++]
08a2 dc14  st   $14,[y,x++]
08a3 dc15  st   $15,[y,x++]
08a4 dc3f  st   $3f,[y,x++]
08a5 dc3f  st   $3f,[y,x++]
08a6 dc29  st   $29,[y,x++]
08a7 dc14  st   $14,[y,x++]
08a8 dc14  st   $14,[y,x++]
08a9 dc14  st   $14,[y,x++]
* 5 times
08ac dc3f  st   $3f,[y,x++]
08ad dc2a  st   $2a,[y,x++]
08ae dc15  st   $15,[y,x++]
08af dc15  st   $15,[y,x++]
08b0 dc3f  st   $3f,[y,x++]
08b1 dc3f  st   $3f,[y,x++]
08b2 dc2a  st   $2a,[y,x++]
08b3 dc04  st   $04,[y,x++]
08b4 dc10  st   $10,[y,x++]
08b5 dc14  st   $14,[y,x++]
08b6 dc14  st   $14,[y,x++]
08b7 dc10  st   $10,[y,x++]
08b8 dc15  st   $15,[y,x++]
08b9 dc04  st   $04,[y,x++]
08ba dc14  st   $14,[y,x++]
08bb dc04  st   $04,[y,x++]
08bc dc14  st   $14,[y,x++]
08bd dc14  st   $14,[y,x++]
08be dc14  st   $14,[y,x++]
08bf dc04  st   $04,[y,x++]
08c0 dc3b  st   $3b,[y,x++]
08c1 dc19  st   $19,[y,x++]
08c2 dc14  st   $14,[y,x++]
08c3 dc04  st   $04,[y,x++]
08c4 dc2a  st   $2a,[y,x++]
08c5 dc3f  st   $3f,[y,x++]
08c6 dc3f  st   $3f,[y,x++]
08c7 dc3e  st   $3e,[y,x++]
08c8 dc3f  st   $3f,[y,x++]
08c9 dc19  st   $19,[y,x++]
08ca dc15  st   $15,[y,x++]
08cb dc3f  st   $3f,[y,x++]
08cc dc14  st   $14,[y,x++]
08cd dc14  st   $14,[y,x++]
08ce dc14  st   $14,[y,x++]
08cf dc3e  st   $3e,[y,x++]
08d0 dc15  st   $15,[y,x++]
08d1 dc15  st   $15,[y,x++]
08d2 dc3e  st   $3e,[y,x++]
08d3 dc2a  st   $2a,[y,x++]
08d4 dc2a  st   $2a,[y,x++]
08d5 dc3f  st   $3f,[y,x++]
08d6 dc15  st   $15,[y,x++]
08d7 dc10  st   $10,[y,x++]
08d8 dc04  st   $04,[y,x++]
08d9 dc29  st   $29,[y,x++]
08da dc2f  st   $2f,[y,x++]
08db dc10  st   $10,[y,x++]
08dc dc04  st   $04,[y,x++]
08dd dc15  st   $15,[y,x++]
08de dc14  st   $14,[y,x++]
08df dc14  st   $14,[y,x++]
08e0 dc14  st   $14,[y,x++]
08e1 dc2a  st   $2a,[y,x++]
08e2 dc3e  st   $3e,[y,x++]
08e3 dc15  st   $15,[y,x++]
08e4 dc2a  st   $2a,[y,x++]
08e5 dc3e  st   $3e,[y,x++]
08e6 dc04  st   $04,[y,x++]
08e7 dc14  st   $14,[y,x++]
08e8 dc15  st   $15,[y,x++]
08e9 dc3f  st   $3f,[y,x++]
08ea dc15  st   $15,[y,x++]
08eb dc29  st   $29,[y,x++]
08ec dc3f  st   $3f,[y,x++]
08ed dc19  st   $19,[y,x++]
08ee dc3f  st   $3f,[y,x++]
08ef dc19  st   $19,[y,x++]
08f0 dc14  st   $14,[y,x++]
08f1 dc3e  st   $3e,[y,x++]
08f2 dc3f  st   $3f,[y,x++]
08f3 dc3f  st   $3f,[y,x++]
08f4 dc3f  st   $3f,[y,x++]
08f5 dc2b  st   $2b,[y,x++]
08f6 dc15  st   $15,[y,x++]
08f7 dc25  st   $25,[y,x++]
08f8 dc3b  st   $3b,[y,x++]
08f9 dc14  st   $14,[y,x++]
08fa dc14  st   $14,[y,x++]
08fb dc3f  st   $3f,[y,x++]
08fc dc19  st   $19,[y,x++]
08fd dc14  st   $14,[y,x++]
08fe dc04  st   $04,[y,x++]
08ff dc14  st   $14,[y,x++]
0900 dc00  st   $00,[y,x++]
0901 dc14  st   $14,[y,x++]
0902 dc14  st   $14,[y,x++]
0903 dc14  st   $14,[y,x++]
0904 dc11  st   $11,[y,x++]
0905 dc04  st   $04,[y,x++]
0906 140d  ld   $0d,y
0907 dc14  st   $14,[y,x++]
0908 dc14  st   $14,[y,x++]
0909 dc14  st   $14,[y,x++]
090a dc04  st   $04,[y,x++]
090b dc14  st   $14,[y,x++]
090c dc14  st   $14,[y,x++]
090d dc11  st   $11,[y,x++]
090e dc14  st   $14,[y,x++]
090f dc10  st   $10,[y,x++]
0910 dc14  st   $14,[y,x++]
0911 dc14  st   $14,[y,x++]
0912 dc19  st   $19,[y,x++]
0913 dc3b  st   $3b,[y,x++]
0914 dc14  st   $14,[y,x++]
0915 dc14  st   $14,[y,x++]
0916 dc10  st   $10,[y,x++]
0917 dc14  st   $14,[y,x++]
0918 dc10  st   $10,[y,x++]
0919 dc3f  st   $3f,[y,x++]
091a dc15  st   $15,[y,x++]
091b dc14  st   $14,[y,x++]
091c dc14  st   $14,[y,x++]
091d dc14  st   $14,[y,x++]
091e dc3b  st   $3b,[y,x++]
091f dc19  st   $19,[y,x++]
0920 dc14  st   $14,[y,x++]
0921 dc14  st   $14,[y,x++]
0922 dc14  st   $14,[y,x++]
0923 dc14  st   $14,[y,x++]
0924 dc04  st   $04,[y,x++]
0925 dc14  st   $14,[y,x++]
0926 dc05  st   $05,[y,x++]
0927 dc1b  st   $1b,[y,x++]
0928 dc05  st   $05,[y,x++]
0929 dc14  st   $14,[y,x++]
092a dc14  st   $14,[y,x++]
092b dc04  st   $04,[y,x++]
092c dc15  st   $15,[y,x++]
092d dc03  st   $03,[y,x++]
092e dc15  st   $15,[y,x++]
092f dc14  st   $14,[y,x++]
0930 dc10  st   $10,[y,x++]
0931 dc12  st   $12,[y,x++]
0932 dc06  st   $06,[y,x++]
0933 dc04  st   $04,[y,x++]
0934 dc08  st   $08,[y,x++]
0935 dc18  st   $18,[y,x++]
0936 dc38  st   $38,[y,x++]
0937 dc14  st   $14,[y,x++]
0938 dc14  st   $14,[y,x++]
0939 dc14  st   $14,[y,x++]
093a dc14  st   $14,[y,x++]
093b dc29  st   $29,[y,x++]
093c dc14  st   $14,[y,x++]
093d dc0e  st   $0e,[y,x++]
093e dc15  st   $15,[y,x++]
093f dc14  st   $14,[y,x++]
0940 dc04  st   $04,[y,x++]
0941 dc14  st   $14,[y,x++]
0942 dc14  st   $14,[y,x++]
0943 dc10  st   $10,[y,x++]
0944 dc29  st   $29,[y,x++]
0945 dc3b  st   $3b,[y,x++]
0946 dc04  st   $04,[y,x++]
0947 dc14  st   $14,[y,x++]
0948 dc14  st   $14,[y,x++]
0949 dc14  st   $14,[y,x++]
094a dc3f  st   $3f,[y,x++]
094b dc29  st   $29,[y,x++]
094c dc00  st   $00,[y,x++]
094d dc14  st   $14,[y,x++]
094e dc15  st   $15,[y,x++]
094f dc3f  st   $3f,[y,x++]
0950 dc14  st   $14,[y,x++]
0951 dc3f  st   $3f,[y,x++]
0952 dc25  st   $25,[y,x++]
0953 dc14  st   $14,[y,x++]
0954 dc15  st   $15,[y,x++]
0955 dc3f  st   $3f,[y,x++]
0956 dc14  st   $14,[y,x++]
0957 dc14  st   $14,[y,x++]
0958 dc2e  st   $2e,[y,x++]
0959 dc2a  st   $2a,[y,x++]
095a dc14  st   $14,[y,x++]
095b dc3e  st   $3e,[y,x++]
095c dc2a  st   $2a,[y,x++]
095d dc14  st   $14,[y,x++]
095e dc14  st   $14,[y,x++]
095f dc29  st   $29,[y,x++]
0960 dc3f  st   $3f,[y,x++]
0961 dc29  st   $29,[y,x++]
0962 dc2b  st   $2b,[y,x++]
0963 dc10  st   $10,[y,x++]
0964 dc14  st   $14,[y,x++]
0965 dc14  st   $14,[y,x++]
0966 dc3f  st   $3f,[y,x++]
0967 dc14  st   $14,[y,x++]
0968 dc29  st   $29,[y,x++]
0969 dc3e  st   $3e,[y,x++]
096a dc00  st   $00,[y,x++]
096b dc14  st   $14,[y,x++]
096c dc3f  st   $3f,[y,x++]
096d dc3f  st   $3f,[y,x++]
096e dc2a  st   $2a,[y,x++]
096f dc2a  st   $2a,[y,x++]
0970 dc2a  st   $2a,[y,x++]
0971 dc3e  st   $3e,[y,x++]
0972 dc15  st   $15,[y,x++]
0973 dc2e  st   $2e,[y,x++]
0974 dc14  st   $14,[y,x++]
0975 dc14  st   $14,[y,x++]
0976 dc14  st   $14,[y,x++]
* 5 times
0979 dc10  st   $10,[y,x++]
097a dc14  st   $14,[y,x++]
097b dc15  st   $15,[y,x++]
097c dc14  st   $14,[y,x++]
097d dc14  st   $14,[y,x++]
097e dc3f  st   $3f,[y,x++]
097f dc2a  st   $2a,[y,x++]
0980 dc14  st   $14,[y,x++]
0981 dc14  st   $14,[y,x++]
0982 dc2a  st   $2a,[y,x++]
0983 dc2f  st   $2f,[y,x++]
0984 dc10  st   $10,[y,x++]
0985 dc2a  st   $2a,[y,x++]
0986 dc2a  st   $2a,[y,x++]
0987 dc14  st   $14,[y,x++]
0988 dc3f  st   $3f,[y,x++]
0989 dc3f  st   $3f,[y,x++]
098a dc15  st   $15,[y,x++]
098b dc15  st   $15,[y,x++]
098c dc3f  st   $3f,[y,x++]
098d dc14  st   $14,[y,x++]
098e dc29  st   $29,[y,x++]
098f dc3b  st   $3b,[y,x++]
0990 dc15  st   $15,[y,x++]
0991 dc15  st   $15,[y,x++]
0992 dc14  st   $14,[y,x++]
0993 dc25  st   $25,[y,x++]
0994 dc3b  st   $3b,[y,x++]
0995 dc14  st   $14,[y,x++]
0996 dc14  st   $14,[y,x++]
0997 dc14  st   $14,[y,x++]
0998 dc3f  st   $3f,[y,x++]
0999 dc2a  st   $2a,[y,x++]
099a dc14  st   $14,[y,x++]
099b dc15  st   $15,[y,x++]
099c dc14  st   $14,[y,x++]
099d dc00  st   $00,[y,x++]
099e dc15  st   $15,[y,x++]
099f dc14  st   $14,[y,x++]
09a0 dc04  st   $04,[y,x++]
09a1 dc14  st   $14,[y,x++]
09a2 dc10  st   $10,[y,x++]
09a3 dc14  st   $14,[y,x++]
09a4 dc14  st   $14,[y,x++]
09a5 dc15  st   $15,[y,x++]
09a6 dc2a  st   $2a,[y,x++]
09a7 dc3f  st   $3f,[y,x++]
09a8 dc15  st   $15,[y,x++]
09a9 dc04  st   $04,[y,x++]
09aa dc10  st   $10,[y,x++]
09ab dc14  st   $14,[y,x++]
09ac dc2a  st   $2a,[y,x++]
09ad dc3e  st   $3e,[y,x++]
09ae dc15  st   $15,[y,x++]
09af dc14  st   $14,[y,x++]
09b0 dc29  st   $29,[y,x++]
09b1 dc3f  st   $3f,[y,x++]
09b2 dc3f  st   $3f,[y,x++]
09b3 dc3e  st   $3e,[y,x++]
09b4 dc15  st   $15,[y,x++]
09b5 dc14  st   $14,[y,x++]
09b6 dc14  st   $14,[y,x++]
09b7 dc14  st   $14,[y,x++]
09b8 dc04  st   $04,[y,x++]
09b9 dc14  st   $14,[y,x++]
09ba dc10  st   $10,[y,x++]
09bb dc14  st   $14,[y,x++]
09bc dc11  st   $11,[y,x++]
09bd dc14  st   $14,[y,x++]
09be dc00  st   $00,[y,x++]
09bf dc14  st   $14,[y,x++]
09c0 dc10  st   $10,[y,x++]
09c1 dc2f  st   $2f,[y,x++]
09c2 dc25  st   $25,[y,x++]
09c3 dc10  st   $10,[y,x++]
09c4 dc14  st   $14,[y,x++]
09c5 dc3e  st   $3e,[y,x++]
09c6 dc2a  st   $2a,[y,x++]
09c7 dc15  st   $15,[y,x++]
09c8 dc14  st   $14,[y,x++]
09c9 dc3a  st   $3a,[y,x++]
09ca dc2a  st   $2a,[y,x++]
09cb dc15  st   $15,[y,x++]
09cc dc3e  st   $3e,[y,x++]
09cd dc15  st   $15,[y,x++]
09ce dc14  st   $14,[y,x++]
09cf dc14  st   $14,[y,x++]
09d0 dc3f  st   $3f,[y,x++]
09d1 dc25  st   $25,[y,x++]
09d2 dc14  st   $14,[y,x++]
09d3 dc15  st   $15,[y,x++]
09d4 dc3f  st   $3f,[y,x++]
09d5 dc3f  st   $3f,[y,x++]
09d6 dc14  st   $14,[y,x++]
09d7 dc14  st   $14,[y,x++]
09d8 dc14  st   $14,[y,x++]
09d9 dc14  st   $14,[y,x++]
09da dc15  st   $15,[y,x++]
09db dc3a  st   $3a,[y,x++]
09dc dc14  st   $14,[y,x++]
09dd dc14  st   $14,[y,x++]
09de dc10  st   $10,[y,x++]
09df dc14  st   $14,[y,x++]
09e0 dc04  st   $04,[y,x++]
09e1 dc2a  st   $2a,[y,x++]
09e2 dc3f  st   $3f,[y,x++]
09e3 dc15  st   $15,[y,x++]
09e4 dc10  st   $10,[y,x++]
09e5 dc2a  st   $2a,[y,x++]
09e6 dc2a  st   $2a,[y,x++]
09e7 dc14  st   $14,[y,x++]
09e8 dc14  st   $14,[y,x++]
09e9 dc25  st   $25,[y,x++]
09ea dc3f  st   $3f,[y,x++]
09eb dc14  st   $14,[y,x++]
09ec dc14  st   $14,[y,x++]
09ed dc2a  st   $2a,[y,x++]
09ee dc3f  st   $3f,[y,x++]
09ef dc2a  st   $2a,[y,x++]
09f0 dc14  st   $14,[y,x++]
09f1 dc15  st   $15,[y,x++]
09f2 dc3f  st   $3f,[y,x++]
09f3 dc15  st   $15,[y,x++]
09f4 dc14  st   $14,[y,x++]
09f5 dc14  st   $14,[y,x++]
09f6 dc3f  st   $3f,[y,x++]
09f7 dc2a  st   $2a,[y,x++]
09f8 dc2e  st   $2e,[y,x++]
09f9 dc3f  st   $3f,[y,x++]
09fa dc2a  st   $2a,[y,x++]
09fb dc2a  st   $2a,[y,x++]
09fc dc3f  st   $3f,[y,x++]
09fd dc3a  st   $3a,[y,x++]
09fe dc14  st   $14,[y,x++]
09ff dc10  st   $10,[y,x++]
0a00 dc15  st   $15,[y,x++]
0a01 dc14  st   $14,[y,x++]
0a02 dc14  st   $14,[y,x++]
0a03 dc14  st   $14,[y,x++]
* 6 times
0a07 140e  ld   $0e,y
0a08 dc04  st   $04,[y,x++]
0a09 dc04  st   $04,[y,x++]
0a0a dc14  st   $14,[y,x++]
0a0b dc11  st   $11,[y,x++]
0a0c dc11  st   $11,[y,x++]
0a0d dc14  st   $14,[y,x++]
0a0e dc10  st   $10,[y,x++]
0a0f dc14  st   $14,[y,x++]
0a10 dc14  st   $14,[y,x++]
0a11 dc04  st   $04,[y,x++]
0a12 dc14  st   $14,[y,x++]
0a13 dc15  st   $15,[y,x++]
0a14 dc3b  st   $3b,[y,x++]
0a15 dc04  st   $04,[y,x++]
0a16 dc04  st   $04,[y,x++]
0a17 dc14  st   $14,[y,x++]
0a18 dc14  st   $14,[y,x++]
0a19 dc14  st   $14,[y,x++]
0a1a dc3f  st   $3f,[y,x++]
0a1b dc15  st   $15,[y,x++]
0a1c dc04  st   $04,[y,x++]
0a1d dc14  st   $14,[y,x++]
0a1e dc14  st   $14,[y,x++]
0a1f dc3f  st   $3f,[y,x++]
0a20 dc15  st   $15,[y,x++]
0a21 dc14  st   $14,[y,x++]
0a22 dc10  st   $10,[y,x++]
0a23 dc14  st   $14,[y,x++]
0a24 dc14  st   $14,[y,x++]
0a25 dc11  st   $11,[y,x++]
0a26 dc04  st   $04,[y,x++]
0a27 dc14  st   $14,[y,x++]
0a28 dc1a  st   $1a,[y,x++]
0a29 dc05  st   $05,[y,x++]
0a2a dc14  st   $14,[y,x++]
0a2b dc04  st   $04,[y,x++]
0a2c dc14  st   $14,[y,x++]
0a2d dc15  st   $15,[y,x++]
0a2e dc03  st   $03,[y,x++]
0a2f dc14  st   $14,[y,x++]
0a30 dc04  st   $04,[y,x++]
0a31 dc14  st   $14,[y,x++]
0a32 dc06  st   $06,[y,x++]
0a33 dc03  st   $03,[y,x++]
0a34 dc14  st   $14,[y,x++]
0a35 dc0d  st   $0d,[y,x++]
0a36 dc14  st   $14,[y,x++]
0a37 dc29  st   $29,[y,x++]
0a38 dc25  st   $25,[y,x++]
0a39 dc14  st   $14,[y,x++]
0a3a dc04  st   $04,[y,x++]
0a3b dc24  st   $24,[y,x++]
0a3c dc39  st   $39,[y,x++]
0a3d dc15  st   $15,[y,x++]
0a3e dc0e  st   $0e,[y,x++]
0a3f dc05  st   $05,[y,x++]
0a40 dc04  st   $04,[y,x++]
0a41 dc14  st   $14,[y,x++]
0a42 dc10  st   $10,[y,x++]
0a43 dc04  st   $04,[y,x++]
0a44 dc14  st   $14,[y,x++]
0a45 dc29  st   $29,[y,x++]
0a46 dc2f  st   $2f,[y,x++]
0a47 dc14  st   $14,[y,x++]
0a48 dc04  st   $04,[y,x++]
0a49 dc11  st   $11,[y,x++]
0a4a dc14  st   $14,[y,x++]
0a4b dc3f  st   $3f,[y,x++]
0a4c dc19  st   $19,[y,x++]
0a4d dc14  st   $14,[y,x++]
0a4e dc14  st   $14,[y,x++]
0a4f dc15  st   $15,[y,x++]
0a50 dc3f  st   $3f,[y,x++]
0a51 dc15  st   $15,[y,x++]
0a52 dc3f  st   $3f,[y,x++]
0a53 dc19  st   $19,[y,x++]
0a54 dc14  st   $14,[y,x++]
0a55 dc25  st   $25,[y,x++]
0a56 dc3f  st   $3f,[y,x++]
0a57 dc14  st   $14,[y,x++]
0a58 dc14  st   $14,[y,x++]
0a59 dc2a  st   $2a,[y,x++]
0a5a dc2a  st   $2a,[y,x++]
0a5b dc14  st   $14,[y,x++]
0a5c dc2e  st   $2e,[y,x++]
0a5d dc2a  st   $2a,[y,x++]
0a5e dc05  st   $05,[y,x++]
0a5f dc14  st   $14,[y,x++]
0a60 dc2a  st   $2a,[y,x++]
0a61 dc2a  st   $2a,[y,x++]
0a62 dc15  st   $15,[y,x++]
0a63 dc3e  st   $3e,[y,x++]
0a64 dc15  st   $15,[y,x++]
0a65 dc04  st   $04,[y,x++]
0a66 dc15  st   $15,[y,x++]
0a67 dc3e  st   $3e,[y,x++]
0a68 dc14  st   $14,[y,x++]
0a69 dc2a  st   $2a,[y,x++]
0a6a dc2a  st   $2a,[y,x++]
0a6b dc14  st   $14,[y,x++]
0a6c dc14  st   $14,[y,x++]
0a6d dc3f  st   $3f,[y,x++]
0a6e dc29  st   $29,[y,x++]
0a6f dc15  st   $15,[y,x++]
0a70 dc25  st   $25,[y,x++]
0a71 dc15  st   $15,[y,x++]
0a72 dc24  st   $24,[y,x++]
0a73 dc15  st   $15,[y,x++]
0a74 dc3f  st   $3f,[y,x++]
0a75 dc14  st   $14,[y,x++]
0a76 dc10  st   $10,[y,x++]
0a77 dc04  st   $04,[y,x++]
0a78 dc15  st   $15,[y,x++]
0a79 dc00  st   $00,[y,x++]
0a7a dc04  st   $04,[y,x++]
0a7b dc14  st   $14,[y,x++]
0a7c dc14  st   $14,[y,x++]
0a7d dc14  st   $14,[y,x++]
0a7e dc14  st   $14,[y,x++]
0a7f dc2a  st   $2a,[y,x++]
0a80 dc2a  st   $2a,[y,x++]
0a81 dc14  st   $14,[y,x++]
0a82 dc04  st   $04,[y,x++]
0a83 dc25  st   $25,[y,x++]
0a84 dc3f  st   $3f,[y,x++]
0a85 dc14  st   $14,[y,x++]
0a86 dc2a  st   $2a,[y,x++]
0a87 dc2a  st   $2a,[y,x++]
0a88 dc14  st   $14,[y,x++]
0a89 dc2a  st   $2a,[y,x++]
0a8a dc2a  st   $2a,[y,x++]
0a8b dc10  st   $10,[y,x++]
0a8c dc25  st   $25,[y,x++]
0a8d dc2f  st   $2f,[y,x++]
0a8e dc15  st   $15,[y,x++]
0a8f dc19  st   $19,[y,x++]
0a90 dc3e  st   $3e,[y,x++]
0a91 dc14  st   $14,[y,x++]
0a92 dc14  st   $14,[y,x++]
0a93 dc14  st   $14,[y,x++]
0a94 dc15  st   $15,[y,x++]
0a95 dc3f  st   $3f,[y,x++]
0a96 dc14  st   $14,[y,x++]
0a97 dc04  st   $04,[y,x++]
0a98 dc2a  st   $2a,[y,x++]
0a99 dc3e  st   $3e,[y,x++]
0a9a dc14  st   $14,[y,x++]
0a9b dc14  st   $14,[y,x++]
0a9c dc00  st   $00,[y,x++]
0a9d dc14  st   $14,[y,x++]
0a9e dc04  st   $04,[y,x++]
0a9f dc14  st   $14,[y,x++]
0aa0 dc14  st   $14,[y,x++]
0aa1 dc00  st   $00,[y,x++]
0aa2 dc14  st   $14,[y,x++]
0aa3 dc14  st   $14,[y,x++]
0aa4 dc14  st   $14,[y,x++]
0aa5 dc00  st   $00,[y,x++]
0aa6 dc14  st   $14,[y,x++]
0aa7 dc14  st   $14,[y,x++]
0aa8 dc3b  st   $3b,[y,x++]
0aa9 dc19  st   $19,[y,x++]
0aaa dc14  st   $14,[y,x++]
0aab dc14  st   $14,[y,x++]
0aac dc2a  st   $2a,[y,x++]
0aad dc3f  st   $3f,[y,x++]
0aae dc15  st   $15,[y,x++]
0aaf dc14  st   $14,[y,x++]
0ab0 dc14  st   $14,[y,x++]
0ab1 dc26  st   $26,[y,x++]
0ab2 dc2e  st   $2e,[y,x++]
0ab3 dc14  st   $14,[y,x++]
0ab4 dc3b  st   $3b,[y,x++]
0ab5 dc3f  st   $3f,[y,x++]
0ab6 dc14  st   $14,[y,x++]
0ab7 dc04  st   $04,[y,x++]
0ab8 dc14  st   $14,[y,x++]
0ab9 dc10  st   $10,[y,x++]
0aba dc14  st   $14,[y,x++]
0abb dc14  st   $14,[y,x++]
0abc dc14  st   $14,[y,x++]
* 7 times
0ac1 dc04  st   $04,[y,x++]
0ac2 dc2e  st   $2e,[y,x++]
0ac3 dc25  st   $25,[y,x++]
0ac4 dc14  st   $14,[y,x++]
0ac5 dc04  st   $04,[y,x++]
0ac6 dc3e  st   $3e,[y,x++]
0ac7 dc2a  st   $2a,[y,x++]
0ac8 dc10  st   $10,[y,x++]
0ac9 dc04  st   $04,[y,x++]
0aca dc15  st   $15,[y,x++]
0acb dc3f  st   $3f,[y,x++]
0acc dc29  st   $29,[y,x++]
0acd dc3f  st   $3f,[y,x++]
0ace dc14  st   $14,[y,x++]
0acf dc10  st   $10,[y,x++]
0ad0 dc15  st   $15,[y,x++]
0ad1 dc3f  st   $3f,[y,x++]
0ad2 dc19  st   $19,[y,x++]
0ad3 dc14  st   $14,[y,x++]
0ad4 dc15  st   $15,[y,x++]
0ad5 dc3f  st   $3f,[y,x++]
0ad6 dc3f  st   $3f,[y,x++]
0ad7 dc15  st   $15,[y,x++]
0ad8 dc00  st   $00,[y,x++]
0ad9 dc14  st   $14,[y,x++]
0ada dc14  st   $14,[y,x++]
0adb dc19  st   $19,[y,x++]
0adc dc2f  st   $2f,[y,x++]
0add dc14  st   $14,[y,x++]
0ade dc14  st   $14,[y,x++]
0adf dc14  st   $14,[y,x++]
0ae0 dc10  st   $10,[y,x++]
0ae1 dc29  st   $29,[y,x++]
0ae2 dc3f  st   $3f,[y,x++]
0ae3 dc15  st   $15,[y,x++]
0ae4 dc14  st   $14,[y,x++]
0ae5 dc14  st   $14,[y,x++]
0ae6 dc2a  st   $2a,[y,x++]
0ae7 dc3e  st   $3e,[y,x++]
0ae8 dc10  st   $10,[y,x++]
0ae9 dc14  st   $14,[y,x++]
0aea dc19  st   $19,[y,x++]
0aeb dc3f  st   $3f,[y,x++]
0aec dc15  st   $15,[y,x++]
0aed dc14  st   $14,[y,x++]
0aee dc2a  st   $2a,[y,x++]
0aef dc3f  st   $3f,[y,x++]
0af0 dc2a  st   $2a,[y,x++]
0af1 dc14  st   $14,[y,x++]
0af2 dc14  st   $14,[y,x++]
0af3 dc3f  st   $3f,[y,x++]
0af4 dc19  st   $19,[y,x++]
0af5 dc14  st   $14,[y,x++]
0af6 dc14  st   $14,[y,x++]
0af7 dc2a  st   $2a,[y,x++]
0af8 dc3f  st   $3f,[y,x++]
0af9 dc15  st   $15,[y,x++]
0afa dc29  st   $29,[y,x++]
0afb dc2a  st   $2a,[y,x++]
0afc dc2a  st   $2a,[y,x++]
0afd dc2f  st   $2f,[y,x++]
0afe dc2a  st   $2a,[y,x++]
0aff dc15  st   $15,[y,x++]
0b00 dc14  st   $14,[y,x++]
0b01 dc14  st   $14,[y,x++]
0b02 dc14  st   $14,[y,x++]
0b03 dc14  st   $14,[y,x++]
0b04 dc15  st   $15,[y,x++]
0b05 dc00  st   $00,[y,x++]
0b06 dc00  st   $00,[y,x++]
0b07 dc14  st   $14,[y,x++]
0b08 140f  ld   $0f,y
0b09 dc14  st   $14,[y,x++]
0b0a dc10  st   $10,[y,x++]
0b0b dc14  st   $14,[y,x++]
0b0c dc14  st   $14,[y,x++]
0b0d dc04  st   $04,[y,x++]
0b0e dc14  st   $14,[y,x++]
0b0f dc04  st   $04,[y,x++]
0b10 dc14  st   $14,[y,x++]
0b11 dc10  st   $10,[y,x++]
0b12 dc14  st   $14,[y,x++]
0b13 dc14  st   $14,[y,x++]
0b14 dc25  st   $25,[y,x++]
0b15 dc2e  st   $2e,[y,x++]
0b16 dc15  st   $15,[y,x++]
0b17 dc14  st   $14,[y,x++]
0b18 dc10  st   $10,[y,x++]
0b19 dc04  st   $04,[y,x++]
0b1a dc14  st   $14,[y,x++]
0b1b dc3a  st   $3a,[y,x++]
0b1c dc19  st   $19,[y,x++]
0b1d dc11  st   $11,[y,x++]
0b1e dc14  st   $14,[y,x++]
0b1f dc14  st   $14,[y,x++]
0b20 dc3e  st   $3e,[y,x++]
0b21 dc29  st   $29,[y,x++]
0b22 dc15  st   $15,[y,x++]
0b23 dc04  st   $04,[y,x++]
0b24 dc14  st   $14,[y,x++]
0b25 dc14  st   $14,[y,x++]
0b26 dc14  st   $14,[y,x++]
0b27 dc14  st   $14,[y,x++]
0b28 dc10  st   $10,[y,x++]
0b29 dc17  st   $17,[y,x++]
0b2a dc1a  st   $1a,[y,x++]
0b2b dc14  st   $14,[y,x++]
0b2c dc10  st   $10,[y,x++]
0b2d dc05  st   $05,[y,x++]
0b2e dc14  st   $14,[y,x++]
0b2f dc03  st   $03,[y,x++]
0b30 dc12  st   $12,[y,x++]
0b31 dc14  st   $14,[y,x++]
0b32 dc14  st   $14,[y,x++]
0b33 dc12  st   $12,[y,x++]
0b34 dc12  st   $12,[y,x++]
0b35 dc14  st   $14,[y,x++]
0b36 dc08  st   $08,[y,x++]
0b37 dc14  st   $14,[y,x++]
0b38 dc39  st   $39,[y,x++]
0b39 dc28  st   $28,[y,x++]
0b3a dc14  st   $14,[y,x++]
0b3b dc14  st   $14,[y,x++]
0b3c dc29  st   $29,[y,x++]
0b3d dc28  st   $28,[y,x++]
0b3e dc14  st   $14,[y,x++]
0b3f dc1f  st   $1f,[y,x++]
0b40 dc15  st   $15,[y,x++]
0b41 dc14  st   $14,[y,x++]
0b42 dc10  st   $10,[y,x++]
0b43 dc14  st   $14,[y,x++]
0b44 dc14  st   $14,[y,x++]
0b45 dc04  st   $04,[y,x++]
0b46 dc25  st   $25,[y,x++]
0b47 dc3f  st   $3f,[y,x++]
0b48 dc15  st   $15,[y,x++]
0b49 dc14  st   $14,[y,x++]
0b4a dc14  st   $14,[y,x++]
0b4b dc15  st   $15,[y,x++]
0b4c dc3e  st   $3e,[y,x++]
0b4d dc2a  st   $2a,[y,x++]
0b4e dc10  st   $10,[y,x++]
0b4f dc14  st   $14,[y,x++]
0b50 dc2a  st   $2a,[y,x++]
0b51 dc2e  st   $2e,[y,x++]
0b52 dc14  st   $14,[y,x++]
0b53 dc3a  st   $3a,[y,x++]
0b54 dc15  st   $15,[y,x++]
0b55 dc15  st   $15,[y,x++]
0b56 dc15  st   $15,[y,x++]
0b57 dc2f  st   $2f,[y,x++]
0b58 dc14  st   $14,[y,x++]
0b59 dc14  st   $14,[y,x++]
0b5a dc2a  st   $2a,[y,x++]
0b5b dc2a  st   $2a,[y,x++]
0b5c dc14  st   $14,[y,x++]
0b5d dc2a  st   $2a,[y,x++]
0b5e dc2a  st   $2a,[y,x++]
0b5f dc10  st   $10,[y,x++]
0b60 dc14  st   $14,[y,x++]
0b61 dc3e  st   $3e,[y,x++]
0b62 dc2a  st   $2a,[y,x++]
0b63 dc15  st   $15,[y,x++]
0b64 dc3f  st   $3f,[y,x++]
0b65 dc15  st   $15,[y,x++]
0b66 dc14  st   $14,[y,x++]
0b67 dc2a  st   $2a,[y,x++]
0b68 dc2e  st   $2e,[y,x++]
0b69 dc15  st   $15,[y,x++]
0b6a dc19  st   $19,[y,x++]
0b6b dc3f  st   $3f,[y,x++]
0b6c dc14  st   $14,[y,x++]
0b6d dc14  st   $14,[y,x++]
0b6e dc2b  st   $2b,[y,x++]
0b6f dc2a  st   $2a,[y,x++]
0b70 dc14  st   $14,[y,x++]
0b71 dc14  st   $14,[y,x++]
0b72 dc14  st   $14,[y,x++]
0b73 dc19  st   $19,[y,x++]
0b74 dc15  st   $15,[y,x++]
0b75 dc2b  st   $2b,[y,x++]
0b76 dc15  st   $15,[y,x++]
0b77 dc14  st   $14,[y,x++]
0b78 dc14  st   $14,[y,x++]
0b79 dc14  st   $14,[y,x++]
0b7a dc10  st   $10,[y,x++]
0b7b dc14  st   $14,[y,x++]
0b7c dc14  st   $14,[y,x++]
0b7d dc10  st   $10,[y,x++]
0b7e dc04  st   $04,[y,x++]
0b7f dc14  st   $14,[y,x++]
0b80 dc2a  st   $2a,[y,x++]
0b81 dc3f  st   $3f,[y,x++]
0b82 dc14  st   $14,[y,x++]
0b83 dc14  st   $14,[y,x++]
0b84 dc2a  st   $2a,[y,x++]
0b85 dc2a  st   $2a,[y,x++]
0b86 dc14  st   $14,[y,x++]
0b87 dc3f  st   $3f,[y,x++]
0b88 dc29  st   $29,[y,x++]
0b89 dc14  st   $14,[y,x++]
0b8a dc14  st   $14,[y,x++]
0b8b dc14  st   $14,[y,x++]
0b8c dc14  st   $14,[y,x++]
0b8d dc19  st   $19,[y,x++]
0b8e dc3f  st   $3f,[y,x++]
0b8f dc14  st   $14,[y,x++]
0b90 dc25  st   $25,[y,x++]
0b91 dc2b  st   $2b,[y,x++]
0b92 dc14  st   $14,[y,x++]
0b93 dc04  st   $04,[y,x++]
0b94 dc14  st   $14,[y,x++]
0b95 dc15  st   $15,[y,x++]
0b96 dc3e  st   $3e,[y,x++]
0b97 dc14  st   $14,[y,x++]
0b98 dc2a  st   $2a,[y,x++]
0b99 dc3f  st   $3f,[y,x++]
0b9a dc15  st   $15,[y,x++]
0b9b dc00  st   $00,[y,x++]
0b9c dc14  st   $14,[y,x++]
0b9d dc14  st   $14,[y,x++]
0b9e dc14  st   $14,[y,x++]
0b9f dc10  st   $10,[y,x++]
0ba0 dc14  st   $14,[y,x++]
0ba1 dc14  st   $14,[y,x++]
0ba2 dc15  st   $15,[y,x++]
0ba3 dc14  st   $14,[y,x++]
0ba4 dc14  st   $14,[y,x++]
0ba5 dc15  st   $15,[y,x++]
0ba6 dc14  st   $14,[y,x++]
0ba7 dc14  st   $14,[y,x++]
0ba8 dc15  st   $15,[y,x++]
0ba9 dc3f  st   $3f,[y,x++]
0baa dc19  st   $19,[y,x++]
0bab dc10  st   $10,[y,x++]
0bac dc2a  st   $2a,[y,x++]
0bad dc3f  st   $3f,[y,x++]
0bae dc15  st   $15,[y,x++]
0baf dc04  st   $04,[y,x++]
0bb0 dc14  st   $14,[y,x++]
0bb1 dc04  st   $04,[y,x++]
0bb2 dc2a  st   $2a,[y,x++]
0bb3 dc3b  st   $3b,[y,x++]
0bb4 dc14  st   $14,[y,x++]
0bb5 dc14  st   $14,[y,x++]
0bb6 dc3f  st   $3f,[y,x++]
0bb7 dc2a  st   $2a,[y,x++]
0bb8 dc14  st   $14,[y,x++]
0bb9 dc15  st   $15,[y,x++]
0bba dc04  st   $04,[y,x++]
0bbb dc14  st   $14,[y,x++]
0bbc dc10  st   $10,[y,x++]
0bbd dc04  st   $04,[y,x++]
0bbe dc04  st   $04,[y,x++]
0bbf dc11  st   $11,[y,x++]
0bc0 dc14  st   $14,[y,x++]
0bc1 dc14  st   $14,[y,x++]
0bc2 dc14  st   $14,[y,x++]
0bc3 dc3b  st   $3b,[y,x++]
0bc4 dc19  st   $19,[y,x++]
0bc5 dc15  st   $15,[y,x++]
0bc6 dc10  st   $10,[y,x++]
0bc7 dc2a  st   $2a,[y,x++]
0bc8 dc3f  st   $3f,[y,x++]
0bc9 dc14  st   $14,[y,x++]
0bca dc14  st   $14,[y,x++]
0bcb dc2a  st   $2a,[y,x++]
0bcc dc3f  st   $3f,[y,x++]
0bcd dc15  st   $15,[y,x++]
0bce dc3f  st   $3f,[y,x++]
0bcf dc19  st   $19,[y,x++]
0bd0 dc14  st   $14,[y,x++]
0bd1 dc15  st   $15,[y,x++]
0bd2 dc3f  st   $3f,[y,x++]
0bd3 dc15  st   $15,[y,x++]
0bd4 dc14  st   $14,[y,x++]
0bd5 dc3e  st   $3e,[y,x++]
0bd6 dc2a  st   $2a,[y,x++]
0bd7 dc2a  st   $2a,[y,x++]
0bd8 dc3e  st   $3e,[y,x++]
0bd9 dc15  st   $15,[y,x++]
0bda dc14  st   $14,[y,x++]
0bdb dc14  st   $14,[y,x++]
0bdc dc25  st   $25,[y,x++]
0bdd dc2a  st   $2a,[y,x++]
0bde dc14  st   $14,[y,x++]
0bdf dc04  st   $04,[y,x++]
0be0 dc14  st   $14,[y,x++]
0be1 dc2a  st   $2a,[y,x++]
0be2 dc3f  st   $3f,[y,x++]
0be3 dc19  st   $19,[y,x++]
0be4 dc14  st   $14,[y,x++]
0be5 dc14  st   $14,[y,x++]
0be6 dc04  st   $04,[y,x++]
0be7 dc15  st   $15,[y,x++]
0be8 dc3f  st   $3f,[y,x++]
0be9 dc15  st   $15,[y,x++]
0bea dc14  st   $14,[y,x++]
0beb dc2a  st   $2a,[y,x++]
0bec dc2a  st   $2a,[y,x++]
0bed dc14  st   $14,[y,x++]
0bee dc29  st   $29,[y,x++]
0bef dc3f  st   $3f,[y,x++]
0bf0 dc2a  st   $2a,[y,x++]
0bf1 dc3f  st   $3f,[y,x++]
0bf2 dc15  st   $15,[y,x++]
0bf3 dc14  st   $14,[y,x++]
0bf4 dc3f  st   $3f,[y,x++]
0bf5 dc2a  st   $2a,[y,x++]
0bf6 dc10  st   $10,[y,x++]
0bf7 dc14  st   $14,[y,x++]
0bf8 dc3e  st   $3e,[y,x++]
0bf9 dc2a  st   $2a,[y,x++]
0bfa dc14  st   $14,[y,x++]
0bfb dc14  st   $14,[y,x++]
0bfc dc04  st   $04,[y,x++]
0bfd dc14  st   $14,[y,x++]
0bfe dc3e  st   $3e,[y,x++]
0bff dc15  st   $15,[y,x++]
0c00 dc14  st   $14,[y,x++]
0c01 dc00  st   $00,[y,x++]
0c02 dc14  st   $14,[y,x++]
0c03 dc00  st   $00,[y,x++]
0c04 dc14  st   $14,[y,x++]
0c05 dc14  st   $14,[y,x++]
0c06 dc14  st   $14,[y,x++]
* 5 times
0c09 1410  ld   $10,y
0c0a dc14  st   $14,[y,x++]
0c0b dc05  st   $05,[y,x++]
0c0c dc14  st   $14,[y,x++]
0c0d dc04  st   $04,[y,x++]
0c0e dc10  st   $10,[y,x++]
0c0f dc14  st   $14,[y,x++]
0c10 dc10  st   $10,[y,x++]
0c11 dc14  st   $14,[y,x++]
0c12 dc14  st   $14,[y,x++]
0c13 dc14  st   $14,[y,x++]
0c14 dc14  st   $14,[y,x++]
0c15 dc19  st   $19,[y,x++]
0c16 dc3b  st   $3b,[y,x++]
0c17 dc14  st   $14,[y,x++]
0c18 dc14  st   $14,[y,x++]
0c19 dc04  st   $04,[y,x++]
0c1a dc14  st   $14,[y,x++]
0c1b dc10  st   $10,[y,x++]
0c1c dc2f  st   $2f,[y,x++]
0c1d dc29  st   $29,[y,x++]
0c1e dc14  st   $14,[y,x++]
0c1f dc14  st   $14,[y,x++]
0c20 dc15  st   $15,[y,x++]
0c21 dc3f  st   $3f,[y,x++]
0c22 dc3f  st   $3f,[y,x++]
0c23 dc3f  st   $3f,[y,x++]
* 5 times
0c26 dc15  st   $15,[y,x++]
0c27 dc14  st   $14,[y,x++]
0c28 dc14  st   $14,[y,x++]
0c29 dc10  st   $10,[y,x++]
0c2a dc05  st   $05,[y,x++]
0c2b dc1b  st   $1b,[y,x++]
0c2c dc07  st   $07,[y,x++]
0c2d dc1a  st   $1a,[y,x++]
0c2e dc0b  st   $0b,[y,x++]
0c2f dc14  st   $14,[y,x++]
0c30 dc11  st   $11,[y,x++]
0c31 dc03  st   $03,[y,x++]
0c32 dc12  st   $12,[y,x++]
0c33 dc02  st   $02,[y,x++]
0c34 dc13  st   $13,[y,x++]
0c35 dc14  st   $14,[y,x++]
0c36 dc04  st   $04,[y,x++]
0c37 dc1c  st   $1c,[y,x++]
0c38 dc04  st   $04,[y,x++]
0c39 dc15  st   $15,[y,x++]
0c3a dc38  st   $38,[y,x++]
0c3b dc29  st   $29,[y,x++]
0c3c dc38  st   $38,[y,x++]
0c3d dc39  st   $39,[y,x++]
0c3e dc14  st   $14,[y,x++]
0c3f dc14  st   $14,[y,x++]
0c40 dc0f  st   $0f,[y,x++]
0c41 dc05  st   $05,[y,x++]
0c42 dc04  st   $04,[y,x++]
0c43 dc15  st   $15,[y,x++]
0c44 dc15  st   $15,[y,x++]
0c45 dc10  st   $10,[y,x++]
0c46 dc14  st   $14,[y,x++]
0c47 dc14  st   $14,[y,x++]
0c48 dc2a  st   $2a,[y,x++]
0c49 dc3f  st   $3f,[y,x++]
0c4a dc2a  st   $2a,[y,x++]
0c4b dc3f  st   $3f,[y,x++]
0c4c dc2a  st   $2a,[y,x++]
0c4d dc15  st   $15,[y,x++]
0c4e dc3f  st   $3f,[y,x++]
0c4f dc3e  st   $3e,[y,x++]
0c50 dc3f  st   $3f,[y,x++]
0c51 dc3f  st   $3f,[y,x++]
0c52 dc15  st   $15,[y,x++]
0c53 dc14  st   $14,[y,x++]
0c54 dc3f  st   $3f,[y,x++]
0c55 dc15  st   $15,[y,x++]
0c56 dc14  st   $14,[y,x++]
0c57 dc29  st   $29,[y,x++]
0c58 dc3f  st   $3f,[y,x++]
0c59 dc14  st   $14,[y,x++]
0c5a dc14  st   $14,[y,x++]
0c5b dc2a  st   $2a,[y,x++]
0c5c dc2e  st   $2e,[y,x++]
0c5d dc14  st   $14,[y,x++]
0c5e dc3e  st   $3e,[y,x++]
0c5f dc3f  st   $3f,[y,x++]
0c60 dc2a  st   $2a,[y,x++]
0c61 dc3f  st   $3f,[y,x++]
0c62 dc3f  st   $3f,[y,x++]
0c63 dc15  st   $15,[y,x++]
0c64 dc14  st   $14,[y,x++]
0c65 dc3b  st   $3b,[y,x++]
0c66 dc3b  st   $3b,[y,x++]
0c67 dc2f  st   $2f,[y,x++]
0c68 dc3f  st   $3f,[y,x++]
0c69 dc3f  st   $3f,[y,x++]
0c6a dc14  st   $14,[y,x++]
0c6b dc15  st   $15,[y,x++]
0c6c dc3f  st   $3f,[y,x++]
0c6d dc3f  st   $3f,[y,x++]
0c6e dc2a  st   $2a,[y,x++]
0c6f dc15  st   $15,[y,x++]
0c70 dc3f  st   $3f,[y,x++]
0c71 dc2f  st   $2f,[y,x++]
0c72 dc2a  st   $2a,[y,x++]
0c73 dc3f  st   $3f,[y,x++]
0c74 dc2a  st   $2a,[y,x++]
0c75 dc15  st   $15,[y,x++]
0c76 dc3f  st   $3f,[y,x++]
0c77 dc14  st   $14,[y,x++]
0c78 dc04  st   $04,[y,x++]
0c79 dc14  st   $14,[y,x++]
0c7a dc14  st   $14,[y,x++]
0c7b dc04  st   $04,[y,x++]
0c7c dc14  st   $14,[y,x++]
0c7d dc14  st   $14,[y,x++]
0c7e dc01  st   $01,[y,x++]
0c7f dc14  st   $14,[y,x++]
0c80 dc14  st   $14,[y,x++]
0c81 dc14  st   $14,[y,x++]
0c82 dc3f  st   $3f,[y,x++]
0c83 dc3f  st   $3f,[y,x++]
0c84 dc2a  st   $2a,[y,x++]
0c85 dc3f  st   $3f,[y,x++]
0c86 dc29  st   $29,[y,x++]
0c87 dc14  st   $14,[y,x++]
0c88 dc2a  st   $2a,[y,x++]
0c89 dc2a  st   $2a,[y,x++]
0c8a dc14  st   $14,[y,x++]
0c8b dc14  st   $14,[y,x++]
0c8c dc14  st   $14,[y,x++]
0c8d dc14  st   $14,[y,x++]
0c8e dc29  st   $29,[y,x++]
0c8f dc3f  st   $3f,[y,x++]
0c90 dc14  st   $14,[y,x++]
0c91 dc29  st   $29,[y,x++]
0c92 dc2b  st   $2b,[y,x++]
0c93 dc14  st   $14,[y,x++]
0c94 dc14  st   $14,[y,x++]
0c95 dc14  st   $14,[y,x++]
0c96 dc25  st   $25,[y,x++]
0c97 dc3f  st   $3f,[y,x++]
0c98 dc14  st   $14,[y,x++]
0c99 dc3e  st   $3e,[y,x++]
0c9a dc3f  st   $3f,[y,x++]
0c9b dc2b  st   $2b,[y,x++]
0c9c dc2e  st   $2e,[y,x++]
0c9d dc2a  st   $2a,[y,x++]
0c9e dc14  st   $14,[y,x++]
0c9f dc14  st   $14,[y,x++]
0ca0 dc14  st   $14,[y,x++]
0ca1 dc10  st   $10,[y,x++]
0ca2 dc14  st   $14,[y,x++]
0ca3 dc04  st   $04,[y,x++]
0ca4 dc04  st   $04,[y,x++]
0ca5 dc14  st   $14,[y,x++]
0ca6 dc3b  st   $3b,[y,x++]
0ca7 dc3e  st   $3e,[y,x++]
0ca8 dc2a  st   $2a,[y,x++]
0ca9 dc3f  st   $3f,[y,x++]
0caa dc2a  st   $2a,[y,x++]
0cab dc14  st   $14,[y,x++]
0cac dc15  st   $15,[y,x++]
0cad dc3f  st   $3f,[y,x++]
0cae dc3f  st   $3f,[y,x++]
0caf dc2a  st   $2a,[y,x++]
0cb0 dc2f  st   $2f,[y,x++]
0cb1 dc3a  st   $3a,[y,x++]
0cb2 dc25  st   $25,[y,x++]
0cb3 dc29  st   $29,[y,x++]
0cb4 dc2f  st   $2f,[y,x++]
0cb5 dc14  st   $14,[y,x++]
0cb6 dc14  st   $14,[y,x++]
0cb7 dc19  st   $19,[y,x++]
0cb8 dc3f  st   $3f,[y,x++]
0cb9 dc2a  st   $2a,[y,x++]
0cba dc14  st   $14,[y,x++]
0cbb dc04  st   $04,[y,x++]
0cbc dc14  st   $14,[y,x++]
0cbd dc14  st   $14,[y,x++]
0cbe dc14  st   $14,[y,x++]
0cbf dc04  st   $04,[y,x++]
0cc0 dc10  st   $10,[y,x++]
0cc1 dc14  st   $14,[y,x++]
0cc2 dc15  st   $15,[y,x++]
0cc3 dc2a  st   $2a,[y,x++]
0cc4 dc3f  st   $3f,[y,x++]
0cc5 dc3f  st   $3f,[y,x++]
0cc6 dc2a  st   $2a,[y,x++]
0cc7 dc14  st   $14,[y,x++]
0cc8 dc15  st   $15,[y,x++]
0cc9 dc2b  st   $2b,[y,x++]
0cca dc3f  st   $3f,[y,x++]
0ccb dc2a  st   $2a,[y,x++]
0ccc dc3f  st   $3f,[y,x++]
0ccd dc15  st   $15,[y,x++]
0cce dc04  st   $04,[y,x++]
0ccf dc2a  st   $2a,[y,x++]
0cd0 dc3f  st   $3f,[y,x++]
0cd1 dc2a  st   $2a,[y,x++]
0cd2 dc3f  st   $3f,[y,x++]
0cd3 dc2a  st   $2a,[y,x++]
0cd4 dc14  st   $14,[y,x++]
0cd5 dc2a  st   $2a,[y,x++]
0cd6 dc3f  st   $3f,[y,x++]
0cd7 dc14  st   $14,[y,x++]
0cd8 dc14  st   $14,[y,x++]
0cd9 dc3f  st   $3f,[y,x++]
0cda dc2a  st   $2a,[y,x++]
0cdb dc14  st   $14,[y,x++]
0cdc dc2a  st   $2a,[y,x++]
0cdd dc2f  st   $2f,[y,x++]
0cde dc3f  st   $3f,[y,x++]
0cdf dc2a  st   $2a,[y,x++]
0ce0 dc15  st   $15,[y,x++]
0ce1 dc14  st   $14,[y,x++]
0ce2 dc3f  st   $3f,[y,x++]
0ce3 dc3f  st   $3f,[y,x++]
0ce4 dc3f  st   $3f,[y,x++]
0ce5 dc3a  st   $3a,[y,x++]
0ce6 dc2e  st   $2e,[y,x++]
0ce7 dc2a  st   $2a,[y,x++]
0ce8 dc14  st   $14,[y,x++]
0ce9 dc2a  st   $2a,[y,x++]
0cea dc2f  st   $2f,[y,x++]
0ceb dc2a  st   $2a,[y,x++]
0cec dc3f  st   $3f,[y,x++]
0ced dc19  st   $19,[y,x++]
0cee dc14  st   $14,[y,x++]
0cef dc3f  st   $3f,[y,x++]
0cf0 dc2a  st   $2a,[y,x++]
0cf1 dc04  st   $04,[y,x++]
0cf2 dc2a  st   $2a,[y,x++]
0cf3 dc3f  st   $3f,[y,x++]
0cf4 dc15  st   $15,[y,x++]
0cf5 dc19  st   $19,[y,x++]
0cf6 dc3f  st   $3f,[y,x++]
0cf7 dc2a  st   $2a,[y,x++]
0cf8 dc3f  st   $3f,[y,x++]
0cf9 dc3f  st   $3f,[y,x++]
0cfa dc15  st   $15,[y,x++]
0cfb dc14  st   $14,[y,x++]
0cfc dc15  st   $15,[y,x++]
0cfd dc14  st   $14,[y,x++]
0cfe dc14  st   $14,[y,x++]
0cff dc3f  st   $3f,[y,x++]
0d00 dc15  st   $15,[y,x++]
0d01 dc14  st   $14,[y,x++]
0d02 dc14  st   $14,[y,x++]
0d03 dc15  st   $15,[y,x++]
0d04 dc15  st   $15,[y,x++]
0d05 dc14  st   $14,[y,x++]
0d06 dc04  st   $04,[y,x++]
0d07 dc14  st   $14,[y,x++]
0d08 dc14  st   $14,[y,x++]
0d09 dc00  st   $00,[y,x++]
0d0a 1411  ld   $11,y
0d0b dc04  st   $04,[y,x++]
0d0c dc14  st   $14,[y,x++]
0d0d dc10  st   $10,[y,x++]
0d0e dc14  st   $14,[y,x++]
0d0f dc14  st   $14,[y,x++]
0d10 dc04  st   $04,[y,x++]
0d11 dc14  st   $14,[y,x++]
0d12 dc14  st   $14,[y,x++]
0d13 dc15  st   $15,[y,x++]
0d14 dc04  st   $04,[y,x++]
0d15 dc10  st   $10,[y,x++]
0d16 dc15  st   $15,[y,x++]
0d17 dc15  st   $15,[y,x++]
0d18 dc14  st   $14,[y,x++]
0d19 dc10  st   $10,[y,x++]
0d1a dc14  st   $14,[y,x++]
0d1b dc04  st   $04,[y,x++]
0d1c dc14  st   $14,[y,x++]
0d1d dc15  st   $15,[y,x++]
0d1e dc14  st   $14,[y,x++]
0d1f dc14  st   $14,[y,x++]
0d20 dc11  st   $11,[y,x++]
0d21 dc14  st   $14,[y,x++]
0d22 dc15  st   $15,[y,x++]
0d23 dc15  st   $15,[y,x++]
0d24 dc25  st   $25,[y,x++]
0d25 dc15  st   $15,[y,x++]
0d26 dc15  st   $15,[y,x++]
0d27 dc14  st   $14,[y,x++]
0d28 dc14  st   $14,[y,x++]
0d29 dc05  st   $05,[y,x++]
0d2a dc14  st   $14,[y,x++]
0d2b dc14  st   $14,[y,x++]
0d2c dc14  st   $14,[y,x++]
0d2d dc05  st   $05,[y,x++]
0d2e dc15  st   $15,[y,x++]
0d2f dc15  st   $15,[y,x++]
0d30 dc00  st   $00,[y,x++]
0d31 dc14  st   $14,[y,x++]
0d32 dc05  st   $05,[y,x++]
0d33 dc15  st   $15,[y,x++]
0d34 dc02  st   $02,[y,x++]
0d35 dc05  st   $05,[y,x++]
0d36 dc14  st   $14,[y,x++]
0d37 dc14  st   $14,[y,x++]
0d38 dc05  st   $05,[y,x++]
0d39 dc14  st   $14,[y,x++]
0d3a dc10  st   $10,[y,x++]
0d3b dc14  st   $14,[y,x++]
0d3c dc29  st   $29,[y,x++]
0d3d dc14  st   $14,[y,x++]
0d3e dc15  st   $15,[y,x++]
0d3f dc14  st   $14,[y,x++]
0d40 dc14  st   $14,[y,x++]
0d41 dc15  st   $15,[y,x++]
0d42 dc14  st   $14,[y,x++]
0d43 dc14  st   $14,[y,x++]
0d44 dc10  st   $10,[y,x++]
0d45 dc04  st   $04,[y,x++]
0d46 dc14  st   $14,[y,x++]
0d47 dc04  st   $04,[y,x++]
0d48 dc14  st   $14,[y,x++]
0d49 dc14  st   $14,[y,x++]
0d4a dc15  st   $15,[y,x++]
0d4b dc19  st   $19,[y,x++]
0d4c dc15  st   $15,[y,x++]
0d4d dc14  st   $14,[y,x++]
0d4e dc14  st   $14,[y,x++]
0d4f dc14  st   $14,[y,x++]
0d50 dc16  st   $16,[y,x++]
0d51 dc15  st   $15,[y,x++]
0d52 dc15  st   $15,[y,x++]
0d53 dc14  st   $14,[y,x++]
0d54 dc14  st   $14,[y,x++]
0d55 dc15  st   $15,[y,x++]
0d56 dc14  st   $14,[y,x++]
0d57 dc04  st   $04,[y,x++]
0d58 dc14  st   $14,[y,x++]
0d59 dc15  st   $15,[y,x++]
0d5a dc11  st   $11,[y,x++]
0d5b dc14  st   $14,[y,x++]
0d5c dc15  st   $15,[y,x++]
0d5d dc15  st   $15,[y,x++]
0d5e dc14  st   $14,[y,x++]
0d5f dc2a  st   $2a,[y,x++]
0d60 dc2a  st   $2a,[y,x++]
0d61 dc29  st   $29,[y,x++]
0d62 dc15  st   $15,[y,x++]
0d63 dc14  st   $14,[y,x++]
0d64 dc04  st   $04,[y,x++]
0d65 dc14  st   $14,[y,x++]
0d66 dc14  st   $14,[y,x++]
0d67 dc19  st   $19,[y,x++]
0d68 dc25  st   $25,[y,x++]
0d69 dc14  st   $14,[y,x++]
0d6a dc25  st   $25,[y,x++]
0d6b dc14  st   $14,[y,x++]
0d6c dc14  st   $14,[y,x++]
0d6d dc15  st   $15,[y,x++]
0d6e dc15  st   $15,[y,x++]
0d6f dc25  st   $25,[y,x++]
0d70 dc14  st   $14,[y,x++]
0d71 dc14  st   $14,[y,x++]
0d72 dc25  st   $25,[y,x++]
0d73 dc25  st   $25,[y,x++]
0d74 dc19  st   $19,[y,x++]
0d75 dc14  st   $14,[y,x++]
0d76 dc15  st   $15,[y,x++]
0d77 dc25  st   $25,[y,x++]
0d78 dc14  st   $14,[y,x++]
0d79 dc10  st   $10,[y,x++]
0d7a dc15  st   $15,[y,x++]
0d7b dc14  st   $14,[y,x++]
0d7c dc14  st   $14,[y,x++]
0d7d dc10  st   $10,[y,x++]
0d7e dc14  st   $14,[y,x++]
0d7f dc14  st   $14,[y,x++]
0d80 dc14  st   $14,[y,x++]
0d81 dc11  st   $11,[y,x++]
0d82 dc04  st   $04,[y,x++]
0d83 dc14  st   $14,[y,x++]
0d84 dc15  st   $15,[y,x++]
0d85 dc29  st   $29,[y,x++]
0d86 dc15  st   $15,[y,x++]
0d87 dc14  st   $14,[y,x++]
0d88 dc14  st   $14,[y,x++]
0d89 dc15  st   $15,[y,x++]
0d8a dc15  st   $15,[y,x++]
0d8b dc10  st   $10,[y,x++]
0d8c dc04  st   $04,[y,x++]
0d8d dc14  st   $14,[y,x++]
0d8e dc00  st   $00,[y,x++]
0d8f dc15  st   $15,[y,x++]
0d90 dc15  st   $15,[y,x++]
0d91 dc14  st   $14,[y,x++]
0d92 dc15  st   $15,[y,x++]
0d93 dc15  st   $15,[y,x++]
0d94 dc14  st   $14,[y,x++]
0d95 dc10  st   $10,[y,x++]
0d96 dc04  st   $04,[y,x++]
0d97 dc14  st   $14,[y,x++]
0d98 dc19  st   $19,[y,x++]
0d99 dc15  st   $15,[y,x++]
0d9a dc15  st   $15,[y,x++]
0d9b dc19  st   $19,[y,x++]
0d9c dc29  st   $29,[y,x++]
0d9d dc25  st   $25,[y,x++]
0d9e dc15  st   $15,[y,x++]
0d9f dc15  st   $15,[y,x++]
0da0 dc00  st   $00,[y,x++]
0da1 dc14  st   $14,[y,x++]
0da2 dc14  st   $14,[y,x++]
0da3 dc14  st   $14,[y,x++]
0da4 dc10  st   $10,[y,x++]
0da5 dc15  st   $15,[y,x++]
0da6 dc14  st   $14,[y,x++]
0da7 dc15  st   $15,[y,x++]
0da8 dc29  st   $29,[y,x++]
0da9 dc29  st   $29,[y,x++]
0daa dc15  st   $15,[y,x++]
0dab dc15  st   $15,[y,x++]
0dac dc04  st   $04,[y,x++]
0dad dc14  st   $14,[y,x++]
0dae dc29  st   $29,[y,x++]
0daf dc15  st   $15,[y,x++]
0db0 dc29  st   $29,[y,x++]
0db1 dc25  st   $25,[y,x++]
0db2 dc19  st   $19,[y,x++]
0db3 dc14  st   $14,[y,x++]
0db4 dc15  st   $15,[y,x++]
0db5 dc15  st   $15,[y,x++]
0db6 dc10  st   $10,[y,x++]
0db7 dc14  st   $14,[y,x++]
0db8 dc14  st   $14,[y,x++]
0db9 dc15  st   $15,[y,x++]
0dba dc15  st   $15,[y,x++]
0dbb dc10  st   $10,[y,x++]
0dbc dc14  st   $14,[y,x++]
0dbd dc04  st   $04,[y,x++]
0dbe dc11  st   $11,[y,x++]
0dbf dc14  st   $14,[y,x++]
0dc0 dc15  st   $15,[y,x++]
0dc1 dc14  st   $14,[y,x++]
0dc2 dc14  st   $14,[y,x++]
0dc3 dc15  st   $15,[y,x++]
0dc4 dc29  st   $29,[y,x++]
0dc5 dc29  st   $29,[y,x++]
0dc6 dc15  st   $15,[y,x++]
0dc7 dc15  st   $15,[y,x++]
0dc8 dc14  st   $14,[y,x++]
0dc9 dc14  st   $14,[y,x++]
0dca dc14  st   $14,[y,x++]
0dcb dc29  st   $29,[y,x++]
0dcc dc29  st   $29,[y,x++]
0dcd dc15  st   $15,[y,x++]
0dce dc14  st   $14,[y,x++]
0dcf dc14  st   $14,[y,x++]
0dd0 dc14  st   $14,[y,x++]
0dd1 dc25  st   $25,[y,x++]
0dd2 dc1a  st   $1a,[y,x++]
0dd3 dc15  st   $15,[y,x++]
0dd4 dc14  st   $14,[y,x++]
0dd5 dc14  st   $14,[y,x++]
0dd6 dc15  st   $15,[y,x++]
0dd7 dc15  st   $15,[y,x++]
0dd8 dc14  st   $14,[y,x++]
0dd9 dc14  st   $14,[y,x++]
0dda dc15  st   $15,[y,x++]
0ddb dc15  st   $15,[y,x++]
0ddc dc04  st   $04,[y,x++]
0ddd dc25  st   $25,[y,x++]
0dde dc29  st   $29,[y,x++]
0ddf dc25  st   $25,[y,x++]
0de0 dc29  st   $29,[y,x++]
0de1 dc15  st   $15,[y,x++]
0de2 dc14  st   $14,[y,x++]
0de3 dc15  st   $15,[y,x++]
0de4 dc29  st   $29,[y,x++]
0de5 dc15  st   $15,[y,x++]
0de6 dc1a  st   $1a,[y,x++]
0de7 dc25  st   $25,[y,x++]
0de8 dc15  st   $15,[y,x++]
0de9 dc14  st   $14,[y,x++]
0dea dc15  st   $15,[y,x++]
0deb dc25  st   $25,[y,x++]
0dec dc29  st   $29,[y,x++]
0ded dc15  st   $15,[y,x++]
0dee dc14  st   $14,[y,x++]
0def dc15  st   $15,[y,x++]
0df0 dc19  st   $19,[y,x++]
0df1 dc14  st   $14,[y,x++]
0df2 dc10  st   $10,[y,x++]
0df3 dc14  st   $14,[y,x++]
0df4 dc25  st   $25,[y,x++]
0df5 dc14  st   $14,[y,x++]
0df6 dc14  st   $14,[y,x++]
0df7 dc15  st   $15,[y,x++]
0df8 dc29  st   $29,[y,x++]
0df9 dc15  st   $15,[y,x++]
0dfa dc14  st   $14,[y,x++]
0dfb dc14  st   $14,[y,x++]
0dfc dc14  st   $14,[y,x++]
0dfd dc00  st   $00,[y,x++]
0dfe dc10  st   $10,[y,x++]
0dff dc14  st   $14,[y,x++]
0e00 dc15  st   $15,[y,x++]
0e01 dc15  st   $15,[y,x++]
0e02 dc00  st   $00,[y,x++]
0e03 dc04  st   $04,[y,x++]
0e04 dc14  st   $14,[y,x++]
0e05 dc14  st   $14,[y,x++]
0e06 dc00  st   $00,[y,x++]
0e07 dc10  st   $10,[y,x++]
0e08 dc14  st   $14,[y,x++]
0e09 dc15  st   $15,[y,x++]
0e0a dc14  st   $14,[y,x++]
0e0b 1412  ld   $12,y
0e0c dc14  st   $14,[y,x++]
0e0d dc10  st   $10,[y,x++]
0e0e dc14  st   $14,[y,x++]
0e0f dc10  st   $10,[y,x++]
0e10 dc05  st   $05,[y,x++]
0e11 dc14  st   $14,[y,x++]
0e12 dc14  st   $14,[y,x++]
0e13 dc05  st   $05,[y,x++]
0e14 dc10  st   $10,[y,x++]
0e15 dc14  st   $14,[y,x++]
0e16 dc04  st   $04,[y,x++]
0e17 dc14  st   $14,[y,x++]
0e18 dc10  st   $10,[y,x++]
0e19 dc14  st   $14,[y,x++]
0e1a dc14  st   $14,[y,x++]
0e1b dc04  st   $04,[y,x++]
0e1c dc14  st   $14,[y,x++]
0e1d dc10  st   $10,[y,x++]
0e1e dc14  st   $14,[y,x++]
0e1f dc04  st   $04,[y,x++]
0e20 dc04  st   $04,[y,x++]
0e21 dc14  st   $14,[y,x++]
0e22 dc10  st   $10,[y,x++]
0e23 dc04  st   $04,[y,x++]
0e24 dc14  st   $14,[y,x++]
0e25 dc14  st   $14,[y,x++]
0e26 dc04  st   $04,[y,x++]
0e27 dc14  st   $14,[y,x++]
0e28 dc10  st   $10,[y,x++]
0e29 dc14  st   $14,[y,x++]
0e2a dc14  st   $14,[y,x++]
0e2b dc04  st   $04,[y,x++]
0e2c dc14  st   $14,[y,x++]
0e2d dc14  st   $14,[y,x++]
0e2e dc14  st   $14,[y,x++]
* 5 times
0e31 dc15  st   $15,[y,x++]
0e32 dc14  st   $14,[y,x++]
0e33 dc00  st   $00,[y,x++]
0e34 dc04  st   $04,[y,x++]
0e35 dc14  st   $14,[y,x++]
0e36 dc14  st   $14,[y,x++]
0e37 dc10  st   $10,[y,x++]
0e38 dc10  st   $10,[y,x++]
0e39 dc14  st   $14,[y,x++]
0e3a dc14  st   $14,[y,x++]
0e3b dc14  st   $14,[y,x++]
* 5 times
0e3e dc10  st   $10,[y,x++]
0e3f dc04  st   $04,[y,x++]
0e40 dc14  st   $14,[y,x++]
0e41 dc14  st   $14,[y,x++]
0e42 dc05  st   $05,[y,x++]
0e43 dc14  st   $14,[y,x++]
0e44 dc04  st   $04,[y,x++]
0e45 dc14  st   $14,[y,x++]
0e46 dc10  st   $10,[y,x++]
0e47 dc14  st   $14,[y,x++]
0e48 dc14  st   $14,[y,x++]
0e49 dc10  st   $10,[y,x++]
0e4a dc14  st   $14,[y,x++]
0e4b dc14  st   $14,[y,x++]
0e4c dc11  st   $11,[y,x++]
0e4d dc14  st   $14,[y,x++]
0e4e dc14  st   $14,[y,x++]
0e4f dc14  st   $14,[y,x++]
* 8 times
0e55 dc04  st   $04,[y,x++]
0e56 dc14  st   $14,[y,x++]
0e57 dc14  st   $14,[y,x++]
0e58 dc14  st   $14,[y,x++]
0e59 dc11  st   $11,[y,x++]
0e5a dc14  st   $14,[y,x++]
0e5b dc04  st   $04,[y,x++]
0e5c dc14  st   $14,[y,x++]
0e5d dc10  st   $10,[y,x++]
0e5e dc14  st   $14,[y,x++]
0e5f dc04  st   $04,[y,x++]
0e60 dc2b  st   $2b,[y,x++]
0e61 dc29  st   $29,[y,x++]
0e62 dc14  st   $14,[y,x++]
0e63 dc14  st   $14,[y,x++]
0e64 dc11  st   $11,[y,x++]
0e65 dc04  st   $04,[y,x++]
0e66 dc14  st   $14,[y,x++]
0e67 dc14  st   $14,[y,x++]
0e68 dc04  st   $04,[y,x++]
0e69 dc14  st   $14,[y,x++]
0e6a dc14  st   $14,[y,x++]
0e6b dc14  st   $14,[y,x++]
0e6c dc14  st   $14,[y,x++]
0e6d dc10  st   $10,[y,x++]
0e6e dc04  st   $04,[y,x++]
0e6f dc14  st   $14,[y,x++]
0e70 dc14  st   $14,[y,x++]
0e71 dc14  st   $14,[y,x++]
0e72 dc04  st   $04,[y,x++]
0e73 dc14  st   $14,[y,x++]
0e74 dc14  st   $14,[y,x++]
0e75 dc14  st   $14,[y,x++]
0e76 dc14  st   $14,[y,x++]
0e77 dc04  st   $04,[y,x++]
0e78 dc10  st   $10,[y,x++]
0e79 dc14  st   $14,[y,x++]
0e7a dc14  st   $14,[y,x++]
0e7b dc04  st   $04,[y,x++]
0e7c dc14  st   $14,[y,x++]
0e7d dc01  st   $01,[y,x++]
0e7e dc14  st   $14,[y,x++]
0e7f dc14  st   $14,[y,x++]
0e80 dc04  st   $04,[y,x++]
0e81 dc14  st   $14,[y,x++]
0e82 dc10  st   $10,[y,x++]
0e83 dc14  st   $14,[y,x++]
0e84 dc14  st   $14,[y,x++]
0e85 dc00  st   $00,[y,x++]
0e86 dc14  st   $14,[y,x++]
0e87 dc14  st   $14,[y,x++]
0e88 dc00  st   $00,[y,x++]
0e89 dc14  st   $14,[y,x++]
0e8a dc04  st   $04,[y,x++]
0e8b dc14  st   $14,[y,x++]
0e8c dc04  st   $04,[y,x++]
0e8d dc10  st   $10,[y,x++]
0e8e dc15  st   $15,[y,x++]
0e8f dc14  st   $14,[y,x++]
0e90 dc10  st   $10,[y,x++]
0e91 dc04  st   $04,[y,x++]
0e92 dc14  st   $14,[y,x++]
0e93 dc14  st   $14,[y,x++]
0e94 dc10  st   $10,[y,x++]
0e95 dc04  st   $04,[y,x++]
0e96 dc14  st   $14,[y,x++]
0e97 dc14  st   $14,[y,x++]
0e98 dc14  st   $14,[y,x++]
0e99 dc00  st   $00,[y,x++]
0e9a dc14  st   $14,[y,x++]
0e9b dc15  st   $15,[y,x++]
0e9c dc14  st   $14,[y,x++]
0e9d dc10  st   $10,[y,x++]
0e9e dc10  st   $10,[y,x++]
0e9f dc14  st   $14,[y,x++]
0ea0 dc04  st   $04,[y,x++]
0ea1 dc14  st   $14,[y,x++]
0ea2 dc15  st   $15,[y,x++]
0ea3 dc04  st   $04,[y,x++]
0ea4 dc14  st   $14,[y,x++]
0ea5 dc04  st   $04,[y,x++]
0ea6 dc10  st   $10,[y,x++]
0ea7 dc14  st   $14,[y,x++]
0ea8 dc14  st   $14,[y,x++]
0ea9 dc04  st   $04,[y,x++]
0eaa dc14  st   $14,[y,x++]
0eab dc10  st   $10,[y,x++]
0eac dc10  st   $10,[y,x++]
0ead dc14  st   $14,[y,x++]
0eae dc14  st   $14,[y,x++]
0eaf dc04  st   $04,[y,x++]
0eb0 dc14  st   $14,[y,x++]
0eb1 dc15  st   $15,[y,x++]
0eb2 dc14  st   $14,[y,x++]
0eb3 dc10  st   $10,[y,x++]
0eb4 dc14  st   $14,[y,x++]
0eb5 dc14  st   $14,[y,x++]
0eb6 dc14  st   $14,[y,x++]
0eb7 dc04  st   $04,[y,x++]
0eb8 dc10  st   $10,[y,x++]
0eb9 dc14  st   $14,[y,x++]
0eba dc14  st   $14,[y,x++]
0ebb dc14  st   $14,[y,x++]
0ebc dc14  st   $14,[y,x++]
0ebd dc00  st   $00,[y,x++]
0ebe dc14  st   $14,[y,x++]
0ebf dc14  st   $14,[y,x++]
0ec0 dc14  st   $14,[y,x++]
0ec1 dc00  st   $00,[y,x++]
0ec2 dc14  st   $14,[y,x++]
0ec3 dc14  st   $14,[y,x++]
0ec4 dc14  st   $14,[y,x++]
* 5 times
0ec7 dc00  st   $00,[y,x++]
0ec8 dc00  st   $00,[y,x++]
0ec9 dc14  st   $14,[y,x++]
0eca dc14  st   $14,[y,x++]
0ecb dc14  st   $14,[y,x++]
0ecc dc10  st   $10,[y,x++]
0ecd dc04  st   $04,[y,x++]
0ece dc14  st   $14,[y,x++]
0ecf dc04  st   $04,[y,x++]
0ed0 dc14  st   $14,[y,x++]
0ed1 dc10  st   $10,[y,x++]
0ed2 dc14  st   $14,[y,x++]
0ed3 dc10  st   $10,[y,x++]
0ed4 dc14  st   $14,[y,x++]
0ed5 dc14  st   $14,[y,x++]
0ed6 dc14  st   $14,[y,x++]
0ed7 dc04  st   $04,[y,x++]
0ed8 dc14  st   $14,[y,x++]
0ed9 dc04  st   $04,[y,x++]
0eda dc10  st   $10,[y,x++]
0edb dc14  st   $14,[y,x++]
0edc dc14  st   $14,[y,x++]
0edd dc14  st   $14,[y,x++]
0ede dc10  st   $10,[y,x++]
0edf dc14  st   $14,[y,x++]
0ee0 dc14  st   $14,[y,x++]
0ee1 dc04  st   $04,[y,x++]
0ee2 dc14  st   $14,[y,x++]
0ee3 dc10  st   $10,[y,x++]
0ee4 dc14  st   $14,[y,x++]
0ee5 dc04  st   $04,[y,x++]
0ee6 dc04  st   $04,[y,x++]
0ee7 dc10  st   $10,[y,x++]
0ee8 dc14  st   $14,[y,x++]
0ee9 dc14  st   $14,[y,x++]
0eea dc14  st   $14,[y,x++]
* 5 times
0eed dc10  st   $10,[y,x++]
0eee dc14  st   $14,[y,x++]
0eef dc14  st   $14,[y,x++]
0ef0 dc14  st   $14,[y,x++]
0ef1 dc10  st   $10,[y,x++]
0ef2 dc14  st   $14,[y,x++]
0ef3 dc05  st   $05,[y,x++]
0ef4 dc14  st   $14,[y,x++]
0ef5 dc14  st   $14,[y,x++]
0ef6 dc14  st   $14,[y,x++]
0ef7 dc00  st   $00,[y,x++]
0ef8 dc00  st   $00,[y,x++]
0ef9 dc15  st   $15,[y,x++]
0efa dc14  st   $14,[y,x++]
0efb dc14  st   $14,[y,x++]
0efc dc14  st   $14,[y,x++]
0efd dc00  st   $00,[y,x++]
0efe dc04  st   $04,[y,x++]
0eff dc14  st   $14,[y,x++]
0f00 dc14  st   $14,[y,x++]
0f01 dc14  st   $14,[y,x++]
* 5 times
0f04 dc10  st   $10,[y,x++]
0f05 dc14  st   $14,[y,x++]
0f06 dc14  st   $14,[y,x++]
0f07 dc14  st   $14,[y,x++]
* 6 times
0f0b dc00  st   $00,[y,x++]
0f0c 1000  ld   $00,x
0f0d 1413  ld   $13,y
0f0e dc00  st   $00,[y,x++]
0f0f dc00  st   $00,[y,x++]
0f10 dc00  st   $00,[y,x++]
* 160 times
0fae 1000  ld   $00,x
0faf 1414  ld   $14,y
0fb0 dc00  st   $00,[y,x++]
0fb1 dc00  st   $00,[y,x++]
0fb2 dc00  st   $00,[y,x++]
* 160 times
1050 1000  ld   $00,x
1051 1415  ld   $15,y
1052 dc00  st   $00,[y,x++]
1053 dc00  st   $00,[y,x++]
1054 dc00  st   $00,[y,x++]
* 160 times
10f2 1000  ld   $00,x
10f3 1416  ld   $16,y
10f4 dc00  st   $00,[y,x++]
10f5 dc00  st   $00,[y,x++]
10f6 dc00  st   $00,[y,x++]
* 160 times
1194 1000  ld   $00,x
1195 1417  ld   $17,y
1196 dc00  st   $00,[y,x++]
1197 dc00  st   $00,[y,x++]
1198 dc00  st   $00,[y,x++]
* 160 times
1236 1000  ld   $00,x
1237 1418  ld   $18,y
1238 dc00  st   $00,[y,x++]
1239 dc00  st   $00,[y,x++]
123a dc00  st   $00,[y,x++]
* 160 times
12d8 1000  ld   $00,x
12d9 1419  ld   $19,y
12da dc00  st   $00,[y,x++]
12db dc00  st   $00,[y,x++]
12dc dc00  st   $00,[y,x++]
* 160 times
137a 1000  ld   $00,x
137b 141a  ld   $1a,y
137c dc00  st   $00,[y,x++]
137d dc00  st   $00,[y,x++]
137e dc00  st   $00,[y,x++]
* 160 times
141c 1000  ld   $00,x
141d 141b  ld   $1b,y
141e dc00  st   $00,[y,x++]
141f dc00  st   $00,[y,x++]
1420 dc00  st   $00,[y,x++]
* 160 times
14be 1000  ld   $00,x
14bf 141c  ld   $1c,y
14c0 dc00  st   $00,[y,x++]
14c1 dc00  st   $00,[y,x++]
14c2 dc00  st   $00,[y,x++]
* 160 times
1560 1000  ld   $00,x
1561 141d  ld   $1d,y
1562 dc00  st   $00,[y,x++]
1563 dc00  st   $00,[y,x++]
1564 dc00  st   $00,[y,x++]
* 160 times
1602 1000  ld   $00,x
1603 141e  ld   $1e,y
1604 dc00  st   $00,[y,x++]
1605 dc00  st   $00,[y,x++]
1606 dc00  st   $00,[y,x++]
* 160 times
16a4 1000  ld   $00,x
16a5 141f  ld   $1f,y
16a6 dc00  st   $00,[y,x++]
16a7 dc00  st   $00,[y,x++]
16a8 dc00  st   $00,[y,x++]
* 160 times
1746 1000  ld   $00,x
1747 1420  ld   $20,y
1748 dc00  st   $00,[y,x++]
1749 dc00  st   $00,[y,x++]
174a dc00  st   $00,[y,x++]
* 6 times
174e dc28  st   $28,[y,x++]
174f dc28  st   $28,[y,x++]
1750 dc28  st   $28,[y,x++]
* 12 times
175a dc00  st   $00,[y,x++]
175b dc00  st   $00,[y,x++]
175c dc00  st   $00,[y,x++]
* 5 times
175f dc30  st   $30,[y,x++]
1760 dc30  st   $30,[y,x++]
1761 dc30  st   $30,[y,x++]
* 12 times
176b dc00  st   $00,[y,x++]
176c dc00  st   $00,[y,x++]
176d dc00  st   $00,[y,x++]
* 5 times
1770 dc38  st   $38,[y,x++]
1771 dc38  st   $38,[y,x++]
1772 dc38  st   $38,[y,x++]
* 12 times
177c dc00  st   $00,[y,x++]
177d dc00  st   $00,[y,x++]
177e dc00  st   $00,[y,x++]
* 5 times
1781 dc01  st   $01,[y,x++]
1782 dc01  st   $01,[y,x++]
1783 dc01  st   $01,[y,x++]
* 12 times
178d dc00  st   $00,[y,x++]
178e dc00  st   $00,[y,x++]
178f dc00  st   $00,[y,x++]
* 5 times
1792 dc09  st   $09,[y,x++]
1793 dc09  st   $09,[y,x++]
1794 dc09  st   $09,[y,x++]
* 12 times
179e dc00  st   $00,[y,x++]
179f dc00  st   $00,[y,x++]
17a0 dc00  st   $00,[y,x++]
* 5 times
17a3 dc11  st   $11,[y,x++]
17a4 dc11  st   $11,[y,x++]
17a5 dc11  st   $11,[y,x++]
* 12 times
17af dc00  st   $00,[y,x++]
17b0 dc00  st   $00,[y,x++]
17b1 dc00  st   $00,[y,x++]
* 5 times
17b4 dc19  st   $19,[y,x++]
17b5 dc19  st   $19,[y,x++]
17b6 dc19  st   $19,[y,x++]
* 12 times
17c0 dc00  st   $00,[y,x++]
17c1 dc00  st   $00,[y,x++]
17c2 dc00  st   $00,[y,x++]
* 5 times
17c5 dc21  st   $21,[y,x++]
17c6 dc21  st   $21,[y,x++]
17c7 dc21  st   $21,[y,x++]
* 12 times
17d1 dc00  st   $00,[y,x++]
17d2 dc00  st   $00,[y,x++]
17d3 dc00  st   $00,[y,x++]
* 5 times
17d6 dc29  st   $29,[y,x++]
17d7 dc29  st   $29,[y,x++]
17d8 dc29  st   $29,[y,x++]
* 12 times
17e2 dc00  st   $00,[y,x++]
17e3 dc00  st   $00,[y,x++]
17e4 dc00  st   $00,[y,x++]
* 6 times
17e8 1000  ld   $00,x
17e9 1421  ld   $21,y
17ea dc00  st   $00,[y,x++]
17eb dc00  st   $00,[y,x++]
17ec dc00  st   $00,[y,x++]
* 6 times
17f0 dc28  st   $28,[y,x++]
17f1 dc28  st   $28,[y,x++]
17f2 dc28  st   $28,[y,x++]
* 12 times
17fc dc00  st   $00,[y,x++]
17fd dc00  st   $00,[y,x++]
17fe dc00  st   $00,[y,x++]
* 5 times
1801 dc30  st   $30,[y,x++]
1802 dc30  st   $30,[y,x++]
1803 dc30  st   $30,[y,x++]
* 12 times
180d dc00  st   $00,[y,x++]
180e dc00  st   $00,[y,x++]
180f dc00  st   $00,[y,x++]
* 5 times
1812 dc38  st   $38,[y,x++]
1813 dc38  st   $38,[y,x++]
1814 dc38  st   $38,[y,x++]
* 12 times
181e dc00  st   $00,[y,x++]
181f dc00  st   $00,[y,x++]
1820 dc00  st   $00,[y,x++]
* 5 times
1823 dc01  st   $01,[y,x++]
1824 dc01  st   $01,[y,x++]
1825 dc01  st   $01,[y,x++]
* 12 times
182f dc00  st   $00,[y,x++]
1830 dc00  st   $00,[y,x++]
1831 dc00  st   $00,[y,x++]
* 5 times
1834 dc09  st   $09,[y,x++]
1835 dc09  st   $09,[y,x++]
1836 dc09  st   $09,[y,x++]
* 12 times
1840 dc00  st   $00,[y,x++]
1841 dc00  st   $00,[y,x++]
1842 dc00  st   $00,[y,x++]
* 5 times
1845 dc11  st   $11,[y,x++]
1846 dc11  st   $11,[y,x++]
1847 dc11  st   $11,[y,x++]
* 12 times
1851 dc00  st   $00,[y,x++]
1852 dc00  st   $00,[y,x++]
1853 dc00  st   $00,[y,x++]
* 5 times
1856 dc19  st   $19,[y,x++]
1857 dc19  st   $19,[y,x++]
1858 dc19  st   $19,[y,x++]
* 12 times
1862 dc00  st   $00,[y,x++]
1863 dc00  st   $00,[y,x++]
1864 dc00  st   $00,[y,x++]
* 5 times
1867 dc21  st   $21,[y,x++]
1868 dc21  st   $21,[y,x++]
1869 dc21  st   $21,[y,x++]
* 12 times
1873 dc00  st   $00,[y,x++]
1874 dc00  st   $00,[y,x++]
1875 dc00  st   $00,[y,x++]
* 5 times
1878 dc29  st   $29,[y,x++]
1879 dc29  st   $29,[y,x++]
187a dc29  st   $29,[y,x++]
* 12 times
1884 dc00  st   $00,[y,x++]
1885 dc00  st   $00,[y,x++]
1886 dc00  st   $00,[y,x++]
* 6 times
188a 1000  ld   $00,x
188b 1422  ld   $22,y
188c dc00  st   $00,[y,x++]
188d dc00  st   $00,[y,x++]
188e dc00  st   $00,[y,x++]
* 6 times
1892 dc28  st   $28,[y,x++]
1893 dc28  st   $28,[y,x++]
1894 dc28  st   $28,[y,x++]
* 12 times
189e dc00  st   $00,[y,x++]
189f dc00  st   $00,[y,x++]
18a0 dc00  st   $00,[y,x++]
* 5 times
18a3 dc30  st   $30,[y,x++]
18a4 dc30  st   $30,[y,x++]
18a5 dc30  st   $30,[y,x++]
* 12 times
18af dc00  st   $00,[y,x++]
18b0 dc00  st   $00,[y,x++]
18b1 dc00  st   $00,[y,x++]
* 5 times
18b4 dc38  st   $38,[y,x++]
18b5 dc38  st   $38,[y,x++]
18b6 dc38  st   $38,[y,x++]
* 12 times
18c0 dc00  st   $00,[y,x++]
18c1 dc00  st   $00,[y,x++]
18c2 dc00  st   $00,[y,x++]
* 5 times
18c5 dc01  st   $01,[y,x++]
18c6 dc01  st   $01,[y,x++]
18c7 dc01  st   $01,[y,x++]
* 12 times
18d1 dc00  st   $00,[y,x++]
18d2 dc00  st   $00,[y,x++]
18d3 dc00  st   $00,[y,x++]
* 5 times
18d6 dc09  st   $09,[y,x++]
18d7 dc09  st   $09,[y,x++]
18d8 dc09  st   $09,[y,x++]
* 12 times
18e2 dc00  st   $00,[y,x++]
18e3 dc00  st   $00,[y,x++]
18e4 dc00  st   $00,[y,x++]
* 5 times
18e7 dc11  st   $11,[y,x++]
18e8 dc11  st   $11,[y,x++]
18e9 dc11  st   $11,[y,x++]
* 12 times
18f3 dc00  st   $00,[y,x++]
18f4 dc00  st   $00,[y,x++]
18f5 dc00  st   $00,[y,x++]
* 5 times
18f8 dc19  st   $19,[y,x++]
18f9 dc19  st   $19,[y,x++]
18fa dc19  st   $19,[y,x++]
* 12 times
1904 dc00  st   $00,[y,x++]
1905 dc00  st   $00,[y,x++]
1906 dc00  st   $00,[y,x++]
* 5 times
1909 dc21  st   $21,[y,x++]
190a dc21  st   $21,[y,x++]
190b dc21  st   $21,[y,x++]
* 12 times
1915 dc00  st   $00,[y,x++]
1916 dc00  st   $00,[y,x++]
1917 dc00  st   $00,[y,x++]
* 5 times
191a dc29  st   $29,[y,x++]
191b dc29  st   $29,[y,x++]
191c dc29  st   $29,[y,x++]
* 12 times
1926 dc00  st   $00,[y,x++]
1927 dc00  st   $00,[y,x++]
1928 dc00  st   $00,[y,x++]
* 6 times
192c 1000  ld   $00,x
192d 1423  ld   $23,y
192e dc00  st   $00,[y,x++]
192f dc00  st   $00,[y,x++]
1930 dc00  st   $00,[y,x++]
* 6 times
1934 dc28  st   $28,[y,x++]
1935 dc28  st   $28,[y,x++]
1936 dc28  st   $28,[y,x++]
* 12 times
1940 dc00  st   $00,[y,x++]
1941 dc00  st   $00,[y,x++]
1942 dc00  st   $00,[y,x++]
* 5 times
1945 dc30  st   $30,[y,x++]
1946 dc30  st   $30,[y,x++]
1947 dc30  st   $30,[y,x++]
* 12 times
1951 dc00  st   $00,[y,x++]
1952 dc00  st   $00,[y,x++]
1953 dc00  st   $00,[y,x++]
* 5 times
1956 dc38  st   $38,[y,x++]
1957 dc38  st   $38,[y,x++]
1958 dc38  st   $38,[y,x++]
* 12 times
1962 dc00  st   $00,[y,x++]
1963 dc00  st   $00,[y,x++]
1964 dc00  st   $00,[y,x++]
* 5 times
1967 dc01  st   $01,[y,x++]
1968 dc01  st   $01,[y,x++]
1969 dc01  st   $01,[y,x++]
* 12 times
1973 dc00  st   $00,[y,x++]
1974 dc00  st   $00,[y,x++]
1975 dc00  st   $00,[y,x++]
* 5 times
1978 dc09  st   $09,[y,x++]
1979 dc09  st   $09,[y,x++]
197a dc09  st   $09,[y,x++]
* 12 times
1984 dc00  st   $00,[y,x++]
1985 dc00  st   $00,[y,x++]
1986 dc00  st   $00,[y,x++]
* 5 times
1989 dc11  st   $11,[y,x++]
198a dc11  st   $11,[y,x++]
198b dc11  st   $11,[y,x++]
* 12 times
1995 dc00  st   $00,[y,x++]
1996 dc00  st   $00,[y,x++]
1997 dc00  st   $00,[y,x++]
* 5 times
199a dc19  st   $19,[y,x++]
199b dc19  st   $19,[y,x++]
199c dc19  st   $19,[y,x++]
* 12 times
19a6 dc00  st   $00,[y,x++]
19a7 dc00  st   $00,[y,x++]
19a8 dc00  st   $00,[y,x++]
* 5 times
19ab dc21  st   $21,[y,x++]
19ac dc21  st   $21,[y,x++]
19ad dc21  st   $21,[y,x++]
* 12 times
19b7 dc00  st   $00,[y,x++]
19b8 dc00  st   $00,[y,x++]
19b9 dc00  st   $00,[y,x++]
* 5 times
19bc dc29  st   $29,[y,x++]
19bd dc29  st   $29,[y,x++]
19be dc29  st   $29,[y,x++]
* 12 times
19c8 dc00  st   $00,[y,x++]
19c9 dc00  st   $00,[y,x++]
19ca dc00  st   $00,[y,x++]
* 6 times
19ce 1000  ld   $00,x
19cf 1424  ld   $24,y
19d0 dc00  st   $00,[y,x++]
19d1 dc00  st   $00,[y,x++]
19d2 dc00  st   $00,[y,x++]
* 6 times
19d6 dc28  st   $28,[y,x++]
19d7 dc28  st   $28,[y,x++]
19d8 dc28  st   $28,[y,x++]
* 12 times
19e2 dc00  st   $00,[y,x++]
19e3 dc00  st   $00,[y,x++]
19e4 dc00  st   $00,[y,x++]
* 5 times
19e7 dc30  st   $30,[y,x++]
19e8 dc30  st   $30,[y,x++]
19e9 dc30  st   $30,[y,x++]
* 12 times
19f3 dc00  st   $00,[y,x++]
19f4 dc00  st   $00,[y,x++]
19f5 dc00  st   $00,[y,x++]
* 5 times
19f8 dc38  st   $38,[y,x++]
19f9 dc38  st   $38,[y,x++]
19fa dc38  st   $38,[y,x++]
* 12 times
1a04 dc00  st   $00,[y,x++]
1a05 dc00  st   $00,[y,x++]
1a06 dc00  st   $00,[y,x++]
* 5 times
1a09 dc01  st   $01,[y,x++]
1a0a dc01  st   $01,[y,x++]
1a0b dc01  st   $01,[y,x++]
* 12 times
1a15 dc00  st   $00,[y,x++]
1a16 dc00  st   $00,[y,x++]
1a17 dc00  st   $00,[y,x++]
* 5 times
1a1a dc09  st   $09,[y,x++]
1a1b dc09  st   $09,[y,x++]
1a1c dc09  st   $09,[y,x++]
* 12 times
1a26 dc00  st   $00,[y,x++]
1a27 dc00  st   $00,[y,x++]
1a28 dc00  st   $00,[y,x++]
* 5 times
1a2b dc11  st   $11,[y,x++]
1a2c dc11  st   $11,[y,x++]
1a2d dc11  st   $11,[y,x++]
* 12 times
1a37 dc00  st   $00,[y,x++]
1a38 dc00  st   $00,[y,x++]
1a39 dc00  st   $00,[y,x++]
* 5 times
1a3c dc19  st   $19,[y,x++]
1a3d dc19  st   $19,[y,x++]
1a3e dc19  st   $19,[y,x++]
* 12 times
1a48 dc00  st   $00,[y,x++]
1a49 dc00  st   $00,[y,x++]
1a4a dc00  st   $00,[y,x++]
* 5 times
1a4d dc21  st   $21,[y,x++]
1a4e dc21  st   $21,[y,x++]
1a4f dc21  st   $21,[y,x++]
* 12 times
1a59 dc00  st   $00,[y,x++]
1a5a dc00  st   $00,[y,x++]
1a5b dc00  st   $00,[y,x++]
* 5 times
1a5e dc29  st   $29,[y,x++]
1a5f dc29  st   $29,[y,x++]
1a60 dc29  st   $29,[y,x++]
* 12 times
1a6a dc00  st   $00,[y,x++]
1a6b dc00  st   $00,[y,x++]
1a6c dc00  st   $00,[y,x++]
* 6 times
1a70 1000  ld   $00,x
1a71 1425  ld   $25,y
1a72 dc00  st   $00,[y,x++]
1a73 dc00  st   $00,[y,x++]
1a74 dc00  st   $00,[y,x++]
* 160 times
1b12 1000  ld   $00,x
1b13 1426  ld   $26,y
1b14 dc00  st   $00,[y,x++]
1b15 dc00  st   $00,[y,x++]
1b16 dc00  st   $00,[y,x++]
* 160 times
1bb4 1000  ld   $00,x
1bb5 1427  ld   $27,y
1bb6 dc00  st   $00,[y,x++]
1bb7 dc00  st   $00,[y,x++]
1bb8 dc00  st   $00,[y,x++]
* 160 times
1c56 1000  ld   $00,x
1c57 1428  ld   $28,y
1c58 dc00  st   $00,[y,x++]
1c59 dc00  st   $00,[y,x++]
1c5a dc00  st   $00,[y,x++]
* 160 times
1cf8 1000  ld   $00,x
1cf9 1429  ld   $29,y
1cfa dc00  st   $00,[y,x++]
1cfb dc00  st   $00,[y,x++]
1cfc dc00  st   $00,[y,x++]
* 6 times
1d00 dc10  st   $10,[y,x++]
1d01 dc10  st   $10,[y,x++]
1d02 dc10  st   $10,[y,x++]
* 12 times
1d0c dc00  st   $00,[y,x++]
1d0d dc00  st   $00,[y,x++]
1d0e dc00  st   $00,[y,x++]
* 5 times
1d11 dc18  st   $18,[y,x++]
1d12 dc18  st   $18,[y,x++]
1d13 dc18  st   $18,[y,x++]
* 12 times
1d1d dc00  st   $00,[y,x++]
1d1e dc00  st   $00,[y,x++]
1d1f dc00  st   $00,[y,x++]
* 5 times
1d22 dc20  st   $20,[y,x++]
1d23 dc20  st   $20,[y,x++]
1d24 dc20  st   $20,[y,x++]
* 12 times
1d2e dc00  st   $00,[y,x++]
1d2f dc00  st   $00,[y,x++]
1d30 dc00  st   $00,[y,x++]
* 5 times
1d33 dc28  st   $28,[y,x++]
1d34 dc28  st   $28,[y,x++]
1d35 dc28  st   $28,[y,x++]
* 12 times
1d3f dc00  st   $00,[y,x++]
1d40 dc00  st   $00,[y,x++]
1d41 dc00  st   $00,[y,x++]
* 5 times
1d44 dc30  st   $30,[y,x++]
1d45 dc30  st   $30,[y,x++]
1d46 dc30  st   $30,[y,x++]
* 12 times
1d50 dc00  st   $00,[y,x++]
1d51 dc00  st   $00,[y,x++]
1d52 dc00  st   $00,[y,x++]
* 5 times
1d55 dc38  st   $38,[y,x++]
1d56 dc38  st   $38,[y,x++]
1d57 dc38  st   $38,[y,x++]
* 12 times
1d61 dc00  st   $00,[y,x++]
1d62 dc00  st   $00,[y,x++]
1d63 dc00  st   $00,[y,x++]
* 5 times
1d66 dc01  st   $01,[y,x++]
1d67 dc01  st   $01,[y,x++]
1d68 dc01  st   $01,[y,x++]
* 12 times
1d72 dc00  st   $00,[y,x++]
1d73 dc00  st   $00,[y,x++]
1d74 dc00  st   $00,[y,x++]
* 5 times
1d77 dc09  st   $09,[y,x++]
1d78 dc09  st   $09,[y,x++]
1d79 dc09  st   $09,[y,x++]
* 12 times
1d83 dc00  st   $00,[y,x++]
1d84 dc00  st   $00,[y,x++]
1d85 dc00  st   $00,[y,x++]
* 5 times
1d88 dc11  st   $11,[y,x++]
1d89 dc11  st   $11,[y,x++]
1d8a dc11  st   $11,[y,x++]
* 12 times
1d94 dc00  st   $00,[y,x++]
1d95 dc00  st   $00,[y,x++]
1d96 dc00  st   $00,[y,x++]
* 6 times
1d9a 1000  ld   $00,x
1d9b 142a  ld   $2a,y
1d9c dc00  st   $00,[y,x++]
1d9d dc00  st   $00,[y,x++]
1d9e dc00  st   $00,[y,x++]
* 6 times
1da2 dc10  st   $10,[y,x++]
1da3 dc10  st   $10,[y,x++]
1da4 dc10  st   $10,[y,x++]
* 12 times
1dae dc00  st   $00,[y,x++]
1daf dc00  st   $00,[y,x++]
1db0 dc00  st   $00,[y,x++]
* 5 times
1db3 dc18  st   $18,[y,x++]
1db4 dc18  st   $18,[y,x++]
1db5 dc18  st   $18,[y,x++]
* 12 times
1dbf dc00  st   $00,[y,x++]
1dc0 dc00  st   $00,[y,x++]
1dc1 dc00  st   $00,[y,x++]
* 5 times
1dc4 dc20  st   $20,[y,x++]
1dc5 dc20  st   $20,[y,x++]
1dc6 dc20  st   $20,[y,x++]
* 12 times
1dd0 dc00  st   $00,[y,x++]
1dd1 dc00  st   $00,[y,x++]
1dd2 dc00  st   $00,[y,x++]
* 5 times
1dd5 dc28  st   $28,[y,x++]
1dd6 dc28  st   $28,[y,x++]
1dd7 dc28  st   $28,[y,x++]
* 12 times
1de1 dc00  st   $00,[y,x++]
1de2 dc00  st   $00,[y,x++]
1de3 dc00  st   $00,[y,x++]
* 5 times
1de6 dc30  st   $30,[y,x++]
1de7 dc30  st   $30,[y,x++]
1de8 dc30  st   $30,[y,x++]
* 12 times
1df2 dc00  st   $00,[y,x++]
1df3 dc00  st   $00,[y,x++]
1df4 dc00  st   $00,[y,x++]
* 5 times
1df7 dc38  st   $38,[y,x++]
1df8 dc38  st   $38,[y,x++]
1df9 dc38  st   $38,[y,x++]
* 12 times
1e03 dc00  st   $00,[y,x++]
1e04 dc00  st   $00,[y,x++]
1e05 dc00  st   $00,[y,x++]
* 5 times
1e08 dc01  st   $01,[y,x++]
1e09 dc01  st   $01,[y,x++]
1e0a dc01  st   $01,[y,x++]
* 12 times
1e14 dc00  st   $00,[y,x++]
1e15 dc00  st   $00,[y,x++]
1e16 dc00  st   $00,[y,x++]
* 5 times
1e19 dc09  st   $09,[y,x++]
1e1a dc09  st   $09,[y,x++]
1e1b dc09  st   $09,[y,x++]
* 12 times
1e25 dc00  st   $00,[y,x++]
1e26 dc00  st   $00,[y,x++]
1e27 dc00  st   $00,[y,x++]
* 5 times
1e2a dc11  st   $11,[y,x++]
1e2b dc11  st   $11,[y,x++]
1e2c dc11  st   $11,[y,x++]
* 12 times
1e36 dc00  st   $00,[y,x++]
1e37 dc00  st   $00,[y,x++]
1e38 dc00  st   $00,[y,x++]
* 6 times
1e3c 1000  ld   $00,x
1e3d 142b  ld   $2b,y
1e3e dc00  st   $00,[y,x++]
1e3f dc00  st   $00,[y,x++]
1e40 dc00  st   $00,[y,x++]
* 6 times
1e44 dc10  st   $10,[y,x++]
1e45 dc10  st   $10,[y,x++]
1e46 dc10  st   $10,[y,x++]
* 12 times
1e50 dc00  st   $00,[y,x++]
1e51 dc00  st   $00,[y,x++]
1e52 dc00  st   $00,[y,x++]
* 5 times
1e55 dc18  st   $18,[y,x++]
1e56 dc18  st   $18,[y,x++]
1e57 dc18  st   $18,[y,x++]
* 12 times
1e61 dc00  st   $00,[y,x++]
1e62 dc00  st   $00,[y,x++]
1e63 dc00  st   $00,[y,x++]
* 5 times
1e66 dc20  st   $20,[y,x++]
1e67 dc20  st   $20,[y,x++]
1e68 dc20  st   $20,[y,x++]
* 12 times
1e72 dc00  st   $00,[y,x++]
1e73 dc00  st   $00,[y,x++]
1e74 dc00  st   $00,[y,x++]
* 5 times
1e77 dc28  st   $28,[y,x++]
1e78 dc28  st   $28,[y,x++]
1e79 dc28  st   $28,[y,x++]
* 12 times
1e83 dc00  st   $00,[y,x++]
1e84 dc00  st   $00,[y,x++]
1e85 dc00  st   $00,[y,x++]
* 5 times
1e88 dc30  st   $30,[y,x++]
1e89 dc30  st   $30,[y,x++]
1e8a dc30  st   $30,[y,x++]
* 12 times
1e94 dc00  st   $00,[y,x++]
1e95 dc00  st   $00,[y,x++]
1e96 dc00  st   $00,[y,x++]
* 5 times
1e99 dc38  st   $38,[y,x++]
1e9a dc38  st   $38,[y,x++]
1e9b dc38  st   $38,[y,x++]
* 12 times
1ea5 dc00  st   $00,[y,x++]
1ea6 dc00  st   $00,[y,x++]
1ea7 dc00  st   $00,[y,x++]
* 5 times
1eaa dc01  st   $01,[y,x++]
1eab dc01  st   $01,[y,x++]
1eac dc01  st   $01,[y,x++]
* 12 times
1eb6 dc00  st   $00,[y,x++]
1eb7 dc00  st   $00,[y,x++]
1eb8 dc00  st   $00,[y,x++]
* 5 times
1ebb dc09  st   $09,[y,x++]
1ebc dc09  st   $09,[y,x++]
1ebd dc09  st   $09,[y,x++]
* 12 times
1ec7 dc00  st   $00,[y,x++]
1ec8 dc00  st   $00,[y,x++]
1ec9 dc00  st   $00,[y,x++]
* 5 times
1ecc dc11  st   $11,[y,x++]
1ecd dc11  st   $11,[y,x++]
1ece dc11  st   $11,[y,x++]
* 12 times
1ed8 dc00  st   $00,[y,x++]
1ed9 dc00  st   $00,[y,x++]
1eda dc00  st   $00,[y,x++]
* 6 times
1ede 1000  ld   $00,x
1edf 142c  ld   $2c,y
1ee0 dc00  st   $00,[y,x++]
1ee1 dc00  st   $00,[y,x++]
1ee2 dc00  st   $00,[y,x++]
* 6 times
1ee6 dc10  st   $10,[y,x++]
1ee7 dc10  st   $10,[y,x++]
1ee8 dc10  st   $10,[y,x++]
* 12 times
1ef2 dc00  st   $00,[y,x++]
1ef3 dc00  st   $00,[y,x++]
1ef4 dc00  st   $00,[y,x++]
* 5 times
1ef7 dc18  st   $18,[y,x++]
1ef8 dc18  st   $18,[y,x++]
1ef9 dc18  st   $18,[y,x++]
* 12 times
1f03 dc00  st   $00,[y,x++]
1f04 dc00  st   $00,[y,x++]
1f05 dc00  st   $00,[y,x++]
* 5 times
1f08 dc20  st   $20,[y,x++]
1f09 dc20  st   $20,[y,x++]
1f0a dc20  st   $20,[y,x++]
* 12 times
1f14 dc00  st   $00,[y,x++]
1f15 dc00  st   $00,[y,x++]
1f16 dc00  st   $00,[y,x++]
* 5 times
1f19 dc28  st   $28,[y,x++]
1f1a dc28  st   $28,[y,x++]
1f1b dc28  st   $28,[y,x++]
* 12 times
1f25 dc00  st   $00,[y,x++]
1f26 dc00  st   $00,[y,x++]
1f27 dc00  st   $00,[y,x++]
* 5 times
1f2a dc30  st   $30,[y,x++]
1f2b dc30  st   $30,[y,x++]
1f2c dc30  st   $30,[y,x++]
* 12 times
1f36 dc00  st   $00,[y,x++]
1f37 dc00  st   $00,[y,x++]
1f38 dc00  st   $00,[y,x++]
* 5 times
1f3b dc38  st   $38,[y,x++]
1f3c dc38  st   $38,[y,x++]
1f3d dc38  st   $38,[y,x++]
* 12 times
1f47 dc00  st   $00,[y,x++]
1f48 dc00  st   $00,[y,x++]
1f49 dc00  st   $00,[y,x++]
* 5 times
1f4c dc01  st   $01,[y,x++]
1f4d dc01  st   $01,[y,x++]
1f4e dc01  st   $01,[y,x++]
* 12 times
1f58 dc00  st   $00,[y,x++]
1f59 dc00  st   $00,[y,x++]
1f5a dc00  st   $00,[y,x++]
* 5 times
1f5d dc09  st   $09,[y,x++]
1f5e dc09  st   $09,[y,x++]
1f5f dc09  st   $09,[y,x++]
* 12 times
1f69 dc00  st   $00,[y,x++]
1f6a dc00  st   $00,[y,x++]
1f6b dc00  st   $00,[y,x++]
* 5 times
1f6e dc11  st   $11,[y,x++]
1f6f dc11  st   $11,[y,x++]
1f70 dc11  st   $11,[y,x++]
* 12 times
1f7a dc00  st   $00,[y,x++]
1f7b dc00  st   $00,[y,x++]
1f7c dc00  st   $00,[y,x++]
* 6 times
1f80 1000  ld   $00,x
1f81 142d  ld   $2d,y
1f82 dc00  st   $00,[y,x++]
1f83 dc00  st   $00,[y,x++]
1f84 dc00  st   $00,[y,x++]
* 6 times
1f88 dc10  st   $10,[y,x++]
1f89 dc10  st   $10,[y,x++]
1f8a dc10  st   $10,[y,x++]
* 12 times
1f94 dc00  st   $00,[y,x++]
1f95 dc00  st   $00,[y,x++]
1f96 dc00  st   $00,[y,x++]
* 5 times
1f99 dc18  st   $18,[y,x++]
1f9a dc18  st   $18,[y,x++]
1f9b dc18  st   $18,[y,x++]
* 12 times
1fa5 dc00  st   $00,[y,x++]
1fa6 dc00  st   $00,[y,x++]
1fa7 dc00  st   $00,[y,x++]
* 5 times
1faa dc20  st   $20,[y,x++]
1fab dc20  st   $20,[y,x++]
1fac dc20  st   $20,[y,x++]
* 12 times
1fb6 dc00  st   $00,[y,x++]
1fb7 dc00  st   $00,[y,x++]
1fb8 dc00  st   $00,[y,x++]
* 5 times
1fbb dc28  st   $28,[y,x++]
1fbc dc28  st   $28,[y,x++]
1fbd dc28  st   $28,[y,x++]
* 12 times
1fc7 dc00  st   $00,[y,x++]
1fc8 dc00  st   $00,[y,x++]
1fc9 dc00  st   $00,[y,x++]
* 5 times
1fcc dc30  st   $30,[y,x++]
1fcd dc30  st   $30,[y,x++]
1fce dc30  st   $30,[y,x++]
* 12 times
1fd8 dc00  st   $00,[y,x++]
1fd9 dc00  st   $00,[y,x++]
1fda dc00  st   $00,[y,x++]
* 5 times
1fdd dc38  st   $38,[y,x++]
1fde dc38  st   $38,[y,x++]
1fdf dc38  st   $38,[y,x++]
* 12 times
1fe9 dc00  st   $00,[y,x++]
1fea dc00  st   $00,[y,x++]
1feb dc00  st   $00,[y,x++]
* 5 times
1fee dc01  st   $01,[y,x++]
1fef dc01  st   $01,[y,x++]
1ff0 dc01  st   $01,[y,x++]
* 12 times
1ffa dc00  st   $00,[y,x++]
1ffb dc00  st   $00,[y,x++]
1ffc dc00  st   $00,[y,x++]
* 5 times
1fff dc09  st   $09,[y,x++]
2000 dc09  st   $09,[y,x++]
2001 dc09  st   $09,[y,x++]
* 12 times
200b dc00  st   $00,[y,x++]
200c dc00  st   $00,[y,x++]
200d dc00  st   $00,[y,x++]
* 5 times
2010 dc11  st   $11,[y,x++]
2011 dc11  st   $11,[y,x++]
2012 dc11  st   $11,[y,x++]
* 12 times
201c dc00  st   $00,[y,x++]
201d dc00  st   $00,[y,x++]
201e dc00  st   $00,[y,x++]
* 6 times
2022 1000  ld   $00,x
2023 142e  ld   $2e,y
2024 dc00  st   $00,[y,x++]
2025 dc00  st   $00,[y,x++]
2026 dc00  st   $00,[y,x++]
* 160 times
20c4 1000  ld   $00,x
20c5 142f  ld   $2f,y
20c6 dc00  st   $00,[y,x++]
20c7 dc00  st   $00,[y,x++]
20c8 dc00  st   $00,[y,x++]
* 160 times
2166 1000  ld   $00,x
2167 1430  ld   $30,y
2168 dc00  st   $00,[y,x++]
2169 dc00  st   $00,[y,x++]
216a dc00  st   $00,[y,x++]
* 160 times
2208 1000  ld   $00,x
2209 1431  ld   $31,y
220a dc00  st   $00,[y,x++]
220b dc00  st   $00,[y,x++]
220c dc00  st   $00,[y,x++]
* 160 times
22aa 1000  ld   $00,x
22ab 1432  ld   $32,y
22ac dc00  st   $00,[y,x++]
22ad dc00  st   $00,[y,x++]
22ae dc00  st   $00,[y,x++]
* 6 times
22b2 dc37  st   $37,[y,x++]
22b3 dc37  st   $37,[y,x++]
22b4 dc37  st   $37,[y,x++]
* 12 times
22be dc00  st   $00,[y,x++]
22bf dc00  st   $00,[y,x++]
22c0 dc00  st   $00,[y,x++]
* 5 times
22c3 dc3f  st   $3f,[y,x++]
22c4 dc3f  st   $3f,[y,x++]
22c5 dc3f  st   $3f,[y,x++]
* 12 times
22cf dc00  st   $00,[y,x++]
22d0 dc00  st   $00,[y,x++]
22d1 dc00  st   $00,[y,x++]
* 5 times
22d4 dc08  st   $08,[y,x++]
22d5 dc08  st   $08,[y,x++]
22d6 dc08  st   $08,[y,x++]
* 12 times
22e0 dc00  st   $00,[y,x++]
22e1 dc00  st   $00,[y,x++]
22e2 dc00  st   $00,[y,x++]
* 5 times
22e5 dc10  st   $10,[y,x++]
22e6 dc10  st   $10,[y,x++]
22e7 dc10  st   $10,[y,x++]
* 12 times
22f1 dc00  st   $00,[y,x++]
22f2 dc00  st   $00,[y,x++]
22f3 dc00  st   $00,[y,x++]
* 5 times
22f6 dc18  st   $18,[y,x++]
22f7 dc18  st   $18,[y,x++]
22f8 dc18  st   $18,[y,x++]
* 12 times
2302 dc00  st   $00,[y,x++]
2303 dc00  st   $00,[y,x++]
2304 dc00  st   $00,[y,x++]
* 5 times
2307 dc20  st   $20,[y,x++]
2308 dc20  st   $20,[y,x++]
2309 dc20  st   $20,[y,x++]
* 12 times
2313 dc00  st   $00,[y,x++]
2314 dc00  st   $00,[y,x++]
2315 dc00  st   $00,[y,x++]
* 5 times
2318 dc28  st   $28,[y,x++]
2319 dc28  st   $28,[y,x++]
231a dc28  st   $28,[y,x++]
* 12 times
2324 dc00  st   $00,[y,x++]
2325 dc00  st   $00,[y,x++]
2326 dc00  st   $00,[y,x++]
* 5 times
2329 dc30  st   $30,[y,x++]
232a dc30  st   $30,[y,x++]
232b dc30  st   $30,[y,x++]
* 12 times
2335 dc00  st   $00,[y,x++]
2336 dc00  st   $00,[y,x++]
2337 dc00  st   $00,[y,x++]
* 5 times
233a dc38  st   $38,[y,x++]
233b dc38  st   $38,[y,x++]
233c dc38  st   $38,[y,x++]
* 12 times
2346 dc00  st   $00,[y,x++]
2347 dc00  st   $00,[y,x++]
2348 dc00  st   $00,[y,x++]
* 6 times
234c 1000  ld   $00,x
234d 1433  ld   $33,y
234e dc00  st   $00,[y,x++]
234f dc00  st   $00,[y,x++]
2350 dc00  st   $00,[y,x++]
* 6 times
2354 dc37  st   $37,[y,x++]
2355 dc37  st   $37,[y,x++]
2356 dc37  st   $37,[y,x++]
* 12 times
2360 dc00  st   $00,[y,x++]
2361 dc00  st   $00,[y,x++]
2362 dc00  st   $00,[y,x++]
* 5 times
2365 dc3f  st   $3f,[y,x++]
2366 dc3f  st   $3f,[y,x++]
2367 dc3f  st   $3f,[y,x++]
* 12 times
2371 dc00  st   $00,[y,x++]
2372 dc00  st   $00,[y,x++]
2373 dc00  st   $00,[y,x++]
* 5 times
2376 dc08  st   $08,[y,x++]
2377 dc08  st   $08,[y,x++]
2378 dc08  st   $08,[y,x++]
* 12 times
2382 dc00  st   $00,[y,x++]
2383 dc00  st   $00,[y,x++]
2384 dc00  st   $00,[y,x++]
* 5 times
2387 dc10  st   $10,[y,x++]
2388 dc10  st   $10,[y,x++]
2389 dc10  st   $10,[y,x++]
* 12 times
2393 dc00  st   $00,[y,x++]
2394 dc00  st   $00,[y,x++]
2395 dc00  st   $00,[y,x++]
* 5 times
2398 dc18  st   $18,[y,x++]
2399 dc18  st   $18,[y,x++]
239a dc18  st   $18,[y,x++]
* 12 times
23a4 dc00  st   $00,[y,x++]
23a5 dc00  st   $00,[y,x++]
23a6 dc00  st   $00,[y,x++]
* 5 times
23a9 dc20  st   $20,[y,x++]
23aa dc20  st   $20,[y,x++]
23ab dc20  st   $20,[y,x++]
* 12 times
23b5 dc00  st   $00,[y,x++]
23b6 dc00  st   $00,[y,x++]
23b7 dc00  st   $00,[y,x++]
* 5 times
23ba dc28  st   $28,[y,x++]
23bb dc28  st   $28,[y,x++]
23bc dc28  st   $28,[y,x++]
* 12 times
23c6 dc00  st   $00,[y,x++]
23c7 dc00  st   $00,[y,x++]
23c8 dc00  st   $00,[y,x++]
* 5 times
23cb dc30  st   $30,[y,x++]
23cc dc30  st   $30,[y,x++]
23cd dc30  st   $30,[y,x++]
* 12 times
23d7 dc00  st   $00,[y,x++]
23d8 dc00  st   $00,[y,x++]
23d9 dc00  st   $00,[y,x++]
* 5 times
23dc dc38  st   $38,[y,x++]
23dd dc38  st   $38,[y,x++]
23de dc38  st   $38,[y,x++]
* 12 times
23e8 dc00  st   $00,[y,x++]
23e9 dc00  st   $00,[y,x++]
23ea dc00  st   $00,[y,x++]
* 6 times
23ee 1000  ld   $00,x
23ef 1434  ld   $34,y
23f0 dc00  st   $00,[y,x++]
23f1 dc00  st   $00,[y,x++]
23f2 dc00  st   $00,[y,x++]
* 6 times
23f6 dc37  st   $37,[y,x++]
23f7 dc37  st   $37,[y,x++]
23f8 dc37  st   $37,[y,x++]
* 12 times
2402 dc00  st   $00,[y,x++]
2403 dc00  st   $00,[y,x++]
2404 dc00  st   $00,[y,x++]
* 5 times
2407 dc3f  st   $3f,[y,x++]
2408 dc3f  st   $3f,[y,x++]
2409 dc3f  st   $3f,[y,x++]
* 12 times
2413 dc00  st   $00,[y,x++]
2414 dc00  st   $00,[y,x++]
2415 dc00  st   $00,[y,x++]
* 5 times
2418 dc08  st   $08,[y,x++]
2419 dc08  st   $08,[y,x++]
241a dc08  st   $08,[y,x++]
* 12 times
2424 dc00  st   $00,[y,x++]
2425 dc00  st   $00,[y,x++]
2426 dc00  st   $00,[y,x++]
* 5 times
2429 dc10  st   $10,[y,x++]
242a dc10  st   $10,[y,x++]
242b dc10  st   $10,[y,x++]
* 12 times
2435 dc00  st   $00,[y,x++]
2436 dc00  st   $00,[y,x++]
2437 dc00  st   $00,[y,x++]
* 5 times
243a dc18  st   $18,[y,x++]
243b dc18  st   $18,[y,x++]
243c dc18  st   $18,[y,x++]
* 12 times
2446 dc00  st   $00,[y,x++]
2447 dc00  st   $00,[y,x++]
2448 dc00  st   $00,[y,x++]
* 5 times
244b dc20  st   $20,[y,x++]
244c dc20  st   $20,[y,x++]
244d dc20  st   $20,[y,x++]
* 12 times
2457 dc00  st   $00,[y,x++]
2458 dc00  st   $00,[y,x++]
2459 dc00  st   $00,[y,x++]
* 5 times
245c dc28  st   $28,[y,x++]
245d dc28  st   $28,[y,x++]
245e dc28  st   $28,[y,x++]
* 12 times
2468 dc00  st   $00,[y,x++]
2469 dc00  st   $00,[y,x++]
246a dc00  st   $00,[y,x++]
* 5 times
246d dc30  st   $30,[y,x++]
246e dc30  st   $30,[y,x++]
246f dc30  st   $30,[y,x++]
* 12 times
2479 dc00  st   $00,[y,x++]
247a dc00  st   $00,[y,x++]
247b dc00  st   $00,[y,x++]
* 5 times
247e dc38  st   $38,[y,x++]
247f dc38  st   $38,[y,x++]
2480 dc38  st   $38,[y,x++]
* 12 times
248a dc00  st   $00,[y,x++]
248b dc00  st   $00,[y,x++]
248c dc00  st   $00,[y,x++]
* 6 times
2490 1000  ld   $00,x
2491 1435  ld   $35,y
2492 dc00  st   $00,[y,x++]
2493 dc00  st   $00,[y,x++]
2494 dc00  st   $00,[y,x++]
* 6 times
2498 dc37  st   $37,[y,x++]
2499 dc37  st   $37,[y,x++]
249a dc37  st   $37,[y,x++]
* 12 times
24a4 dc00  st   $00,[y,x++]
24a5 dc00  st   $00,[y,x++]
24a6 dc00  st   $00,[y,x++]
* 5 times
24a9 dc3f  st   $3f,[y,x++]
24aa dc3f  st   $3f,[y,x++]
24ab dc3f  st   $3f,[y,x++]
* 12 times
24b5 dc00  st   $00,[y,x++]
24b6 dc00  st   $00,[y,x++]
24b7 dc00  st   $00,[y,x++]
* 5 times
24ba dc08  st   $08,[y,x++]
24bb dc08  st   $08,[y,x++]
24bc dc08  st   $08,[y,x++]
* 12 times
24c6 dc00  st   $00,[y,x++]
24c7 dc00  st   $00,[y,x++]
24c8 dc00  st   $00,[y,x++]
* 5 times
24cb dc10  st   $10,[y,x++]
24cc dc10  st   $10,[y,x++]
24cd dc10  st   $10,[y,x++]
* 12 times
24d7 dc00  st   $00,[y,x++]
24d8 dc00  st   $00,[y,x++]
24d9 dc00  st   $00,[y,x++]
* 5 times
24dc dc18  st   $18,[y,x++]
24dd dc18  st   $18,[y,x++]
24de dc18  st   $18,[y,x++]
* 12 times
24e8 dc00  st   $00,[y,x++]
24e9 dc00  st   $00,[y,x++]
24ea dc00  st   $00,[y,x++]
* 5 times
24ed dc20  st   $20,[y,x++]
24ee dc20  st   $20,[y,x++]
24ef dc20  st   $20,[y,x++]
* 12 times
24f9 dc00  st   $00,[y,x++]
24fa dc00  st   $00,[y,x++]
24fb dc00  st   $00,[y,x++]
* 5 times
24fe dc28  st   $28,[y,x++]
24ff dc28  st   $28,[y,x++]
2500 dc28  st   $28,[y,x++]
* 12 times
250a dc00  st   $00,[y,x++]
250b dc00  st   $00,[y,x++]
250c dc00  st   $00,[y,x++]
* 5 times
250f dc30  st   $30,[y,x++]
2510 dc30  st   $30,[y,x++]
2511 dc30  st   $30,[y,x++]
* 12 times
251b dc00  st   $00,[y,x++]
251c dc00  st   $00,[y,x++]
251d dc00  st   $00,[y,x++]
* 5 times
2520 dc38  st   $38,[y,x++]
2521 dc38  st   $38,[y,x++]
2522 dc38  st   $38,[y,x++]
* 12 times
252c dc00  st   $00,[y,x++]
252d dc00  st   $00,[y,x++]
252e dc00  st   $00,[y,x++]
* 6 times
2532 1000  ld   $00,x
2533 1436  ld   $36,y
2534 dc00  st   $00,[y,x++]
2535 dc00  st   $00,[y,x++]
2536 dc00  st   $00,[y,x++]
* 6 times
253a dc37  st   $37,[y,x++]
253b dc37  st   $37,[y,x++]
253c dc37  st   $37,[y,x++]
* 12 times
2546 dc00  st   $00,[y,x++]
2547 dc00  st   $00,[y,x++]
2548 dc00  st   $00,[y,x++]
* 5 times
254b dc3f  st   $3f,[y,x++]
254c dc3f  st   $3f,[y,x++]
254d dc3f  st   $3f,[y,x++]
* 12 times
2557 dc00  st   $00,[y,x++]
2558 dc00  st   $00,[y,x++]
2559 dc00  st   $00,[y,x++]
* 5 times
255c dc08  st   $08,[y,x++]
255d dc08  st   $08,[y,x++]
255e dc08  st   $08,[y,x++]
* 12 times
2568 dc00  st   $00,[y,x++]
2569 dc00  st   $00,[y,x++]
256a dc00  st   $00,[y,x++]
* 5 times
256d dc10  st   $10,[y,x++]
256e dc10  st   $10,[y,x++]
256f dc10  st   $10,[y,x++]
* 12 times
2579 dc00  st   $00,[y,x++]
257a dc00  st   $00,[y,x++]
257b dc00  st   $00,[y,x++]
* 5 times
257e dc18  st   $18,[y,x++]
257f dc18  st   $18,[y,x++]
2580 dc18  st   $18,[y,x++]
* 12 times
258a dc00  st   $00,[y,x++]
258b dc00  st   $00,[y,x++]
258c dc00  st   $00,[y,x++]
* 5 times
258f dc20  st   $20,[y,x++]
2590 dc20  st   $20,[y,x++]
2591 dc20  st   $20,[y,x++]
* 12 times
259b dc00  st   $00,[y,x++]
259c dc00  st   $00,[y,x++]
259d dc00  st   $00,[y,x++]
* 5 times
25a0 dc28  st   $28,[y,x++]
25a1 dc28  st   $28,[y,x++]
25a2 dc28  st   $28,[y,x++]
* 12 times
25ac dc00  st   $00,[y,x++]
25ad dc00  st   $00,[y,x++]
25ae dc00  st   $00,[y,x++]
* 5 times
25b1 dc30  st   $30,[y,x++]
25b2 dc30  st   $30,[y,x++]
25b3 dc30  st   $30,[y,x++]
* 12 times
25bd dc00  st   $00,[y,x++]
25be dc00  st   $00,[y,x++]
25bf dc00  st   $00,[y,x++]
* 5 times
25c2 dc38  st   $38,[y,x++]
25c3 dc38  st   $38,[y,x++]
25c4 dc38  st   $38,[y,x++]
* 12 times
25ce dc00  st   $00,[y,x++]
25cf dc00  st   $00,[y,x++]
25d0 dc00  st   $00,[y,x++]
* 6 times
25d4 1000  ld   $00,x
25d5 1437  ld   $37,y
25d6 dc00  st   $00,[y,x++]
25d7 dc00  st   $00,[y,x++]
25d8 dc00  st   $00,[y,x++]
* 160 times
2676 1000  ld   $00,x
2677 1438  ld   $38,y
2678 dc00  st   $00,[y,x++]
2679 dc00  st   $00,[y,x++]
267a dc00  st   $00,[y,x++]
* 160 times
2718 1000  ld   $00,x
2719 1439  ld   $39,y
271a dc00  st   $00,[y,x++]
271b dc00  st   $00,[y,x++]
271c dc00  st   $00,[y,x++]
* 160 times
27ba 1000  ld   $00,x
27bb 143a  ld   $3a,y
27bc dc00  st   $00,[y,x++]
27bd dc00  st   $00,[y,x++]
27be dc00  st   $00,[y,x++]
* 160 times
285c 1000  ld   $00,x
285d 143b  ld   $3b,y
285e dc00  st   $00,[y,x++]
285f dc00  st   $00,[y,x++]
2860 dc00  st   $00,[y,x++]
* 6 times
2864 dc1f  st   $1f,[y,x++]
2865 dc1f  st   $1f,[y,x++]
2866 dc1f  st   $1f,[y,x++]
* 12 times
2870 dc00  st   $00,[y,x++]
2871 dc00  st   $00,[y,x++]
2872 dc00  st   $00,[y,x++]
* 5 times
2875 dc27  st   $27,[y,x++]
2876 dc27  st   $27,[y,x++]
2877 dc27  st   $27,[y,x++]
* 12 times
2881 dc00  st   $00,[y,x++]
2882 dc00  st   $00,[y,x++]
2883 dc00  st   $00,[y,x++]
* 5 times
2886 dc2f  st   $2f,[y,x++]
2887 dc2f  st   $2f,[y,x++]
2888 dc2f  st   $2f,[y,x++]
* 12 times
2892 dc00  st   $00,[y,x++]
2893 dc00  st   $00,[y,x++]
2894 dc00  st   $00,[y,x++]
* 5 times
2897 dc37  st   $37,[y,x++]
2898 dc37  st   $37,[y,x++]
2899 dc37  st   $37,[y,x++]
* 12 times
28a3 dc00  st   $00,[y,x++]
28a4 dc00  st   $00,[y,x++]
28a5 dc00  st   $00,[y,x++]
* 5 times
28a8 dc3f  st   $3f,[y,x++]
28a9 dc3f  st   $3f,[y,x++]
28aa dc3f  st   $3f,[y,x++]
* 12 times
28b4 dc00  st   $00,[y,x++]
28b5 dc00  st   $00,[y,x++]
28b6 dc00  st   $00,[y,x++]
* 5 times
28b9 dc08  st   $08,[y,x++]
28ba dc08  st   $08,[y,x++]
28bb dc08  st   $08,[y,x++]
* 12 times
28c5 dc00  st   $00,[y,x++]
28c6 dc00  st   $00,[y,x++]
28c7 dc00  st   $00,[y,x++]
* 5 times
28ca dc10  st   $10,[y,x++]
28cb dc10  st   $10,[y,x++]
28cc dc10  st   $10,[y,x++]
* 12 times
28d6 dc00  st   $00,[y,x++]
28d7 dc00  st   $00,[y,x++]
28d8 dc00  st   $00,[y,x++]
* 5 times
28db dc18  st   $18,[y,x++]
28dc dc18  st   $18,[y,x++]
28dd dc18  st   $18,[y,x++]
* 12 times
28e7 dc00  st   $00,[y,x++]
28e8 dc00  st   $00,[y,x++]
28e9 dc00  st   $00,[y,x++]
* 5 times
28ec dc20  st   $20,[y,x++]
28ed dc20  st   $20,[y,x++]
28ee dc20  st   $20,[y,x++]
* 12 times
28f8 dc00  st   $00,[y,x++]
28f9 dc00  st   $00,[y,x++]
28fa dc00  st   $00,[y,x++]
* 6 times
28fe 1000  ld   $00,x
28ff 143c  ld   $3c,y
2900 dc00  st   $00,[y,x++]
2901 dc00  st   $00,[y,x++]
2902 dc00  st   $00,[y,x++]
* 6 times
2906 dc1f  st   $1f,[y,x++]
2907 dc1f  st   $1f,[y,x++]
2908 dc1f  st   $1f,[y,x++]
* 12 times
2912 dc00  st   $00,[y,x++]
2913 dc00  st   $00,[y,x++]
2914 dc00  st   $00,[y,x++]
* 5 times
2917 dc27  st   $27,[y,x++]
2918 dc27  st   $27,[y,x++]
2919 dc27  st   $27,[y,x++]
* 12 times
2923 dc00  st   $00,[y,x++]
2924 dc00  st   $00,[y,x++]
2925 dc00  st   $00,[y,x++]
* 5 times
2928 dc2f  st   $2f,[y,x++]
2929 dc2f  st   $2f,[y,x++]
292a dc2f  st   $2f,[y,x++]
* 12 times
2934 dc00  st   $00,[y,x++]
2935 dc00  st   $00,[y,x++]
2936 dc00  st   $00,[y,x++]
* 5 times
2939 dc37  st   $37,[y,x++]
293a dc37  st   $37,[y,x++]
293b dc37  st   $37,[y,x++]
* 12 times
2945 dc00  st   $00,[y,x++]
2946 dc00  st   $00,[y,x++]
2947 dc00  st   $00,[y,x++]
* 5 times
294a dc3f  st   $3f,[y,x++]
294b dc3f  st   $3f,[y,x++]
294c dc3f  st   $3f,[y,x++]
* 12 times
2956 dc00  st   $00,[y,x++]
2957 dc00  st   $00,[y,x++]
2958 dc00  st   $00,[y,x++]
* 5 times
295b dc08  st   $08,[y,x++]
295c dc08  st   $08,[y,x++]
295d dc08  st   $08,[y,x++]
* 12 times
2967 dc00  st   $00,[y,x++]
2968 dc00  st   $00,[y,x++]
2969 dc00  st   $00,[y,x++]
* 5 times
296c dc10  st   $10,[y,x++]
296d dc10  st   $10,[y,x++]
296e dc10  st   $10,[y,x++]
* 12 times
2978 dc00  st   $00,[y,x++]
2979 dc00  st   $00,[y,x++]
297a dc00  st   $00,[y,x++]
* 5 times
297d dc18  st   $18,[y,x++]
297e dc18  st   $18,[y,x++]
297f dc18  st   $18,[y,x++]
* 12 times
2989 dc00  st   $00,[y,x++]
298a dc00  st   $00,[y,x++]
298b dc00  st   $00,[y,x++]
* 5 times
298e dc20  st   $20,[y,x++]
298f dc20  st   $20,[y,x++]
2990 dc20  st   $20,[y,x++]
* 12 times
299a dc00  st   $00,[y,x++]
299b dc00  st   $00,[y,x++]
299c dc00  st   $00,[y,x++]
* 6 times
29a0 1000  ld   $00,x
29a1 143d  ld   $3d,y
29a2 dc00  st   $00,[y,x++]
29a3 dc00  st   $00,[y,x++]
29a4 dc00  st   $00,[y,x++]
* 6 times
29a8 dc1f  st   $1f,[y,x++]
29a9 dc1f  st   $1f,[y,x++]
29aa dc1f  st   $1f,[y,x++]
* 12 times
29b4 dc00  st   $00,[y,x++]
29b5 dc00  st   $00,[y,x++]
29b6 dc00  st   $00,[y,x++]
* 5 times
29b9 dc27  st   $27,[y,x++]
29ba dc27  st   $27,[y,x++]
29bb dc27  st   $27,[y,x++]
* 12 times
29c5 dc00  st   $00,[y,x++]
29c6 dc00  st   $00,[y,x++]
29c7 dc00  st   $00,[y,x++]
* 5 times
29ca dc2f  st   $2f,[y,x++]
29cb dc2f  st   $2f,[y,x++]
29cc dc2f  st   $2f,[y,x++]
* 12 times
29d6 dc00  st   $00,[y,x++]
29d7 dc00  st   $00,[y,x++]
29d8 dc00  st   $00,[y,x++]
* 5 times
29db dc37  st   $37,[y,x++]
29dc dc37  st   $37,[y,x++]
29dd dc37  st   $37,[y,x++]
* 12 times
29e7 dc00  st   $00,[y,x++]
29e8 dc00  st   $00,[y,x++]
29e9 dc00  st   $00,[y,x++]
* 5 times
29ec dc3f  st   $3f,[y,x++]
29ed dc3f  st   $3f,[y,x++]
29ee dc3f  st   $3f,[y,x++]
* 12 times
29f8 dc00  st   $00,[y,x++]
29f9 dc00  st   $00,[y,x++]
29fa dc00  st   $00,[y,x++]
* 5 times
29fd dc08  st   $08,[y,x++]
29fe dc08  st   $08,[y,x++]
29ff dc08  st   $08,[y,x++]
* 12 times
2a09 dc00  st   $00,[y,x++]
2a0a dc00  st   $00,[y,x++]
2a0b dc00  st   $00,[y,x++]
* 5 times
2a0e dc10  st   $10,[y,x++]
2a0f dc10  st   $10,[y,x++]
2a10 dc10  st   $10,[y,x++]
* 12 times
2a1a dc00  st   $00,[y,x++]
2a1b dc00  st   $00,[y,x++]
2a1c dc00  st   $00,[y,x++]
* 5 times
2a1f dc18  st   $18,[y,x++]
2a20 dc18  st   $18,[y,x++]
2a21 dc18  st   $18,[y,x++]
* 12 times
2a2b dc00  st   $00,[y,x++]
2a2c dc00  st   $00,[y,x++]
2a2d dc00  st   $00,[y,x++]
* 5 times
2a30 dc20  st   $20,[y,x++]
2a31 dc20  st   $20,[y,x++]
2a32 dc20  st   $20,[y,x++]
* 12 times
2a3c dc00  st   $00,[y,x++]
2a3d dc00  st   $00,[y,x++]
2a3e dc00  st   $00,[y,x++]
* 6 times
2a42 1000  ld   $00,x
2a43 143e  ld   $3e,y
2a44 dc00  st   $00,[y,x++]
2a45 dc00  st   $00,[y,x++]
2a46 dc00  st   $00,[y,x++]
* 6 times
2a4a dc1f  st   $1f,[y,x++]
2a4b dc1f  st   $1f,[y,x++]
2a4c dc1f  st   $1f,[y,x++]
* 12 times
2a56 dc00  st   $00,[y,x++]
2a57 dc00  st   $00,[y,x++]
2a58 dc00  st   $00,[y,x++]
* 5 times
2a5b dc27  st   $27,[y,x++]
2a5c dc27  st   $27,[y,x++]
2a5d dc27  st   $27,[y,x++]
* 12 times
2a67 dc00  st   $00,[y,x++]
2a68 dc00  st   $00,[y,x++]
2a69 dc00  st   $00,[y,x++]
* 5 times
2a6c dc2f  st   $2f,[y,x++]
2a6d dc2f  st   $2f,[y,x++]
2a6e dc2f  st   $2f,[y,x++]
* 12 times
2a78 dc00  st   $00,[y,x++]
2a79 dc00  st   $00,[y,x++]
2a7a dc00  st   $00,[y,x++]
* 5 times
2a7d dc37  st   $37,[y,x++]
2a7e dc37  st   $37,[y,x++]
2a7f dc37  st   $37,[y,x++]
* 12 times
2a89 dc00  st   $00,[y,x++]
2a8a dc00  st   $00,[y,x++]
2a8b dc00  st   $00,[y,x++]
* 5 times
2a8e dc3f  st   $3f,[y,x++]
2a8f dc3f  st   $3f,[y,x++]
2a90 dc3f  st   $3f,[y,x++]
* 12 times
2a9a dc00  st   $00,[y,x++]
2a9b dc00  st   $00,[y,x++]
2a9c dc00  st   $00,[y,x++]
* 5 times
2a9f dc08  st   $08,[y,x++]
2aa0 dc08  st   $08,[y,x++]
2aa1 dc08  st   $08,[y,x++]
* 12 times
2aab dc00  st   $00,[y,x++]
2aac dc00  st   $00,[y,x++]
2aad dc00  st   $00,[y,x++]
* 5 times
2ab0 dc10  st   $10,[y,x++]
2ab1 dc10  st   $10,[y,x++]
2ab2 dc10  st   $10,[y,x++]
* 12 times
2abc dc00  st   $00,[y,x++]
2abd dc00  st   $00,[y,x++]
2abe dc00  st   $00,[y,x++]
* 5 times
2ac1 dc18  st   $18,[y,x++]
2ac2 dc18  st   $18,[y,x++]
2ac3 dc18  st   $18,[y,x++]
* 12 times
2acd dc00  st   $00,[y,x++]
2ace dc00  st   $00,[y,x++]
2acf dc00  st   $00,[y,x++]
* 5 times
2ad2 dc20  st   $20,[y,x++]
2ad3 dc20  st   $20,[y,x++]
2ad4 dc20  st   $20,[y,x++]
* 12 times
2ade dc00  st   $00,[y,x++]
2adf dc00  st   $00,[y,x++]
2ae0 dc00  st   $00,[y,x++]
* 6 times
2ae4 1000  ld   $00,x
2ae5 143f  ld   $3f,y
2ae6 dc00  st   $00,[y,x++]
2ae7 dc00  st   $00,[y,x++]
2ae8 dc00  st   $00,[y,x++]
* 6 times
2aec dc1f  st   $1f,[y,x++]
2aed dc1f  st   $1f,[y,x++]
2aee dc1f  st   $1f,[y,x++]
* 12 times
2af8 dc00  st   $00,[y,x++]
2af9 dc00  st   $00,[y,x++]
2afa dc00  st   $00,[y,x++]
* 5 times
2afd dc27  st   $27,[y,x++]
2afe dc27  st   $27,[y,x++]
2aff dc27  st   $27,[y,x++]
* 12 times
2b09 dc00  st   $00,[y,x++]
2b0a dc00  st   $00,[y,x++]
2b0b dc00  st   $00,[y,x++]
* 5 times
2b0e dc2f  st   $2f,[y,x++]
2b0f dc2f  st   $2f,[y,x++]
2b10 dc2f  st   $2f,[y,x++]
* 12 times
2b1a dc00  st   $00,[y,x++]
2b1b dc00  st   $00,[y,x++]
2b1c dc00  st   $00,[y,x++]
* 5 times
2b1f dc37  st   $37,[y,x++]
2b20 dc37  st   $37,[y,x++]
2b21 dc37  st   $37,[y,x++]
* 12 times
2b2b dc00  st   $00,[y,x++]
2b2c dc00  st   $00,[y,x++]
2b2d dc00  st   $00,[y,x++]
* 5 times
2b30 dc3f  st   $3f,[y,x++]
2b31 dc3f  st   $3f,[y,x++]
2b32 dc3f  st   $3f,[y,x++]
* 12 times
2b3c dc00  st   $00,[y,x++]
2b3d dc00  st   $00,[y,x++]
2b3e dc00  st   $00,[y,x++]
* 5 times
2b41 dc08  st   $08,[y,x++]
2b42 dc08  st   $08,[y,x++]
2b43 dc08  st   $08,[y,x++]
* 12 times
2b4d dc00  st   $00,[y,x++]
2b4e dc00  st   $00,[y,x++]
2b4f dc00  st   $00,[y,x++]
* 5 times
2b52 dc10  st   $10,[y,x++]
2b53 dc10  st   $10,[y,x++]
2b54 dc10  st   $10,[y,x++]
* 12 times
2b5e dc00  st   $00,[y,x++]
2b5f dc00  st   $00,[y,x++]
2b60 dc00  st   $00,[y,x++]
* 5 times
2b63 dc18  st   $18,[y,x++]
2b64 dc18  st   $18,[y,x++]
2b65 dc18  st   $18,[y,x++]
* 12 times
2b6f dc00  st   $00,[y,x++]
2b70 dc00  st   $00,[y,x++]
2b71 dc00  st   $00,[y,x++]
* 5 times
2b74 dc20  st   $20,[y,x++]
2b75 dc20  st   $20,[y,x++]
2b76 dc20  st   $20,[y,x++]
* 12 times
2b80 dc00  st   $00,[y,x++]
2b81 dc00  st   $00,[y,x++]
2b82 dc00  st   $00,[y,x++]
* 6 times
2b86 1000  ld   $00,x
2b87 1440  ld   $40,y
2b88 dc00  st   $00,[y,x++]
2b89 dc00  st   $00,[y,x++]
2b8a dc00  st   $00,[y,x++]
* 160 times
2c28 1000  ld   $00,x
2c29 1441  ld   $41,y
2c2a dc00  st   $00,[y,x++]
2c2b dc00  st   $00,[y,x++]
2c2c dc00  st   $00,[y,x++]
* 160 times
2cca 1000  ld   $00,x
2ccb 1442  ld   $42,y
2ccc dc00  st   $00,[y,x++]
2ccd dc00  st   $00,[y,x++]
2cce dc00  st   $00,[y,x++]
* 160 times
2d6c 1000  ld   $00,x
2d6d 1443  ld   $43,y
2d6e dc00  st   $00,[y,x++]
2d6f dc00  st   $00,[y,x++]
2d70 dc00  st   $00,[y,x++]
* 160 times
2e0e 1000  ld   $00,x
2e0f 1444  ld   $44,y
2e10 dc00  st   $00,[y,x++]
2e11 dc00  st   $00,[y,x++]
2e12 dc00  st   $00,[y,x++]
* 6 times
2e16 dc07  st   $07,[y,x++]
2e17 dc07  st   $07,[y,x++]
2e18 dc07  st   $07,[y,x++]
* 12 times
2e22 dc00  st   $00,[y,x++]
2e23 dc00  st   $00,[y,x++]
2e24 dc00  st   $00,[y,x++]
* 5 times
2e27 dc0f  st   $0f,[y,x++]
2e28 dc0f  st   $0f,[y,x++]
2e29 dc0f  st   $0f,[y,x++]
* 12 times
2e33 dc00  st   $00,[y,x++]
2e34 dc00  st   $00,[y,x++]
2e35 dc00  st   $00,[y,x++]
* 5 times
2e38 dc17  st   $17,[y,x++]
2e39 dc17  st   $17,[y,x++]
2e3a dc17  st   $17,[y,x++]
* 12 times
2e44 dc00  st   $00,[y,x++]
2e45 dc00  st   $00,[y,x++]
2e46 dc00  st   $00,[y,x++]
* 5 times
2e49 dc1f  st   $1f,[y,x++]
2e4a dc1f  st   $1f,[y,x++]
2e4b dc1f  st   $1f,[y,x++]
* 12 times
2e55 dc00  st   $00,[y,x++]
2e56 dc00  st   $00,[y,x++]
2e57 dc00  st   $00,[y,x++]
* 5 times
2e5a dc27  st   $27,[y,x++]
2e5b dc27  st   $27,[y,x++]
2e5c dc27  st   $27,[y,x++]
* 12 times
2e66 dc00  st   $00,[y,x++]
2e67 dc00  st   $00,[y,x++]
2e68 dc00  st   $00,[y,x++]
* 5 times
2e6b dc2f  st   $2f,[y,x++]
2e6c dc2f  st   $2f,[y,x++]
2e6d dc2f  st   $2f,[y,x++]
* 12 times
2e77 dc00  st   $00,[y,x++]
2e78 dc00  st   $00,[y,x++]
2e79 dc00  st   $00,[y,x++]
* 5 times
2e7c dc37  st   $37,[y,x++]
2e7d dc37  st   $37,[y,x++]
2e7e dc37  st   $37,[y,x++]
* 12 times
2e88 dc00  st   $00,[y,x++]
2e89 dc00  st   $00,[y,x++]
2e8a dc00  st   $00,[y,x++]
* 5 times
2e8d dc3f  st   $3f,[y,x++]
2e8e dc3f  st   $3f,[y,x++]
2e8f dc3f  st   $3f,[y,x++]
* 12 times
2e99 dc00  st   $00,[y,x++]
2e9a dc00  st   $00,[y,x++]
2e9b dc00  st   $00,[y,x++]
* 5 times
2e9e dc08  st   $08,[y,x++]
2e9f dc08  st   $08,[y,x++]
2ea0 dc08  st   $08,[y,x++]
* 12 times
2eaa dc00  st   $00,[y,x++]
2eab dc00  st   $00,[y,x++]
2eac dc00  st   $00,[y,x++]
* 6 times
2eb0 1000  ld   $00,x
2eb1 1445  ld   $45,y
2eb2 dc00  st   $00,[y,x++]
2eb3 dc00  st   $00,[y,x++]
2eb4 dc00  st   $00,[y,x++]
* 6 times
2eb8 dc07  st   $07,[y,x++]
2eb9 dc07  st   $07,[y,x++]
2eba dc07  st   $07,[y,x++]
* 12 times
2ec4 dc00  st   $00,[y,x++]
2ec5 dc00  st   $00,[y,x++]
2ec6 dc00  st   $00,[y,x++]
* 5 times
2ec9 dc0f  st   $0f,[y,x++]
2eca dc0f  st   $0f,[y,x++]
2ecb dc0f  st   $0f,[y,x++]
* 12 times
2ed5 dc00  st   $00,[y,x++]
2ed6 dc00  st   $00,[y,x++]
2ed7 dc00  st   $00,[y,x++]
* 5 times
2eda dc17  st   $17,[y,x++]
2edb dc17  st   $17,[y,x++]
2edc dc17  st   $17,[y,x++]
* 12 times
2ee6 dc00  st   $00,[y,x++]
2ee7 dc00  st   $00,[y,x++]
2ee8 dc00  st   $00,[y,x++]
* 5 times
2eeb dc1f  st   $1f,[y,x++]
2eec dc1f  st   $1f,[y,x++]
2eed dc1f  st   $1f,[y,x++]
* 12 times
2ef7 dc00  st   $00,[y,x++]
2ef8 dc00  st   $00,[y,x++]
2ef9 dc00  st   $00,[y,x++]
* 5 times
2efc dc27  st   $27,[y,x++]
2efd dc27  st   $27,[y,x++]
2efe dc27  st   $27,[y,x++]
* 12 times
2f08 dc00  st   $00,[y,x++]
2f09 dc00  st   $00,[y,x++]
2f0a dc00  st   $00,[y,x++]
* 5 times
2f0d dc2f  st   $2f,[y,x++]
2f0e dc2f  st   $2f,[y,x++]
2f0f dc2f  st   $2f,[y,x++]
* 12 times
2f19 dc00  st   $00,[y,x++]
2f1a dc00  st   $00,[y,x++]
2f1b dc00  st   $00,[y,x++]
* 5 times
2f1e dc37  st   $37,[y,x++]
2f1f dc37  st   $37,[y,x++]
2f20 dc37  st   $37,[y,x++]
* 12 times
2f2a dc00  st   $00,[y,x++]
2f2b dc00  st   $00,[y,x++]
2f2c dc00  st   $00,[y,x++]
* 5 times
2f2f dc3f  st   $3f,[y,x++]
2f30 dc3f  st   $3f,[y,x++]
2f31 dc3f  st   $3f,[y,x++]
* 12 times
2f3b dc00  st   $00,[y,x++]
2f3c dc00  st   $00,[y,x++]
2f3d dc00  st   $00,[y,x++]
* 5 times
2f40 dc08  st   $08,[y,x++]
2f41 dc08  st   $08,[y,x++]
2f42 dc08  st   $08,[y,x++]
* 12 times
2f4c dc00  st   $00,[y,x++]
2f4d dc00  st   $00,[y,x++]
2f4e dc00  st   $00,[y,x++]
* 6 times
2f52 1000  ld   $00,x
2f53 1446  ld   $46,y
2f54 dc00  st   $00,[y,x++]
2f55 dc00  st   $00,[y,x++]
2f56 dc00  st   $00,[y,x++]
* 6 times
2f5a dc07  st   $07,[y,x++]
2f5b dc07  st   $07,[y,x++]
2f5c dc07  st   $07,[y,x++]
* 12 times
2f66 dc00  st   $00,[y,x++]
2f67 dc00  st   $00,[y,x++]
2f68 dc00  st   $00,[y,x++]
* 5 times
2f6b dc0f  st   $0f,[y,x++]
2f6c dc0f  st   $0f,[y,x++]
2f6d dc0f  st   $0f,[y,x++]
* 12 times
2f77 dc00  st   $00,[y,x++]
2f78 dc00  st   $00,[y,x++]
2f79 dc00  st   $00,[y,x++]
* 5 times
2f7c dc17  st   $17,[y,x++]
2f7d dc17  st   $17,[y,x++]
2f7e dc17  st   $17,[y,x++]
* 12 times
2f88 dc00  st   $00,[y,x++]
2f89 dc00  st   $00,[y,x++]
2f8a dc00  st   $00,[y,x++]
* 5 times
2f8d dc1f  st   $1f,[y,x++]
2f8e dc1f  st   $1f,[y,x++]
2f8f dc1f  st   $1f,[y,x++]
* 12 times
2f99 dc00  st   $00,[y,x++]
2f9a dc00  st   $00,[y,x++]
2f9b dc00  st   $00,[y,x++]
* 5 times
2f9e dc27  st   $27,[y,x++]
2f9f dc27  st   $27,[y,x++]
2fa0 dc27  st   $27,[y,x++]
* 12 times
2faa dc00  st   $00,[y,x++]
2fab dc00  st   $00,[y,x++]
2fac dc00  st   $00,[y,x++]
* 5 times
2faf dc2f  st   $2f,[y,x++]
2fb0 dc2f  st   $2f,[y,x++]
2fb1 dc2f  st   $2f,[y,x++]
* 12 times
2fbb dc00  st   $00,[y,x++]
2fbc dc00  st   $00,[y,x++]
2fbd dc00  st   $00,[y,x++]
* 5 times
2fc0 dc37  st   $37,[y,x++]
2fc1 dc37  st   $37,[y,x++]
2fc2 dc37  st   $37,[y,x++]
* 12 times
2fcc dc00  st   $00,[y,x++]
2fcd dc00  st   $00,[y,x++]
2fce dc00  st   $00,[y,x++]
* 5 times
2fd1 dc3f  st   $3f,[y,x++]
2fd2 dc3f  st   $3f,[y,x++]
2fd3 dc3f  st   $3f,[y,x++]
* 12 times
2fdd dc00  st   $00,[y,x++]
2fde dc00  st   $00,[y,x++]
2fdf dc00  st   $00,[y,x++]
* 5 times
2fe2 dc08  st   $08,[y,x++]
2fe3 dc08  st   $08,[y,x++]
2fe4 dc08  st   $08,[y,x++]
* 12 times
2fee dc00  st   $00,[y,x++]
2fef dc00  st   $00,[y,x++]
2ff0 dc00  st   $00,[y,x++]
* 6 times
2ff4 1000  ld   $00,x
2ff5 1447  ld   $47,y
2ff6 dc00  st   $00,[y,x++]
2ff7 dc00  st   $00,[y,x++]
2ff8 dc00  st   $00,[y,x++]
* 6 times
2ffc dc07  st   $07,[y,x++]
2ffd dc07  st   $07,[y,x++]
2ffe dc07  st   $07,[y,x++]
* 12 times
3008 dc00  st   $00,[y,x++]
3009 dc00  st   $00,[y,x++]
300a dc00  st   $00,[y,x++]
* 5 times
300d dc0f  st   $0f,[y,x++]
300e dc0f  st   $0f,[y,x++]
300f dc0f  st   $0f,[y,x++]
* 12 times
3019 dc00  st   $00,[y,x++]
301a dc00  st   $00,[y,x++]
301b dc00  st   $00,[y,x++]
* 5 times
301e dc17  st   $17,[y,x++]
301f dc17  st   $17,[y,x++]
3020 dc17  st   $17,[y,x++]
* 12 times
302a dc00  st   $00,[y,x++]
302b dc00  st   $00,[y,x++]
302c dc00  st   $00,[y,x++]
* 5 times
302f dc1f  st   $1f,[y,x++]
3030 dc1f  st   $1f,[y,x++]
3031 dc1f  st   $1f,[y,x++]
* 12 times
303b dc00  st   $00,[y,x++]
303c dc00  st   $00,[y,x++]
303d dc00  st   $00,[y,x++]
* 5 times
3040 dc27  st   $27,[y,x++]
3041 dc27  st   $27,[y,x++]
3042 dc27  st   $27,[y,x++]
* 12 times
304c dc00  st   $00,[y,x++]
304d dc00  st   $00,[y,x++]
304e dc00  st   $00,[y,x++]
* 5 times
3051 dc2f  st   $2f,[y,x++]
3052 dc2f  st   $2f,[y,x++]
3053 dc2f  st   $2f,[y,x++]
* 12 times
305d dc00  st   $00,[y,x++]
305e dc00  st   $00,[y,x++]
305f dc00  st   $00,[y,x++]
* 5 times
3062 dc37  st   $37,[y,x++]
3063 dc37  st   $37,[y,x++]
3064 dc37  st   $37,[y,x++]
* 12 times
306e dc00  st   $00,[y,x++]
306f dc00  st   $00,[y,x++]
3070 dc00  st   $00,[y,x++]
* 5 times
3073 dc3f  st   $3f,[y,x++]
3074 dc3f  st   $3f,[y,x++]
3075 dc3f  st   $3f,[y,x++]
* 12 times
307f dc00  st   $00,[y,x++]
3080 dc00  st   $00,[y,x++]
3081 dc00  st   $00,[y,x++]
* 5 times
3084 dc08  st   $08,[y,x++]
3085 dc08  st   $08,[y,x++]
3086 dc08  st   $08,[y,x++]
* 12 times
3090 dc00  st   $00,[y,x++]
3091 dc00  st   $00,[y,x++]
3092 dc00  st   $00,[y,x++]
* 6 times
3096 1000  ld   $00,x
3097 1448  ld   $48,y
3098 dc00  st   $00,[y,x++]
3099 dc00  st   $00,[y,x++]
309a dc00  st   $00,[y,x++]
* 6 times
309e dc07  st   $07,[y,x++]
309f dc07  st   $07,[y,x++]
30a0 dc07  st   $07,[y,x++]
* 12 times
30aa dc00  st   $00,[y,x++]
30ab dc00  st   $00,[y,x++]
30ac dc00  st   $00,[y,x++]
* 5 times
30af dc0f  st   $0f,[y,x++]
30b0 dc0f  st   $0f,[y,x++]
30b1 dc0f  st   $0f,[y,x++]
* 12 times
30bb dc00  st   $00,[y,x++]
30bc dc00  st   $00,[y,x++]
30bd dc00  st   $00,[y,x++]
* 5 times
30c0 dc17  st   $17,[y,x++]
30c1 dc17  st   $17,[y,x++]
30c2 dc17  st   $17,[y,x++]
* 12 times
30cc dc00  st   $00,[y,x++]
30cd dc00  st   $00,[y,x++]
30ce dc00  st   $00,[y,x++]
* 5 times
30d1 dc1f  st   $1f,[y,x++]
30d2 dc1f  st   $1f,[y,x++]
30d3 dc1f  st   $1f,[y,x++]
* 12 times
30dd dc00  st   $00,[y,x++]
30de dc00  st   $00,[y,x++]
30df dc00  st   $00,[y,x++]
* 5 times
30e2 dc27  st   $27,[y,x++]
30e3 dc27  st   $27,[y,x++]
30e4 dc27  st   $27,[y,x++]
* 12 times
30ee dc00  st   $00,[y,x++]
30ef dc00  st   $00,[y,x++]
30f0 dc00  st   $00,[y,x++]
* 5 times
30f3 dc2f  st   $2f,[y,x++]
30f4 dc2f  st   $2f,[y,x++]
30f5 dc2f  st   $2f,[y,x++]
* 12 times
30ff dc00  st   $00,[y,x++]
3100 dc00  st   $00,[y,x++]
3101 dc00  st   $00,[y,x++]
* 5 times
3104 dc37  st   $37,[y,x++]
3105 dc37  st   $37,[y,x++]
3106 dc37  st   $37,[y,x++]
* 12 times
3110 dc00  st   $00,[y,x++]
3111 dc00  st   $00,[y,x++]
3112 dc00  st   $00,[y,x++]
* 5 times
3115 dc3f  st   $3f,[y,x++]
3116 dc3f  st   $3f,[y,x++]
3117 dc3f  st   $3f,[y,x++]
* 12 times
3121 dc00  st   $00,[y,x++]
3122 dc00  st   $00,[y,x++]
3123 dc00  st   $00,[y,x++]
* 5 times
3126 dc08  st   $08,[y,x++]
3127 dc08  st   $08,[y,x++]
3128 dc08  st   $08,[y,x++]
* 12 times
3132 dc00  st   $00,[y,x++]
3133 dc00  st   $00,[y,x++]
3134 dc00  st   $00,[y,x++]
* 6 times
3138 1000  ld   $00,x
3139 1449  ld   $49,y
313a dc00  st   $00,[y,x++]
313b dc00  st   $00,[y,x++]
313c dc00  st   $00,[y,x++]
* 160 times
31da 1000  ld   $00,x
31db 144a  ld   $4a,y
31dc dc00  st   $00,[y,x++]
31dd dc00  st   $00,[y,x++]
31de dc00  st   $00,[y,x++]
* 160 times
327c 1000  ld   $00,x
327d 144b  ld   $4b,y
327e dc00  st   $00,[y,x++]
327f dc00  st   $00,[y,x++]
3280 dc00  st   $00,[y,x++]
* 160 times
331e 1000  ld   $00,x
331f 144c  ld   $4c,y
3320 dc00  st   $00,[y,x++]
3321 dc00  st   $00,[y,x++]
3322 dc00  st   $00,[y,x++]
* 160 times
33c0 1000  ld   $00,x
33c1 144d  ld   $4d,y
33c2 dc00  st   $00,[y,x++]
33c3 dc00  st   $00,[y,x++]
33c4 dc00  st   $00,[y,x++]
* 6 times
33c8 dc2e  st   $2e,[y,x++]
33c9 dc2e  st   $2e,[y,x++]
33ca dc2e  st   $2e,[y,x++]
* 12 times
33d4 dc00  st   $00,[y,x++]
33d5 dc00  st   $00,[y,x++]
33d6 dc00  st   $00,[y,x++]
* 5 times
33d9 dc36  st   $36,[y,x++]
33da dc36  st   $36,[y,x++]
33db dc36  st   $36,[y,x++]
* 12 times
33e5 dc00  st   $00,[y,x++]
33e6 dc00  st   $00,[y,x++]
33e7 dc00  st   $00,[y,x++]
* 5 times
33ea dc3e  st   $3e,[y,x++]
33eb dc3e  st   $3e,[y,x++]
33ec dc3e  st   $3e,[y,x++]
* 12 times
33f6 dc00  st   $00,[y,x++]
33f7 dc00  st   $00,[y,x++]
33f8 dc00  st   $00,[y,x++]
* 5 times
33fb dc07  st   $07,[y,x++]
33fc dc07  st   $07,[y,x++]
33fd dc07  st   $07,[y,x++]
* 12 times
3407 dc00  st   $00,[y,x++]
3408 dc00  st   $00,[y,x++]
3409 dc00  st   $00,[y,x++]
* 5 times
340c dc0f  st   $0f,[y,x++]
340d dc0f  st   $0f,[y,x++]
340e dc0f  st   $0f,[y,x++]
* 12 times
3418 dc00  st   $00,[y,x++]
3419 dc00  st   $00,[y,x++]
341a dc00  st   $00,[y,x++]
* 5 times
341d dc17  st   $17,[y,x++]
341e dc17  st   $17,[y,x++]
341f dc17  st   $17,[y,x++]
* 12 times
3429 dc00  st   $00,[y,x++]
342a dc00  st   $00,[y,x++]
342b dc00  st   $00,[y,x++]
* 5 times
342e dc1f  st   $1f,[y,x++]
342f dc1f  st   $1f,[y,x++]
3430 dc1f  st   $1f,[y,x++]
* 12 times
343a dc00  st   $00,[y,x++]
343b dc00  st   $00,[y,x++]
343c dc00  st   $00,[y,x++]
* 5 times
343f dc27  st   $27,[y,x++]
3440 dc27  st   $27,[y,x++]
3441 dc27  st   $27,[y,x++]
* 12 times
344b dc00  st   $00,[y,x++]
344c dc00  st   $00,[y,x++]
344d dc00  st   $00,[y,x++]
* 5 times
3450 dc2f  st   $2f,[y,x++]
3451 dc2f  st   $2f,[y,x++]
3452 dc2f  st   $2f,[y,x++]
* 12 times
345c dc00  st   $00,[y,x++]
345d dc00  st   $00,[y,x++]
345e dc00  st   $00,[y,x++]
* 6 times
3462 1000  ld   $00,x
3463 144e  ld   $4e,y
3464 dc00  st   $00,[y,x++]
3465 dc00  st   $00,[y,x++]
3466 dc00  st   $00,[y,x++]
* 6 times
346a dc2e  st   $2e,[y,x++]
346b dc2e  st   $2e,[y,x++]
346c dc2e  st   $2e,[y,x++]
* 12 times
3476 dc00  st   $00,[y,x++]
3477 dc00  st   $00,[y,x++]
3478 dc00  st   $00,[y,x++]
* 5 times
347b dc36  st   $36,[y,x++]
347c dc36  st   $36,[y,x++]
347d dc36  st   $36,[y,x++]
* 12 times
3487 dc00  st   $00,[y,x++]
3488 dc00  st   $00,[y,x++]
3489 dc00  st   $00,[y,x++]
* 5 times
348c dc3e  st   $3e,[y,x++]
348d dc3e  st   $3e,[y,x++]
348e dc3e  st   $3e,[y,x++]
* 12 times
3498 dc00  st   $00,[y,x++]
3499 dc00  st   $00,[y,x++]
349a dc00  st   $00,[y,x++]
* 5 times
349d dc07  st   $07,[y,x++]
349e dc07  st   $07,[y,x++]
349f dc07  st   $07,[y,x++]
* 12 times
34a9 dc00  st   $00,[y,x++]
34aa dc00  st   $00,[y,x++]
34ab dc00  st   $00,[y,x++]
* 5 times
34ae dc0f  st   $0f,[y,x++]
34af dc0f  st   $0f,[y,x++]
34b0 dc0f  st   $0f,[y,x++]
* 12 times
34ba dc00  st   $00,[y,x++]
34bb dc00  st   $00,[y,x++]
34bc dc00  st   $00,[y,x++]
* 5 times
34bf dc17  st   $17,[y,x++]
34c0 dc17  st   $17,[y,x++]
34c1 dc17  st   $17,[y,x++]
* 12 times
34cb dc00  st   $00,[y,x++]
34cc dc00  st   $00,[y,x++]
34cd dc00  st   $00,[y,x++]
* 5 times
34d0 dc1f  st   $1f,[y,x++]
34d1 dc1f  st   $1f,[y,x++]
34d2 dc1f  st   $1f,[y,x++]
* 12 times
34dc dc00  st   $00,[y,x++]
34dd dc00  st   $00,[y,x++]
34de dc00  st   $00,[y,x++]
* 5 times
34e1 dc27  st   $27,[y,x++]
34e2 dc27  st   $27,[y,x++]
34e3 dc27  st   $27,[y,x++]
* 12 times
34ed dc00  st   $00,[y,x++]
34ee dc00  st   $00,[y,x++]
34ef dc00  st   $00,[y,x++]
* 5 times
34f2 dc2f  st   $2f,[y,x++]
34f3 dc2f  st   $2f,[y,x++]
34f4 dc2f  st   $2f,[y,x++]
* 12 times
34fe dc00  st   $00,[y,x++]
34ff dc00  st   $00,[y,x++]
3500 dc00  st   $00,[y,x++]
* 6 times
3504 1000  ld   $00,x
3505 144f  ld   $4f,y
3506 dc00  st   $00,[y,x++]
3507 dc00  st   $00,[y,x++]
3508 dc00  st   $00,[y,x++]
* 6 times
350c dc2e  st   $2e,[y,x++]
350d dc2e  st   $2e,[y,x++]
350e dc2e  st   $2e,[y,x++]
* 12 times
3518 dc00  st   $00,[y,x++]
3519 dc00  st   $00,[y,x++]
351a dc00  st   $00,[y,x++]
* 5 times
351d dc36  st   $36,[y,x++]
351e dc36  st   $36,[y,x++]
351f dc36  st   $36,[y,x++]
* 12 times
3529 dc00  st   $00,[y,x++]
352a dc00  st   $00,[y,x++]
352b dc00  st   $00,[y,x++]
* 5 times
352e dc3e  st   $3e,[y,x++]
352f dc3e  st   $3e,[y,x++]
3530 dc3e  st   $3e,[y,x++]
* 12 times
353a dc00  st   $00,[y,x++]
353b dc00  st   $00,[y,x++]
353c dc00  st   $00,[y,x++]
* 5 times
353f dc07  st   $07,[y,x++]
3540 dc07  st   $07,[y,x++]
3541 dc07  st   $07,[y,x++]
* 12 times
354b dc00  st   $00,[y,x++]
354c dc00  st   $00,[y,x++]
354d dc00  st   $00,[y,x++]
* 5 times
3550 dc0f  st   $0f,[y,x++]
3551 dc0f  st   $0f,[y,x++]
3552 dc0f  st   $0f,[y,x++]
* 12 times
355c dc00  st   $00,[y,x++]
355d dc00  st   $00,[y,x++]
355e dc00  st   $00,[y,x++]
* 5 times
3561 dc17  st   $17,[y,x++]
3562 dc17  st   $17,[y,x++]
3563 dc17  st   $17,[y,x++]
* 12 times
356d dc00  st   $00,[y,x++]
356e dc00  st   $00,[y,x++]
356f dc00  st   $00,[y,x++]
* 5 times
3572 dc1f  st   $1f,[y,x++]
3573 dc1f  st   $1f,[y,x++]
3574 dc1f  st   $1f,[y,x++]
* 12 times
357e dc00  st   $00,[y,x++]
357f dc00  st   $00,[y,x++]
3580 dc00  st   $00,[y,x++]
* 5 times
3583 dc27  st   $27,[y,x++]
3584 dc27  st   $27,[y,x++]
3585 dc27  st   $27,[y,x++]
* 12 times
358f dc00  st   $00,[y,x++]
3590 dc00  st   $00,[y,x++]
3591 dc00  st   $00,[y,x++]
* 5 times
3594 dc2f  st   $2f,[y,x++]
3595 dc2f  st   $2f,[y,x++]
3596 dc2f  st   $2f,[y,x++]
* 12 times
35a0 dc00  st   $00,[y,x++]
35a1 dc00  st   $00,[y,x++]
35a2 dc00  st   $00,[y,x++]
* 6 times
35a6 1000  ld   $00,x
35a7 1450  ld   $50,y
35a8 dc00  st   $00,[y,x++]
35a9 dc00  st   $00,[y,x++]
35aa dc00  st   $00,[y,x++]
* 6 times
35ae dc2e  st   $2e,[y,x++]
35af dc2e  st   $2e,[y,x++]
35b0 dc2e  st   $2e,[y,x++]
* 12 times
35ba dc00  st   $00,[y,x++]
35bb dc00  st   $00,[y,x++]
35bc dc00  st   $00,[y,x++]
* 5 times
35bf dc36  st   $36,[y,x++]
35c0 dc36  st   $36,[y,x++]
35c1 dc36  st   $36,[y,x++]
* 12 times
35cb dc00  st   $00,[y,x++]
35cc dc00  st   $00,[y,x++]
35cd dc00  st   $00,[y,x++]
* 5 times
35d0 dc3e  st   $3e,[y,x++]
35d1 dc3e  st   $3e,[y,x++]
35d2 dc3e  st   $3e,[y,x++]
* 12 times
35dc dc00  st   $00,[y,x++]
35dd dc00  st   $00,[y,x++]
35de dc00  st   $00,[y,x++]
* 5 times
35e1 dc07  st   $07,[y,x++]
35e2 dc07  st   $07,[y,x++]
35e3 dc07  st   $07,[y,x++]
* 12 times
35ed dc00  st   $00,[y,x++]
35ee dc00  st   $00,[y,x++]
35ef dc00  st   $00,[y,x++]
* 5 times
35f2 dc0f  st   $0f,[y,x++]
35f3 dc0f  st   $0f,[y,x++]
35f4 dc0f  st   $0f,[y,x++]
* 12 times
35fe dc00  st   $00,[y,x++]
35ff dc00  st   $00,[y,x++]
3600 dc00  st   $00,[y,x++]
* 5 times
3603 dc17  st   $17,[y,x++]
3604 dc17  st   $17,[y,x++]
3605 dc17  st   $17,[y,x++]
* 12 times
360f dc00  st   $00,[y,x++]
3610 dc00  st   $00,[y,x++]
3611 dc00  st   $00,[y,x++]
* 5 times
3614 dc1f  st   $1f,[y,x++]
3615 dc1f  st   $1f,[y,x++]
3616 dc1f  st   $1f,[y,x++]
* 12 times
3620 dc00  st   $00,[y,x++]
3621 dc00  st   $00,[y,x++]
3622 dc00  st   $00,[y,x++]
* 5 times
3625 dc27  st   $27,[y,x++]
3626 dc27  st   $27,[y,x++]
3627 dc27  st   $27,[y,x++]
* 12 times
3631 dc00  st   $00,[y,x++]
3632 dc00  st   $00,[y,x++]
3633 dc00  st   $00,[y,x++]
* 5 times
3636 dc2f  st   $2f,[y,x++]
3637 dc2f  st   $2f,[y,x++]
3638 dc2f  st   $2f,[y,x++]
* 12 times
3642 dc00  st   $00,[y,x++]
3643 dc00  st   $00,[y,x++]
3644 dc00  st   $00,[y,x++]
* 6 times
3648 1000  ld   $00,x
3649 1451  ld   $51,y
364a dc00  st   $00,[y,x++]
364b dc00  st   $00,[y,x++]
364c dc00  st   $00,[y,x++]
* 6 times
3650 dc2e  st   $2e,[y,x++]
3651 dc2e  st   $2e,[y,x++]
3652 dc2e  st   $2e,[y,x++]
* 12 times
365c dc00  st   $00,[y,x++]
365d dc00  st   $00,[y,x++]
365e dc00  st   $00,[y,x++]
* 5 times
3661 dc36  st   $36,[y,x++]
3662 dc36  st   $36,[y,x++]
3663 dc36  st   $36,[y,x++]
* 12 times
366d dc00  st   $00,[y,x++]
366e dc00  st   $00,[y,x++]
366f dc00  st   $00,[y,x++]
* 5 times
3672 dc3e  st   $3e,[y,x++]
3673 dc3e  st   $3e,[y,x++]
3674 dc3e  st   $3e,[y,x++]
* 12 times
367e dc00  st   $00,[y,x++]
367f dc00  st   $00,[y,x++]
3680 dc00  st   $00,[y,x++]
* 5 times
3683 dc07  st   $07,[y,x++]
3684 dc07  st   $07,[y,x++]
3685 dc07  st   $07,[y,x++]
* 12 times
368f dc00  st   $00,[y,x++]
3690 dc00  st   $00,[y,x++]
3691 dc00  st   $00,[y,x++]
* 5 times
3694 dc0f  st   $0f,[y,x++]
3695 dc0f  st   $0f,[y,x++]
3696 dc0f  st   $0f,[y,x++]
* 12 times
36a0 dc00  st   $00,[y,x++]
36a1 dc00  st   $00,[y,x++]
36a2 dc00  st   $00,[y,x++]
* 5 times
36a5 dc17  st   $17,[y,x++]
36a6 dc17  st   $17,[y,x++]
36a7 dc17  st   $17,[y,x++]
* 12 times
36b1 dc00  st   $00,[y,x++]
36b2 dc00  st   $00,[y,x++]
36b3 dc00  st   $00,[y,x++]
* 5 times
36b6 dc1f  st   $1f,[y,x++]
36b7 dc1f  st   $1f,[y,x++]
36b8 dc1f  st   $1f,[y,x++]
* 12 times
36c2 dc00  st   $00,[y,x++]
36c3 dc00  st   $00,[y,x++]
36c4 dc00  st   $00,[y,x++]
* 5 times
36c7 dc27  st   $27,[y,x++]
36c8 dc27  st   $27,[y,x++]
36c9 dc27  st   $27,[y,x++]
* 12 times
36d3 dc00  st   $00,[y,x++]
36d4 dc00  st   $00,[y,x++]
36d5 dc00  st   $00,[y,x++]
* 5 times
36d8 dc2f  st   $2f,[y,x++]
36d9 dc2f  st   $2f,[y,x++]
36da dc2f  st   $2f,[y,x++]
* 12 times
36e4 dc00  st   $00,[y,x++]
36e5 dc00  st   $00,[y,x++]
36e6 dc00  st   $00,[y,x++]
* 6 times
36ea 1000  ld   $00,x
36eb 1452  ld   $52,y
36ec dc00  st   $00,[y,x++]
36ed dc00  st   $00,[y,x++]
36ee dc00  st   $00,[y,x++]
* 160 times
378c 1000  ld   $00,x
378d 1453  ld   $53,y
378e dc00  st   $00,[y,x++]
378f dc00  st   $00,[y,x++]
3790 dc00  st   $00,[y,x++]
* 160 times
382e 1000  ld   $00,x
382f 1454  ld   $54,y
3830 dc00  st   $00,[y,x++]
3831 dc00  st   $00,[y,x++]
3832 dc00  st   $00,[y,x++]
* 160 times
38d0 1000  ld   $00,x
38d1 1455  ld   $55,y
38d2 dc00  st   $00,[y,x++]
38d3 dc00  st   $00,[y,x++]
38d4 dc00  st   $00,[y,x++]
* 160 times
3972 1000  ld   $00,x
3973 1456  ld   $56,y
3974 dc00  st   $00,[y,x++]
3975 dc00  st   $00,[y,x++]
3976 dc00  st   $00,[y,x++]
* 6 times
397a dc16  st   $16,[y,x++]
397b dc16  st   $16,[y,x++]
397c dc16  st   $16,[y,x++]
* 12 times
3986 dc00  st   $00,[y,x++]
3987 dc00  st   $00,[y,x++]
3988 dc00  st   $00,[y,x++]
* 5 times
398b dc1e  st   $1e,[y,x++]
398c dc1e  st   $1e,[y,x++]
398d dc1e  st   $1e,[y,x++]
* 12 times
3997 dc00  st   $00,[y,x++]
3998 dc00  st   $00,[y,x++]
3999 dc00  st   $00,[y,x++]
* 5 times
399c dc26  st   $26,[y,x++]
399d dc26  st   $26,[y,x++]
399e dc26  st   $26,[y,x++]
* 12 times
39a8 dc00  st   $00,[y,x++]
39a9 dc00  st   $00,[y,x++]
39aa dc00  st   $00,[y,x++]
* 5 times
39ad dc2e  st   $2e,[y,x++]
39ae dc2e  st   $2e,[y,x++]
39af dc2e  st   $2e,[y,x++]
* 12 times
39b9 dc00  st   $00,[y,x++]
39ba dc00  st   $00,[y,x++]
39bb dc00  st   $00,[y,x++]
* 5 times
39be dc36  st   $36,[y,x++]
39bf dc36  st   $36,[y,x++]
39c0 dc36  st   $36,[y,x++]
* 12 times
39ca dc00  st   $00,[y,x++]
39cb dc00  st   $00,[y,x++]
39cc dc00  st   $00,[y,x++]
* 5 times
39cf dc3e  st   $3e,[y,x++]
39d0 dc3e  st   $3e,[y,x++]
39d1 dc3e  st   $3e,[y,x++]
* 12 times
39db dc00  st   $00,[y,x++]
39dc dc00  st   $00,[y,x++]
39dd dc00  st   $00,[y,x++]
* 5 times
39e0 dc07  st   $07,[y,x++]
39e1 dc07  st   $07,[y,x++]
39e2 dc07  st   $07,[y,x++]
* 12 times
39ec dc00  st   $00,[y,x++]
39ed dc00  st   $00,[y,x++]
39ee dc00  st   $00,[y,x++]
* 5 times
39f1 dc0f  st   $0f,[y,x++]
39f2 dc0f  st   $0f,[y,x++]
39f3 dc0f  st   $0f,[y,x++]
* 12 times
39fd dc00  st   $00,[y,x++]
39fe dc00  st   $00,[y,x++]
39ff dc00  st   $00,[y,x++]
* 5 times
3a02 dc17  st   $17,[y,x++]
3a03 dc17  st   $17,[y,x++]
3a04 dc17  st   $17,[y,x++]
* 12 times
3a0e dc00  st   $00,[y,x++]
3a0f dc00  st   $00,[y,x++]
3a10 dc00  st   $00,[y,x++]
* 6 times
3a14 1000  ld   $00,x
3a15 1457  ld   $57,y
3a16 dc00  st   $00,[y,x++]
3a17 dc00  st   $00,[y,x++]
3a18 dc00  st   $00,[y,x++]
* 6 times
3a1c dc16  st   $16,[y,x++]
3a1d dc16  st   $16,[y,x++]
3a1e dc16  st   $16,[y,x++]
* 12 times
3a28 dc00  st   $00,[y,x++]
3a29 dc00  st   $00,[y,x++]
3a2a dc00  st   $00,[y,x++]
* 5 times
3a2d dc1e  st   $1e,[y,x++]
3a2e dc1e  st   $1e,[y,x++]
3a2f dc1e  st   $1e,[y,x++]
* 12 times
3a39 dc00  st   $00,[y,x++]
3a3a dc00  st   $00,[y,x++]
3a3b dc00  st   $00,[y,x++]
* 5 times
3a3e dc26  st   $26,[y,x++]
3a3f dc26  st   $26,[y,x++]
3a40 dc26  st   $26,[y,x++]
* 12 times
3a4a dc00  st   $00,[y,x++]
3a4b dc00  st   $00,[y,x++]
3a4c dc00  st   $00,[y,x++]
* 5 times
3a4f dc2e  st   $2e,[y,x++]
3a50 dc2e  st   $2e,[y,x++]
3a51 dc2e  st   $2e,[y,x++]
* 12 times
3a5b dc00  st   $00,[y,x++]
3a5c dc00  st   $00,[y,x++]
3a5d dc00  st   $00,[y,x++]
* 5 times
3a60 dc36  st   $36,[y,x++]
3a61 dc36  st   $36,[y,x++]
3a62 dc36  st   $36,[y,x++]
* 12 times
3a6c dc00  st   $00,[y,x++]
3a6d dc00  st   $00,[y,x++]
3a6e dc00  st   $00,[y,x++]
* 5 times
3a71 dc3e  st   $3e,[y,x++]
3a72 dc3e  st   $3e,[y,x++]
3a73 dc3e  st   $3e,[y,x++]
* 12 times
3a7d dc00  st   $00,[y,x++]
3a7e dc00  st   $00,[y,x++]
3a7f dc00  st   $00,[y,x++]
* 5 times
3a82 dc07  st   $07,[y,x++]
3a83 dc07  st   $07,[y,x++]
3a84 dc07  st   $07,[y,x++]
* 12 times
3a8e dc00  st   $00,[y,x++]
3a8f dc00  st   $00,[y,x++]
3a90 dc00  st   $00,[y,x++]
* 5 times
3a93 dc0f  st   $0f,[y,x++]
3a94 dc0f  st   $0f,[y,x++]
3a95 dc0f  st   $0f,[y,x++]
* 12 times
3a9f dc00  st   $00,[y,x++]
3aa0 dc00  st   $00,[y,x++]
3aa1 dc00  st   $00,[y,x++]
* 5 times
3aa4 dc17  st   $17,[y,x++]
3aa5 dc17  st   $17,[y,x++]
3aa6 dc17  st   $17,[y,x++]
* 12 times
3ab0 dc00  st   $00,[y,x++]
3ab1 dc00  st   $00,[y,x++]
3ab2 dc00  st   $00,[y,x++]
* 6 times
3ab6 1000  ld   $00,x
3ab7 1458  ld   $58,y
3ab8 dc00  st   $00,[y,x++]
3ab9 dc00  st   $00,[y,x++]
3aba dc00  st   $00,[y,x++]
* 6 times
3abe dc16  st   $16,[y,x++]
3abf dc16  st   $16,[y,x++]
3ac0 dc16  st   $16,[y,x++]
* 12 times
3aca dc00  st   $00,[y,x++]
3acb dc00  st   $00,[y,x++]
3acc dc00  st   $00,[y,x++]
* 5 times
3acf dc1e  st   $1e,[y,x++]
3ad0 dc1e  st   $1e,[y,x++]
3ad1 dc1e  st   $1e,[y,x++]
* 12 times
3adb dc00  st   $00,[y,x++]
3adc dc00  st   $00,[y,x++]
3add dc00  st   $00,[y,x++]
* 5 times
3ae0 dc26  st   $26,[y,x++]
3ae1 dc26  st   $26,[y,x++]
3ae2 dc26  st   $26,[y,x++]
* 12 times
3aec dc00  st   $00,[y,x++]
3aed dc00  st   $00,[y,x++]
3aee dc00  st   $00,[y,x++]
* 5 times
3af1 dc2e  st   $2e,[y,x++]
3af2 dc2e  st   $2e,[y,x++]
3af3 dc2e  st   $2e,[y,x++]
* 12 times
3afd dc00  st   $00,[y,x++]
3afe dc00  st   $00,[y,x++]
3aff dc00  st   $00,[y,x++]
* 5 times
3b02 dc36  st   $36,[y,x++]
3b03 dc36  st   $36,[y,x++]
3b04 dc36  st   $36,[y,x++]
* 12 times
3b0e dc00  st   $00,[y,x++]
3b0f dc00  st   $00,[y,x++]
3b10 dc00  st   $00,[y,x++]
* 5 times
3b13 dc3e  st   $3e,[y,x++]
3b14 dc3e  st   $3e,[y,x++]
3b15 dc3e  st   $3e,[y,x++]
* 12 times
3b1f dc00  st   $00,[y,x++]
3b20 dc00  st   $00,[y,x++]
3b21 dc00  st   $00,[y,x++]
* 5 times
3b24 dc07  st   $07,[y,x++]
3b25 dc07  st   $07,[y,x++]
3b26 dc07  st   $07,[y,x++]
* 12 times
3b30 dc00  st   $00,[y,x++]
3b31 dc00  st   $00,[y,x++]
3b32 dc00  st   $00,[y,x++]
* 5 times
3b35 dc0f  st   $0f,[y,x++]
3b36 dc0f  st   $0f,[y,x++]
3b37 dc0f  st   $0f,[y,x++]
* 12 times
3b41 dc00  st   $00,[y,x++]
3b42 dc00  st   $00,[y,x++]
3b43 dc00  st   $00,[y,x++]
* 5 times
3b46 dc17  st   $17,[y,x++]
3b47 dc17  st   $17,[y,x++]
3b48 dc17  st   $17,[y,x++]
* 12 times
3b52 dc00  st   $00,[y,x++]
3b53 dc00  st   $00,[y,x++]
3b54 dc00  st   $00,[y,x++]
* 6 times
3b58 1000  ld   $00,x
3b59 1459  ld   $59,y
3b5a dc00  st   $00,[y,x++]
3b5b dc00  st   $00,[y,x++]
3b5c dc00  st   $00,[y,x++]
* 6 times
3b60 dc16  st   $16,[y,x++]
3b61 dc16  st   $16,[y,x++]
3b62 dc16  st   $16,[y,x++]
* 12 times
3b6c dc00  st   $00,[y,x++]
3b6d dc00  st   $00,[y,x++]
3b6e dc00  st   $00,[y,x++]
* 5 times
3b71 dc1e  st   $1e,[y,x++]
3b72 dc1e  st   $1e,[y,x++]
3b73 dc1e  st   $1e,[y,x++]
* 12 times
3b7d dc00  st   $00,[y,x++]
3b7e dc00  st   $00,[y,x++]
3b7f dc00  st   $00,[y,x++]
* 5 times
3b82 dc26  st   $26,[y,x++]
3b83 dc26  st   $26,[y,x++]
3b84 dc26  st   $26,[y,x++]
* 12 times
3b8e dc00  st   $00,[y,x++]
3b8f dc00  st   $00,[y,x++]
3b90 dc00  st   $00,[y,x++]
* 5 times
3b93 dc2e  st   $2e,[y,x++]
3b94 dc2e  st   $2e,[y,x++]
3b95 dc2e  st   $2e,[y,x++]
* 12 times
3b9f dc00  st   $00,[y,x++]
3ba0 dc00  st   $00,[y,x++]
3ba1 dc00  st   $00,[y,x++]
* 5 times
3ba4 dc36  st   $36,[y,x++]
3ba5 dc36  st   $36,[y,x++]
3ba6 dc36  st   $36,[y,x++]
* 12 times
3bb0 dc00  st   $00,[y,x++]
3bb1 dc00  st   $00,[y,x++]
3bb2 dc00  st   $00,[y,x++]
* 5 times
3bb5 dc3e  st   $3e,[y,x++]
3bb6 dc3e  st   $3e,[y,x++]
3bb7 dc3e  st   $3e,[y,x++]
* 12 times
3bc1 dc00  st   $00,[y,x++]
3bc2 dc00  st   $00,[y,x++]
3bc3 dc00  st   $00,[y,x++]
* 5 times
3bc6 dc07  st   $07,[y,x++]
3bc7 dc07  st   $07,[y,x++]
3bc8 dc07  st   $07,[y,x++]
* 12 times
3bd2 dc00  st   $00,[y,x++]
3bd3 dc00  st   $00,[y,x++]
3bd4 dc00  st   $00,[y,x++]
* 5 times
3bd7 dc0f  st   $0f,[y,x++]
3bd8 dc0f  st   $0f,[y,x++]
3bd9 dc0f  st   $0f,[y,x++]
* 12 times
3be3 dc00  st   $00,[y,x++]
3be4 dc00  st   $00,[y,x++]
3be5 dc00  st   $00,[y,x++]
* 5 times
3be8 dc17  st   $17,[y,x++]
3be9 dc17  st   $17,[y,x++]
3bea dc17  st   $17,[y,x++]
* 12 times
3bf4 dc00  st   $00,[y,x++]
3bf5 dc00  st   $00,[y,x++]
3bf6 dc00  st   $00,[y,x++]
* 6 times
3bfa 1000  ld   $00,x
3bfb 145a  ld   $5a,y
3bfc dc00  st   $00,[y,x++]
3bfd dc00  st   $00,[y,x++]
3bfe dc00  st   $00,[y,x++]
* 6 times
3c02 dc16  st   $16,[y,x++]
3c03 dc16  st   $16,[y,x++]
3c04 dc16  st   $16,[y,x++]
* 12 times
3c0e dc00  st   $00,[y,x++]
3c0f dc00  st   $00,[y,x++]
3c10 dc00  st   $00,[y,x++]
* 5 times
3c13 dc1e  st   $1e,[y,x++]
3c14 dc1e  st   $1e,[y,x++]
3c15 dc1e  st   $1e,[y,x++]
* 12 times
3c1f dc00  st   $00,[y,x++]
3c20 dc00  st   $00,[y,x++]
3c21 dc00  st   $00,[y,x++]
* 5 times
3c24 dc26  st   $26,[y,x++]
3c25 dc26  st   $26,[y,x++]
3c26 dc26  st   $26,[y,x++]
* 12 times
3c30 dc00  st   $00,[y,x++]
3c31 dc00  st   $00,[y,x++]
3c32 dc00  st   $00,[y,x++]
* 5 times
3c35 dc2e  st   $2e,[y,x++]
3c36 dc2e  st   $2e,[y,x++]
3c37 dc2e  st   $2e,[y,x++]
* 12 times
3c41 dc00  st   $00,[y,x++]
3c42 dc00  st   $00,[y,x++]
3c43 dc00  st   $00,[y,x++]
* 5 times
3c46 dc36  st   $36,[y,x++]
3c47 dc36  st   $36,[y,x++]
3c48 dc36  st   $36,[y,x++]
* 12 times
3c52 dc00  st   $00,[y,x++]
3c53 dc00  st   $00,[y,x++]
3c54 dc00  st   $00,[y,x++]
* 5 times
3c57 dc3e  st   $3e,[y,x++]
3c58 dc3e  st   $3e,[y,x++]
3c59 dc3e  st   $3e,[y,x++]
* 12 times
3c63 dc00  st   $00,[y,x++]
3c64 dc00  st   $00,[y,x++]
3c65 dc00  st   $00,[y,x++]
* 5 times
3c68 dc07  st   $07,[y,x++]
3c69 dc07  st   $07,[y,x++]
3c6a dc07  st   $07,[y,x++]
* 12 times
3c74 dc00  st   $00,[y,x++]
3c75 dc00  st   $00,[y,x++]
3c76 dc00  st   $00,[y,x++]
* 5 times
3c79 dc0f  st   $0f,[y,x++]
3c7a dc0f  st   $0f,[y,x++]
3c7b dc0f  st   $0f,[y,x++]
* 12 times
3c85 dc00  st   $00,[y,x++]
3c86 dc00  st   $00,[y,x++]
3c87 dc00  st   $00,[y,x++]
* 5 times
3c8a dc17  st   $17,[y,x++]
3c8b dc17  st   $17,[y,x++]
3c8c dc17  st   $17,[y,x++]
* 12 times
3c96 dc00  st   $00,[y,x++]
3c97 dc00  st   $00,[y,x++]
3c98 dc00  st   $00,[y,x++]
* 6 times
3c9c 1000  ld   $00,x
3c9d 145b  ld   $5b,y
3c9e dc00  st   $00,[y,x++]
3c9f dc00  st   $00,[y,x++]
3ca0 dc00  st   $00,[y,x++]
* 160 times
3d3e 1000  ld   $00,x
3d3f 145c  ld   $5c,y
3d40 dc00  st   $00,[y,x++]
3d41 dc00  st   $00,[y,x++]
3d42 dc00  st   $00,[y,x++]
* 160 times
3de0 1000  ld   $00,x
3de1 145d  ld   $5d,y
3de2 dc00  st   $00,[y,x++]
3de3 dc00  st   $00,[y,x++]
3de4 dc00  st   $00,[y,x++]
* 160 times
3e82 1000  ld   $00,x
3e83 145e  ld   $5e,y
3e84 dc00  st   $00,[y,x++]
3e85 dc00  st   $00,[y,x++]
3e86 dc00  st   $00,[y,x++]
* 160 times
3f24 1000  ld   $00,x
3f25 145f  ld   $5f,y
3f26 dc00  st   $00,[y,x++]
3f27 dc00  st   $00,[y,x++]
3f28 dc00  st   $00,[y,x++]
* 160 times
3fc6 1000  ld   $00,x
3fc7 1460  ld   $60,y
3fc8 dc00  st   $00,[y,x++]
3fc9 dc00  st   $00,[y,x++]
3fca dc00  st   $00,[y,x++]
* 160 times
4068 1000  ld   $00,x
4069 1461  ld   $61,y
406a dc00  st   $00,[y,x++]
406b dc00  st   $00,[y,x++]
406c dc00  st   $00,[y,x++]
* 160 times
410a 1000  ld   $00,x
410b 1462  ld   $62,y
410c dc00  st   $00,[y,x++]
410d dc00  st   $00,[y,x++]
410e dc00  st   $00,[y,x++]
* 160 times
41ac 1000  ld   $00,x
41ad 1463  ld   $63,y
41ae dc00  st   $00,[y,x++]
41af dc00  st   $00,[y,x++]
41b0 dc00  st   $00,[y,x++]
* 160 times
424e 1000  ld   $00,x
424f 1464  ld   $64,y
4250 dc00  st   $00,[y,x++]
4251 dc00  st   $00,[y,x++]
4252 dc00  st   $00,[y,x++]
* 160 times
42f0 1000  ld   $00,x
42f1 1465  ld   $65,y
42f2 dc00  st   $00,[y,x++]
42f3 dc00  st   $00,[y,x++]
42f4 dc00  st   $00,[y,x++]
* 160 times
4392 1000  ld   $00,x
4393 1466  ld   $66,y
4394 dc00  st   $00,[y,x++]
4395 dc00  st   $00,[y,x++]
4396 dc00  st   $00,[y,x++]
* 160 times
4434 1000  ld   $00,x
4435 1467  ld   $67,y
4436 dc00  st   $00,[y,x++]
4437 dc00  st   $00,[y,x++]
4438 dc00  st   $00,[y,x++]
* 160 times
44d6 1000  ld   $00,x
44d7 1468  ld   $68,y
44d8 dc00  st   $00,[y,x++]
44d9 dc00  st   $00,[y,x++]
44da dc00  st   $00,[y,x++]
* 160 times
4578 1000  ld   $00,x
4579 1469  ld   $69,y
457a dc00  st   $00,[y,x++]
457b dc00  st   $00,[y,x++]
457c dc00  st   $00,[y,x++]
* 160 times
461a 1000  ld   $00,x
461b 146a  ld   $6a,y
461c dc00  st   $00,[y,x++]
461d dc00  st   $00,[y,x++]
461e dc00  st   $00,[y,x++]
* 160 times
46bc 1000  ld   $00,x
46bd 146b  ld   $6b,y
46be dc00  st   $00,[y,x++]
46bf dc00  st   $00,[y,x++]
46c0 dc00  st   $00,[y,x++]
* 160 times
475e 1000  ld   $00,x
475f 146c  ld   $6c,y
4760 dc00  st   $00,[y,x++]
4761 dc00  st   $00,[y,x++]
4762 dc00  st   $00,[y,x++]
* 160 times
4800 1000  ld   $00,x
4801 146d  ld   $6d,y
4802 dc00  st   $00,[y,x++]
4803 dc00  st   $00,[y,x++]
4804 dc00  st   $00,[y,x++]
* 160 times
48a2 1000  ld   $00,x
48a3 146e  ld   $6e,y
48a4 dc00  st   $00,[y,x++]
48a5 dc00  st   $00,[y,x++]
48a6 dc00  st   $00,[y,x++]
* 160 times
4944 1000  ld   $00,x
4945 146f  ld   $6f,y
4946 dc00  st   $00,[y,x++]
4947 dc00  st   $00,[y,x++]
4948 dc00  st   $00,[y,x++]
* 160 times
49e6 1000  ld   $00,x
49e7 1470  ld   $70,y
49e8 dc00  st   $00,[y,x++]
49e9 dc00  st   $00,[y,x++]
49ea dc00  st   $00,[y,x++]
* 160 times
4a88 1000  ld   $00,x
4a89 1471  ld   $71,y
4a8a dc00  st   $00,[y,x++]
4a8b dc00  st   $00,[y,x++]
4a8c dc00  st   $00,[y,x++]
* 160 times
4b2a 1000  ld   $00,x
4b2b 1472  ld   $72,y
4b2c dc00  st   $00,[y,x++]
4b2d dc00  st   $00,[y,x++]
4b2e dc00  st   $00,[y,x++]
* 160 times
4bcc 1000  ld   $00,x
4bcd 1473  ld   $73,y
4bce dc00  st   $00,[y,x++]
4bcf dc00  st   $00,[y,x++]
4bd0 dc00  st   $00,[y,x++]
* 160 times
4c6e 1000  ld   $00,x
4c6f 1474  ld   $74,y
4c70 dc00  st   $00,[y,x++]
4c71 dc00  st   $00,[y,x++]
4c72 dc00  st   $00,[y,x++]
* 160 times
4d10 1000  ld   $00,x
4d11 1475  ld   $75,y
4d12 dc00  st   $00,[y,x++]
4d13 dc00  st   $00,[y,x++]
4d14 dc00  st   $00,[y,x++]
* 160 times
4db2 1000  ld   $00,x
4db3 1476  ld   $76,y
4db4 dc00  st   $00,[y,x++]
4db5 dc00  st   $00,[y,x++]
4db6 dc00  st   $00,[y,x++]
* 160 times
4e54 1000  ld   $00,x
4e55 1477  ld   $77,y
4e56 dc00  st   $00,[y,x++]
4e57 dc00  st   $00,[y,x++]
4e58 dc00  st   $00,[y,x++]
* 160 times
4ef6 1000  ld   $00,x
4ef7 1478  ld   $78,y
4ef8 dc00  st   $00,[y,x++]
4ef9 dc00  st   $00,[y,x++]
4efa dc00  st   $00,[y,x++]
* 160 times
4f98 1000  ld   $00,x
4f99 1479  ld   $79,y
4f9a dc00  st   $00,[y,x++]
4f9b dc00  st   $00,[y,x++]
4f9c dc00  st   $00,[y,x++]
* 160 times
503a 1000  ld   $00,x
503b 147a  ld   $7a,y
503c dc00  st   $00,[y,x++]
503d dc00  st   $00,[y,x++]
503e dc00  st   $00,[y,x++]
* 160 times
50dc 1000  ld   $00,x
50dd 147b  ld   $7b,y
50de dc00  st   $00,[y,x++]
50df dc00  st   $00,[y,x++]
50e0 dc00  st   $00,[y,x++]
* 160 times
517e 1000  ld   $00,x
517f 147c  ld   $7c,y
5180 dc00  st   $00,[y,x++]
5181 dc00  st   $00,[y,x++]
5182 dc00  st   $00,[y,x++]
* 160 times
5220 1000  ld   $00,x
5221 147d  ld   $7d,y
5222 dc00  st   $00,[y,x++]
5223 dc00  st   $00,[y,x++]
5224 dc00  st   $00,[y,x++]
* 160 times
52c2 1000  ld   $00,x
52c3 147e  ld   $7e,y
52c4 dc00  st   $00,[y,x++]
52c5 dc00  st   $00,[y,x++]
52c6 dc00  st   $00,[y,x++]
* 160 times
5364 1000  ld   $00,x
5365 147f  ld   $7f,y
5366 dc00  st   $00,[y,x++]
5367 dc00  st   $00,[y,x++]
5368 dc00  st   $00,[y,x++]
* 160 times
5406 150d  ld   [$0d],y
5407 e10c  jmp  y,[$0c]
5408 0200  nop
5409
