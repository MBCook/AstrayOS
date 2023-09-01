#include "types.h"

#ifndef __memory_h__
#define	__memory_h__

void free(void *ptr);
void *allocate(uint32 size);
void panic_out_of_memory(uint32 size) __attribute__((__noreturn__));
void panic_bad_pointer(void *ptr) __attribute__((__noreturn__));

#endif
