#import "types.h"
#import "uart.h"
#import "memory.h"

// QEMU gives us a total of 0x3c000000 bytes of memory (3 gigs) starting at 0x00000000
// The next address after the stack is _end, so that's where it's safe to start allocating memory
// We'll also start alignment at a nice even address. I'll pick the 1MB boundary at 0x100000

#define SAFE_MEMORY_START           ((void *) 0x00100000)           // The 1MB mark

// We'll use bitmaps to keep track of allocated blocks for quick searches

#define BITMAP_BYTES                128                             // 0x0400 bytes
#define BITS_PER_BYTE               8
#define BITMAP_TOTAL_SIZE           (BITMAP_BYTES * BITS_PER_BYTE)  // 1024 records max

// We'll let our OS allocate things in three sizes: 64 bytes, 1024 bytes, and 16k bytes

#define SMALL_SIZE_BYTES            64
#define SMALL_SIZE_TOTAL            (64 * BITMAP_TOTAL_SIZE)        // 0x00010000 - 65,536 bytes

#define MEDIUM_SIZE_BYTES           1024
#define MEDIUM_SIZE_TOTAL           (1024 * BITMAP_TOTAL_SIZE)      // 0x00100000 - 1,048,576 bytes

#define LARGE_SIZE_BYTES            16384;
#define LARGE_SIZE_TOTAL            (16384 * BITMAP_TOTAL_SIZE)     // 0x01000000 - 16,777,216 bytes

// Let's keep track of where we last saw free memory to speed up scans

uint8 small_pool_first_bit = 0;     // The double word (so out of 128 / 8 which is 16) a free page was last seen in
uint8 medium_pool_first_bit = 0;    // 0xFF means we're full
uint8 large_pool_first_bit = 0;

// Where the bitmaps exist in memory, starting at we've decided is a safe address

const void* small_memory_bitmap = SAFE_MEMORY_START;                            // 0x00100000
const void* medium_memory_bitmap = SAFE_MEMORY_START + BITMAP_TOTAL_SIZE;       // 0x00100400
const void* large_memory_bitmap = SAFE_MEMORY_START + BITMAP_TOTAL_SIZE * 2;    // 0x00100800

// Where the actual allocation pools are, right after the bitmaps

const void* small_pool_start = SAFE_MEMORY_START + BITMAP_TOTAL_SIZE * 3;       // 0x00100C00
const void* medium_pool_start = SAFE_MEMORY_START + BITMAP_TOTAL_SIZE * 3       // 0x00200C00
                                                    + SMALL_SIZE_TOTAL;
const void* large_pool_start = SAFE_MEMORY_START + BITMAP_TOTAL_SIZE * 3        // 0x01200C00
                                                    + SMALL_SIZE_TOTAL
                                                    + MEDIUM_SIZE_TOTAL;

// Functions

void free(void *ptr) {
    // Figure out which pool we're in

    void *bitmap;
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

    uint16 bit_of_total = (uint64) offset / size;       // 0 to 1023
    uint8 double_word_with_bit = bit_of_total / 64;     // 0 to 15

    if (double_word_with_bit < *first_bit) {
       *first_bit = double_word_with_bit;
    }

    // Free it in the bitmap

    uint64 *double_word = bitmap + double_word_with_bit;
    uint8 bit_in_double_word = bit_of_total / 8;                        // 0 to 127

    double_word = (void *) ((uint64) double_word & ~(1 << (127 - bit_in_double_word)));     // Now 127 to 0
}

void *allocate(uint32 size) {
    // Remember to zero it out!

    return null;
}

void panic_out_of_memory(uint32 size) {
    char error[] = {0, 0, 'N', 'o', ' ', 'm', 'e', 'm', 'o', 'r', 'y', ' ',
                            'o', 'f', ' ', 's', 'i', 'z' ,'e', ' ', 'l', 'e', 'f', 't', ' '};

    pstring *s = (pstring *) &error;
    s->length = sizeof(error);

    uart_send_string(s);

    uart_send_word_in_hex(size, true);

    while (true) {};
}

void panic_bad_pointer(void *ptr) {
    char error[] = {0, 0, 'I', 'n', 'v', 'a', 'l', 'i', 'd', ' ',
                            'p', 'o', 'o', 'l', ' ', 'p', 'o', 'i', 'n', 't', 'e', 'r', ' '};

    pstring *s = (pstring *) &error;
    s->length = sizeof(error);

    uart_send_string(s);

    uart_send_word_in_hex((uint64) ptr >> 32, true);
    uart_send_word_in_hex((uint64) ptr & 0xFFFFFFFF, false);

    while (true) {};
}