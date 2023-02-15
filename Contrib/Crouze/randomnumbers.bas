10 mode 1
15 poke 42,16:poke 46,1
20 W(0)=124:W(1)=47
25 W(2)=45:W(3)=92
30 cls:poke 43,32:C=0
35 ?"<q>uit at pause":?
40 poke 43,53
45 ?"Calculating numbers"
50 ?"between 0 and 15":?
55 if C=8 goto 30
60 C=C+1:poke 43,3
65 for T=0 to 128
70 at 2:put W(T%4)
75 poke 20,rnd(16):next T
80 poke 20,0:for T=0 to 3
85 B(T)=rnd(2):next T
90 N=B(0)+B(1)*2+B(2)*4
95 N=N+B(3)*8:poke 43,63
100 L=B(3)+B(2)*2+B(1)*4
105 L=L+B(0)*8:poke 20,L
110 at 2:?N:for T=0 to 50
115 K=peek(15)
120 if K=113 goto 130
125 next T:goto 55
130 poke 42,63:poke 43,53
135 poke 46,0:cls:end
