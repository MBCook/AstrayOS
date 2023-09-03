#import "../types.h"
#import <stdio.h>
#import <stdlib.h>

// We're going to test a copy of the memory code. It will use far less memory for simplicity but keep the ideas.

#define BITMAP_BYTES                8                              // Two words
#define BITMAP_DOUBLE_WORDS         (BITMAP_BYTES / 4)
#define BITS_PER_BYTE               8
#define BITMAP_TOTAL_SIZE           (BITMAP_BYTES * BITS_PER_BYTE)

#define SMALL_SIZE_BYTES            64
#define SMALL_SIZE_TOTAL            (64 * BITMAP_TOTAL_SIZE)

#define MEDIUM_SIZE_BYTES           1024
#define MEDIUM_SIZE_TOTAL           (1024 * BITMAP_TOTAL_SIZE)

#define LARGE_SIZE_BYTES            13184
#define LARGE_SIZE_TOTAL            (13184 * BITMAP_TOTAL_SIZE)

#define AMOUNT_TO_ALLOCATE          (2^20 * 4)                      // 4 megs will hold everything we need

uint8 small_pool_first_bit = 0;     // The first double word a free page was last seen in, to save time
uint8 medium_pool_first_bit = 0;    // 0xFF means we're full
uint8 large_pool_first_bit = 0;

uint32 * small_memory_bitmap = 0;
uint32 * medium_memory_bitmap = 0;
uint32 * large_memory_bitmap = 0;

void * small_pool_start = 0;
void * medium_pool_start = 0;
void * large_pool_start = 0;

void *from_malloc = 0;

void print_binary_int(uint32 num) {
    for (int i = 31; i >= 0; i--) {
        printf("%u", (num & (1 << i)) >> i);
    }
}

void print_pool_allocations(uint32 *bitmap, uint32 first_bit) {
    printf("Pool: ");
    print_binary_int(bitmap[0]);
    printf(" ");
    print_binary_int(bitmap[1]);
    printf(" - %u", first_bit);
    printf("\n");
}

__attribute__((__noreturn__)) void panic_out_of_memory(uint16 size) {
    printf("\n\nUnable to allocate %d bytes\n\n", size);
    exit(1);
}

__attribute__((__noreturn__)) void panic_bad_pointer(void *ptr) {
    printf("\n\n\nAsked to free bad pointer: %p\n\n", ptr);
    exit(1);
}

uint8 find_first_unset_bit_from_left(uint32 double_word) {
    // CLS counts how many bits match the high bit, so we need to check the high bit is set
    // If not then that means the first unset bit is the highest bit, 0 from left

    if ((double_word & 0x80000000) == 0)
        return 0;

    uint32 result;

    asm ("cls %w0, %w1"
            : "=r" (result)
            : "r" (double_word));

    // CLS counts bits after the sign bit, so this will return 1 to 32

    return (uint8) result + 1;
}

void zero_memory(void *start, uint16 size_divisble_by_4) {
    for (uint16 i = 0; i < size_divisble_by_4 / 4; i++)
        ((uint32 *) start)[i] = 0;
}

// Functions

void init_memory_pools() {
    zero_memory((void *) small_memory_bitmap, BITMAP_BYTES);
    zero_memory((void *) medium_memory_bitmap, BITMAP_BYTES);
    zero_memory((void *) large_memory_bitmap, BITMAP_BYTES);
}

void my_free(void *ptr) {
    // Figure out which pool we're in

    uint32 *bitmap;
    void *offset;
    uint16 size;
    uint8 *first_bit;

    if (ptr >= small_pool_start && ptr < medium_pool_start) {
        offset = (void *) (ptr - small_pool_start);
        bitmap = (void *) small_memory_bitmap;
        size = SMALL_SIZE_BYTES;
        first_bit = &small_pool_first_bit;
    } else if (ptr >= medium_pool_start && ptr < large_pool_start) {
        offset = (void *) (ptr - medium_pool_start);
        bitmap = (void *) medium_memory_bitmap;
        size = MEDIUM_SIZE_BYTES;
        first_bit = &medium_pool_first_bit;
    } else if (ptr >= large_pool_start && ptr < large_pool_start + LARGE_SIZE_TOTAL) {
        offset = (void *) (ptr - large_pool_start);
        bitmap = (void *) large_memory_bitmap;
        size = LARGE_SIZE_BYTES;
        first_bit = &large_pool_first_bit;
    } else {
       panic_bad_pointer(ptr);
    }

    // Figure out which bit of the bitmap the page was, zero to the right

    uint16 bit_of_total = (uint32)(uintptr_t) offset / size;    // 0 to 16 * 2
    uint8 double_word_with_bit = bit_of_total / 32;             // 0 to 2

    if (double_word_with_bit < *first_bit) {
       *first_bit = double_word_with_bit;
    }

    // Free it in the bitmap

    uint8 bit_in_double_word = bit_of_total % 32;         // 0 to 31
    uint8 bit_from_left = 32 - 1 - bit_in_double_word;    // Now 31 to 0

    bitmap[double_word_with_bit] &= ~(1 << bit_from_left);
}

void *allocate(uint16 size) {
    // Find the size class

    uint32 *bitmap;
    void *pool;
    uint8 *pool_first_bit;

    if (size <= SMALL_SIZE_BYTES && small_pool_first_bit != 0xFF) {
        bitmap = (uint32 *) small_memory_bitmap;
        pool = small_pool_start;
        pool_first_bit = &small_pool_first_bit;
        size = SMALL_SIZE_BYTES;
    } else if (size <= MEDIUM_SIZE_BYTES && medium_pool_first_bit != 0xFF) {
        bitmap = (uint32 *) medium_memory_bitmap;
        pool = medium_pool_start;
        pool_first_bit = &medium_pool_first_bit;
        size = MEDIUM_SIZE_BYTES;
    } else if (size <= LARGE_SIZE_BYTES && large_pool_first_bit != 0xFF) {
        bitmap = (uint32 *) large_memory_bitmap;
        pool = large_pool_start;
        pool_first_bit = &large_pool_first_bit;
        size = LARGE_SIZE_BYTES;
    } else {
        panic_out_of_memory(size);
    }

    // Find a free block

    uint8 double_word_with_clear_bit = 0xFF;
    uint16 clear_bit_from_left;

    for (uint8 i = *pool_first_bit; i < BITMAP_DOUBLE_WORDS; i++) {
        clear_bit_from_left = find_first_unset_bit_from_left(bitmap[i]);    // 0 to 31 if found, 32 if not

        if (clear_bit_from_left != 32) {
            double_word_with_clear_bit = i;
            break;
        }
    }

    if (clear_bit_from_left == 32) {
        // Shouldn't get here. If we do it's a bug, we'll use a sentinel value
        panic_out_of_memory(0xDEAD);
    }

    // Calculate the bit number for the shift, then set it in the bitmap

    uint8 bit_from_right = 32 - clear_bit_from_left - 1;      // Now 31 to 0

    bitmap[double_word_with_clear_bit] |= (1 << bit_from_right);

    // Either update first_pool_bit or mark that we're full

    if (bitmap[double_word_with_clear_bit] == ~0 && double_word_with_clear_bit == BITMAP_DOUBLE_WORDS - 1) {
        // We were in the last double word and set the last bit (~0 = all Fs)
        *pool_first_bit = 0xFF;      // Mark we're full
    } else {
        // Things aren't full, record this double word as where the last clear bit was seen
        *pool_first_bit = double_word_with_clear_bit;
    }

    // Adjust our offset from double word relative, to full pool relative

    clear_bit_from_left += 32 * double_word_with_clear_bit;

    void *address = pool + (clear_bit_from_left * size);

    // Zero and return

    zero_memory(address, size);

    return address;
}

//////

void test_allocations_from_empty() {
    void *one = 0;
    void *two = 0;
    void *three = 0;
    void *four = 0;

    printf("\nPrinting initial pool state\n");

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);

    printf("\nAllocating one... ");

    one = allocate(64);

    printf("%p\n", one);

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);

    printf("\nAllocating two... ");

    two = allocate(64);

    printf("%p\n", two);

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);

    printf("\nAllocating three... ");

    three = allocate(64);

    printf("%p\n", three);

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);

    printf("\nFreeing one...\n");

    my_free(one);

    one = 0;

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);

    printf("\nAllocating one... ");

    one = allocate(64);

    printf("%p\n", one);

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);

    printf("\nFreeing two...\n");

    my_free(two);

    two = 0;

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);

    printf("\nAllocating two... ");

    two = allocate(64);

    printf("%p\n", two);

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);

    printf("\nAllocating four... ");

    four = allocate(64);

    printf("%p\n", four);

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);
}

void test_allocations_crossing_boundary() {
    void *one = 0;
    void *two = 0;
    void *three = 0;
    void *four = 0;

    printf("\nPrinting initial pool state\n");

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);

    printf("\nSimulating area one almost full");

    small_memory_bitmap[0] = 0xFFFFFFFE;

    printf("\n");

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);

    printf("\nAllocating one... ");

    one = allocate(64);

    printf("%p\n", one);

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);

    printf("\nAllocating two... ");

    two = allocate(64);

    printf("%p\n", two);

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);

    printf("\nAllocating three... ");

    three = allocate(64);

    printf("%p\n", three);

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);

    printf("\nFreeing one...\n");

    my_free(one);

    one = 0;

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);

    printf("\nAllocating one... ");

    one = allocate(64);

    printf("%p\n", one);

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);

    printf("\nFreeing two...\n");

    my_free(two);

    two = 0;

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);

    printf("\nAllocating two... ");

    two = allocate(64);

    printf("%p\n", two);

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);

    printf("\nAllocating four... ");

    four = allocate(64);

    printf("%p\n", four);

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);
}

void test_allocations_crossing_sizes() {
    void *one = 0;
    void *two = 0;
    void *three = 0;
    void *four = 0;

    printf("\nPrinting initial pool state\n");

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);
    print_pool_allocations(medium_memory_bitmap, medium_pool_first_bit);

    printf("\nSimulating small size almost full");

    small_memory_bitmap[0] = 0xFFFFFFFF;
    small_memory_bitmap[1] = 0xFFFFFFFE;

    small_pool_first_bit = 1;

    printf("\n");

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);
    print_pool_allocations(medium_memory_bitmap, medium_pool_first_bit);

    printf("\nAllocating one... ");

    one = allocate(64);

    printf("%p\n", one);

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);
    print_pool_allocations(medium_memory_bitmap, medium_pool_first_bit);

    printf("\nAllocating two... ");

    two = allocate(64);

    printf("%p\n", two);

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);
    print_pool_allocations(medium_memory_bitmap, medium_pool_first_bit);

    printf("\nAllocating three... ");

    three = allocate(64);

    printf("%p\n", three);

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);
    print_pool_allocations(medium_memory_bitmap, medium_pool_first_bit);

    printf("\nFreeing one...\n");

    my_free(one);

    one = 0;

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);
    print_pool_allocations(medium_memory_bitmap, medium_pool_first_bit);

    printf("\nAllocating one... ");

    one = allocate(64);

    printf("%p\n", one);

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);
    print_pool_allocations(medium_memory_bitmap, medium_pool_first_bit);

    printf("\nFreeing two...\n");

    my_free(two);

    two = 0;

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);
    print_pool_allocations(medium_memory_bitmap, medium_pool_first_bit);

    printf("\nAllocating two... ");

    two = allocate(64);

    printf("%p\n", two);

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);
    print_pool_allocations(medium_memory_bitmap, medium_pool_first_bit);

    printf("\nAllocating four... ");

    four = allocate(64);

    printf("%p\n", four);

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);
    print_pool_allocations(medium_memory_bitmap, medium_pool_first_bit);
}

void test_allocations_when_full() {
    printf("\nPrinting initial pool state\n");

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);
    print_pool_allocations(medium_memory_bitmap, medium_pool_first_bit);
    print_pool_allocations(large_memory_bitmap, large_pool_first_bit);

    printf("\nSimulating all memory but the last spot is full");

    small_memory_bitmap[0] = 0xFFFFFFFF;
    small_memory_bitmap[1] = 0xFFFFFFFF;
    medium_memory_bitmap[0] = 0xFFFFFFFF;
    medium_memory_bitmap[1] = 0xFFFFFFFF;
    large_memory_bitmap[0] = 0xFFFFFFFF;
    large_memory_bitmap[1] = 0xFFFFFFFE;

    small_pool_first_bit = 0xFF;
    medium_pool_first_bit = 0xFF;
    large_pool_first_bit = 1;

    printf("\nAllocating one... ");

    void *one = allocate(64);

    printf("%p\n", one);

    print_pool_allocations(small_memory_bitmap, small_pool_first_bit);
    print_pool_allocations(medium_memory_bitmap, medium_pool_first_bit);
    print_pool_allocations(large_memory_bitmap, large_pool_first_bit);

    printf("\nAllocating two, this should panic... ");

    void *two = allocate(64);

    // Should never get here

    printf("\nBUG, you shouldn't see this: %p\n", two);
}

void test_freeing_invalid_address() {
    printf("\nTesting freeing a pointer that is not ours\n");

    printf("\nFreeing 0x0000000000000BAD, should panic...");

    my_free((void *) 0x0000000000000BAD);

    printf("\nBUG, you shouldn't see this\n");
}

int main() {
    printf("Getting memory\n");

    // First allocate memory to play with

    from_malloc = malloc(AMOUNT_TO_ALLOCATE);

    if (from_malloc == 0) {
        printf("Can't get memory\n");
        exit(1);
    }

    // Setup variables

    small_memory_bitmap = from_malloc;
    medium_memory_bitmap = from_malloc + BITMAP_TOTAL_SIZE;
    large_memory_bitmap = from_malloc + BITMAP_TOTAL_SIZE * 2;

    small_pool_start = from_malloc + BITMAP_TOTAL_SIZE * 3;
    medium_pool_start = from_malloc + BITMAP_TOTAL_SIZE * 3
                                    + SMALL_SIZE_TOTAL;
    large_pool_start = from_malloc + BITMAP_TOTAL_SIZE * 3
                                          + SMALL_SIZE_TOTAL
                                          + MEDIUM_SIZE_TOTAL;

    // Initialize the pool

    printf("Initializing pool\n");

    init_memory_pools();

    // Run a test (only run one at a time)

//    test_allocations_from_empty();
//    test_allocations_crossing_boundary();
//    test_allocations_crossing_sizes();
//    test_allocations_when_full();
    test_freeing_invalid_address();

    // Done

    printf("\nFreeing memory\n");

    free(from_malloc);

    printf("\n");
}
