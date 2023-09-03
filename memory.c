#import "types.h"
#import "uart.h"
#import "memory.h"

// QEMU gives us a total of 0x3c000000 bytes of memory (3 gigs) starting at 0x00000000
// The next address after the stack is _end, so that's where it's safe to start allocating memory
// We'll also start alignment at a nice even address. I'll pick the 1MB boundary at 0x100000

#define SAFE_MEMORY_START           ((void *) 0x00100000)           // The 1MB mark

// We'll use bitmaps to keep track of allocated blocks for quick searches. A set bit indicates it's in use.

#define BITMAP_BYTES                128                             // 0x0400 bytes
#define BITMAP_DOUBLE_WORDS         (BITMAP_BYTES / 8)              // 0x0400 bytes
#define BITS_PER_BYTE               8
#define BITMAP_TOTAL_SIZE           (BITMAP_BYTES * BITS_PER_BYTE)  // 1024 records max

// We'll let our OS allocate things in three sizes: 64 bytes, 1024 bytes, and 16k bytes

#define SMALL_SIZE_BYTES            64
#define SMALL_SIZE_TOTAL            (64 * BITMAP_TOTAL_SIZE)        // 0x00010000 - 65,536 bytes

#define MEDIUM_SIZE_BYTES           1024
#define MEDIUM_SIZE_TOTAL           (1024 * BITMAP_TOTAL_SIZE)      // 0x00100000 - 1,048,576 bytes

#define LARGE_SIZE_BYTES            16384
#define LARGE_SIZE_TOTAL            (16384 * BITMAP_TOTAL_SIZE)     // 0x01000000 - 16,777,216 bytes

// Let's keep track of where we last saw free memory to speed up scans

static uint8 small_pool_first_bit = 0;     // The first double word a free page was last seen in, to save time
static uint8 medium_pool_first_bit = 0;    // 0xFF means we're full
static uint8 large_pool_first_bit = 0;

// Where the bitmaps exist in memory, starting at we've decided is a safe address

static uint64 * const small_memory_bitmap = SAFE_MEMORY_START;                         // 0x00100000
static uint64 * const medium_memory_bitmap = SAFE_MEMORY_START + BITMAP_TOTAL_SIZE;    // 0x00100400
static uint64 * const large_memory_bitmap = SAFE_MEMORY_START + BITMAP_TOTAL_SIZE * 2; // 0x00100800

// Where the actual allocation pools are, right after the bitmaps

static void * const small_pool_start = SAFE_MEMORY_START + BITMAP_TOTAL_SIZE * 3;      // 0x00100C00
static void * const medium_pool_start = SAFE_MEMORY_START + BITMAP_TOTAL_SIZE * 3      // 0x00200C00
                                                    + SMALL_SIZE_TOTAL;
static void * const large_pool_start = SAFE_MEMORY_START + BITMAP_TOTAL_SIZE * 3       // 0x01200C00
                                                    + SMALL_SIZE_TOTAL
                                                    + MEDIUM_SIZE_TOTAL;

// Local functions

static __attribute__((__noreturn__)) void panic_out_of_memory(uint16 size) {
    char error[] = {0, 0, 'N', 'o', ' ', 'm', 'e', 'm', 'o', 'r', 'y', ' ',
                            'o', 'f', ' ', 's', 'i', 'z' ,'e', ' ', 'l', 'e', 'f', 't', ' '};

    pstring *s = (pstring *) &error;
    s->length = sizeof(error);

    uart_send_string(s);

    uart_send_word_in_hex((uint32) size, true);

    while (true) {};
}

static __attribute__((__noreturn__)) void panic_bad_pointer(void *ptr) {
    char error[] = {0, 0, 'I', 'n', 'v', 'a', 'l', 'i', 'd', ' ',
                            'p', 'o', 'o', 'l', ' ', 'p', 'o', 'i', 'n', 't', 'e', 'r', ' '};

    pstring *s = (pstring *) &error;
    s->length = sizeof(error);

    uart_send_string(s);

    uart_send_word_in_hex((uint64) ptr >> 32, true);
    uart_send_word_in_hex((uint64) ptr & 0xFFFFFFFF, false);

    while (true) {};
}

static uint8 find_first_unset_bit_from_left(uint64 double_word) {
    // CLS counts how many bits match the high bit, so we need to check the high bit is set
    // If not then that means the first unset bit is the highest bit, 0 from left

    if ((double_word & 0x8000000000000000) == 0)
        return 0;

    uint64 result;

    asm ("cls %0, %1"
            : "=r" (result)
            : "r" (double_word));

    // CLS counts bits after the sign bit, so this will return 1 to 64

    return (uint8) result + 1;
}

static void zero_memory(void *start, uint16 size_divisble_by_8) {
    for (uint16 i = 0; i < size_divisble_by_8 / 8; i++)
        ((uint64 *) start)[i] = 0;
}

// Functions

void init_memory_pools() {
    // Clear all the bits in our tracking pools

    zero_memory((void *) small_memory_bitmap, BITMAP_BYTES);
    zero_memory((void *) medium_memory_bitmap, BITMAP_BYTES);
    zero_memory((void *) large_memory_bitmap, BITMAP_BYTES);
}

void free(void *ptr) {
    // Figure out which pool we're in

    uint64 *bitmap;
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

    uint16 bit_of_total = (uint64) offset / size;                       // 0 to 1023
    uint8 double_word_with_bit = bit_of_total / DOUBLE_WORD_BITS;       // 0 to 15

    if (double_word_with_bit < *first_bit) {
       *first_bit = double_word_with_bit;
    }

    // Free it in the bitmap

    uint8 bit_in_double_word = bit_of_total % DOUBLE_WORD_BITS;         // 0 to 63
    uint8 bit_from_left = DOUBLE_WORD_BITS - 1 - bit_in_double_word;    // Now 63 to 0

    bitmap[double_word_with_bit] &= ~(1 << bit_from_left);
}

void *allocate(uint16 size) {
    // Find the size class

    uint64 *bitmap;
    void *pool;
    uint8 *pool_first_bit;

    if (size <= SMALL_SIZE_BYTES && small_pool_first_bit != 0xFF) {
        bitmap = (uint64 *) small_memory_bitmap;
        pool = small_pool_start;
        pool_first_bit = &small_pool_first_bit;
        size = SMALL_SIZE_BYTES;
    } else if (size <= MEDIUM_SIZE_BYTES && medium_pool_first_bit != 0xFF) {
        bitmap = (uint64 *) medium_memory_bitmap;
        pool = medium_pool_start;
        pool_first_bit = &medium_pool_first_bit;
        size = MEDIUM_SIZE_BYTES;
    } else if (size <= LARGE_SIZE_BYTES && large_pool_first_bit != 0xFF) {
        bitmap = (uint64 *) large_memory_bitmap;
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
        clear_bit_from_left = find_first_unset_bit_from_left(bitmap[i]);    // 0 to 63 if found, 64 if not

        if (clear_bit_from_left != DOUBLE_WORD_BITS) {
            double_word_with_clear_bit = i;
            break;
        }
    }

    if (clear_bit_from_left == DOUBLE_WORD_BITS) {
        // Shouldn't get here. If we do it's a bug, we'll use a sentinel value
        panic_out_of_memory(0xDEAD);
    }

    // Calculate the bit number for the shift, then set it in the bitmap

    uint8 bit_from_right = DOUBLE_WORD_BITS - clear_bit_from_left - 1;      // Now 63 to 0

    bitmap[double_word_with_clear_bit] |= (1 << bit_from_right);

    // Either update first_pool_bit or mark that we're full

    if (bitmap[double_word_with_clear_bit] == ~0 && double_word_with_clear_bit == BITMAP_DOUBLE_WORDS - 1) {
        // We were in the last double word and set the last bit (~0 = all Fs)
        *pool_first_bit = 0xFF;     // Mark we're full
    } else {
        // Things aren't full, record this double word as where the last clear bit was seen
        *pool_first_bit = double_word_with_clear_bit;
    }

    // Adjust our offset from double word relative, to full pool relative

    clear_bit_from_left += DOUBLE_WORD_BITS * double_word_with_clear_bit;

    void *address = pool + (clear_bit_from_left * size);

    // Zero and return

    zero_memory(address, size);

    return address;
}
