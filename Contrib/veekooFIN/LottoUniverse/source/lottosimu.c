/*----------------------------------------------------------------------+
 |                                                                      |
 |     lottosimu.c -- demonstrate lotto number simulation               |
 |                    / quick and dirty Eurojackpot                     |
 |                                                                      |
 +----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
//#include <time.h>

void main() 
{ 
  int b0[5],bb[5],i,ii,total,game,cash,sw,saldo,jp,it;
  int bt0[2],bbt[2];
  int totalbt,eit,egame;
  int sil; //printout silence mode
  int cup; //cupoung silence mode  

//srand(time(0)); 
printf("\nEurojackpot simulation: ");
printf("\nEnter number of iterations 1-100000: ");
scanf("%d",&eit); 
printf("\nEnter number of game rounds 1-3380: ");
scanf("%d",&egame); 
printf("\nPrintout silence mode 0/1: ");
scanf("%d",&sil); 
printf("\nCupoung silence mode 0/1: "); 
scanf("%d",&cup);

 for(it=0;it<eit;it++) //iterations 1-100000
 {
 cash=0;

 for(game=0;game<egame;game++) //game rounds how many weeks 1-3380   
 {

/*----------------------------------------------------------------------+
 |                                                                      |
 |                                                                      |
 |                Eurojackpot - input rows                              |
 |                                                                      |
 +----------------------------------------------------------------------*/ 
 
/*----------------------------------------------------------------------+
 |                                                                      |
 |                                                                      |
 |                Eurojackpot - input row 1                             |
 |                                                                      |
 +----------------------------------------------------------------------*/  
  
  sw=0;
  //printf("\nEnter eurojackpot numbers 5pcs 1-50 R1: "); 
  for(i=0;i<5;i++) 
  { 
    b0[i]=rand() % 50 +1;  
    for(ii=0;ii<5;ii++) 
    {   
      if(b0[ii] == b0[i] && (ii != i)) sw=1;
    }
    if(sw==1){ 
    i--;
    sw=0;
    }
  }
  sw=0;
  //printf("\nEnter eurojackpot starnumbers 2pcs 1-10 R1: "); 
  for(i=0;i<2;i++) 
  {
    bt0[i]=rand() % 10 +1;
    for(ii=0;ii<2;ii++) 
    {  
      if(bt0[ii] == bt0[i] && (ii != i)) sw=1;
    }
    if(sw==1){ 
    i--;
    sw=0;
    }
  }
  
  if(cup != 1) printf("\nEurojackpot R1:\t\t\t %d + %d + %d + %d + %d * %d + %d\n",b0[0],b0[1],b0[2],b0[3],b0[4],bt0[0],bt0[1]);
   
/*----------------------------------------------------------------------+
 |                                                                      |
 |                                                                      |
 |                Eurojackpot - input right row                         |
 |                                                                      |
 +----------------------------------------------------------------------*/  
  sw=0;
  //printf("\nEnter winning eurojackpot numbers 5pcs 1-50: "); 
  for(i=0;i<5;i++) 
  { 
    bb[i]=rand() % 50 +1;
    for(ii=0;ii<5;ii++) 
    {     
      if(bb[ii] == bb[i] && (ii != i)) sw=1;
    }
    if(sw==1){ 
    i--;
    sw=0;
    }
  }
  sw=0;
  //printf("\nEnter winning eurojackpot starnumbers 2pcs 1-10: ");
  for(i=0;i<2;i++) 
  {
    bbt[i]=rand() % 10 +1;
    for(ii=0;ii<2;ii++) 
    {  
      if(bbt[ii] == bbt[i] && (ii != i)) sw=1;
    }
    if(sw==1){ 
    i--;
    sw=0;
    }
  }
  
  if(cup != 1) printf("\nEurojackpot winning row:\t\t\t %d + %d + %d + %d + %d * %d + %d\n",bb[0],bb[1],bb[2],bb[3],bb[4],bbt[0],bbt[1]);

/*----------------------------------------------------------------------+
 |                                                                      |
 |                                                                      |
 |                Eurojackpot - output results                          |
 |                                                                      |
 +----------------------------------------------------------------------*/

  jp=rand() % 80000000 + 10000000; //jackpot 10-90 milj. euro

/*----------------------------------------------------------------------+
 |                                                                      |
 |                                                                      |
 |                Eurojackpot - output row 1                            |
 |                                                                      |
 +----------------------------------------------------------------------*/

  total=0;  
  for(i=0;i<5;i++) 
  { 
    for(ii=0;ii<5;ii++) 
    { 
      if(bb[i]==b0[ii])
      { 
        total++;
      }
    }  
  }
  totalbt=0;
  for(i=0;i<2;i++) 
  { 
    for(ii=0;ii<2;ii++) 
    { 
      if(bbt[i]==bt0[ii])
      { 
        totalbt++;
      }
    }  
  }
      
  if(cup != 1) printf("\nEurojackpot numbers correct R1:\t\t\t %d * %d\n",total,totalbt);
  if(total==5 && totalbt==2) {
    cash=cash+jp;
    if(sil != 1) printf("\nEurojackpot\t\t\t %d euro win\n",jp);
     }
  if(total==5 && totalbt==1) {
    cash=cash+500000;
    if(sil != 1) printf("\nEurojackpot 500 000 euro win\n");
  }
  if(total==5 && totalbt==0) {
    cash=cash+100000;
    if(sil != 1) printf("\nEurojackpot 100 000 euro win\n");
  }
  if(total==4 && totalbt==2) {
    cash=cash+4200;
    if(sil != 1) printf("\nEurojackpot 4200 euro win\n");
  }
  if(total==4 && totalbt==1) {
    cash=cash+240;
    if(sil != 1) printf("\nEurojackpot 240 euro win\n");
  }
  if(total==4 && totalbt==0) {
    cash=cash+100;
    if(sil != 1) printf("\nEurojackpot 100 euro win\n");
  }
  if(total==3 && totalbt==2) {
    cash=cash+60;
    if(sil != 1) printf("\nEurojackpot 60 euro win\n");
  }
  if(total==2 && totalbt==2) {
    cash=cash+20;
    if(sil != 1) printf("\nEurojackpot 20 euro win\n");
  }
  if(total==3 && totalbt==1) {
    cash=cash+18;
    if(sil != 1) printf("\nEurojackpot 18 euro win\n");
  }
  if(total==1 && totalbt==2) {
    cash=cash+10;
    if(sil != 1) printf("\nEurojackpot 10 euro win\n");
  }
  if(total==2 && totalbt==1) {
    cash=cash+8;
    if(sil != 1) printf("\nEurojackpot 8 euro win\n");
  }
  
/*----------------------------------------------------------------------+
 |                                                                      |
 |                                                                      |
 |                Eurojackpot - reports                                 |
 |                                                                      |
 +----------------------------------------------------------------------*/
 
  saldo = cash - game * 2 -2;

  if(sil != 1 && saldo > 1) printf("\nEurojackpot game round, win, 1 euro saldo:\t\t\t %d+%d+%d\n",game,cash,saldo);
  if(sil != 1 && saldo > 10) printf("\nEurojackpot game round, win, 10 euro saldo:\t\t\t %d+%d+%d\n",game,cash,saldo);
  if(sil != 1 && saldo > 100) printf("\nEurojackpot game round, win, 100 euro saldo:\t\t\t %d+%d+%d\n",game,cash,saldo);
  if(sil != 1 && saldo > 1000) printf("\nEurojackpot game round, win, 1 000 euro saldo:\t\t\t %d+%d+%d\n",game,cash,saldo);
  if(sil != 1 && saldo > 10000) printf("\nEurojackpot game round, win, 10 000 euro saldo:\t\t\t %d+%d+%d\n",game,cash,saldo);
  if(sil != 1 && saldo > 100000) printf("\nEurojackpot game round, win, 100 000 euro saldo:\t\t\t %d+%d+%d\n",game,cash,saldo);
  if(sil != 1 && saldo > 1000000) printf("\nEurojackpot game round, win, 1 000 000 euro saldo:\t\t\t %d+%d+%d\n",game,cash,saldo);
  if(sil != 1 && saldo > 10000000) printf("\nEurojackpot game round, win, 10 000 000 euro saldo:\t\t\t %d+%d+%d\n",game,cash,saldo);
  if(sil != 1 && saldo > 100000000) printf("\nEurojackpot game round, win, 100 000 000 euro saldo:\t\t\t %d+%d+%d\n",game,cash,saldo);
  }
 
  //if(saldo > 1000000) 
  printf("\nEurojackpot iteration, game round, win, saldo:\t\t\t %d + %d + %d + %d\n",it,game,cash,saldo);
}
}  

