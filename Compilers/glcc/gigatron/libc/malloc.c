#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <gigatron/libc.h>


/* ============ definitions ============ */

/* Block head for both free and used blocks */
typedef struct head_s {
	int size;                      /* low bit set for used blocks */
	struct head_s *bnext, *bprev;  /* links all blocks, both used and free */
	struct head_s *fnext, *fprev;  /* only for free blocks */
} head_t;


/* The list head */
static head_t head;

/* Head initialization */
static head_t head = { 0, &head, &head, &head, &head };

#define DEBUG 0

#if DEBUG
extern void _malloc_map(void);
#endif

/* ============ utilities ============ */

static void __free_block(register head_t *b)
{
	register int size = b->size;
	register head_t *pa, *pb;
	pa = b->bnext;
	while (pa->size & 1)
		pa = pa->bnext;
	pb = pa->fprev;
	b->fprev = pb;
	pb->fnext = b;
	b->fnext = pa;
	pa->fprev = b;
}

static void __list_block(head_t *b, head_t *pa)
{
	head_t *pb = pa->bprev;
	b->bprev = pb;
	b->bnext = pa;
	pb->bnext = b;
	pa->bprev = b;
}

static void __assume_block_position(head_t *b, head_t *z, int size)
{
	head_t *pa,*pb;  /* warning: b->bprev and b->fprev */
	b->size = size;  /* must be aleady set and correct.*/
	pa  = z->bnext;
	b->bnext  = pa;
	pa->bprev  = b;
	pb  = b->bprev;
	pb->bnext  = b;
	pa  = z->fnext;
	b->fnext  = pa;
	pa->fprev  = b;
	pb  = b->fprev;
	pb->fnext  = b;
}

static int try_merge_with_next(register head_t *b)
{
	register head_t *p = b->bnext;
	register int size, s;
	if (((size = b->size) & 1) ||
	    ((s = p->size) & 1) ||
	    (char*)b + size != (char*) p)
		return 0;
	__assume_block_position(b, p, size + s);
	return 1;
}

static head_t *find_block(register int size)
{
	register int d, bsize;
	register head_t *b = &head;
	for(;;) {
		b = b->fnext;
		if (b == &head)
			return 0;
		if ((d = (bsize = b->size) - size) >= 0)
			break;
	}
	if ((int)(d - sizeof(head_t)) < 0) {
		b->fnext->fprev = b->fprev;
		b->fprev->fnext = b->fnext;
		b->size = bsize | 1;
	} else {
		register head_t *nb = (head_t*)((char*)b + size);
		nb->bprev = b->bprev;
		nb->fprev = b->fprev;
		__assume_block_position(nb, b, d);
		b->size = size | 1;
		__list_block(b, nb);
	} 
	return b;
}

int __chk_block_header(register head_t *b)
{
	if ((b->size & 1) == 1
	    && b->bnext->bprev == b
	    && b->bprev->bnext == b)
		return (b->size | 1) ^ 1;
	return 0;
}

/* ============ public functions ============ */

void free(register void *ptr)
{
	register head_t *b;
	register int size;	
	if (ptr) {
		size = __chk_block_header(b = (head_t*)((char*)ptr - 8));
		if (! (b->size = size)) {
#if DEBUG
			malloc_map();
#endif
			_exitm(10, "free: corrupted heap!");
		}
		__free_block(b);
		try_merge_with_next(b);
		try_merge_with_next(b->bprev);
	}
#if DEBUG
	printf("After free(%04x) blk=%04x asz=%d\n", ptr, b, size);
	malloc_map();
#endif
}

void *malloc(register size_t sz)
{
	register head_t *b;
	if (sz < 4)
		sz = 4;
	if ((sz = (((sz + 8 + 3) | 3) ^ 3)) < 0x8000u)
		if (b = find_block(sz)) {
#if DEBUG
			printf("After malloc() sz=%d blk=%04x\n", sz, b);
			malloc_map();
#endif
			return (char*)b + 8;
		}
	return 0;
}



/* ============ initialization ============ */

/* glink collects all malloc areas into this singly linked list. */
head_t *__glink_magic_heap = (void*)0xBEEF;

static void malloc_init(void)
{
	register head_t *p = __glink_magic_heap;
	__glink_magic_heap = 0;
	while (p && p != (void*)0xBEEF) {
		register head_t *n = p->bnext;
		__list_block(p, head.bnext);
		__free_block(p);
		p = n;
	}
}

DECLARE_INIT_FUNCTION(malloc_init);


/* ============ debug ============ */

#if DEBUG

void print_block(head_t *b)
{
	printf("  block@%04x (%d) %c %04x:%04x %04x:%04x\n",
	       b, b->size & ~1, (b->size & 1) ? 'U' : 'F',
	       b->bnext, b->bprev, b->fnext, b->fprev );
}

void malloc_map(void)
{
	int i;
	head_t *q = &head;
	head_t *p = head.bnext;
	printf(" Blks:\t");
	i = 0;
	while (p != &head) {
		if (++i % 8 == 0)
			printf("\n\t");
		printf("%04x(%d,%c) ", p, p->size&~1, (p->size&1)?'U':'F');
		if (p->bprev != q)
			printf("{bad bprev %04x} ", p->bprev);
		q = p;
		p = p->bnext;
	}
	printf("\n Free:\t");
	q = &head;
	p = head.fnext;
	i = 0;
	while(p != &head) {
		if (++i % 8 == 0)
			printf("\n\t");
		printf("%04x(%d) ", p, p->size);
		if (p->fprev != q)
			printf("{bad fprev %04x} ", p->fprev);
		q = p;
		p = p->fnext;
	}
	printf("\n");
}


#endif
