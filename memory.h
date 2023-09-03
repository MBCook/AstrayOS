#include "types.h"

#ifndef __memory_h__
#define	__memory_h__

// Initializes the dynamic kernel memory subsystem
void init_memory_pools();

// Frees the memory pointed to and nulls out the pointer
void free(void **ptr);

// Allocates the requested mount of memory, panics on failure, returning a pointer
void *allocate(uint16 size);

// Zeros a block of memory
void zero_memory(void *prt, uint16 size);

// Copies the size bytes from src to dest
void copy_memory(void *src, void *dest, uint16 size);

#endif
