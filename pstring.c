#import "types.h"
#import "memory.h"
#import "uart.h"

// Local functions

static __attribute__((__noreturn__)) void panic_string_too_big() {
    uart_send_string(pstring_from_cstring("Requested string too big"));

    while (true) {};
}

// Functions

pstring *empty_pstring(uint16 length) {
    // Ensure it will fit in our allocation budget

    if (length >= 0xFFFF - 2)
        panic_string_too_big();

    // Returns an empty string with a length of 0.
    // It's the caller's responsibility to remember the max size.

    return (pstring *) allocate(length + 2);
}

pstring *pstring_from_cstring(char data[]) {
    // Figure out how long the C string is

    uint16 length = 0;

    while (data[length] != 0x00)
        length++;

    // Ensure it will fit in our allocation budget

    if (length >= 0xFFFF - 2)
        panic_string_too_big();

    // Add 2 to the length so it's the final size of the structure

    length += 2;

    pstring *result = allocate(length);

    result->length = length;

    // Copy the string data minus (our length does not include the null byte)

    copy_memory((void *) data, (void *) &result->data, length - 2);

    return result;
}

pstring *append_pstrings(pstring *one, pstring *two) {
    // Figure out how things will be. Total sizes of the two minus two.
    // One and two both have 2 bytes to hold a length, but that's two more than we'll need.

    uint32 length = (uint32) one->length + (uint32) two->length - 2;

    // Ensure it will fit in our allocation budget

    if (length >= 0x0000FFFF)
        panic_string_too_big();

    // Allocate, setup, and copy

    pstring *result = allocate(length);

    result->length = length;

    // Copy the string data minus (our length does not include the null byte)

    copy_memory((void *) ((uint64) one + 2), (void *) ((uint64) &result->data), one->length - 2);
    copy_memory((void *) ((uint64) two + 2), (void *) (((uint64) &result->data) + one->length - 2), two->length - 2);

    return result;
}
