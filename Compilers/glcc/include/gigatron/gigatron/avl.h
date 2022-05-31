#ifndef __GIGATRON_AVL
#define __GIGATRON_AVL

#include <stdlib.h>
#include <stdio.h>

/* ==== Self-balancing AVL trees for the gigatron ==== */

/* avlnode_t: type of the tree nodes as seen by the tree code. The
   payload must be located at a known offset from this structure,
   e.g., "struct mynode_s { avlnode_t node; mydata_t payload; };".
   The tree head is a variable "avlnode_t *root = 0;". */

typedef struct avlnode_s {
	int height;
	struct avlnode_s *left, *right;
} avlnode_t;

/* avlcmp_t: type of the comparison function to order the tree nodes.
   This function should compare the payloads associated with the two
   nodes and return -1, 0, or +1. */

typedef int (*avlcmp_t)(avlnode_t*, avlnode_t*);

/* _avl_add: adds node elt to the tree. Argument proot should point
   to the tree head. Argument elt should point to a node and its
   payload.  If no node in the tree has a payload equal to elt's, the
   node elt is inserted into the tree, the function returns 0,
   and elt nows belongs to the tree.  Otherwise the function returns
   a pointer to the tree element whose payload is equal to that
   of elt. */

extern avlnode_t *_avl_add(avlnode_t **proot, avlnode_t *elt, avlcmp_t cmp);

/* _avl_del: remove a node from the tree. Argument proot should point
   to the tree head. If the tree contains a node whose payload is
   equal to elt's, the node will be removed from the tree, and the
   function returns the node pointer to indicate that it no longers
   belongs to the tree and can be freed. Otherwise the function
   returns 0. Argument elt is only used for its payload. */

extern avlnode_t *_avl_del(avlnode_t **proot, avlnode_t *elt, avlcmp_t cmp);

#endif
