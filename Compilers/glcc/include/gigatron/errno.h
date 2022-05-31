#ifndef __ERRNO
#define __ERRNO

extern int errno;

/* ANSI C errors */
#define EDOM	 1
#define ERANGE	 2

/* Other errors */
#define EINVAL    3
#define ENOENT    4
#define ENOTDIR   5
#define ENFILE    6
#define ENOMEM    7
#define EIO       8
#define EPERM     9
#define ENOTSUP  10

#endif /* __ERRNO */
