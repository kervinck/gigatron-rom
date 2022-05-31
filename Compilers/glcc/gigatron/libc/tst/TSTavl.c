#include <stdlib.h>
#include <stdio.h>
#include <gigatron/avl.h>


typedef struct {
	avlnode_t node;
	int payload;
} mynode_t;

avlnode_t *root;

#define PAYLOAD(n) ((mynode_t*)(n))->payload

static int cmp(avlnode_t *a, avlnode_t *b)
{
	return (PAYLOAD(a) - PAYLOAD(b));
}

static void print_tree(avlnode_t *root)
{
	if (root) {
		printf("[");
		print_tree(root->left);
		printf("%d", PAYLOAD(root));
		print_tree(root->right);
		printf("]");
	} else
		printf(".");
}

static int check_tree(mynode_t *n)
{
	int lh, rh, h;
	if (n) {
		h = lh = rh = 0;
		if (n->node.left) {
			lh = check_tree((mynode_t*)n->node.left);
			if (lh > h)
				h = lh;
			if (PAYLOAD(n) <= PAYLOAD(n->node.left))
				printf("Node %p (%d) has bad left ordering\n", n, n->payload);
		}
		if (n->node.right) {
			rh = check_tree((mynode_t*)n->node.right);
			if (rh > h)
				h = rh;
			if (PAYLOAD(n) >= PAYLOAD(n->node.right))
				printf("Node %p (%d) has bad right ordering\n", n, n->payload);
		}
		if (h + 1 != n->node.height)
			printf("Node %p (%d) has incorrect height\n", n, n->payload);
		if (abs(lh - rh) > 1)
			printf("Node %p (%d) has incorrect balance\n", n, n->payload);
		return h + 1;
	}
	return 0;
}

#define REPS 10
#define SIZE 40
int arr[SIZE];

int main()
{
	int i, j;
	srand(314);
	for (i=0; i!=REPS; i++) {
		printf("Rep#%d\n", i+1);
		for (j=0; j!=SIZE; j++) {
			mynode_t *n = malloc(sizeof(*n));
			arr[j] = n->payload = rand() & 0x3f;
			if (_avl_add(&root, (avlnode_t*)n, cmp))
				free(n);
			if (j % 8 == 0 || j + 1 == SIZE) {
				print_tree(root);
				printf("\n");
				check_tree((mynode_t*)root);
			}
		}
		for (j=0; j!=SIZE; j++) {
			mynode_t dummy, *n;
			dummy.payload = arr[j];
			n = (mynode_t*)_avl_del(&root, (avlnode_t*)&dummy, cmp);
			if (n)
				free(n);
			if (j % 8 == 0 || j + 1 == SIZE) {
				print_tree(root);
				printf("\n");
				check_tree((mynode_t*)root);
			}
		}
	}
	return 0;
}
