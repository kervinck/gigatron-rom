1  'calculate primes
2  'between 3 and L
10 L=10000:mode 2
15 poke42,16:poke46,1:cls
20 R=0:B(0)=124:B(1)=47
25 B(2)=45:B(3)=92:C(0)=0
30 C(1)=8:C(2)=4:C(3)=12
35 for N=3 to L:poke43,3
36 X=0:Y=0:Z=0:poke 14,0
40 if N%2=0 goto 75
45 for D=2 to N-1
46 Y=peek(14)
47 if Y<X Z=Z+427
50 X=Y:at 2:put B(D%4)
55 poke 20,1+(D%2)+C(R)
60 if N%D=0 goto 75
65 nextD:poke43,63
67 S=(Z+(427/256*Y))/100
68 at2:?N;" -> ";S;"s"
70 R=R+1:if R=4 R=0
75 next N:at 2:put 32:?
80 poke 43,63:poke 46,0
85 mode 1:end
