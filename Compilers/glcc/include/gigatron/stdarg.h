#ifndef __STDARG
#define __STDARG

#if !defined(_SIZE_T) && !defined(_SIZE_T_) && !defined(_SIZE_T_DEFINED)
#define _SIZE_T
#define _SIZE_T_
#define _SIZE_T_DEFINED
typedef unsigned int size_t;
#endif

#if !defined(_VA_LIST) && !defined(_VA_LIST_DEFINED)
#define _VA_LIST
#define _VA_LIST_DEFINED
typedef char *__va_list;
#endif

typedef __va_list va_list;

#define __va_alignof(mode)\
  ((size_t)&(((struct{char c; mode m;}*)(0))->m))

#define __va_arg_ptr(list, mode, n)\
	&((list=(__va_list)(((size_t)list+(sizeof(mode)+(n)))&~(n)))[-(int)sizeof(mode)])

#define va_start(list, start)\
  ((void)((list)=(__va_list)&((&start)[1])))

#define va_arg(list, mode)\
  (*(mode*)((__typecode(mode)==5 && sizeof(mode)==1) ? __va_arg_ptr(list,int,1U) : \
            (__typecode(mode)==6 && sizeof(mode)==1) ? __va_arg_ptr(list,unsigned,1U) : \
            (__va_arg_ptr(list,mode,__va_alignof(mode)-1)) ))

#define va_end(list)\
  ((void) 0)

#endif
