#include "types.h"

#ifndef __memory_h__
#define	__memory_h__

void free(void **ptr);
void *allocate(uint16 size);
void init_memory_pools();
void zero_memory(void *start, uint16 size);
void copy_memory(void *src, void *dest, uint16 size);

#endif
