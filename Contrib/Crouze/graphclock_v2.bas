'graph display with clock
'(hr:min) using 59.98Hz
'counter. UnREM lines 125
'and 425 to see seconds
'for debugging, adjust I
'(1/100 sec/min) and J
'(1/100 sec/hour) for
'clock finetuning.
'Set time with Famicom
'A button + arrows,
'toggle fixed clock
'colour with B button.

10 poke 14,0:poke 46,1
20 A=0:B=0:I=-3:J=8
30 K=0:H=0:M=0:S=0
40 poke 42,0
50 cls:C=rnd(23)+1
60 P=rnd(101)+43
70 P=P*(rnd(2)+1)
80 U=5400:V=0
90 for R=0 to 15
100 Z=S+(427/256*B)
110 if Z<6000 goto 160
120 M=M+1:S=S+I-6000
125 'Z=Z-6000
130 if M<60 goto 160
140 M=0:H=H+1:S=S+J
150 if H=24 H=0
160 T=rnd(128)+2
170 if K=0 poke 43,C
180 gosub 400
190 for X=0to159:L=X/20
200 if peek(20)<8 L=L+8
210 poke 20,L:Q=peek(17)
220 if Q<>255 gosub 500
230 Y=72-U/100
240 poke Y*256+X,C
250 U=U+V:V=V-U/P
260 B=peek(14)
270 if B<A S=S+427
280 A=B:next X:C=C+23
290 if C%64=0 C=C+23
300 at T,1:?"     ";
310 next R:goto 50

400 at T,1
410 ?H/10;H%10;":";
420 ?M/10;M%10;
425 'at2,112:?Z/100;" ";
430 return

500 if Q<>191 goto 530
510 K=K+1:if K=64 K=0
520 goto 700
530 poke 43,63
540 if Q<>119 goto 570
550 M=M+1:if M>59 M=0
560 S=0:goto 650
570 if Q<>123 goto 600
580 M=M-1:if M<0 M=59
590 S=0:goto 650
600 if Q<>125 goto 630
610 H=H-1:if H<0 H=23
620 goto 650
630 if Q<>126 goto 650
640 H=H+1:if H>23 H=0
650 gosub 400:Q=peek(17)
660 if Q<>255 goto 540
670 if S=0 poke 14,0
680 if K>0 goto 700
690 poke 43,C:goto 710
700 poke 43,K
710 gosub 400:return
