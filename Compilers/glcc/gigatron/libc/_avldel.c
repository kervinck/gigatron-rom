#include <stdio.h>
#include <stdlib.h>
#include <gigatron/avl.h>

#define AVL_STKSIZE   16

extern void __avl_rebal(avlnode_t ***);

avlnode_t *_avl_del(register avlnode_t **proot,
		    register avlnode_t *elt, register avlcmp_t cmp)
{
	avlnode_t **stack[AVL_STKSIZE];
	register avlnode_t ***sp = stack + (AVL_STKSIZE - 1);
	register avlnode_t **p;
	*sp = 0;
	for (;;) {
		register int c;
	more:	sp -= 1;
		*sp = proot;
		if (! *proot)
			return 0;
		if ((c = cmp(elt, *proot)) == 0)
			break;
		else if (c < 0)
			proot = &((*proot)->left);
		else
			proot = &((*proot)->right);
	}
	elt = *proot;
	if (! elt->left) {
		*proot = elt->right;
	} else if (! *(p = &elt->right)) {
		*proot = elt->left;
	} else {
		register avlnode_t *tmp;
		while ((*p)->left)
			p = &((*p)->left);
		tmp = (*p)->right;
		*proot = *p;
		*p = elt; /* possibly overwriting elt->right! */
		(*proot)->left = elt->left;
		(*proot)->right = elt->right;
		elt->left = 0;
		elt->right = tmp;
		proot = &(*proot)->right;
		goto more;
	}
	__avl_rebal(sp);
	return elt;
}
