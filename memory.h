#include "types.h"

#ifndef __memory_h__
#define	__memory_h__

void free(void *ptr);
void *allocate(uint16 size);
void init_memory_pools();

#endif
