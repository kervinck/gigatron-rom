#ifndef __ASSERT
#define __ASSERT

void assert(int);

#endif /* __ASSERT */

#undef assert
#ifdef NDEBUG
#define assert(ignore) ((void)0)
#else
extern int _assertfail(const char *);
#define _assertfail2(e,f,l) _assertfail(f ":" #l ": Assertion " e " failed.")
#define _assertfail1(e,f,l) _assertfail2(e, f, l)
#define assert(e) ((void)((e)||_assertfail1(#e, __FILE__, __LINE__)))
#endif /* NDEBUG */
