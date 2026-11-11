/* mem: the memory of the editor, from the C library (malloc) */
#ifndef MEM_H
#define MEM_H

#include <stddef.h>

void *mem_alloc(size_t n);
void mem_free(void *p);
char *mem_strdup(const char *s);

#endif
