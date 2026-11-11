#include <stdlib.h>
#include <string.h>
#include "mem.h"

void *
mem_alloc(size_t n)
{
	return malloc(n ? n : 1);
}

void
mem_free(void *p)
{
	free(p);
}

char *
mem_strdup(const char *s)
{
	size_t n = strlen(s) + 1;
	char *p = mem_alloc(n);

	if(p)
		memcpy(p, s, n);
	return p;
}
