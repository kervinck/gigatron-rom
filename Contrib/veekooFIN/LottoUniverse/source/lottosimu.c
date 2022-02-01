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
  int b0[5],bb[5],i,ii,total,game,cash,sw,saldo,jp,it;
  int bt0[2],bbt[2];
  int totalbt,eit,egame;
  int sil; //printout silence mode
  int cup; //cupoung silence mode

  cprintf("\nEurojackpot simulation:"); 
  eit=readint("\nNbr of iterations:");
  egame=readint("\nNbr of games:");
  sil=readint("\nPrintout silence 0/1:");
  cup=readint("\nCupoung silence 0/1:");

  for(it=0;it<eit;it++) //iterations 1-100000
  {
    cash=0;

    for(game=0;game<egame;game++) //game rounds how many weeks 1-3380
    {
      sw=0;

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

      if(cup != 1) cprintf("\nRow1:%d+%d+%d+%d+%d*%d+%d\n",b0[0],b0[1],b0[2],b0[3],b0[4],bt0[0],bt0[1]);
      sw=0;

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

      if(cup != 1) cprintf("\nWinning row:%d+%d+%d+%d+%d*%d+%d\n",bb[0],bb[1],bb[2],bb[3],bb[4],bbt[0],bbt[1]);

      jp=rand() % 80000000 + 10000000; //jackpot 10-90 milj. euro
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

      if(cup != 1) cprintf("\nNumbers correct Row1:%d*%d\n",total,totalbt);

      if(total==5 && totalbt==2) {
      cash=cash+jp;
      if(sil != 1) cprintf("\n%d euro win\n",jp);
      }
      if(total==5 && totalbt==1) {
      cash=cash+500000;
      if(sil != 1) cprintf("\n500 000 euro win\n");
      }
      if(total==5 && totalbt==0) {
      cash=cash+100000;
      if(sil != 1) cprintf("\n100 000 euro win\n");
      }
      if(total==4 && totalbt==2) {
      cash=cash+4200;
      if(sil != 1) cprintf("\n4200 euro win\n");
      }
      if(total==4 && totalbt==1) {
      cash=cash+240;
      if(sil != 1) cprintf("\n240 euro win\n");
      }
      if(total==4 && totalbt==0) {
      cash=cash+100;
      if(sil != 1) cprintf("\n100 euro win\n");
      }
      if(total==3 && totalbt==2) {
      cash=cash+60;
      if(sil != 1) cprintf("\n60 euro win\n");
      }
      if(total==2 && totalbt==2) {
      cash=cash+20;
      if(sil != 1) cprintf("\n20 euro win\n");
      }
      if(total==3 && totalbt==1) {
      cash=cash+18;
      if(sil != 1) cprintf("\n18 euro win\n");
      }
      if(total==1 && totalbt==2) {
      cash=cash+10;
      if(sil != 1) cprintf("\n10 euro win\n");
      }
      if(total==2 && totalbt==1) {
      cash=cash+8;
      if(sil != 1) cprintf("\n8 euro win\n");
      }

      saldo = cash - game * 2 -2;

      if(sil != 1 && saldo > 100) cprintf("\nGame,win,100 euro saldo:%d+%d+%d\n",game,cash,saldo);
      if(sil != 1 && saldo > 1000) cprintf("\nGame,win,1000 euro saldo:%d+%d+%d\n",game,cash,saldo);
      if(sil != 1 && saldo > 10000) cprintf("\nGame,win,10000 euro saldo:%d+%d+%d\n",game,cash,saldo);
      if(sil != 1 && saldo > 100000) cprintf("\nGame,win,100000 euro saldo:%d+%d+%d\n",game,cash,saldo);
      if(sil != 1 && saldo > 1000000) cprintf("\nGame,win,1000000 euro saldo:%d+%d+%d\n",game,cash,saldo);
      if(sil != 1 && saldo > 10000000) cprintf("\nGame,win,10000000 euro saldo:%d+%d+%d\n",game,cash,saldo);
    }
    cprintf("\nIteration,game,win,saldo:\n");
    cprintf("\n%d+%d+%d+%d\n",it,game,cash,saldo);
  }
}
