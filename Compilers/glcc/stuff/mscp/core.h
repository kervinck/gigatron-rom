/* -*- C -*- */

#ifndef __gigatron
# error "This file is Gigatron specific"
#endif
#if _GLCC_VER < 105077
# error "This file needs a more recent GLCC version"
#endif

typedef unsigned long uint32_t;
typedef unsigned int uint16_t;
typedef unsigned char uint8_t;
typedef long int32_t;
typedef int int16_t;
typedef signed char int8_t;

#define near __near

#define SUBTRACTIVE_RND 1
#define AVOID_SCANF 1
#define MAXDEPTH 2
#define CORE 2048

/* Number of opening book entries */

extern near int booksize;

/* Transposition table and opening book */

struct tt {                     /* Transposition table entry */
  uint16_t hash;          /* - Identifies position */ 
  int16_t move;           /* - Best recorded move */
  int16_t score;          /* - Score */
  char flag;              /* - How to interpret score */
  char depth;             /* - Remaining search depth */
};

struct bk {                     /* Opening book entry */
  uint32_t hash;          /* - Identifies position */
  int16_t move;           /* - Move for this position */
  uint16_t count;         /* - Frequency */
};


extern void set_computing_mode(int enabled);
extern void clr_ttable(void);
extern struct tt *get_ttable(int x);
extern void set_ttable(int x, struct tt *tt);
extern struct bk *get_book(int x);

extern void preload_book(const char *filename);
extern void load_book(const char *filename);


#define SET_COMP_MODE(x) set_computing_mode(x)
#define CLR_TTABLE()     clr_ttable()
#define GET_TTABLE(x)    get_ttable(x)
#define SET_TTABLE(x,tt) set_ttable(x,tt)
#define CLR_TTABLE()     clr_ttable()
#define BOOK(x)          get_book(x)
