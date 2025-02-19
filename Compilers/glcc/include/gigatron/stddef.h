#ifndef __STDDEF
#define __STDDEF

/* $Id$ */

#ifndef NULL
#define NULL ((void*)0)
#endif
#ifndef offsetof
#define offsetof(ty,mem) ((size_t)((char*)&((ty*)0)->mem - (char*)0))
#endif
#ifndef alignof
#define alignof(ty) ((size_t)&(((struct{char c; ty m;}*)(0))->m))
#endif

typedef int ptrdiff_t;

#if !defined(_SIZE_T) && !defined(_SIZE_T_) && !defined(_SIZE_T_DEFINED)
#define _SIZE_T
#define _SIZE_T_
#define _SIZE_T_DEFINED
typedef unsigned int size_t;
#endif

#if !defined(_WCHAR_T) && !defined(_WCHAR_T_) && !defined(_WCHAR_T_DEFINED)
#define _WCHAR_T
#define _WCHAR_T_
#define _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#endif

#endif /* __STDDEF */
