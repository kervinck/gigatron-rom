/* -*- C -*- */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gigatron/sys.h>

#include "core.h"

int prebooksize = 0;
near int booksize = 0;         /* Number of opening book entries */
near int bkindex = -1;         /* Current book table index */
struct tt ttentry;             /* Current transposition table entry */
struct bk bkentry;             /* Current book table entry */

#define BKPTR ((struct bk*)(0xC000))
#define TTPTR ((struct tt*)(0x8000))

void set_computing_mode(int enabled)
{
#ifndef REPEATABLE_RND
  if (enabled)
    SYS_SetMode(3);
  else
    SYS_SetMode(2);
#endif
}

/* The following functions operate with bank3 selected.
   Since the stack grows down from 0xfffc, it is important
   to make sure that these functions do not touch the 
   stack while bank3 is selected. */

void clr_ttable(void)
{
  register byte cb = ctrlBits_v5;
  SYS_ExpanderControl(cb | 0xc0);
  memset((void*)TTPTR, 0, CORE * sizeof(struct tt)); /* memset does not PUSH */
  SYS_ExpanderControl(cb);
}

struct tt *get_ttable(int x)
{
  register byte cb = ctrlBits_v5;
  SYS_ExpanderControl(cb | 0xc0);
  ttentry = TTPTR[x];
  SYS_ExpanderControl(cb);
  return &ttentry;
}

void set_ttable(int x, struct tt *tt)
{
  register byte cb = ctrlBits_v5;
  SYS_ExpanderControl(cb | 0xc0);
  TTPTR[x] = *tt;
  SYS_ExpanderControl(cb);
}

struct bk *get_book(int x)
{
  register byte cb = ctrlBits_v5;
  if (x != bkindex) {
    bkindex = x;
    SYS_ExpanderControl(cb | 0xc0);
    bkentry = BKPTR[x];
    SYS_ExpanderControl(cb);
  }
  return &bkentry;
}


void preload_book(const char *filename)
{
  register byte cb = ctrlBits_v5;
#ifdef BOOKSIZE
  /* Load book from ROM */
  register void *p = 0;
  char buf[9];
  do {
    p = SYS_ReadRomDir(p, buf);
    buf[8] = 0;
  } while (p != 0 && strcmp(buf,"MSCPbook") != 0);
  if (p) {
    SYS_ExpanderControl(cb | 0xc0);
    SYS_Exec(p, (void*)(unsigned)-1);
    SYS_ExpanderControl(cb);
    prebooksize = BOOKSIZE / sizeof(struct bk);
  }
#endif
}

void load_book(const char *filename)
{
  if (prebooksize) 
    booksize = prebooksize / sizeof(struct bk);
  printf("Opening book size: %d\n", booksize);
}


/* Local Variables: */
/* mode: c */
/* c-basic-offset: 2 */
/* indent-tabs-mode: () */
/* End: */
