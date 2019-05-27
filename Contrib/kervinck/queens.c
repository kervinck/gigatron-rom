/*
 queens.c by Marcel van Kervinck <marcelk@stack.urc.tue.nl>
 solves the problem of placing n queens on
 an n*n board without attacking eachother
*/
#include <stdio.h>
t(a,b,c){int d=0,e=a&~b&~c,f=1;if(a)for(f=0;e-=d,d=e&-e;f+=t(a-d,(b+d)*2,(
c+d)/2));return f;}main(q){scanf("%d",&q);printf("%d\n",t(~(~0<<q),0,0));}
