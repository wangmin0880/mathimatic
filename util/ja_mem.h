#ifndef __JA_MEMORY__
#define __JA_MEMORY__

#include "stdlib.h"
static inline void *ja_malloc(unsigned int size, unsigned int flags)
{
	return malloc(size);
}
static inline void ja_free(void *p)
{
	if (p)
		free(p);
	p = NULL;
}

#endif
