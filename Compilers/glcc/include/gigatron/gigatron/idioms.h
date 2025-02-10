#ifndef __GIGATRON_IDIOMS
#define __GIGATRON_IDIOMS


/* The compiler recognizes these idioms and
   produce smarter code when they are used. */


/* Internal (implementation) */
#define _l2l(w) ((char)(unsigned)(w))
#define _h2l(w) ((char)(((unsigned)(w))>>8))
#define _l2h(w) (((unsigned)(char)(w))<<8)
#define _h2h(w) (((unsigned)(w))&0xff00u)


/* Return the high of an integer or pointer */
#define hi(w) _h2l(w)

/* Return the low of an integer or pointer */
#define lo(w) _l2l(w)

/* Make a word using the specified high and low bytes */
#define makew(hi,lo)   (_l2h(hi)+_l2l(lo))
/* Make a pointer using the specified high and low bytes */
#define makep(hi,lo)   ((void*)makew(hi,lo))

/* Make a word with the high byte of hi and the low byte of lo */
#define mashw(hh,lo)   (_h2h(hh)+_l2l(lo))

/* Make a pointer using the specified high and low bytes */
#define mashp(hh,lo)   ((void*)mashw(hh,lo))

/* Copy the low of high byte of `v` into `var` */
#define copylo(var,v)  do {(var)=(__typeof(var))(_h2h(var)+_l2l(v));} while(0)
#define copyhi(var,v)  do {(var)=(__typeof(var))(_l2l(var)+_h2h(v));} while(0)

/* Make the low or high byte of `var` equal to the low byte of `v`. */
#define setlo(var,v)   do {(var)=(__typeof(var))(_h2h(var)+_l2l(v));} while(0)
#define sethi(var,v)   do {(var)=(__typeof(var))(_l2l(var)+_l2h(v));} while(0)

#endif
