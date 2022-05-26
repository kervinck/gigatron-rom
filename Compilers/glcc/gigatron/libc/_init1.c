#include <string.h>

/* Initializer for bss segments */

/* This is magically populated by glink */
struct bsschain {
  unsigned size;
  struct bsschain *next;
} *__glink_magic_bss = (void*)0xBEEF;


void _init_bss(void)
{
  struct bsschain *r = __glink_magic_bss;
  while (r != 0 && r != (void*)0xBEEF)
    {
      struct bsschain *n = r->next;
      __glink_magic_bss = 0;
      memset(r, 0, r->size);
      r = n;
    }
}

/* Local Variables: */
/* mode: c */
/* c-basic-offset: 2 */
/* indent-tabs-mode: () */
/* End: */
