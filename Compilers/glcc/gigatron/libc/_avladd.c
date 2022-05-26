#include <stdio.h>
#include <stdlib.h>
#include <gigatron/avl.h>


#define AVL_REBAL_ASM 1
#define AVL_STKSIZE   16

#if AVL_REBAL_ASM

/* The rebalancing function seems long and complicated but (1) is a
   frameless leaf function, and (2) has a lot of redundant part.
   Taking advantage of both properties can make it small enough to be
   useful on the Gigatron. Alas LCC cannot take advantage of both
   these properties. This is why a compact assembly code version is
   preferred. */

extern void __avl_rebal(avlnode_t ***);

#else

void __avl_rebal(register avlnode_t ***sp)
{
	register avlnode_t **pelt, *elt, **r, *rtmp;
	register int lh, rh, tmp;

#define GETH do { \
		lh = rh = 0;\
		if (elt->left)\
			lh = elt->left->height;\
		if (elt->right)\
			rh = elt->right->height;\
	} while(0)
#define CALC do {\
		GETH;\
		tmp = 0;\
		if (lh > tmp)\
			tmp = lh;\
		if (rh > tmp)\
			tmp = rh;\
		elt->height = tmp + 1;\
	} while(0)
#define LROT do {\
		rtmp = (*r)->right;\
		(*r)->right = rtmp->left;\
		rtmp->left = (*r);\
		elt = *r; CALC; \
		elt = rtmp; CALC; \
		*r = rtmp;\
	} while(0)
#define RROT do {\
		rtmp = (*r)->left;\
		(*r)->left = rtmp->right;\
		rtmp->right = (*r);\
		elt = *r; CALC;\
		elt = rtmp; CALC;\
		*r = rtmp;\
	} while(0)

	while (pelt = *sp++) {
		register avlnode_t *elt;
		register int lh;
		register int rh;
		if (! (elt = *pelt))
			continue;
		CALC;
		if (rh - lh == -2) {
			elt = elt->left;
			GETH;
			if (rh - lh > 0) {
				r = &((*pelt)->left); LROT;
			}
			r = pelt; RROT;
		} else if (rh - lh == +2) {
			elt = elt->right;
			GETH;
			if (rh - lh < 0) {
				r = &((*pelt)->right); RROT;
			}
			r = pelt; LROT;
		}
	}
}

#endif

avlnode_t *_avl_add(register avlnode_t **proot,
	     register avlnode_t *elt, register avlcmp_t cmp)
{
	avlnode_t **stack[AVL_STKSIZE];
	register avlnode_t ***sp = stack + (AVL_STKSIZE - 1);
	*sp = 0;
	while (--sp, *(*sp = proot)) {
		register int c;
		if ((c = cmp(elt, *proot)) == 0)
			return *proot;
		else if (c < 0)
			proot = &((*proot)->left);
		else
			proot = &((*proot)->right);
	}
	elt->left = elt->right = 0;
	*proot = elt;
	__avl_rebal(sp);
	return 0;
}

