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
0017 d21a  st   [$1a],x
0018 d61b  st   [$1b],y
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
0023 011a  ld   [$1a]
0024 8001  adda $01
0025 ec19  bne  $19
0026 d21a  st   [$1a],x
0027 011b  ld   [$1b]
0028 8001  adda $01
0029 ec19  bne  $19
002a d61b  st   [$1b],y
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
0047 c21a  st   [$1a]
0048 8200  adda ac
0049 9200  adda ac,x
004a 011a  ld   [$1a]
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
0066 010e  ld   [$0e]
0067 207f  anda $7f
0068 8010  adda $10
0069 c215  st   [$15]
006a c219  st   [$19]
006b 007f  ld   $7f
006c c217  st   [$17]
006d 00ff  ld   $ff
006e c216  st   [$16]
006f c218  st   [$18]
0070 c213  st   [$13]
0071 c214  st   [$14]
0072 0007  ld   $07
0073 1880  ld   $80,out
0074 18c0  ld   $c0,out
0075 007b  ld   $7b
0076 c20c  st   [$0c]
0077 0000  ld   $00
0078 c20d  st   [$0d]
0079 1404  ld   $04,y
007a e000  jmp  y,$00
007b 000f  ld   $0f
007c 1880  ld   $80,out
007d 18c0  ld   $c0,out
007e c207  st   [$07]
007f 1401  ld   $01,y
0080 e000  jmp  y,$00
0081 00c0  ld   $c0
0082 0000  ld   $00
0083 0000  ld   $00
0084 0000  ld   $00
* 126 times
0100 c209  st   [$09]
0101 0080  ld   $80
0102 c20a  st   [$0a]
0103 0110  ld   [$10]
0104 ec2d  bne  $2d
0105 0009  ld   $09
0106 8111  adda [$11]
0107 fe00  bra  ac
0108 fc21  bra  $21
0109 000f  ld   $0f
010a 0007  ld   $07
010b 0003  ld   $03
010c 0001  ld   $01
010d 0002  ld   $02
010e 0004  ld   $04
010f 0008  ld   $08
0110 0004  ld   $04
0111 0002  ld   $02
0112 0001  ld   $01
0113 0003  ld   $03
0114 0007  ld   $07
0115 000f  ld   $0f
0116 000e  ld   $0e
0117 000c  ld   $0c
0118 0008  ld   $08
0119 0004  ld   $04
011a 0002  ld   $02
011b 0001  ld   $01
011c 0002  ld   $02
011d 0004  ld   $04
011e 0008  ld   $08
011f 000c  ld   $0c
0120 008e  ld   $8e
0121 c207  st   [$07]
0122 e825  blt  $25
0123 fc26  bra  $26
0124 0111  ld   [$11]
0125 00ff  ld   $ff
0126 8001  adda $01
0127 c211  st   [$11]
0128 0107  ld   [$07]
0129 200f  anda $0f
012a c207  st   [$07]
012b fc32  bra  $32
012c 0112  ld   [$12]
012d 0005  ld   $05
012e ec2e  bne  $2e
012f a001  suba $01
0130 0110  ld   [$10]
0131 a001  suba $01
0132 c210  st   [$10]
0133 1401  ld   $01,y
0134 0901  ld   [y,$01]
0135 8001  adda $01
0136 ca01  st   [y,$01]
0137 0001  ld   $01
0138 ec38  bne  $38
0139 a001  suba $01
013a 0200  nop
013b 0917  ld   [y,$17]
013c a001  suba $01
013d ca17  st   [y,$17]
013e 1115  ld   [$15],x
013f 1517  ld   [$17],y
0140 cc00  st   $00,[y,x]
0141 0115  ld   [$15]
0142 8116  adda [$16]
0143 f446  bge  $46
0144 fc47  bra  $47
0145 0000  ld   $00
0146 0001  ld   $01
0147 c21a  st   [$1a]
0148 0115  ld   [$15]
0149 8116  adda [$16]
014a a0a0  suba $a0
014b e84e  blt  $4e
014c fc4f  bra  $4f
014d 011a  ld   [$1a]
014e 0001  ld   $01
014f ec54  bne  $54
0150 0000  ld   $00
0151 a116  suba [$16]
0152 fc57  bra  $57
0153 c216  st   [$16]
0154 0200  nop
0155 0200  nop
0156 0200  nop
0157 0115  ld   [$15]
0158 9116  adda [$16],x
0159 0d00  ld   [y,x]
015a ec60  bne  $60
015b 0000  ld   $00
015c 0115  ld   [$15]
015d 8116  adda [$16]
015e fc64  bra  $64
015f d215  st   [$15],x
0160 cc00  st   $00,[y,x]
0161 a116  suba [$16]
0162 c216  st   [$16]
0163 1115  ld   [$15],x
0164 0117  ld   [$17]
0165 8118  adda [$18]
0166 207f  anda $7f
0167 a013  suba $13
0168 f46d  bge  $6d
0169 0000  ld   $00
016a a118  suba [$18]
016b fc70  bra  $70
016c c218  st   [$18]
016d 0200  nop
016e 0200  nop
016f 0200  nop
0170 0117  ld   [$17]
0171 9518  adda [$18],y
0172 0d00  ld   [y,x]
0173 ec79  bne  $79
0174 0000  ld   $00
0175 0117  ld   [$17]
0176 8118  adda [$18]
0177 fc7d  bra  $7d
0178 d617  st   [$17],y
0179 cc00  st   $00,[y,x]
017a a118  suba [$18]
017b c218  st   [$18]
017c 1517  ld   [$17],y
017d cc3f  st   $3f,[y,x]
017e 0028  ld   $28
017f ec7f  bne  $7f
0180 a001  suba $01
0181 0026  ld   $26
0182 c208  st   [$08]
0183 1909  ld   [$09],out
0184 0102  ld   [$02]
0185 2003  anda $03
0186 8001  adda $01
0187 190a  ld   [$0a],out
0188 d602  st   [$02],y
0189 007f  ld   $7f
018a 29fe  anda [y,$fe]
018b 89fa  adda [y,$fa]
018c cafe  st   [y,$fe]
018d 3080  anda $80,x
018e 0500  ld   [x]
018f 89ff  adda [y,$ff]
0190 89fb  adda [y,$fb]
0191 caff  st   [y,$ff]
0192 0200  nop
0193 0200  nop
0194 30fc  anda $fc,x
0195 1402  ld   $02,y
0196 0d00  ld   [y,x]
0197 8103  adda [$03]
0198 c203  st   [$03]
0199 0200  nop
019a 0200  nop
019b 0200  nop
019c 0200  nop
019d 0106  ld   [$06]
019e 0200  nop
019f 1909  ld   [$09],out
01a0 0108  ld   [$08]
01a1 f0cc  beq  $cc
01a2 a001  suba $01
01a3 c208  st   [$08]
01a4 a020  suba $20
01a5 ecaa  bne  $aa
01a6 a002  suba $02
01a7 00c0  ld   $c0
01a8 fcaf  bra  $af
01a9 c209  st   [$09]
01aa ecae  bne  $ae
01ab 0040  ld   $40
01ac fcb0  bra  $b0
01ad c209  st   [$09]
01ae 0109  ld   [$09]
01af 0200  nop
01b0 6040  xora $40
01b1 c20a  st   [$0a]
01b2 0108  ld   [$08]
01b3 6018  xora $18
01b4 ecb7  bne  $b7
01b5 fcb8  bra  $b8
01b6 c313  st   in,[$13]
01b7 0200  nop
01b8 0108  ld   [$08]
01b9 2003  anda $03
01ba ecc6  bne  $c6
01bb 0103  ld   [$03]
01bc 20f0  anda $f0
01bd 4107  ora  [$07]
01be c206  st   [$06]
01bf c003  st   $03,[$03]
01c0 0046  ld   $46
01c1 ecc1  bne  $c1
01c2 a001  suba $01
01c3 0200  nop
01c4 fc84  bra  $84
01c5 1909  ld   [$09],out
01c6 0048  ld   $48
01c7 ecc7  bne  $c7
01c8 a001  suba $01
01c9 0200  nop
01ca fc84  bra  $84
01cb 1909  ld   [$09],out
01cc 0000  ld   $00
01cd c209  st   [$09]
01ce c20b  st   [$0b]
01cf 0113  ld   [$13]
01d0 f0d3  beq  $d3
01d1 fcd4  bra  $d4
01d2 c214  st   [$14]
01d3 0040  ld   $40
01d4 4114  ora  [$14]
01d5 c214  st   [$14]
01d6 2001  anda $01
01d7 ecda  bne  $da
01d8 fcdb  bra  $db
01d9 0000  ld   $00
01da 0001  ld   $01
01db 8119  adda [$19]
01dc c219  st   [$19]
01dd 0114  ld   [$14]
01de 2002  anda $02
01df ece2  bne  $e2
01e0 fce3  bra  $e3
01e1 0000  ld   $00
01e2 00ff  ld   $ff
01e3 8119  adda [$19]
01e4 c219  st   [$19]
01e5 147f  ld   $7f,y
01e6 1119  ld   [$19],x
01e7 dc00  st   $00,[y,x++]
01e8 0001  ld   $01
01e9 c21a  st   [$1a]
01ea 2114  anda [$14]
01eb f0ee  beq  $ee
01ec fcef  bra  $ef
01ed dc0c  st   $0c,[y,x++]
01ee dc08  st   $08,[y,x++]
01ef 011a  ld   [$1a]
01f0 f4e9  bge  $e9
01f1 8200  adda ac
01f2 dc00  st   $00,[y,x++]
01f3 0024  ld   $24
01f4 ecf4  bne  $f4
01f5 a001  suba $01
01f6 0200  nop
01f7 0102  ld   [$02]
01f8 2003  anda $03
01f9 8001  adda $01
01fa 1402  ld   $02,y
01fb e0b0  jmp  y,$b0
01fc 1880  ld   $80,out
01fd 0000  ld   $00
01fe 0000  ld   $00
01ff 0000  ld   $00
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
* 6 times
16ac dc3f  st   $3f,[y,x++]
16ad dc3f  st   $3f,[y,x++]
16ae dc3f  st   $3f,[y,x++]
* 12 times
16b8 dc00  st   $00,[y,x++]
16b9 dc00  st   $00,[y,x++]
16ba dc00  st   $00,[y,x++]
* 5 times
16bd dc2f  st   $2f,[y,x++]
16be dc2f  st   $2f,[y,x++]
16bf dc2f  st   $2f,[y,x++]
* 12 times
16c9 dc00  st   $00,[y,x++]
16ca dc00  st   $00,[y,x++]
16cb dc00  st   $00,[y,x++]
* 5 times
16ce dc1f  st   $1f,[y,x++]
16cf dc1f  st   $1f,[y,x++]
16d0 dc1f  st   $1f,[y,x++]
* 12 times
16da dc00  st   $00,[y,x++]
16db dc00  st   $00,[y,x++]
16dc dc00  st   $00,[y,x++]
* 5 times
16df dc0f  st   $0f,[y,x++]
16e0 dc0f  st   $0f,[y,x++]
16e1 dc0f  st   $0f,[y,x++]
* 12 times
16eb dc00  st   $00,[y,x++]
16ec dc00  st   $00,[y,x++]
16ed dc00  st   $00,[y,x++]
* 5 times
16f0 dc0e  st   $0e,[y,x++]
16f1 dc0e  st   $0e,[y,x++]
16f2 dc0e  st   $0e,[y,x++]
* 12 times
16fc dc00  st   $00,[y,x++]
16fd dc00  st   $00,[y,x++]
16fe dc00  st   $00,[y,x++]
* 5 times
1701 dc0a  st   $0a,[y,x++]
1702 dc0a  st   $0a,[y,x++]
1703 dc0a  st   $0a,[y,x++]
* 12 times
170d dc00  st   $00,[y,x++]
170e dc00  st   $00,[y,x++]
170f dc00  st   $00,[y,x++]
* 5 times
1712 dc0b  st   $0b,[y,x++]
1713 dc0b  st   $0b,[y,x++]
1714 dc0b  st   $0b,[y,x++]
* 12 times
171e dc00  st   $00,[y,x++]
171f dc00  st   $00,[y,x++]
1720 dc00  st   $00,[y,x++]
* 5 times
1723 dc07  st   $07,[y,x++]
1724 dc07  st   $07,[y,x++]
1725 dc07  st   $07,[y,x++]
* 12 times
172f dc00  st   $00,[y,x++]
1730 dc00  st   $00,[y,x++]
1731 dc00  st   $00,[y,x++]
* 5 times
1734 dc1b  st   $1b,[y,x++]
1735 dc1b  st   $1b,[y,x++]
1736 dc1b  st   $1b,[y,x++]
* 12 times
1740 dc00  st   $00,[y,x++]
1741 dc00  st   $00,[y,x++]
1742 dc00  st   $00,[y,x++]
* 6 times
1746 1000  ld   $00,x
1747 1420  ld   $20,y
1748 dc00  st   $00,[y,x++]
1749 dc00  st   $00,[y,x++]
174a dc00  st   $00,[y,x++]
* 6 times
174e dc3f  st   $3f,[y,x++]
174f dc3f  st   $3f,[y,x++]
1750 dc3f  st   $3f,[y,x++]
* 12 times
175a dc00  st   $00,[y,x++]
175b dc00  st   $00,[y,x++]
175c dc00  st   $00,[y,x++]
* 5 times
175f dc2f  st   $2f,[y,x++]
1760 dc2f  st   $2f,[y,x++]
1761 dc2f  st   $2f,[y,x++]
* 12 times
176b dc00  st   $00,[y,x++]
176c dc00  st   $00,[y,x++]
176d dc00  st   $00,[y,x++]
* 5 times
1770 dc1f  st   $1f,[y,x++]
1771 dc1f  st   $1f,[y,x++]
1772 dc1f  st   $1f,[y,x++]
* 12 times
177c dc00  st   $00,[y,x++]
177d dc00  st   $00,[y,x++]
177e dc00  st   $00,[y,x++]
* 5 times
1781 dc0f  st   $0f,[y,x++]
1782 dc0f  st   $0f,[y,x++]
1783 dc0f  st   $0f,[y,x++]
* 12 times
178d dc00  st   $00,[y,x++]
178e dc00  st   $00,[y,x++]
178f dc00  st   $00,[y,x++]
* 5 times
1792 dc0e  st   $0e,[y,x++]
1793 dc0e  st   $0e,[y,x++]
1794 dc0e  st   $0e,[y,x++]
* 12 times
179e dc00  st   $00,[y,x++]
179f dc00  st   $00,[y,x++]
17a0 dc00  st   $00,[y,x++]
* 5 times
17a3 dc0a  st   $0a,[y,x++]
17a4 dc0a  st   $0a,[y,x++]
17a5 dc0a  st   $0a,[y,x++]
* 12 times
17af dc00  st   $00,[y,x++]
17b0 dc00  st   $00,[y,x++]
17b1 dc00  st   $00,[y,x++]
* 5 times
17b4 dc0b  st   $0b,[y,x++]
17b5 dc0b  st   $0b,[y,x++]
17b6 dc0b  st   $0b,[y,x++]
* 12 times
17c0 dc00  st   $00,[y,x++]
17c1 dc00  st   $00,[y,x++]
17c2 dc00  st   $00,[y,x++]
* 5 times
17c5 dc07  st   $07,[y,x++]
17c6 dc07  st   $07,[y,x++]
17c7 dc07  st   $07,[y,x++]
* 12 times
17d1 dc00  st   $00,[y,x++]
17d2 dc00  st   $00,[y,x++]
17d3 dc00  st   $00,[y,x++]
* 5 times
17d6 dc1b  st   $1b,[y,x++]
17d7 dc1b  st   $1b,[y,x++]
17d8 dc1b  st   $1b,[y,x++]
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
17f0 dc3f  st   $3f,[y,x++]
17f1 dc3f  st   $3f,[y,x++]
17f2 dc3f  st   $3f,[y,x++]
* 12 times
17fc dc00  st   $00,[y,x++]
17fd dc00  st   $00,[y,x++]
17fe dc00  st   $00,[y,x++]
* 5 times
1801 dc2f  st   $2f,[y,x++]
1802 dc2f  st   $2f,[y,x++]
1803 dc2f  st   $2f,[y,x++]
* 12 times
180d dc00  st   $00,[y,x++]
180e dc00  st   $00,[y,x++]
180f dc00  st   $00,[y,x++]
* 5 times
1812 dc1f  st   $1f,[y,x++]
1813 dc1f  st   $1f,[y,x++]
1814 dc1f  st   $1f,[y,x++]
* 12 times
181e dc00  st   $00,[y,x++]
181f dc00  st   $00,[y,x++]
1820 dc00  st   $00,[y,x++]
* 5 times
1823 dc0f  st   $0f,[y,x++]
1824 dc0f  st   $0f,[y,x++]
1825 dc0f  st   $0f,[y,x++]
* 12 times
182f dc00  st   $00,[y,x++]
1830 dc00  st   $00,[y,x++]
1831 dc00  st   $00,[y,x++]
* 5 times
1834 dc0e  st   $0e,[y,x++]
1835 dc0e  st   $0e,[y,x++]
1836 dc0e  st   $0e,[y,x++]
* 12 times
1840 dc00  st   $00,[y,x++]
1841 dc00  st   $00,[y,x++]
1842 dc00  st   $00,[y,x++]
* 5 times
1845 dc0a  st   $0a,[y,x++]
1846 dc0a  st   $0a,[y,x++]
1847 dc0a  st   $0a,[y,x++]
* 12 times
1851 dc00  st   $00,[y,x++]
1852 dc00  st   $00,[y,x++]
1853 dc00  st   $00,[y,x++]
* 5 times
1856 dc0b  st   $0b,[y,x++]
1857 dc0b  st   $0b,[y,x++]
1858 dc0b  st   $0b,[y,x++]
* 12 times
1862 dc00  st   $00,[y,x++]
1863 dc00  st   $00,[y,x++]
1864 dc00  st   $00,[y,x++]
* 5 times
1867 dc07  st   $07,[y,x++]
1868 dc07  st   $07,[y,x++]
1869 dc07  st   $07,[y,x++]
* 12 times
1873 dc00  st   $00,[y,x++]
1874 dc00  st   $00,[y,x++]
1875 dc00  st   $00,[y,x++]
* 5 times
1878 dc1b  st   $1b,[y,x++]
1879 dc1b  st   $1b,[y,x++]
187a dc1b  st   $1b,[y,x++]
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
1892 dc3f  st   $3f,[y,x++]
1893 dc3f  st   $3f,[y,x++]
1894 dc3f  st   $3f,[y,x++]
* 12 times
189e dc00  st   $00,[y,x++]
189f dc00  st   $00,[y,x++]
18a0 dc00  st   $00,[y,x++]
* 5 times
18a3 dc2f  st   $2f,[y,x++]
18a4 dc2f  st   $2f,[y,x++]
18a5 dc2f  st   $2f,[y,x++]
* 12 times
18af dc00  st   $00,[y,x++]
18b0 dc00  st   $00,[y,x++]
18b1 dc00  st   $00,[y,x++]
* 5 times
18b4 dc1f  st   $1f,[y,x++]
18b5 dc1f  st   $1f,[y,x++]
18b6 dc1f  st   $1f,[y,x++]
* 12 times
18c0 dc00  st   $00,[y,x++]
18c1 dc00  st   $00,[y,x++]
18c2 dc00  st   $00,[y,x++]
* 5 times
18c5 dc0f  st   $0f,[y,x++]
18c6 dc0f  st   $0f,[y,x++]
18c7 dc0f  st   $0f,[y,x++]
* 12 times
18d1 dc00  st   $00,[y,x++]
18d2 dc00  st   $00,[y,x++]
18d3 dc00  st   $00,[y,x++]
* 5 times
18d6 dc0e  st   $0e,[y,x++]
18d7 dc0e  st   $0e,[y,x++]
18d8 dc0e  st   $0e,[y,x++]
* 12 times
18e2 dc00  st   $00,[y,x++]
18e3 dc00  st   $00,[y,x++]
18e4 dc00  st   $00,[y,x++]
* 5 times
18e7 dc0a  st   $0a,[y,x++]
18e8 dc0a  st   $0a,[y,x++]
18e9 dc0a  st   $0a,[y,x++]
* 12 times
18f3 dc00  st   $00,[y,x++]
18f4 dc00  st   $00,[y,x++]
18f5 dc00  st   $00,[y,x++]
* 5 times
18f8 dc0b  st   $0b,[y,x++]
18f9 dc0b  st   $0b,[y,x++]
18fa dc0b  st   $0b,[y,x++]
* 12 times
1904 dc00  st   $00,[y,x++]
1905 dc00  st   $00,[y,x++]
1906 dc00  st   $00,[y,x++]
* 5 times
1909 dc07  st   $07,[y,x++]
190a dc07  st   $07,[y,x++]
190b dc07  st   $07,[y,x++]
* 12 times
1915 dc00  st   $00,[y,x++]
1916 dc00  st   $00,[y,x++]
1917 dc00  st   $00,[y,x++]
* 5 times
191a dc1b  st   $1b,[y,x++]
191b dc1b  st   $1b,[y,x++]
191c dc1b  st   $1b,[y,x++]
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
1934 dc3f  st   $3f,[y,x++]
1935 dc3f  st   $3f,[y,x++]
1936 dc3f  st   $3f,[y,x++]
* 12 times
1940 dc00  st   $00,[y,x++]
1941 dc00  st   $00,[y,x++]
1942 dc00  st   $00,[y,x++]
* 5 times
1945 dc2f  st   $2f,[y,x++]
1946 dc2f  st   $2f,[y,x++]
1947 dc2f  st   $2f,[y,x++]
* 12 times
1951 dc00  st   $00,[y,x++]
1952 dc00  st   $00,[y,x++]
1953 dc00  st   $00,[y,x++]
* 5 times
1956 dc1f  st   $1f,[y,x++]
1957 dc1f  st   $1f,[y,x++]
1958 dc1f  st   $1f,[y,x++]
* 12 times
1962 dc00  st   $00,[y,x++]
1963 dc00  st   $00,[y,x++]
1964 dc00  st   $00,[y,x++]
* 5 times
1967 dc0f  st   $0f,[y,x++]
1968 dc0f  st   $0f,[y,x++]
1969 dc0f  st   $0f,[y,x++]
* 12 times
1973 dc00  st   $00,[y,x++]
1974 dc00  st   $00,[y,x++]
1975 dc00  st   $00,[y,x++]
* 5 times
1978 dc0e  st   $0e,[y,x++]
1979 dc0e  st   $0e,[y,x++]
197a dc0e  st   $0e,[y,x++]
* 12 times
1984 dc00  st   $00,[y,x++]
1985 dc00  st   $00,[y,x++]
1986 dc00  st   $00,[y,x++]
* 5 times
1989 dc0a  st   $0a,[y,x++]
198a dc0a  st   $0a,[y,x++]
198b dc0a  st   $0a,[y,x++]
* 12 times
1995 dc00  st   $00,[y,x++]
1996 dc00  st   $00,[y,x++]
1997 dc00  st   $00,[y,x++]
* 5 times
199a dc0b  st   $0b,[y,x++]
199b dc0b  st   $0b,[y,x++]
199c dc0b  st   $0b,[y,x++]
* 12 times
19a6 dc00  st   $00,[y,x++]
19a7 dc00  st   $00,[y,x++]
19a8 dc00  st   $00,[y,x++]
* 5 times
19ab dc07  st   $07,[y,x++]
19ac dc07  st   $07,[y,x++]
19ad dc07  st   $07,[y,x++]
* 12 times
19b7 dc00  st   $00,[y,x++]
19b8 dc00  st   $00,[y,x++]
19b9 dc00  st   $00,[y,x++]
* 5 times
19bc dc1b  st   $1b,[y,x++]
19bd dc1b  st   $1b,[y,x++]
19be dc1b  st   $1b,[y,x++]
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
* 160 times
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
* 6 times
1bbc dc3e  st   $3e,[y,x++]
1bbd dc3e  st   $3e,[y,x++]
1bbe dc3e  st   $3e,[y,x++]
* 12 times
1bc8 dc00  st   $00,[y,x++]
1bc9 dc00  st   $00,[y,x++]
1bca dc00  st   $00,[y,x++]
* 5 times
1bcd dc2e  st   $2e,[y,x++]
1bce dc2e  st   $2e,[y,x++]
1bcf dc2e  st   $2e,[y,x++]
* 12 times
1bd9 dc00  st   $00,[y,x++]
1bda dc00  st   $00,[y,x++]
1bdb dc00  st   $00,[y,x++]
* 5 times
1bde dc1e  st   $1e,[y,x++]
1bdf dc1e  st   $1e,[y,x++]
1be0 dc1e  st   $1e,[y,x++]
* 12 times
1bea dc00  st   $00,[y,x++]
1beb dc00  st   $00,[y,x++]
1bec dc00  st   $00,[y,x++]
* 5 times
1bef dc0d  st   $0d,[y,x++]
1bf0 dc0d  st   $0d,[y,x++]
1bf1 dc0d  st   $0d,[y,x++]
* 12 times
1bfb dc00  st   $00,[y,x++]
1bfc dc00  st   $00,[y,x++]
1bfd dc00  st   $00,[y,x++]
* 5 times
1c00 dc09  st   $09,[y,x++]
1c01 dc09  st   $09,[y,x++]
1c02 dc09  st   $09,[y,x++]
* 12 times
1c0c dc00  st   $00,[y,x++]
1c0d dc00  st   $00,[y,x++]
1c0e dc00  st   $00,[y,x++]
* 5 times
1c11 dc06  st   $06,[y,x++]
1c12 dc06  st   $06,[y,x++]
1c13 dc06  st   $06,[y,x++]
* 12 times
1c1d dc00  st   $00,[y,x++]
1c1e dc00  st   $00,[y,x++]
1c1f dc00  st   $00,[y,x++]
* 5 times
1c22 dc02  st   $02,[y,x++]
1c23 dc02  st   $02,[y,x++]
1c24 dc02  st   $02,[y,x++]
* 12 times
1c2e dc00  st   $00,[y,x++]
1c2f dc00  st   $00,[y,x++]
1c30 dc00  st   $00,[y,x++]
* 5 times
1c33 dc03  st   $03,[y,x++]
1c34 dc03  st   $03,[y,x++]
1c35 dc03  st   $03,[y,x++]
* 12 times
1c3f dc00  st   $00,[y,x++]
1c40 dc00  st   $00,[y,x++]
1c41 dc00  st   $00,[y,x++]
* 5 times
1c44 dc17  st   $17,[y,x++]
1c45 dc17  st   $17,[y,x++]
1c46 dc17  st   $17,[y,x++]
* 12 times
1c50 dc00  st   $00,[y,x++]
1c51 dc00  st   $00,[y,x++]
1c52 dc00  st   $00,[y,x++]
* 6 times
1c56 1000  ld   $00,x
1c57 1428  ld   $28,y
1c58 dc00  st   $00,[y,x++]
1c59 dc00  st   $00,[y,x++]
1c5a dc00  st   $00,[y,x++]
* 6 times
1c5e dc3e  st   $3e,[y,x++]
1c5f dc3e  st   $3e,[y,x++]
1c60 dc3e  st   $3e,[y,x++]
* 12 times
1c6a dc00  st   $00,[y,x++]
1c6b dc00  st   $00,[y,x++]
1c6c dc00  st   $00,[y,x++]
* 5 times
1c6f dc2e  st   $2e,[y,x++]
1c70 dc2e  st   $2e,[y,x++]
1c71 dc2e  st   $2e,[y,x++]
* 12 times
1c7b dc00  st   $00,[y,x++]
1c7c dc00  st   $00,[y,x++]
1c7d dc00  st   $00,[y,x++]
* 5 times
1c80 dc1e  st   $1e,[y,x++]
1c81 dc1e  st   $1e,[y,x++]
1c82 dc1e  st   $1e,[y,x++]
* 12 times
1c8c dc00  st   $00,[y,x++]
1c8d dc00  st   $00,[y,x++]
1c8e dc00  st   $00,[y,x++]
* 5 times
1c91 dc0d  st   $0d,[y,x++]
1c92 dc0d  st   $0d,[y,x++]
1c93 dc0d  st   $0d,[y,x++]
* 12 times
1c9d dc00  st   $00,[y,x++]
1c9e dc00  st   $00,[y,x++]
1c9f dc00  st   $00,[y,x++]
* 5 times
1ca2 dc09  st   $09,[y,x++]
1ca3 dc09  st   $09,[y,x++]
1ca4 dc09  st   $09,[y,x++]
* 12 times
1cae dc00  st   $00,[y,x++]
1caf dc00  st   $00,[y,x++]
1cb0 dc00  st   $00,[y,x++]
* 5 times
1cb3 dc06  st   $06,[y,x++]
1cb4 dc06  st   $06,[y,x++]
1cb5 dc06  st   $06,[y,x++]
* 12 times
1cbf dc00  st   $00,[y,x++]
1cc0 dc00  st   $00,[y,x++]
1cc1 dc00  st   $00,[y,x++]
* 5 times
1cc4 dc02  st   $02,[y,x++]
1cc5 dc02  st   $02,[y,x++]
1cc6 dc02  st   $02,[y,x++]
* 12 times
1cd0 dc00  st   $00,[y,x++]
1cd1 dc00  st   $00,[y,x++]
1cd2 dc00  st   $00,[y,x++]
* 5 times
1cd5 dc03  st   $03,[y,x++]
1cd6 dc03  st   $03,[y,x++]
1cd7 dc03  st   $03,[y,x++]
* 12 times
1ce1 dc00  st   $00,[y,x++]
1ce2 dc00  st   $00,[y,x++]
1ce3 dc00  st   $00,[y,x++]
* 5 times
1ce6 dc17  st   $17,[y,x++]
1ce7 dc17  st   $17,[y,x++]
1ce8 dc17  st   $17,[y,x++]
* 12 times
1cf2 dc00  st   $00,[y,x++]
1cf3 dc00  st   $00,[y,x++]
1cf4 dc00  st   $00,[y,x++]
* 6 times
1cf8 1000  ld   $00,x
1cf9 1429  ld   $29,y
1cfa dc00  st   $00,[y,x++]
1cfb dc00  st   $00,[y,x++]
1cfc dc00  st   $00,[y,x++]
* 6 times
1d00 dc3e  st   $3e,[y,x++]
1d01 dc3e  st   $3e,[y,x++]
1d02 dc3e  st   $3e,[y,x++]
* 12 times
1d0c dc00  st   $00,[y,x++]
1d0d dc00  st   $00,[y,x++]
1d0e dc00  st   $00,[y,x++]
* 5 times
1d11 dc2e  st   $2e,[y,x++]
1d12 dc2e  st   $2e,[y,x++]
1d13 dc2e  st   $2e,[y,x++]
* 12 times
1d1d dc00  st   $00,[y,x++]
1d1e dc00  st   $00,[y,x++]
1d1f dc00  st   $00,[y,x++]
* 5 times
1d22 dc1e  st   $1e,[y,x++]
1d23 dc1e  st   $1e,[y,x++]
1d24 dc1e  st   $1e,[y,x++]
* 12 times
1d2e dc00  st   $00,[y,x++]
1d2f dc00  st   $00,[y,x++]
1d30 dc00  st   $00,[y,x++]
* 5 times
1d33 dc0d  st   $0d,[y,x++]
1d34 dc0d  st   $0d,[y,x++]
1d35 dc0d  st   $0d,[y,x++]
* 12 times
1d3f dc00  st   $00,[y,x++]
1d40 dc00  st   $00,[y,x++]
1d41 dc00  st   $00,[y,x++]
* 5 times
1d44 dc09  st   $09,[y,x++]
1d45 dc09  st   $09,[y,x++]
1d46 dc09  st   $09,[y,x++]
* 12 times
1d50 dc00  st   $00,[y,x++]
1d51 dc00  st   $00,[y,x++]
1d52 dc00  st   $00,[y,x++]
* 5 times
1d55 dc06  st   $06,[y,x++]
1d56 dc06  st   $06,[y,x++]
1d57 dc06  st   $06,[y,x++]
* 12 times
1d61 dc00  st   $00,[y,x++]
1d62 dc00  st   $00,[y,x++]
1d63 dc00  st   $00,[y,x++]
* 5 times
1d66 dc02  st   $02,[y,x++]
1d67 dc02  st   $02,[y,x++]
1d68 dc02  st   $02,[y,x++]
* 12 times
1d72 dc00  st   $00,[y,x++]
1d73 dc00  st   $00,[y,x++]
1d74 dc00  st   $00,[y,x++]
* 5 times
1d77 dc03  st   $03,[y,x++]
1d78 dc03  st   $03,[y,x++]
1d79 dc03  st   $03,[y,x++]
* 12 times
1d83 dc00  st   $00,[y,x++]
1d84 dc00  st   $00,[y,x++]
1d85 dc00  st   $00,[y,x++]
* 5 times
1d88 dc17  st   $17,[y,x++]
1d89 dc17  st   $17,[y,x++]
1d8a dc17  st   $17,[y,x++]
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
1da2 dc3e  st   $3e,[y,x++]
1da3 dc3e  st   $3e,[y,x++]
1da4 dc3e  st   $3e,[y,x++]
* 12 times
1dae dc00  st   $00,[y,x++]
1daf dc00  st   $00,[y,x++]
1db0 dc00  st   $00,[y,x++]
* 5 times
1db3 dc2e  st   $2e,[y,x++]
1db4 dc2e  st   $2e,[y,x++]
1db5 dc2e  st   $2e,[y,x++]
* 12 times
1dbf dc00  st   $00,[y,x++]
1dc0 dc00  st   $00,[y,x++]
1dc1 dc00  st   $00,[y,x++]
* 5 times
1dc4 dc1e  st   $1e,[y,x++]
1dc5 dc1e  st   $1e,[y,x++]
1dc6 dc1e  st   $1e,[y,x++]
* 12 times
1dd0 dc00  st   $00,[y,x++]
1dd1 dc00  st   $00,[y,x++]
1dd2 dc00  st   $00,[y,x++]
* 5 times
1dd5 dc0d  st   $0d,[y,x++]
1dd6 dc0d  st   $0d,[y,x++]
1dd7 dc0d  st   $0d,[y,x++]
* 12 times
1de1 dc00  st   $00,[y,x++]
1de2 dc00  st   $00,[y,x++]
1de3 dc00  st   $00,[y,x++]
* 5 times
1de6 dc09  st   $09,[y,x++]
1de7 dc09  st   $09,[y,x++]
1de8 dc09  st   $09,[y,x++]
* 12 times
1df2 dc00  st   $00,[y,x++]
1df3 dc00  st   $00,[y,x++]
1df4 dc00  st   $00,[y,x++]
* 5 times
1df7 dc06  st   $06,[y,x++]
1df8 dc06  st   $06,[y,x++]
1df9 dc06  st   $06,[y,x++]
* 12 times
1e03 dc00  st   $00,[y,x++]
1e04 dc00  st   $00,[y,x++]
1e05 dc00  st   $00,[y,x++]
* 5 times
1e08 dc02  st   $02,[y,x++]
1e09 dc02  st   $02,[y,x++]
1e0a dc02  st   $02,[y,x++]
* 12 times
1e14 dc00  st   $00,[y,x++]
1e15 dc00  st   $00,[y,x++]
1e16 dc00  st   $00,[y,x++]
* 5 times
1e19 dc03  st   $03,[y,x++]
1e1a dc03  st   $03,[y,x++]
1e1b dc03  st   $03,[y,x++]
* 12 times
1e25 dc00  st   $00,[y,x++]
1e26 dc00  st   $00,[y,x++]
1e27 dc00  st   $00,[y,x++]
* 5 times
1e2a dc17  st   $17,[y,x++]
1e2b dc17  st   $17,[y,x++]
1e2c dc17  st   $17,[y,x++]
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
1e44 dc3e  st   $3e,[y,x++]
1e45 dc3e  st   $3e,[y,x++]
1e46 dc3e  st   $3e,[y,x++]
* 12 times
1e50 dc00  st   $00,[y,x++]
1e51 dc00  st   $00,[y,x++]
1e52 dc00  st   $00,[y,x++]
* 5 times
1e55 dc2e  st   $2e,[y,x++]
1e56 dc2e  st   $2e,[y,x++]
1e57 dc2e  st   $2e,[y,x++]
* 12 times
1e61 dc00  st   $00,[y,x++]
1e62 dc00  st   $00,[y,x++]
1e63 dc00  st   $00,[y,x++]
* 5 times
1e66 dc1e  st   $1e,[y,x++]
1e67 dc1e  st   $1e,[y,x++]
1e68 dc1e  st   $1e,[y,x++]
* 12 times
1e72 dc00  st   $00,[y,x++]
1e73 dc00  st   $00,[y,x++]
1e74 dc00  st   $00,[y,x++]
* 5 times
1e77 dc0d  st   $0d,[y,x++]
1e78 dc0d  st   $0d,[y,x++]
1e79 dc0d  st   $0d,[y,x++]
* 12 times
1e83 dc00  st   $00,[y,x++]
1e84 dc00  st   $00,[y,x++]
1e85 dc00  st   $00,[y,x++]
* 5 times
1e88 dc09  st   $09,[y,x++]
1e89 dc09  st   $09,[y,x++]
1e8a dc09  st   $09,[y,x++]
* 12 times
1e94 dc00  st   $00,[y,x++]
1e95 dc00  st   $00,[y,x++]
1e96 dc00  st   $00,[y,x++]
* 5 times
1e99 dc06  st   $06,[y,x++]
1e9a dc06  st   $06,[y,x++]
1e9b dc06  st   $06,[y,x++]
* 12 times
1ea5 dc00  st   $00,[y,x++]
1ea6 dc00  st   $00,[y,x++]
1ea7 dc00  st   $00,[y,x++]
* 5 times
1eaa dc02  st   $02,[y,x++]
1eab dc02  st   $02,[y,x++]
1eac dc02  st   $02,[y,x++]
* 12 times
1eb6 dc00  st   $00,[y,x++]
1eb7 dc00  st   $00,[y,x++]
1eb8 dc00  st   $00,[y,x++]
* 5 times
1ebb dc03  st   $03,[y,x++]
1ebc dc03  st   $03,[y,x++]
1ebd dc03  st   $03,[y,x++]
* 12 times
1ec7 dc00  st   $00,[y,x++]
1ec8 dc00  st   $00,[y,x++]
1ec9 dc00  st   $00,[y,x++]
* 5 times
1ecc dc17  st   $17,[y,x++]
1ecd dc17  st   $17,[y,x++]
1ece dc17  st   $17,[y,x++]
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
* 160 times
1f80 1000  ld   $00,x
1f81 142d  ld   $2d,y
1f82 dc00  st   $00,[y,x++]
1f83 dc00  st   $00,[y,x++]
1f84 dc00  st   $00,[y,x++]
* 160 times
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
* 6 times
20cc dc3d  st   $3d,[y,x++]
20cd dc3d  st   $3d,[y,x++]
20ce dc3d  st   $3d,[y,x++]
* 12 times
20d8 dc00  st   $00,[y,x++]
20d9 dc00  st   $00,[y,x++]
20da dc00  st   $00,[y,x++]
* 5 times
20dd dc2d  st   $2d,[y,x++]
20de dc2d  st   $2d,[y,x++]
20df dc2d  st   $2d,[y,x++]
* 12 times
20e9 dc00  st   $00,[y,x++]
20ea dc00  st   $00,[y,x++]
20eb dc00  st   $00,[y,x++]
* 5 times
20ee dc1d  st   $1d,[y,x++]
20ef dc1d  st   $1d,[y,x++]
20f0 dc1d  st   $1d,[y,x++]
* 12 times
20fa dc00  st   $00,[y,x++]
20fb dc00  st   $00,[y,x++]
20fc dc00  st   $00,[y,x++]
* 5 times
20ff dc0c  st   $0c,[y,x++]
2100 dc0c  st   $0c,[y,x++]
2101 dc0c  st   $0c,[y,x++]
* 12 times
210b dc00  st   $00,[y,x++]
210c dc00  st   $00,[y,x++]
210d dc00  st   $00,[y,x++]
* 5 times
2110 dc05  st   $05,[y,x++]
2111 dc05  st   $05,[y,x++]
2112 dc05  st   $05,[y,x++]
* 12 times
211c dc00  st   $00,[y,x++]
211d dc00  st   $00,[y,x++]
211e dc00  st   $00,[y,x++]
* 5 times
2121 dc01  st   $01,[y,x++]
2122 dc01  st   $01,[y,x++]
2123 dc01  st   $01,[y,x++]
* 12 times
212d dc00  st   $00,[y,x++]
212e dc00  st   $00,[y,x++]
212f dc00  st   $00,[y,x++]
* 5 times
2132 dc12  st   $12,[y,x++]
2133 dc12  st   $12,[y,x++]
2134 dc12  st   $12,[y,x++]
* 12 times
213e dc00  st   $00,[y,x++]
213f dc00  st   $00,[y,x++]
2140 dc00  st   $00,[y,x++]
* 5 times
2143 dc13  st   $13,[y,x++]
2144 dc13  st   $13,[y,x++]
2145 dc13  st   $13,[y,x++]
* 12 times
214f dc00  st   $00,[y,x++]
2150 dc00  st   $00,[y,x++]
2151 dc00  st   $00,[y,x++]
* 5 times
2154 dc23  st   $23,[y,x++]
2155 dc23  st   $23,[y,x++]
2156 dc23  st   $23,[y,x++]
* 12 times
2160 dc00  st   $00,[y,x++]
2161 dc00  st   $00,[y,x++]
2162 dc00  st   $00,[y,x++]
* 6 times
2166 1000  ld   $00,x
2167 1430  ld   $30,y
2168 dc00  st   $00,[y,x++]
2169 dc00  st   $00,[y,x++]
216a dc00  st   $00,[y,x++]
* 6 times
216e dc3d  st   $3d,[y,x++]
216f dc3d  st   $3d,[y,x++]
2170 dc3d  st   $3d,[y,x++]
* 12 times
217a dc00  st   $00,[y,x++]
217b dc00  st   $00,[y,x++]
217c dc00  st   $00,[y,x++]
* 5 times
217f dc2d  st   $2d,[y,x++]
2180 dc2d  st   $2d,[y,x++]
2181 dc2d  st   $2d,[y,x++]
* 12 times
218b dc00  st   $00,[y,x++]
218c dc00  st   $00,[y,x++]
218d dc00  st   $00,[y,x++]
* 5 times
2190 dc1d  st   $1d,[y,x++]
2191 dc1d  st   $1d,[y,x++]
2192 dc1d  st   $1d,[y,x++]
* 12 times
219c dc00  st   $00,[y,x++]
219d dc00  st   $00,[y,x++]
219e dc00  st   $00,[y,x++]
* 5 times
21a1 dc0c  st   $0c,[y,x++]
21a2 dc0c  st   $0c,[y,x++]
21a3 dc0c  st   $0c,[y,x++]
* 12 times
21ad dc00  st   $00,[y,x++]
21ae dc00  st   $00,[y,x++]
21af dc00  st   $00,[y,x++]
* 5 times
21b2 dc05  st   $05,[y,x++]
21b3 dc05  st   $05,[y,x++]
21b4 dc05  st   $05,[y,x++]
* 12 times
21be dc00  st   $00,[y,x++]
21bf dc00  st   $00,[y,x++]
21c0 dc00  st   $00,[y,x++]
* 5 times
21c3 dc01  st   $01,[y,x++]
21c4 dc01  st   $01,[y,x++]
21c5 dc01  st   $01,[y,x++]
* 12 times
21cf dc00  st   $00,[y,x++]
21d0 dc00  st   $00,[y,x++]
21d1 dc00  st   $00,[y,x++]
* 5 times
21d4 dc12  st   $12,[y,x++]
21d5 dc12  st   $12,[y,x++]
21d6 dc12  st   $12,[y,x++]
* 12 times
21e0 dc00  st   $00,[y,x++]
21e1 dc00  st   $00,[y,x++]
21e2 dc00  st   $00,[y,x++]
* 5 times
21e5 dc13  st   $13,[y,x++]
21e6 dc13  st   $13,[y,x++]
21e7 dc13  st   $13,[y,x++]
* 12 times
21f1 dc00  st   $00,[y,x++]
21f2 dc00  st   $00,[y,x++]
21f3 dc00  st   $00,[y,x++]
* 5 times
21f6 dc23  st   $23,[y,x++]
21f7 dc23  st   $23,[y,x++]
21f8 dc23  st   $23,[y,x++]
* 12 times
2202 dc00  st   $00,[y,x++]
2203 dc00  st   $00,[y,x++]
2204 dc00  st   $00,[y,x++]
* 6 times
2208 1000  ld   $00,x
2209 1431  ld   $31,y
220a dc00  st   $00,[y,x++]
220b dc00  st   $00,[y,x++]
220c dc00  st   $00,[y,x++]
* 6 times
2210 dc3d  st   $3d,[y,x++]
2211 dc3d  st   $3d,[y,x++]
2212 dc3d  st   $3d,[y,x++]
* 12 times
221c dc00  st   $00,[y,x++]
221d dc00  st   $00,[y,x++]
221e dc00  st   $00,[y,x++]
* 5 times
2221 dc2d  st   $2d,[y,x++]
2222 dc2d  st   $2d,[y,x++]
2223 dc2d  st   $2d,[y,x++]
* 12 times
222d dc00  st   $00,[y,x++]
222e dc00  st   $00,[y,x++]
222f dc00  st   $00,[y,x++]
* 5 times
2232 dc1d  st   $1d,[y,x++]
2233 dc1d  st   $1d,[y,x++]
2234 dc1d  st   $1d,[y,x++]
* 12 times
223e dc00  st   $00,[y,x++]
223f dc00  st   $00,[y,x++]
2240 dc00  st   $00,[y,x++]
* 5 times
2243 dc0c  st   $0c,[y,x++]
2244 dc0c  st   $0c,[y,x++]
2245 dc0c  st   $0c,[y,x++]
* 12 times
224f dc00  st   $00,[y,x++]
2250 dc00  st   $00,[y,x++]
2251 dc00  st   $00,[y,x++]
* 5 times
2254 dc05  st   $05,[y,x++]
2255 dc05  st   $05,[y,x++]
2256 dc05  st   $05,[y,x++]
* 12 times
2260 dc00  st   $00,[y,x++]
2261 dc00  st   $00,[y,x++]
2262 dc00  st   $00,[y,x++]
* 5 times
2265 dc01  st   $01,[y,x++]
2266 dc01  st   $01,[y,x++]
2267 dc01  st   $01,[y,x++]
* 12 times
2271 dc00  st   $00,[y,x++]
2272 dc00  st   $00,[y,x++]
2273 dc00  st   $00,[y,x++]
* 5 times
2276 dc12  st   $12,[y,x++]
2277 dc12  st   $12,[y,x++]
2278 dc12  st   $12,[y,x++]
* 12 times
2282 dc00  st   $00,[y,x++]
2283 dc00  st   $00,[y,x++]
2284 dc00  st   $00,[y,x++]
* 5 times
2287 dc13  st   $13,[y,x++]
2288 dc13  st   $13,[y,x++]
2289 dc13  st   $13,[y,x++]
* 12 times
2293 dc00  st   $00,[y,x++]
2294 dc00  st   $00,[y,x++]
2295 dc00  st   $00,[y,x++]
* 5 times
2298 dc23  st   $23,[y,x++]
2299 dc23  st   $23,[y,x++]
229a dc23  st   $23,[y,x++]
* 12 times
22a4 dc00  st   $00,[y,x++]
22a5 dc00  st   $00,[y,x++]
22a6 dc00  st   $00,[y,x++]
* 6 times
22aa 1000  ld   $00,x
22ab 1432  ld   $32,y
22ac dc00  st   $00,[y,x++]
22ad dc00  st   $00,[y,x++]
22ae dc00  st   $00,[y,x++]
* 6 times
22b2 dc3d  st   $3d,[y,x++]
22b3 dc3d  st   $3d,[y,x++]
22b4 dc3d  st   $3d,[y,x++]
* 12 times
22be dc00  st   $00,[y,x++]
22bf dc00  st   $00,[y,x++]
22c0 dc00  st   $00,[y,x++]
* 5 times
22c3 dc2d  st   $2d,[y,x++]
22c4 dc2d  st   $2d,[y,x++]
22c5 dc2d  st   $2d,[y,x++]
* 12 times
22cf dc00  st   $00,[y,x++]
22d0 dc00  st   $00,[y,x++]
22d1 dc00  st   $00,[y,x++]
* 5 times
22d4 dc1d  st   $1d,[y,x++]
22d5 dc1d  st   $1d,[y,x++]
22d6 dc1d  st   $1d,[y,x++]
* 12 times
22e0 dc00  st   $00,[y,x++]
22e1 dc00  st   $00,[y,x++]
22e2 dc00  st   $00,[y,x++]
* 5 times
22e5 dc0c  st   $0c,[y,x++]
22e6 dc0c  st   $0c,[y,x++]
22e7 dc0c  st   $0c,[y,x++]
* 12 times
22f1 dc00  st   $00,[y,x++]
22f2 dc00  st   $00,[y,x++]
22f3 dc00  st   $00,[y,x++]
* 5 times
22f6 dc05  st   $05,[y,x++]
22f7 dc05  st   $05,[y,x++]
22f8 dc05  st   $05,[y,x++]
* 12 times
2302 dc00  st   $00,[y,x++]
2303 dc00  st   $00,[y,x++]
2304 dc00  st   $00,[y,x++]
* 5 times
2307 dc01  st   $01,[y,x++]
2308 dc01  st   $01,[y,x++]
2309 dc01  st   $01,[y,x++]
* 12 times
2313 dc00  st   $00,[y,x++]
2314 dc00  st   $00,[y,x++]
2315 dc00  st   $00,[y,x++]
* 5 times
2318 dc12  st   $12,[y,x++]
2319 dc12  st   $12,[y,x++]
231a dc12  st   $12,[y,x++]
* 12 times
2324 dc00  st   $00,[y,x++]
2325 dc00  st   $00,[y,x++]
2326 dc00  st   $00,[y,x++]
* 5 times
2329 dc13  st   $13,[y,x++]
232a dc13  st   $13,[y,x++]
232b dc13  st   $13,[y,x++]
* 12 times
2335 dc00  st   $00,[y,x++]
2336 dc00  st   $00,[y,x++]
2337 dc00  st   $00,[y,x++]
* 5 times
233a dc23  st   $23,[y,x++]
233b dc23  st   $23,[y,x++]
233c dc23  st   $23,[y,x++]
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
2354 dc3d  st   $3d,[y,x++]
2355 dc3d  st   $3d,[y,x++]
2356 dc3d  st   $3d,[y,x++]
* 12 times
2360 dc00  st   $00,[y,x++]
2361 dc00  st   $00,[y,x++]
2362 dc00  st   $00,[y,x++]
* 5 times
2365 dc2d  st   $2d,[y,x++]
2366 dc2d  st   $2d,[y,x++]
2367 dc2d  st   $2d,[y,x++]
* 12 times
2371 dc00  st   $00,[y,x++]
2372 dc00  st   $00,[y,x++]
2373 dc00  st   $00,[y,x++]
* 5 times
2376 dc1d  st   $1d,[y,x++]
2377 dc1d  st   $1d,[y,x++]
2378 dc1d  st   $1d,[y,x++]
* 12 times
2382 dc00  st   $00,[y,x++]
2383 dc00  st   $00,[y,x++]
2384 dc00  st   $00,[y,x++]
* 5 times
2387 dc0c  st   $0c,[y,x++]
2388 dc0c  st   $0c,[y,x++]
2389 dc0c  st   $0c,[y,x++]
* 12 times
2393 dc00  st   $00,[y,x++]
2394 dc00  st   $00,[y,x++]
2395 dc00  st   $00,[y,x++]
* 5 times
2398 dc05  st   $05,[y,x++]
2399 dc05  st   $05,[y,x++]
239a dc05  st   $05,[y,x++]
* 12 times
23a4 dc00  st   $00,[y,x++]
23a5 dc00  st   $00,[y,x++]
23a6 dc00  st   $00,[y,x++]
* 5 times
23a9 dc01  st   $01,[y,x++]
23aa dc01  st   $01,[y,x++]
23ab dc01  st   $01,[y,x++]
* 12 times
23b5 dc00  st   $00,[y,x++]
23b6 dc00  st   $00,[y,x++]
23b7 dc00  st   $00,[y,x++]
* 5 times
23ba dc12  st   $12,[y,x++]
23bb dc12  st   $12,[y,x++]
23bc dc12  st   $12,[y,x++]
* 12 times
23c6 dc00  st   $00,[y,x++]
23c7 dc00  st   $00,[y,x++]
23c8 dc00  st   $00,[y,x++]
* 5 times
23cb dc13  st   $13,[y,x++]
23cc dc13  st   $13,[y,x++]
23cd dc13  st   $13,[y,x++]
* 12 times
23d7 dc00  st   $00,[y,x++]
23d8 dc00  st   $00,[y,x++]
23d9 dc00  st   $00,[y,x++]
* 5 times
23dc dc23  st   $23,[y,x++]
23dd dc23  st   $23,[y,x++]
23de dc23  st   $23,[y,x++]
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
* 160 times
2490 1000  ld   $00,x
2491 1435  ld   $35,y
2492 dc00  st   $00,[y,x++]
2493 dc00  st   $00,[y,x++]
2494 dc00  st   $00,[y,x++]
* 160 times
2532 1000  ld   $00,x
2533 1436  ld   $36,y
2534 dc00  st   $00,[y,x++]
2535 dc00  st   $00,[y,x++]
2536 dc00  st   $00,[y,x++]
* 160 times
25d4 1000  ld   $00,x
25d5 1437  ld   $37,y
25d6 dc00  st   $00,[y,x++]
25d7 dc00  st   $00,[y,x++]
25d8 dc00  st   $00,[y,x++]
* 6 times
25dc dc3c  st   $3c,[y,x++]
25dd dc3c  st   $3c,[y,x++]
25de dc3c  st   $3c,[y,x++]
* 12 times
25e8 dc00  st   $00,[y,x++]
25e9 dc00  st   $00,[y,x++]
25ea dc00  st   $00,[y,x++]
* 5 times
25ed dc2c  st   $2c,[y,x++]
25ee dc2c  st   $2c,[y,x++]
25ef dc2c  st   $2c,[y,x++]
* 12 times
25f9 dc00  st   $00,[y,x++]
25fa dc00  st   $00,[y,x++]
25fb dc00  st   $00,[y,x++]
* 5 times
25fe dc1c  st   $1c,[y,x++]
25ff dc1c  st   $1c,[y,x++]
2600 dc1c  st   $1c,[y,x++]
* 12 times
260a dc00  st   $00,[y,x++]
260b dc00  st   $00,[y,x++]
260c dc00  st   $00,[y,x++]
* 5 times
260f dc08  st   $08,[y,x++]
2610 dc08  st   $08,[y,x++]
2611 dc08  st   $08,[y,x++]
* 12 times
261b dc00  st   $00,[y,x++]
261c dc00  st   $00,[y,x++]
261d dc00  st   $00,[y,x++]
* 5 times
2620 dc04  st   $04,[y,x++]
2621 dc04  st   $04,[y,x++]
2622 dc04  st   $04,[y,x++]
* 12 times
262c dc00  st   $00,[y,x++]
262d dc00  st   $00,[y,x++]
262e dc00  st   $00,[y,x++]
* 5 times
2631 dc11  st   $11,[y,x++]
2632 dc11  st   $11,[y,x++]
2633 dc11  st   $11,[y,x++]
* 12 times
263d dc00  st   $00,[y,x++]
263e dc00  st   $00,[y,x++]
263f dc00  st   $00,[y,x++]
* 5 times
2642 dc21  st   $21,[y,x++]
2643 dc21  st   $21,[y,x++]
2644 dc21  st   $21,[y,x++]
* 12 times
264e dc00  st   $00,[y,x++]
264f dc00  st   $00,[y,x++]
2650 dc00  st   $00,[y,x++]
* 5 times
2653 dc22  st   $22,[y,x++]
2654 dc22  st   $22,[y,x++]
2655 dc22  st   $22,[y,x++]
* 12 times
265f dc00  st   $00,[y,x++]
2660 dc00  st   $00,[y,x++]
2661 dc00  st   $00,[y,x++]
* 5 times
2664 dc33  st   $33,[y,x++]
2665 dc33  st   $33,[y,x++]
2666 dc33  st   $33,[y,x++]
* 12 times
2670 dc00  st   $00,[y,x++]
2671 dc00  st   $00,[y,x++]
2672 dc00  st   $00,[y,x++]
* 6 times
2676 1000  ld   $00,x
2677 1438  ld   $38,y
2678 dc00  st   $00,[y,x++]
2679 dc00  st   $00,[y,x++]
267a dc00  st   $00,[y,x++]
* 6 times
267e dc3c  st   $3c,[y,x++]
267f dc3c  st   $3c,[y,x++]
2680 dc3c  st   $3c,[y,x++]
* 12 times
268a dc00  st   $00,[y,x++]
268b dc00  st   $00,[y,x++]
268c dc00  st   $00,[y,x++]
* 5 times
268f dc2c  st   $2c,[y,x++]
2690 dc2c  st   $2c,[y,x++]
2691 dc2c  st   $2c,[y,x++]
* 12 times
269b dc00  st   $00,[y,x++]
269c dc00  st   $00,[y,x++]
269d dc00  st   $00,[y,x++]
* 5 times
26a0 dc1c  st   $1c,[y,x++]
26a1 dc1c  st   $1c,[y,x++]
26a2 dc1c  st   $1c,[y,x++]
* 12 times
26ac dc00  st   $00,[y,x++]
26ad dc00  st   $00,[y,x++]
26ae dc00  st   $00,[y,x++]
* 5 times
26b1 dc08  st   $08,[y,x++]
26b2 dc08  st   $08,[y,x++]
26b3 dc08  st   $08,[y,x++]
* 12 times
26bd dc00  st   $00,[y,x++]
26be dc00  st   $00,[y,x++]
26bf dc00  st   $00,[y,x++]
* 5 times
26c2 dc04  st   $04,[y,x++]
26c3 dc04  st   $04,[y,x++]
26c4 dc04  st   $04,[y,x++]
* 12 times
26ce dc00  st   $00,[y,x++]
26cf dc00  st   $00,[y,x++]
26d0 dc00  st   $00,[y,x++]
* 5 times
26d3 dc11  st   $11,[y,x++]
26d4 dc11  st   $11,[y,x++]
26d5 dc11  st   $11,[y,x++]
* 12 times
26df dc00  st   $00,[y,x++]
26e0 dc00  st   $00,[y,x++]
26e1 dc00  st   $00,[y,x++]
* 5 times
26e4 dc21  st   $21,[y,x++]
26e5 dc21  st   $21,[y,x++]
26e6 dc21  st   $21,[y,x++]
* 12 times
26f0 dc00  st   $00,[y,x++]
26f1 dc00  st   $00,[y,x++]
26f2 dc00  st   $00,[y,x++]
* 5 times
26f5 dc22  st   $22,[y,x++]
26f6 dc22  st   $22,[y,x++]
26f7 dc22  st   $22,[y,x++]
* 12 times
2701 dc00  st   $00,[y,x++]
2702 dc00  st   $00,[y,x++]
2703 dc00  st   $00,[y,x++]
* 5 times
2706 dc33  st   $33,[y,x++]
2707 dc33  st   $33,[y,x++]
2708 dc33  st   $33,[y,x++]
* 12 times
2712 dc00  st   $00,[y,x++]
2713 dc00  st   $00,[y,x++]
2714 dc00  st   $00,[y,x++]
* 6 times
2718 1000  ld   $00,x
2719 1439  ld   $39,y
271a dc00  st   $00,[y,x++]
271b dc00  st   $00,[y,x++]
271c dc00  st   $00,[y,x++]
* 6 times
2720 dc3c  st   $3c,[y,x++]
2721 dc3c  st   $3c,[y,x++]
2722 dc3c  st   $3c,[y,x++]
* 12 times
272c dc00  st   $00,[y,x++]
272d dc00  st   $00,[y,x++]
272e dc00  st   $00,[y,x++]
* 5 times
2731 dc2c  st   $2c,[y,x++]
2732 dc2c  st   $2c,[y,x++]
2733 dc2c  st   $2c,[y,x++]
* 12 times
273d dc00  st   $00,[y,x++]
273e dc00  st   $00,[y,x++]
273f dc00  st   $00,[y,x++]
* 5 times
2742 dc1c  st   $1c,[y,x++]
2743 dc1c  st   $1c,[y,x++]
2744 dc1c  st   $1c,[y,x++]
* 12 times
274e dc00  st   $00,[y,x++]
274f dc00  st   $00,[y,x++]
2750 dc00  st   $00,[y,x++]
* 5 times
2753 dc08  st   $08,[y,x++]
2754 dc08  st   $08,[y,x++]
2755 dc08  st   $08,[y,x++]
* 12 times
275f dc00  st   $00,[y,x++]
2760 dc00  st   $00,[y,x++]
2761 dc00  st   $00,[y,x++]
* 5 times
2764 dc04  st   $04,[y,x++]
2765 dc04  st   $04,[y,x++]
2766 dc04  st   $04,[y,x++]
* 12 times
2770 dc00  st   $00,[y,x++]
2771 dc00  st   $00,[y,x++]
2772 dc00  st   $00,[y,x++]
* 5 times
2775 dc11  st   $11,[y,x++]
2776 dc11  st   $11,[y,x++]
2777 dc11  st   $11,[y,x++]
* 12 times
2781 dc00  st   $00,[y,x++]
2782 dc00  st   $00,[y,x++]
2783 dc00  st   $00,[y,x++]
* 5 times
2786 dc21  st   $21,[y,x++]
2787 dc21  st   $21,[y,x++]
2788 dc21  st   $21,[y,x++]
* 12 times
2792 dc00  st   $00,[y,x++]
2793 dc00  st   $00,[y,x++]
2794 dc00  st   $00,[y,x++]
* 5 times
2797 dc22  st   $22,[y,x++]
2798 dc22  st   $22,[y,x++]
2799 dc22  st   $22,[y,x++]
* 12 times
27a3 dc00  st   $00,[y,x++]
27a4 dc00  st   $00,[y,x++]
27a5 dc00  st   $00,[y,x++]
* 5 times
27a8 dc33  st   $33,[y,x++]
27a9 dc33  st   $33,[y,x++]
27aa dc33  st   $33,[y,x++]
* 12 times
27b4 dc00  st   $00,[y,x++]
27b5 dc00  st   $00,[y,x++]
27b6 dc00  st   $00,[y,x++]
* 6 times
27ba 1000  ld   $00,x
27bb 143a  ld   $3a,y
27bc dc00  st   $00,[y,x++]
27bd dc00  st   $00,[y,x++]
27be dc00  st   $00,[y,x++]
* 6 times
27c2 dc3c  st   $3c,[y,x++]
27c3 dc3c  st   $3c,[y,x++]
27c4 dc3c  st   $3c,[y,x++]
* 12 times
27ce dc00  st   $00,[y,x++]
27cf dc00  st   $00,[y,x++]
27d0 dc00  st   $00,[y,x++]
* 5 times
27d3 dc2c  st   $2c,[y,x++]
27d4 dc2c  st   $2c,[y,x++]
27d5 dc2c  st   $2c,[y,x++]
* 12 times
27df dc00  st   $00,[y,x++]
27e0 dc00  st   $00,[y,x++]
27e1 dc00  st   $00,[y,x++]
* 5 times
27e4 dc1c  st   $1c,[y,x++]
27e5 dc1c  st   $1c,[y,x++]
27e6 dc1c  st   $1c,[y,x++]
* 12 times
27f0 dc00  st   $00,[y,x++]
27f1 dc00  st   $00,[y,x++]
27f2 dc00  st   $00,[y,x++]
* 5 times
27f5 dc08  st   $08,[y,x++]
27f6 dc08  st   $08,[y,x++]
27f7 dc08  st   $08,[y,x++]
* 12 times
2801 dc00  st   $00,[y,x++]
2802 dc00  st   $00,[y,x++]
2803 dc00  st   $00,[y,x++]
* 5 times
2806 dc04  st   $04,[y,x++]
2807 dc04  st   $04,[y,x++]
2808 dc04  st   $04,[y,x++]
* 12 times
2812 dc00  st   $00,[y,x++]
2813 dc00  st   $00,[y,x++]
2814 dc00  st   $00,[y,x++]
* 5 times
2817 dc11  st   $11,[y,x++]
2818 dc11  st   $11,[y,x++]
2819 dc11  st   $11,[y,x++]
* 12 times
2823 dc00  st   $00,[y,x++]
2824 dc00  st   $00,[y,x++]
2825 dc00  st   $00,[y,x++]
* 5 times
2828 dc21  st   $21,[y,x++]
2829 dc21  st   $21,[y,x++]
282a dc21  st   $21,[y,x++]
* 12 times
2834 dc00  st   $00,[y,x++]
2835 dc00  st   $00,[y,x++]
2836 dc00  st   $00,[y,x++]
* 5 times
2839 dc22  st   $22,[y,x++]
283a dc22  st   $22,[y,x++]
283b dc22  st   $22,[y,x++]
* 12 times
2845 dc00  st   $00,[y,x++]
2846 dc00  st   $00,[y,x++]
2847 dc00  st   $00,[y,x++]
* 5 times
284a dc33  st   $33,[y,x++]
284b dc33  st   $33,[y,x++]
284c dc33  st   $33,[y,x++]
* 12 times
2856 dc00  st   $00,[y,x++]
2857 dc00  st   $00,[y,x++]
2858 dc00  st   $00,[y,x++]
* 6 times
285c 1000  ld   $00,x
285d 143b  ld   $3b,y
285e dc00  st   $00,[y,x++]
285f dc00  st   $00,[y,x++]
2860 dc00  st   $00,[y,x++]
* 6 times
2864 dc3c  st   $3c,[y,x++]
2865 dc3c  st   $3c,[y,x++]
2866 dc3c  st   $3c,[y,x++]
* 12 times
2870 dc00  st   $00,[y,x++]
2871 dc00  st   $00,[y,x++]
2872 dc00  st   $00,[y,x++]
* 5 times
2875 dc2c  st   $2c,[y,x++]
2876 dc2c  st   $2c,[y,x++]
2877 dc2c  st   $2c,[y,x++]
* 12 times
2881 dc00  st   $00,[y,x++]
2882 dc00  st   $00,[y,x++]
2883 dc00  st   $00,[y,x++]
* 5 times
2886 dc1c  st   $1c,[y,x++]
2887 dc1c  st   $1c,[y,x++]
2888 dc1c  st   $1c,[y,x++]
* 12 times
2892 dc00  st   $00,[y,x++]
2893 dc00  st   $00,[y,x++]
2894 dc00  st   $00,[y,x++]
* 5 times
2897 dc08  st   $08,[y,x++]
2898 dc08  st   $08,[y,x++]
2899 dc08  st   $08,[y,x++]
* 12 times
28a3 dc00  st   $00,[y,x++]
28a4 dc00  st   $00,[y,x++]
28a5 dc00  st   $00,[y,x++]
* 5 times
28a8 dc04  st   $04,[y,x++]
28a9 dc04  st   $04,[y,x++]
28aa dc04  st   $04,[y,x++]
* 12 times
28b4 dc00  st   $00,[y,x++]
28b5 dc00  st   $00,[y,x++]
28b6 dc00  st   $00,[y,x++]
* 5 times
28b9 dc11  st   $11,[y,x++]
28ba dc11  st   $11,[y,x++]
28bb dc11  st   $11,[y,x++]
* 12 times
28c5 dc00  st   $00,[y,x++]
28c6 dc00  st   $00,[y,x++]
28c7 dc00  st   $00,[y,x++]
* 5 times
28ca dc21  st   $21,[y,x++]
28cb dc21  st   $21,[y,x++]
28cc dc21  st   $21,[y,x++]
* 12 times
28d6 dc00  st   $00,[y,x++]
28d7 dc00  st   $00,[y,x++]
28d8 dc00  st   $00,[y,x++]
* 5 times
28db dc22  st   $22,[y,x++]
28dc dc22  st   $22,[y,x++]
28dd dc22  st   $22,[y,x++]
* 12 times
28e7 dc00  st   $00,[y,x++]
28e8 dc00  st   $00,[y,x++]
28e9 dc00  st   $00,[y,x++]
* 5 times
28ec dc33  st   $33,[y,x++]
28ed dc33  st   $33,[y,x++]
28ee dc33  st   $33,[y,x++]
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
* 160 times
29a0 1000  ld   $00,x
29a1 143d  ld   $3d,y
29a2 dc00  st   $00,[y,x++]
29a3 dc00  st   $00,[y,x++]
29a4 dc00  st   $00,[y,x++]
* 160 times
2a42 1000  ld   $00,x
2a43 143e  ld   $3e,y
2a44 dc00  st   $00,[y,x++]
2a45 dc00  st   $00,[y,x++]
2a46 dc00  st   $00,[y,x++]
* 160 times
2ae4 1000  ld   $00,x
2ae5 143f  ld   $3f,y
2ae6 dc00  st   $00,[y,x++]
2ae7 dc00  st   $00,[y,x++]
2ae8 dc00  st   $00,[y,x++]
* 6 times
2aec dc38  st   $38,[y,x++]
2aed dc38  st   $38,[y,x++]
2aee dc38  st   $38,[y,x++]
* 12 times
2af8 dc00  st   $00,[y,x++]
2af9 dc00  st   $00,[y,x++]
2afa dc00  st   $00,[y,x++]
* 5 times
2afd dc28  st   $28,[y,x++]
2afe dc28  st   $28,[y,x++]
2aff dc28  st   $28,[y,x++]
* 12 times
2b09 dc00  st   $00,[y,x++]
2b0a dc00  st   $00,[y,x++]
2b0b dc00  st   $00,[y,x++]
* 5 times
2b0e dc18  st   $18,[y,x++]
2b0f dc18  st   $18,[y,x++]
2b10 dc18  st   $18,[y,x++]
* 12 times
2b1a dc00  st   $00,[y,x++]
2b1b dc00  st   $00,[y,x++]
2b1c dc00  st   $00,[y,x++]
* 5 times
2b1f dc14  st   $14,[y,x++]
2b20 dc14  st   $14,[y,x++]
2b21 dc14  st   $14,[y,x++]
* 12 times
2b2b dc00  st   $00,[y,x++]
2b2c dc00  st   $00,[y,x++]
2b2d dc00  st   $00,[y,x++]
* 5 times
2b30 dc10  st   $10,[y,x++]
2b31 dc10  st   $10,[y,x++]
2b32 dc10  st   $10,[y,x++]
* 12 times
2b3c dc00  st   $00,[y,x++]
2b3d dc00  st   $00,[y,x++]
2b3e dc00  st   $00,[y,x++]
* 5 times
2b41 dc20  st   $20,[y,x++]
2b42 dc20  st   $20,[y,x++]
2b43 dc20  st   $20,[y,x++]
* 12 times
2b4d dc00  st   $00,[y,x++]
2b4e dc00  st   $00,[y,x++]
2b4f dc00  st   $00,[y,x++]
* 5 times
2b52 dc31  st   $31,[y,x++]
2b53 dc31  st   $31,[y,x++]
2b54 dc31  st   $31,[y,x++]
* 12 times
2b5e dc00  st   $00,[y,x++]
2b5f dc00  st   $00,[y,x++]
2b60 dc00  st   $00,[y,x++]
* 5 times
2b63 dc32  st   $32,[y,x++]
2b64 dc32  st   $32,[y,x++]
2b65 dc32  st   $32,[y,x++]
* 12 times
2b6f dc00  st   $00,[y,x++]
2b70 dc00  st   $00,[y,x++]
2b71 dc00  st   $00,[y,x++]
* 5 times
2b74 dc37  st   $37,[y,x++]
2b75 dc37  st   $37,[y,x++]
2b76 dc37  st   $37,[y,x++]
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
* 6 times
2b8e dc38  st   $38,[y,x++]
2b8f dc38  st   $38,[y,x++]
2b90 dc38  st   $38,[y,x++]
* 12 times
2b9a dc00  st   $00,[y,x++]
2b9b dc00  st   $00,[y,x++]
2b9c dc00  st   $00,[y,x++]
* 5 times
2b9f dc28  st   $28,[y,x++]
2ba0 dc28  st   $28,[y,x++]
2ba1 dc28  st   $28,[y,x++]
* 12 times
2bab dc00  st   $00,[y,x++]
2bac dc00  st   $00,[y,x++]
2bad dc00  st   $00,[y,x++]
* 5 times
2bb0 dc18  st   $18,[y,x++]
2bb1 dc18  st   $18,[y,x++]
2bb2 dc18  st   $18,[y,x++]
* 12 times
2bbc dc00  st   $00,[y,x++]
2bbd dc00  st   $00,[y,x++]
2bbe dc00  st   $00,[y,x++]
* 5 times
2bc1 dc14  st   $14,[y,x++]
2bc2 dc14  st   $14,[y,x++]
2bc3 dc14  st   $14,[y,x++]
* 12 times
2bcd dc00  st   $00,[y,x++]
2bce dc00  st   $00,[y,x++]
2bcf dc00  st   $00,[y,x++]
* 5 times
2bd2 dc10  st   $10,[y,x++]
2bd3 dc10  st   $10,[y,x++]
2bd4 dc10  st   $10,[y,x++]
* 12 times
2bde dc00  st   $00,[y,x++]
2bdf dc00  st   $00,[y,x++]
2be0 dc00  st   $00,[y,x++]
* 5 times
2be3 dc20  st   $20,[y,x++]
2be4 dc20  st   $20,[y,x++]
2be5 dc20  st   $20,[y,x++]
* 12 times
2bef dc00  st   $00,[y,x++]
2bf0 dc00  st   $00,[y,x++]
2bf1 dc00  st   $00,[y,x++]
* 5 times
2bf4 dc31  st   $31,[y,x++]
2bf5 dc31  st   $31,[y,x++]
2bf6 dc31  st   $31,[y,x++]
* 12 times
2c00 dc00  st   $00,[y,x++]
2c01 dc00  st   $00,[y,x++]
2c02 dc00  st   $00,[y,x++]
* 5 times
2c05 dc32  st   $32,[y,x++]
2c06 dc32  st   $32,[y,x++]
2c07 dc32  st   $32,[y,x++]
* 12 times
2c11 dc00  st   $00,[y,x++]
2c12 dc00  st   $00,[y,x++]
2c13 dc00  st   $00,[y,x++]
* 5 times
2c16 dc37  st   $37,[y,x++]
2c17 dc37  st   $37,[y,x++]
2c18 dc37  st   $37,[y,x++]
* 12 times
2c22 dc00  st   $00,[y,x++]
2c23 dc00  st   $00,[y,x++]
2c24 dc00  st   $00,[y,x++]
* 6 times
2c28 1000  ld   $00,x
2c29 1441  ld   $41,y
2c2a dc00  st   $00,[y,x++]
2c2b dc00  st   $00,[y,x++]
2c2c dc00  st   $00,[y,x++]
* 6 times
2c30 dc38  st   $38,[y,x++]
2c31 dc38  st   $38,[y,x++]
2c32 dc38  st   $38,[y,x++]
* 12 times
2c3c dc00  st   $00,[y,x++]
2c3d dc00  st   $00,[y,x++]
2c3e dc00  st   $00,[y,x++]
* 5 times
2c41 dc28  st   $28,[y,x++]
2c42 dc28  st   $28,[y,x++]
2c43 dc28  st   $28,[y,x++]
* 12 times
2c4d dc00  st   $00,[y,x++]
2c4e dc00  st   $00,[y,x++]
2c4f dc00  st   $00,[y,x++]
* 5 times
2c52 dc18  st   $18,[y,x++]
2c53 dc18  st   $18,[y,x++]
2c54 dc18  st   $18,[y,x++]
* 12 times
2c5e dc00  st   $00,[y,x++]
2c5f dc00  st   $00,[y,x++]
2c60 dc00  st   $00,[y,x++]
* 5 times
2c63 dc14  st   $14,[y,x++]
2c64 dc14  st   $14,[y,x++]
2c65 dc14  st   $14,[y,x++]
* 12 times
2c6f dc00  st   $00,[y,x++]
2c70 dc00  st   $00,[y,x++]
2c71 dc00  st   $00,[y,x++]
* 5 times
2c74 dc10  st   $10,[y,x++]
2c75 dc10  st   $10,[y,x++]
2c76 dc10  st   $10,[y,x++]
* 12 times
2c80 dc00  st   $00,[y,x++]
2c81 dc00  st   $00,[y,x++]
2c82 dc00  st   $00,[y,x++]
* 5 times
2c85 dc20  st   $20,[y,x++]
2c86 dc20  st   $20,[y,x++]
2c87 dc20  st   $20,[y,x++]
* 12 times
2c91 dc00  st   $00,[y,x++]
2c92 dc00  st   $00,[y,x++]
2c93 dc00  st   $00,[y,x++]
* 5 times
2c96 dc31  st   $31,[y,x++]
2c97 dc31  st   $31,[y,x++]
2c98 dc31  st   $31,[y,x++]
* 12 times
2ca2 dc00  st   $00,[y,x++]
2ca3 dc00  st   $00,[y,x++]
2ca4 dc00  st   $00,[y,x++]
* 5 times
2ca7 dc32  st   $32,[y,x++]
2ca8 dc32  st   $32,[y,x++]
2ca9 dc32  st   $32,[y,x++]
* 12 times
2cb3 dc00  st   $00,[y,x++]
2cb4 dc00  st   $00,[y,x++]
2cb5 dc00  st   $00,[y,x++]
* 5 times
2cb8 dc37  st   $37,[y,x++]
2cb9 dc37  st   $37,[y,x++]
2cba dc37  st   $37,[y,x++]
* 12 times
2cc4 dc00  st   $00,[y,x++]
2cc5 dc00  st   $00,[y,x++]
2cc6 dc00  st   $00,[y,x++]
* 6 times
2cca 1000  ld   $00,x
2ccb 1442  ld   $42,y
2ccc dc00  st   $00,[y,x++]
2ccd dc00  st   $00,[y,x++]
2cce dc00  st   $00,[y,x++]
* 6 times
2cd2 dc38  st   $38,[y,x++]
2cd3 dc38  st   $38,[y,x++]
2cd4 dc38  st   $38,[y,x++]
* 12 times
2cde dc00  st   $00,[y,x++]
2cdf dc00  st   $00,[y,x++]
2ce0 dc00  st   $00,[y,x++]
* 5 times
2ce3 dc28  st   $28,[y,x++]
2ce4 dc28  st   $28,[y,x++]
2ce5 dc28  st   $28,[y,x++]
* 12 times
2cef dc00  st   $00,[y,x++]
2cf0 dc00  st   $00,[y,x++]
2cf1 dc00  st   $00,[y,x++]
* 5 times
2cf4 dc18  st   $18,[y,x++]
2cf5 dc18  st   $18,[y,x++]
2cf6 dc18  st   $18,[y,x++]
* 12 times
2d00 dc00  st   $00,[y,x++]
2d01 dc00  st   $00,[y,x++]
2d02 dc00  st   $00,[y,x++]
* 5 times
2d05 dc14  st   $14,[y,x++]
2d06 dc14  st   $14,[y,x++]
2d07 dc14  st   $14,[y,x++]
* 12 times
2d11 dc00  st   $00,[y,x++]
2d12 dc00  st   $00,[y,x++]
2d13 dc00  st   $00,[y,x++]
* 5 times
2d16 dc10  st   $10,[y,x++]
2d17 dc10  st   $10,[y,x++]
2d18 dc10  st   $10,[y,x++]
* 12 times
2d22 dc00  st   $00,[y,x++]
2d23 dc00  st   $00,[y,x++]
2d24 dc00  st   $00,[y,x++]
* 5 times
2d27 dc20  st   $20,[y,x++]
2d28 dc20  st   $20,[y,x++]
2d29 dc20  st   $20,[y,x++]
* 12 times
2d33 dc00  st   $00,[y,x++]
2d34 dc00  st   $00,[y,x++]
2d35 dc00  st   $00,[y,x++]
* 5 times
2d38 dc31  st   $31,[y,x++]
2d39 dc31  st   $31,[y,x++]
2d3a dc31  st   $31,[y,x++]
* 12 times
2d44 dc00  st   $00,[y,x++]
2d45 dc00  st   $00,[y,x++]
2d46 dc00  st   $00,[y,x++]
* 5 times
2d49 dc32  st   $32,[y,x++]
2d4a dc32  st   $32,[y,x++]
2d4b dc32  st   $32,[y,x++]
* 12 times
2d55 dc00  st   $00,[y,x++]
2d56 dc00  st   $00,[y,x++]
2d57 dc00  st   $00,[y,x++]
* 5 times
2d5a dc37  st   $37,[y,x++]
2d5b dc37  st   $37,[y,x++]
2d5c dc37  st   $37,[y,x++]
* 12 times
2d66 dc00  st   $00,[y,x++]
2d67 dc00  st   $00,[y,x++]
2d68 dc00  st   $00,[y,x++]
* 6 times
2d6c 1000  ld   $00,x
2d6d 1443  ld   $43,y
2d6e dc00  st   $00,[y,x++]
2d6f dc00  st   $00,[y,x++]
2d70 dc00  st   $00,[y,x++]
* 6 times
2d74 dc38  st   $38,[y,x++]
2d75 dc38  st   $38,[y,x++]
2d76 dc38  st   $38,[y,x++]
* 12 times
2d80 dc00  st   $00,[y,x++]
2d81 dc00  st   $00,[y,x++]
2d82 dc00  st   $00,[y,x++]
* 5 times
2d85 dc28  st   $28,[y,x++]
2d86 dc28  st   $28,[y,x++]
2d87 dc28  st   $28,[y,x++]
* 12 times
2d91 dc00  st   $00,[y,x++]
2d92 dc00  st   $00,[y,x++]
2d93 dc00  st   $00,[y,x++]
* 5 times
2d96 dc18  st   $18,[y,x++]
2d97 dc18  st   $18,[y,x++]
2d98 dc18  st   $18,[y,x++]
* 12 times
2da2 dc00  st   $00,[y,x++]
2da3 dc00  st   $00,[y,x++]
2da4 dc00  st   $00,[y,x++]
* 5 times
2da7 dc14  st   $14,[y,x++]
2da8 dc14  st   $14,[y,x++]
2da9 dc14  st   $14,[y,x++]
* 12 times
2db3 dc00  st   $00,[y,x++]
2db4 dc00  st   $00,[y,x++]
2db5 dc00  st   $00,[y,x++]
* 5 times
2db8 dc10  st   $10,[y,x++]
2db9 dc10  st   $10,[y,x++]
2dba dc10  st   $10,[y,x++]
* 12 times
2dc4 dc00  st   $00,[y,x++]
2dc5 dc00  st   $00,[y,x++]
2dc6 dc00  st   $00,[y,x++]
* 5 times
2dc9 dc20  st   $20,[y,x++]
2dca dc20  st   $20,[y,x++]
2dcb dc20  st   $20,[y,x++]
* 12 times
2dd5 dc00  st   $00,[y,x++]
2dd6 dc00  st   $00,[y,x++]
2dd7 dc00  st   $00,[y,x++]
* 5 times
2dda dc31  st   $31,[y,x++]
2ddb dc31  st   $31,[y,x++]
2ddc dc31  st   $31,[y,x++]
* 12 times
2de6 dc00  st   $00,[y,x++]
2de7 dc00  st   $00,[y,x++]
2de8 dc00  st   $00,[y,x++]
* 5 times
2deb dc32  st   $32,[y,x++]
2dec dc32  st   $32,[y,x++]
2ded dc32  st   $32,[y,x++]
* 12 times
2df7 dc00  st   $00,[y,x++]
2df8 dc00  st   $00,[y,x++]
2df9 dc00  st   $00,[y,x++]
* 5 times
2dfc dc37  st   $37,[y,x++]
2dfd dc37  st   $37,[y,x++]
2dfe dc37  st   $37,[y,x++]
* 12 times
2e08 dc00  st   $00,[y,x++]
2e09 dc00  st   $00,[y,x++]
2e0a dc00  st   $00,[y,x++]
* 6 times
2e0e 1000  ld   $00,x
2e0f 1444  ld   $44,y
2e10 dc00  st   $00,[y,x++]
2e11 dc00  st   $00,[y,x++]
2e12 dc00  st   $00,[y,x++]
* 160 times
2eb0 1000  ld   $00,x
2eb1 1445  ld   $45,y
2eb2 dc00  st   $00,[y,x++]
2eb3 dc00  st   $00,[y,x++]
2eb4 dc00  st   $00,[y,x++]
* 160 times
2f52 1000  ld   $00,x
2f53 1446  ld   $46,y
2f54 dc00  st   $00,[y,x++]
2f55 dc00  st   $00,[y,x++]
2f56 dc00  st   $00,[y,x++]
* 160 times
2ff4 1000  ld   $00,x
2ff5 1447  ld   $47,y
2ff6 dc00  st   $00,[y,x++]
2ff7 dc00  st   $00,[y,x++]
2ff8 dc00  st   $00,[y,x++]
* 6 times
2ffc dc15  st   $15,[y,x++]
2ffd dc15  st   $15,[y,x++]
2ffe dc15  st   $15,[y,x++]
* 12 times
3008 dc00  st   $00,[y,x++]
3009 dc00  st   $00,[y,x++]
300a dc00  st   $00,[y,x++]
* 5 times
300d dc19  st   $19,[y,x++]
300e dc19  st   $19,[y,x++]
300f dc19  st   $19,[y,x++]
* 12 times
3019 dc00  st   $00,[y,x++]
301a dc00  st   $00,[y,x++]
301b dc00  st   $00,[y,x++]
* 5 times
301e dc29  st   $29,[y,x++]
301f dc29  st   $29,[y,x++]
3020 dc29  st   $29,[y,x++]
* 12 times
302a dc00  st   $00,[y,x++]
302b dc00  st   $00,[y,x++]
302c dc00  st   $00,[y,x++]
* 5 times
302f dc24  st   $24,[y,x++]
3030 dc24  st   $24,[y,x++]
3031 dc24  st   $24,[y,x++]
* 12 times
303b dc00  st   $00,[y,x++]
303c dc00  st   $00,[y,x++]
303d dc00  st   $00,[y,x++]
* 5 times
3040 dc30  st   $30,[y,x++]
3041 dc30  st   $30,[y,x++]
3042 dc30  st   $30,[y,x++]
* 12 times
304c dc00  st   $00,[y,x++]
304d dc00  st   $00,[y,x++]
304e dc00  st   $00,[y,x++]
* 5 times
3051 dc35  st   $35,[y,x++]
3052 dc35  st   $35,[y,x++]
3053 dc35  st   $35,[y,x++]
* 12 times
305d dc00  st   $00,[y,x++]
305e dc00  st   $00,[y,x++]
305f dc00  st   $00,[y,x++]
* 5 times
3062 dc36  st   $36,[y,x++]
3063 dc36  st   $36,[y,x++]
3064 dc36  st   $36,[y,x++]
* 12 times
306e dc00  st   $00,[y,x++]
306f dc00  st   $00,[y,x++]
3070 dc00  st   $00,[y,x++]
* 5 times
3073 dc26  st   $26,[y,x++]
3074 dc26  st   $26,[y,x++]
3075 dc26  st   $26,[y,x++]
* 12 times
307f dc00  st   $00,[y,x++]
3080 dc00  st   $00,[y,x++]
3081 dc00  st   $00,[y,x++]
* 5 times
3084 dc27  st   $27,[y,x++]
3085 dc27  st   $27,[y,x++]
3086 dc27  st   $27,[y,x++]
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
309e dc15  st   $15,[y,x++]
309f dc15  st   $15,[y,x++]
30a0 dc15  st   $15,[y,x++]
* 12 times
30aa dc00  st   $00,[y,x++]
30ab dc00  st   $00,[y,x++]
30ac dc00  st   $00,[y,x++]
* 5 times
30af dc19  st   $19,[y,x++]
30b0 dc19  st   $19,[y,x++]
30b1 dc19  st   $19,[y,x++]
* 12 times
30bb dc00  st   $00,[y,x++]
30bc dc00  st   $00,[y,x++]
30bd dc00  st   $00,[y,x++]
* 5 times
30c0 dc29  st   $29,[y,x++]
30c1 dc29  st   $29,[y,x++]
30c2 dc29  st   $29,[y,x++]
* 12 times
30cc dc00  st   $00,[y,x++]
30cd dc00  st   $00,[y,x++]
30ce dc00  st   $00,[y,x++]
* 5 times
30d1 dc24  st   $24,[y,x++]
30d2 dc24  st   $24,[y,x++]
30d3 dc24  st   $24,[y,x++]
* 12 times
30dd dc00  st   $00,[y,x++]
30de dc00  st   $00,[y,x++]
30df dc00  st   $00,[y,x++]
* 5 times
30e2 dc30  st   $30,[y,x++]
30e3 dc30  st   $30,[y,x++]
30e4 dc30  st   $30,[y,x++]
* 12 times
30ee dc00  st   $00,[y,x++]
30ef dc00  st   $00,[y,x++]
30f0 dc00  st   $00,[y,x++]
* 5 times
30f3 dc35  st   $35,[y,x++]
30f4 dc35  st   $35,[y,x++]
30f5 dc35  st   $35,[y,x++]
* 12 times
30ff dc00  st   $00,[y,x++]
3100 dc00  st   $00,[y,x++]
3101 dc00  st   $00,[y,x++]
* 5 times
3104 dc36  st   $36,[y,x++]
3105 dc36  st   $36,[y,x++]
3106 dc36  st   $36,[y,x++]
* 12 times
3110 dc00  st   $00,[y,x++]
3111 dc00  st   $00,[y,x++]
3112 dc00  st   $00,[y,x++]
* 5 times
3115 dc26  st   $26,[y,x++]
3116 dc26  st   $26,[y,x++]
3117 dc26  st   $26,[y,x++]
* 12 times
3121 dc00  st   $00,[y,x++]
3122 dc00  st   $00,[y,x++]
3123 dc00  st   $00,[y,x++]
* 5 times
3126 dc27  st   $27,[y,x++]
3127 dc27  st   $27,[y,x++]
3128 dc27  st   $27,[y,x++]
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
* 6 times
3140 dc15  st   $15,[y,x++]
3141 dc15  st   $15,[y,x++]
3142 dc15  st   $15,[y,x++]
* 12 times
314c dc00  st   $00,[y,x++]
314d dc00  st   $00,[y,x++]
314e dc00  st   $00,[y,x++]
* 5 times
3151 dc19  st   $19,[y,x++]
3152 dc19  st   $19,[y,x++]
3153 dc19  st   $19,[y,x++]
* 12 times
315d dc00  st   $00,[y,x++]
315e dc00  st   $00,[y,x++]
315f dc00  st   $00,[y,x++]
* 5 times
3162 dc29  st   $29,[y,x++]
3163 dc29  st   $29,[y,x++]
3164 dc29  st   $29,[y,x++]
* 12 times
316e dc00  st   $00,[y,x++]
316f dc00  st   $00,[y,x++]
3170 dc00  st   $00,[y,x++]
* 5 times
3173 dc24  st   $24,[y,x++]
3174 dc24  st   $24,[y,x++]
3175 dc24  st   $24,[y,x++]
* 12 times
317f dc00  st   $00,[y,x++]
3180 dc00  st   $00,[y,x++]
3181 dc00  st   $00,[y,x++]
* 5 times
3184 dc30  st   $30,[y,x++]
3185 dc30  st   $30,[y,x++]
3186 dc30  st   $30,[y,x++]
* 12 times
3190 dc00  st   $00,[y,x++]
3191 dc00  st   $00,[y,x++]
3192 dc00  st   $00,[y,x++]
* 5 times
3195 dc35  st   $35,[y,x++]
3196 dc35  st   $35,[y,x++]
3197 dc35  st   $35,[y,x++]
* 12 times
31a1 dc00  st   $00,[y,x++]
31a2 dc00  st   $00,[y,x++]
31a3 dc00  st   $00,[y,x++]
* 5 times
31a6 dc36  st   $36,[y,x++]
31a7 dc36  st   $36,[y,x++]
31a8 dc36  st   $36,[y,x++]
* 12 times
31b2 dc00  st   $00,[y,x++]
31b3 dc00  st   $00,[y,x++]
31b4 dc00  st   $00,[y,x++]
* 5 times
31b7 dc26  st   $26,[y,x++]
31b8 dc26  st   $26,[y,x++]
31b9 dc26  st   $26,[y,x++]
* 12 times
31c3 dc00  st   $00,[y,x++]
31c4 dc00  st   $00,[y,x++]
31c5 dc00  st   $00,[y,x++]
* 5 times
31c8 dc27  st   $27,[y,x++]
31c9 dc27  st   $27,[y,x++]
31ca dc27  st   $27,[y,x++]
* 12 times
31d4 dc00  st   $00,[y,x++]
31d5 dc00  st   $00,[y,x++]
31d6 dc00  st   $00,[y,x++]
* 6 times
31da 1000  ld   $00,x
31db 144a  ld   $4a,y
31dc dc00  st   $00,[y,x++]
31dd dc00  st   $00,[y,x++]
31de dc00  st   $00,[y,x++]
* 6 times
31e2 dc15  st   $15,[y,x++]
31e3 dc15  st   $15,[y,x++]
31e4 dc15  st   $15,[y,x++]
* 12 times
31ee dc00  st   $00,[y,x++]
31ef dc00  st   $00,[y,x++]
31f0 dc00  st   $00,[y,x++]
* 5 times
31f3 dc19  st   $19,[y,x++]
31f4 dc19  st   $19,[y,x++]
31f5 dc19  st   $19,[y,x++]
* 12 times
31ff dc00  st   $00,[y,x++]
3200 dc00  st   $00,[y,x++]
3201 dc00  st   $00,[y,x++]
* 5 times
3204 dc29  st   $29,[y,x++]
3205 dc29  st   $29,[y,x++]
3206 dc29  st   $29,[y,x++]
* 12 times
3210 dc00  st   $00,[y,x++]
3211 dc00  st   $00,[y,x++]
3212 dc00  st   $00,[y,x++]
* 5 times
3215 dc24  st   $24,[y,x++]
3216 dc24  st   $24,[y,x++]
3217 dc24  st   $24,[y,x++]
* 12 times
3221 dc00  st   $00,[y,x++]
3222 dc00  st   $00,[y,x++]
3223 dc00  st   $00,[y,x++]
* 5 times
3226 dc30  st   $30,[y,x++]
3227 dc30  st   $30,[y,x++]
3228 dc30  st   $30,[y,x++]
* 12 times
3232 dc00  st   $00,[y,x++]
3233 dc00  st   $00,[y,x++]
3234 dc00  st   $00,[y,x++]
* 5 times
3237 dc35  st   $35,[y,x++]
3238 dc35  st   $35,[y,x++]
3239 dc35  st   $35,[y,x++]
* 12 times
3243 dc00  st   $00,[y,x++]
3244 dc00  st   $00,[y,x++]
3245 dc00  st   $00,[y,x++]
* 5 times
3248 dc36  st   $36,[y,x++]
3249 dc36  st   $36,[y,x++]
324a dc36  st   $36,[y,x++]
* 12 times
3254 dc00  st   $00,[y,x++]
3255 dc00  st   $00,[y,x++]
3256 dc00  st   $00,[y,x++]
* 5 times
3259 dc26  st   $26,[y,x++]
325a dc26  st   $26,[y,x++]
325b dc26  st   $26,[y,x++]
* 12 times
3265 dc00  st   $00,[y,x++]
3266 dc00  st   $00,[y,x++]
3267 dc00  st   $00,[y,x++]
* 5 times
326a dc27  st   $27,[y,x++]
326b dc27  st   $27,[y,x++]
326c dc27  st   $27,[y,x++]
* 12 times
3276 dc00  st   $00,[y,x++]
3277 dc00  st   $00,[y,x++]
3278 dc00  st   $00,[y,x++]
* 6 times
327c 1000  ld   $00,x
327d 144b  ld   $4b,y
327e dc00  st   $00,[y,x++]
327f dc00  st   $00,[y,x++]
3280 dc00  st   $00,[y,x++]
* 6 times
3284 dc15  st   $15,[y,x++]
3285 dc15  st   $15,[y,x++]
3286 dc15  st   $15,[y,x++]
* 12 times
3290 dc00  st   $00,[y,x++]
3291 dc00  st   $00,[y,x++]
3292 dc00  st   $00,[y,x++]
* 5 times
3295 dc19  st   $19,[y,x++]
3296 dc19  st   $19,[y,x++]
3297 dc19  st   $19,[y,x++]
* 12 times
32a1 dc00  st   $00,[y,x++]
32a2 dc00  st   $00,[y,x++]
32a3 dc00  st   $00,[y,x++]
* 5 times
32a6 dc29  st   $29,[y,x++]
32a7 dc29  st   $29,[y,x++]
32a8 dc29  st   $29,[y,x++]
* 12 times
32b2 dc00  st   $00,[y,x++]
32b3 dc00  st   $00,[y,x++]
32b4 dc00  st   $00,[y,x++]
* 5 times
32b7 dc24  st   $24,[y,x++]
32b8 dc24  st   $24,[y,x++]
32b9 dc24  st   $24,[y,x++]
* 12 times
32c3 dc00  st   $00,[y,x++]
32c4 dc00  st   $00,[y,x++]
32c5 dc00  st   $00,[y,x++]
* 5 times
32c8 dc30  st   $30,[y,x++]
32c9 dc30  st   $30,[y,x++]
32ca dc30  st   $30,[y,x++]
* 12 times
32d4 dc00  st   $00,[y,x++]
32d5 dc00  st   $00,[y,x++]
32d6 dc00  st   $00,[y,x++]
* 5 times
32d9 dc35  st   $35,[y,x++]
32da dc35  st   $35,[y,x++]
32db dc35  st   $35,[y,x++]
* 12 times
32e5 dc00  st   $00,[y,x++]
32e6 dc00  st   $00,[y,x++]
32e7 dc00  st   $00,[y,x++]
* 5 times
32ea dc36  st   $36,[y,x++]
32eb dc36  st   $36,[y,x++]
32ec dc36  st   $36,[y,x++]
* 12 times
32f6 dc00  st   $00,[y,x++]
32f7 dc00  st   $00,[y,x++]
32f8 dc00  st   $00,[y,x++]
* 5 times
32fb dc26  st   $26,[y,x++]
32fc dc26  st   $26,[y,x++]
32fd dc26  st   $26,[y,x++]
* 12 times
3307 dc00  st   $00,[y,x++]
3308 dc00  st   $00,[y,x++]
3309 dc00  st   $00,[y,x++]
* 5 times
330c dc27  st   $27,[y,x++]
330d dc27  st   $27,[y,x++]
330e dc27  st   $27,[y,x++]
* 12 times
3318 dc00  st   $00,[y,x++]
3319 dc00  st   $00,[y,x++]
331a dc00  st   $00,[y,x++]
* 6 times
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
* 160 times
3462 1000  ld   $00,x
3463 144e  ld   $4e,y
3464 dc00  st   $00,[y,x++]
3465 dc00  st   $00,[y,x++]
3466 dc00  st   $00,[y,x++]
* 160 times
3504 1000  ld   $00,x
3505 144f  ld   $4f,y
3506 dc00  st   $00,[y,x++]
3507 dc00  st   $00,[y,x++]
3508 dc00  st   $00,[y,x++]
* 6 times
350c dc16  st   $16,[y,x++]
350d dc16  st   $16,[y,x++]
350e dc16  st   $16,[y,x++]
* 12 times
3518 dc00  st   $00,[y,x++]
3519 dc00  st   $00,[y,x++]
351a dc00  st   $00,[y,x++]
* 5 times
351d dc1a  st   $1a,[y,x++]
351e dc1a  st   $1a,[y,x++]
351f dc1a  st   $1a,[y,x++]
* 12 times
3529 dc00  st   $00,[y,x++]
352a dc00  st   $00,[y,x++]
352b dc00  st   $00,[y,x++]
* 5 times
352e dc2a  st   $2a,[y,x++]
352f dc2a  st   $2a,[y,x++]
3530 dc2a  st   $2a,[y,x++]
* 12 times
353a dc00  st   $00,[y,x++]
353b dc00  st   $00,[y,x++]
353c dc00  st   $00,[y,x++]
* 5 times
353f dc25  st   $25,[y,x++]
3540 dc25  st   $25,[y,x++]
3541 dc25  st   $25,[y,x++]
* 12 times
354b dc00  st   $00,[y,x++]
354c dc00  st   $00,[y,x++]
354d dc00  st   $00,[y,x++]
* 5 times
3550 dc34  st   $34,[y,x++]
3551 dc34  st   $34,[y,x++]
3552 dc34  st   $34,[y,x++]
* 12 times
355c dc00  st   $00,[y,x++]
355d dc00  st   $00,[y,x++]
355e dc00  st   $00,[y,x++]
* 5 times
3561 dc39  st   $39,[y,x++]
3562 dc39  st   $39,[y,x++]
3563 dc39  st   $39,[y,x++]
* 12 times
356d dc00  st   $00,[y,x++]
356e dc00  st   $00,[y,x++]
356f dc00  st   $00,[y,x++]
* 5 times
3572 dc3a  st   $3a,[y,x++]
3573 dc3a  st   $3a,[y,x++]
3574 dc3a  st   $3a,[y,x++]
* 12 times
357e dc00  st   $00,[y,x++]
357f dc00  st   $00,[y,x++]
3580 dc00  st   $00,[y,x++]
* 5 times
3583 dc3b  st   $3b,[y,x++]
3584 dc3b  st   $3b,[y,x++]
3585 dc3b  st   $3b,[y,x++]
* 12 times
358f dc00  st   $00,[y,x++]
3590 dc00  st   $00,[y,x++]
3591 dc00  st   $00,[y,x++]
* 5 times
3594 dc2b  st   $2b,[y,x++]
3595 dc2b  st   $2b,[y,x++]
3596 dc2b  st   $2b,[y,x++]
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
35ae dc16  st   $16,[y,x++]
35af dc16  st   $16,[y,x++]
35b0 dc16  st   $16,[y,x++]
* 12 times
35ba dc00  st   $00,[y,x++]
35bb dc00  st   $00,[y,x++]
35bc dc00  st   $00,[y,x++]
* 5 times
35bf dc1a  st   $1a,[y,x++]
35c0 dc1a  st   $1a,[y,x++]
35c1 dc1a  st   $1a,[y,x++]
* 12 times
35cb dc00  st   $00,[y,x++]
35cc dc00  st   $00,[y,x++]
35cd dc00  st   $00,[y,x++]
* 5 times
35d0 dc2a  st   $2a,[y,x++]
35d1 dc2a  st   $2a,[y,x++]
35d2 dc2a  st   $2a,[y,x++]
* 12 times
35dc dc00  st   $00,[y,x++]
35dd dc00  st   $00,[y,x++]
35de dc00  st   $00,[y,x++]
* 5 times
35e1 dc25  st   $25,[y,x++]
35e2 dc25  st   $25,[y,x++]
35e3 dc25  st   $25,[y,x++]
* 12 times
35ed dc00  st   $00,[y,x++]
35ee dc00  st   $00,[y,x++]
35ef dc00  st   $00,[y,x++]
* 5 times
35f2 dc34  st   $34,[y,x++]
35f3 dc34  st   $34,[y,x++]
35f4 dc34  st   $34,[y,x++]
* 12 times
35fe dc00  st   $00,[y,x++]
35ff dc00  st   $00,[y,x++]
3600 dc00  st   $00,[y,x++]
* 5 times
3603 dc39  st   $39,[y,x++]
3604 dc39  st   $39,[y,x++]
3605 dc39  st   $39,[y,x++]
* 12 times
360f dc00  st   $00,[y,x++]
3610 dc00  st   $00,[y,x++]
3611 dc00  st   $00,[y,x++]
* 5 times
3614 dc3a  st   $3a,[y,x++]
3615 dc3a  st   $3a,[y,x++]
3616 dc3a  st   $3a,[y,x++]
* 12 times
3620 dc00  st   $00,[y,x++]
3621 dc00  st   $00,[y,x++]
3622 dc00  st   $00,[y,x++]
* 5 times
3625 dc3b  st   $3b,[y,x++]
3626 dc3b  st   $3b,[y,x++]
3627 dc3b  st   $3b,[y,x++]
* 12 times
3631 dc00  st   $00,[y,x++]
3632 dc00  st   $00,[y,x++]
3633 dc00  st   $00,[y,x++]
* 5 times
3636 dc2b  st   $2b,[y,x++]
3637 dc2b  st   $2b,[y,x++]
3638 dc2b  st   $2b,[y,x++]
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
3650 dc16  st   $16,[y,x++]
3651 dc16  st   $16,[y,x++]
3652 dc16  st   $16,[y,x++]
* 12 times
365c dc00  st   $00,[y,x++]
365d dc00  st   $00,[y,x++]
365e dc00  st   $00,[y,x++]
* 5 times
3661 dc1a  st   $1a,[y,x++]
3662 dc1a  st   $1a,[y,x++]
3663 dc1a  st   $1a,[y,x++]
* 12 times
366d dc00  st   $00,[y,x++]
366e dc00  st   $00,[y,x++]
366f dc00  st   $00,[y,x++]
* 5 times
3672 dc2a  st   $2a,[y,x++]
3673 dc2a  st   $2a,[y,x++]
3674 dc2a  st   $2a,[y,x++]
* 12 times
367e dc00  st   $00,[y,x++]
367f dc00  st   $00,[y,x++]
3680 dc00  st   $00,[y,x++]
* 5 times
3683 dc25  st   $25,[y,x++]
3684 dc25  st   $25,[y,x++]
3685 dc25  st   $25,[y,x++]
* 12 times
368f dc00  st   $00,[y,x++]
3690 dc00  st   $00,[y,x++]
3691 dc00  st   $00,[y,x++]
* 5 times
3694 dc34  st   $34,[y,x++]
3695 dc34  st   $34,[y,x++]
3696 dc34  st   $34,[y,x++]
* 12 times
36a0 dc00  st   $00,[y,x++]
36a1 dc00  st   $00,[y,x++]
36a2 dc00  st   $00,[y,x++]
* 5 times
36a5 dc39  st   $39,[y,x++]
36a6 dc39  st   $39,[y,x++]
36a7 dc39  st   $39,[y,x++]
* 12 times
36b1 dc00  st   $00,[y,x++]
36b2 dc00  st   $00,[y,x++]
36b3 dc00  st   $00,[y,x++]
* 5 times
36b6 dc3a  st   $3a,[y,x++]
36b7 dc3a  st   $3a,[y,x++]
36b8 dc3a  st   $3a,[y,x++]
* 12 times
36c2 dc00  st   $00,[y,x++]
36c3 dc00  st   $00,[y,x++]
36c4 dc00  st   $00,[y,x++]
* 5 times
36c7 dc3b  st   $3b,[y,x++]
36c8 dc3b  st   $3b,[y,x++]
36c9 dc3b  st   $3b,[y,x++]
* 12 times
36d3 dc00  st   $00,[y,x++]
36d4 dc00  st   $00,[y,x++]
36d5 dc00  st   $00,[y,x++]
* 5 times
36d8 dc2b  st   $2b,[y,x++]
36d9 dc2b  st   $2b,[y,x++]
36da dc2b  st   $2b,[y,x++]
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
* 6 times
36f2 dc16  st   $16,[y,x++]
36f3 dc16  st   $16,[y,x++]
36f4 dc16  st   $16,[y,x++]
* 12 times
36fe dc00  st   $00,[y,x++]
36ff dc00  st   $00,[y,x++]
3700 dc00  st   $00,[y,x++]
* 5 times
3703 dc1a  st   $1a,[y,x++]
3704 dc1a  st   $1a,[y,x++]
3705 dc1a  st   $1a,[y,x++]
* 12 times
370f dc00  st   $00,[y,x++]
3710 dc00  st   $00,[y,x++]
3711 dc00  st   $00,[y,x++]
* 5 times
3714 dc2a  st   $2a,[y,x++]
3715 dc2a  st   $2a,[y,x++]
3716 dc2a  st   $2a,[y,x++]
* 12 times
3720 dc00  st   $00,[y,x++]
3721 dc00  st   $00,[y,x++]
3722 dc00  st   $00,[y,x++]
* 5 times
3725 dc25  st   $25,[y,x++]
3726 dc25  st   $25,[y,x++]
3727 dc25  st   $25,[y,x++]
* 12 times
3731 dc00  st   $00,[y,x++]
3732 dc00  st   $00,[y,x++]
3733 dc00  st   $00,[y,x++]
* 5 times
3736 dc34  st   $34,[y,x++]
3737 dc34  st   $34,[y,x++]
3738 dc34  st   $34,[y,x++]
* 12 times
3742 dc00  st   $00,[y,x++]
3743 dc00  st   $00,[y,x++]
3744 dc00  st   $00,[y,x++]
* 5 times
3747 dc39  st   $39,[y,x++]
3748 dc39  st   $39,[y,x++]
3749 dc39  st   $39,[y,x++]
* 12 times
3753 dc00  st   $00,[y,x++]
3754 dc00  st   $00,[y,x++]
3755 dc00  st   $00,[y,x++]
* 5 times
3758 dc3a  st   $3a,[y,x++]
3759 dc3a  st   $3a,[y,x++]
375a dc3a  st   $3a,[y,x++]
* 12 times
3764 dc00  st   $00,[y,x++]
3765 dc00  st   $00,[y,x++]
3766 dc00  st   $00,[y,x++]
* 5 times
3769 dc3b  st   $3b,[y,x++]
376a dc3b  st   $3b,[y,x++]
376b dc3b  st   $3b,[y,x++]
* 12 times
3775 dc00  st   $00,[y,x++]
3776 dc00  st   $00,[y,x++]
3777 dc00  st   $00,[y,x++]
* 5 times
377a dc2b  st   $2b,[y,x++]
377b dc2b  st   $2b,[y,x++]
377c dc2b  st   $2b,[y,x++]
* 12 times
3786 dc00  st   $00,[y,x++]
3787 dc00  st   $00,[y,x++]
3788 dc00  st   $00,[y,x++]
* 6 times
378c 1000  ld   $00,x
378d 1453  ld   $53,y
378e dc00  st   $00,[y,x++]
378f dc00  st   $00,[y,x++]
3790 dc00  st   $00,[y,x++]
* 6 times
3794 dc16  st   $16,[y,x++]
3795 dc16  st   $16,[y,x++]
3796 dc16  st   $16,[y,x++]
* 12 times
37a0 dc00  st   $00,[y,x++]
37a1 dc00  st   $00,[y,x++]
37a2 dc00  st   $00,[y,x++]
* 5 times
37a5 dc1a  st   $1a,[y,x++]
37a6 dc1a  st   $1a,[y,x++]
37a7 dc1a  st   $1a,[y,x++]
* 12 times
37b1 dc00  st   $00,[y,x++]
37b2 dc00  st   $00,[y,x++]
37b3 dc00  st   $00,[y,x++]
* 5 times
37b6 dc2a  st   $2a,[y,x++]
37b7 dc2a  st   $2a,[y,x++]
37b8 dc2a  st   $2a,[y,x++]
* 12 times
37c2 dc00  st   $00,[y,x++]
37c3 dc00  st   $00,[y,x++]
37c4 dc00  st   $00,[y,x++]
* 5 times
37c7 dc25  st   $25,[y,x++]
37c8 dc25  st   $25,[y,x++]
37c9 dc25  st   $25,[y,x++]
* 12 times
37d3 dc00  st   $00,[y,x++]
37d4 dc00  st   $00,[y,x++]
37d5 dc00  st   $00,[y,x++]
* 5 times
37d8 dc34  st   $34,[y,x++]
37d9 dc34  st   $34,[y,x++]
37da dc34  st   $34,[y,x++]
* 12 times
37e4 dc00  st   $00,[y,x++]
37e5 dc00  st   $00,[y,x++]
37e6 dc00  st   $00,[y,x++]
* 5 times
37e9 dc39  st   $39,[y,x++]
37ea dc39  st   $39,[y,x++]
37eb dc39  st   $39,[y,x++]
* 12 times
37f5 dc00  st   $00,[y,x++]
37f6 dc00  st   $00,[y,x++]
37f7 dc00  st   $00,[y,x++]
* 5 times
37fa dc3a  st   $3a,[y,x++]
37fb dc3a  st   $3a,[y,x++]
37fc dc3a  st   $3a,[y,x++]
* 12 times
3806 dc00  st   $00,[y,x++]
3807 dc00  st   $00,[y,x++]
3808 dc00  st   $00,[y,x++]
* 5 times
380b dc3b  st   $3b,[y,x++]
380c dc3b  st   $3b,[y,x++]
380d dc3b  st   $3b,[y,x++]
* 12 times
3817 dc00  st   $00,[y,x++]
3818 dc00  st   $00,[y,x++]
3819 dc00  st   $00,[y,x++]
* 5 times
381c dc2b  st   $2b,[y,x++]
381d dc2b  st   $2b,[y,x++]
381e dc2b  st   $2b,[y,x++]
* 12 times
3828 dc00  st   $00,[y,x++]
3829 dc00  st   $00,[y,x++]
382a dc00  st   $00,[y,x++]
* 6 times
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
* 160 times
3a14 1000  ld   $00,x
3a15 1457  ld   $57,y
3a16 dc00  st   $00,[y,x++]
3a17 dc00  st   $00,[y,x++]
3a18 dc00  st   $00,[y,x++]
* 160 times
3ab6 1000  ld   $00,x
3ab7 1458  ld   $58,y
3ab8 dc00  st   $00,[y,x++]
3ab9 dc00  st   $00,[y,x++]
3aba dc00  st   $00,[y,x++]
* 160 times
3b58 1000  ld   $00,x
3b59 1459  ld   $59,y
3b5a dc00  st   $00,[y,x++]
3b5b dc00  st   $00,[y,x++]
3b5c dc00  st   $00,[y,x++]
* 160 times
3bfa 1000  ld   $00,x
3bfb 145a  ld   $5a,y
3bfc dc00  st   $00,[y,x++]
3bfd dc00  st   $00,[y,x++]
3bfe dc00  st   $00,[y,x++]
* 160 times
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
