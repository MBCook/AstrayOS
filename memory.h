#include "types.h"

#ifndef __memory_h__
#define	__memory_h__

#define MINIMUM_ALLOCATION_BYTES        64

// Initializes the dynamic kernel memory subsystem
void init_memory_pools();

// Frees the memory pointed to and nulls out the pointer
void free(void **ptr);

// Allocates the requested mount of memory, panics on failure, returning a pointer
void *allocate(uint16 size);

// Increases the size of the block the requested mount of memory, panics on failure, updates the pointer
void reallocate(void **ptr, uint16 size);

// Peek under the covers are return just how big the block of memory is
uint16 memory_block_size(void *ptr);

// Zeros a block of memory
void zero_memory(void *prt, uint16 size);

// Copies the size bytes from src to dest
void copy_memory(void *src, void *dest, uint16 size);

#endif
