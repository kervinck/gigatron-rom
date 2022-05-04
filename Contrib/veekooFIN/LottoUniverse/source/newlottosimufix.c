/*----------------------------------------------------------------------+
 |                                                                      |
 |     lottosimufix.c -- demonstrate lotto number simulation            |
 |                       of Eurojackpot / quick and dirty               |
 |                                                                      |
 +----------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include <gigatron/console.h>

int readint(const char *prompt)
{
	char buffer[32];
	cprintf(prompt, strlen(prompt));
	console_readline(buffer, sizeof(buffer));
	return atoi(buffer);
}

void main()
{
  int b0[5],bb[5],i,ii,total,game,sw,it;
  long cash,jp,saldo;
  int bt0[2],bbt[2];
  int totalbt,eit,egame;
  int sil; //printout silence mode
  int cup; //cupoung silence mode
  int big; //big saldo annoucements mode 0-10000

  cprintf("Eurojackpot simulation:\n");
  cprintf("\nOne fixed row:\n");
  b0[0]=readint("Enter 1-50 nbr#1:");
  b0[1]=readint("Enter 1-50 nbr#2:");
  b0[2]=readint("Enter 1-50 nbr#3:");
  b0[3]=readint("Enter 1-50 nbr#4:");
  b0[4]=readint("Enter 1-50 nbr#5:");
  bt0[0]=readint("Enter 1-12 star nbr#1:");
  bt0[1]=readint("Enter 1-12 star nbr#2:");
  eit=readint("Nbr of iterations:");
  egame=readint("Nbr of games:");
  sil=readint("Printout silence 0/1:");
  cup=readint("Cupoung silence 0/1:");
  big=readint("Big saldo info 0/nbr:");

  for(it=0;it<eit;it++) //iterations 1-10000
  {
    cash=0;

    for(game=0;game<egame;game++) //game rounds how many weeks 1-52
    {
      /*sw=0;

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

      for(i=0;i<2;i++)
      {
        bt0[i]=rand() % 12 +1;
        for(ii=0;ii<2;ii++)
        {
          if(bt0[ii] == bt0[i] && (ii != i)) sw=1;
        }
        if(sw==1){
          i--;
          sw=0;
        }
      }*/

      if(cup != 1) cprintf("\nRow1:%d+%d+%d+%d+%d*%d+%d",b0[0],b0[1],b0[2],b0[3],b0[4],bt0[0],bt0[1]);
      sw=0;
      bb[0]=0;
      bb[1]=0;
      bb[2]=0;
      bb[3]=0;
      bb[4]=0;

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
      bbt[0]=0;
      bbt[1]=0;

      for(i=0;i<2;i++)
      {
        bbt[i]=rand() % 12 +1;
        for(ii=0;ii<2;ii++)
        {
          if(bbt[ii] == bbt[i] && (ii != i)) sw=1;
        }
        if(sw==1){
          i--;
          sw=0;
        }
      }

      if(cup != 1) cprintf("\nWinning row:%d+%d+%d+%d+%d*%d+%d",bb[0],bb[1],bb[2],bb[3],bb[4],bbt[0],bbt[1]);

      jp=rand() % 110000000 + 10000000; //jackpot 10-120 milj. euro
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

      if(cup != 1) cprintf("\nNumbers correct Row1:%d*%d",total,totalbt);

      if(total==5 && totalbt==2) {
      cash=cash+jp;
      if(sil != 1) cprintf("\n%ld euro win",jp);
      }
      if(total==5 && totalbt==1) {
      cash=cash+600000;
      if(sil != 1) cprintf("\n600 000 euro win");
      }
      if(total==5 && totalbt==0) {
      cash=cash+150000;
      if(sil != 1) cprintf("\n150 000 euro win");
      }
      if(total==4 && totalbt==2) {
      cash=cash+5000;
      if(sil != 1) cprintf("\n5000 euro win");
      }
      if(total==4 && totalbt==1) {
      cash=cash+310;
      if(sil != 1) cprintf("\n310 euro win");
      }
      if(total==3 && totalbt==2) {
      cash=cash+155;
      if(sil != 1) cprintf("\n155 euro win");
      }
      if(total==4 && totalbt==0) {
      cash=cash+110;
      if(sil != 1) cprintf("\n110 euro win");
      }
      if(total==2 && totalbt==2) {
      cash=cash+25;
      if(sil != 1) cprintf("\n25 euro win");
      }
      if(total==3 && totalbt==1) {
      cash=cash+20;
      if(sil != 1) cprintf("\n20 euro win");
      }
      if(total==3 && totalbt==0) {
      cash=cash+17;
      if(sil != 1) cprintf("\n17 euro win");
      }   
      if(total==1 && totalbt==2) {
      cash=cash+13;
      if(sil != 1) cprintf("\n13 euro win");
      }
      if(total==2 && totalbt==1) {
      cash=cash+10;
      if(sil != 1) cprintf("\n10 euro win");
      }
      
      saldo = cash - game * 2 -2;

      if(sil != 1 && saldo > 100) cprintf("\nGame,win,100 euro saldo:%d+%ld+%ld",game,cash,saldo);
      if(sil != 1 && saldo > 1000) cprintf("\nGame,win,1000 euro saldo:%d+%ld+%ld",game,cash,saldo);
      if(sil != 1 && saldo > 10000) cprintf("\nGame,win,10000 euro saldo:%d+%ld+%ld",game,cash,saldo);
      if(sil != 1 && saldo > 100000) cprintf("\nGame,win,100000 euro saldo:%d+%ld+%ld",game,cash,saldo);
      if(sil != 1 && saldo > 1000000) cprintf("\nGame,win,1000000 euro saldo:%d+%ld+%ld",game,cash,saldo);
      if(sil != 1 && saldo > 10000000) cprintf("\nGame,win,10000000 euro saldo:%d+%ld+%ld",game,cash,saldo);
    }
    if(big == 0) {
      cprintf("\nIteration,game,win,saldo:");
      cprintf("\n%d+%d+%ld+%ld",it,game,cash,saldo);
    }
    if(big > 0 && saldo > big) {
      cprintf("\nIteration,game,win,saldo:");
      cprintf("\n%d+%d+%ld+%ld",it,game,cash,saldo);
    }
  }
}
