#include <stdlib.h>


typedef struct fini_chain_s fini_chain_t;

extern fini_chain_t *__glink_magic_fini;

struct fini_chain_s {
	void (*f)(void);
	fini_chain_t *next;
};

/* Maximum number of atexit handlers below:
   - in addition of the static handlers defined with DECLARE_FINI_FUNCTION.
   - we could increase this number or use malloc once we have malloc. */

static fini_chain_t atexit_chain[4];

int atexit(void (*f)(void))
{
	int i;
	if (__glink_magic_fini != (void*)0xBEEFu)
		for (i=0; i< sizeof(atexit_chain)/sizeof(atexit_chain[0]); i++)
			if (! atexit_chain[i].f) {
				atexit_chain[i].f = f;
				atexit_chain[i].next = __glink_magic_fini;
				__glink_magic_fini = &atexit_chain[i];
				return 0;
			}
	return -1;
}

