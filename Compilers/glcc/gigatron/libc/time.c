#include <stdlib.h>
#include <time.h>

time_t time(time_t *tloc)
{
	time_t t = 0;
	if (tloc)
		*tloc = t;
	return t;
}
