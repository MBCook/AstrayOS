#import "types.h"
#import "memory.h"
#import "uart.h"

// Local functions

static __attribute__((__noreturn__)) void panic_string_too_big() {
    uart_send_string(string_from_cstring("Requested string too big"));

    while (true) {};
}

static __attribute__((__noreturn__)) void panic_string_out_of_range() {
    uart_send_string(string_from_cstring("Destination string wasn't big enough"));

    while (true) {};
}

// Functions

string *empty_string(uint16 length) {
    // Ensure it will fit in our allocation budget

    if (length >= 0xFFFF - 2)
        panic_string_too_big();

    // Returns an empty string with a length of 0.
    // It's the caller's responsibility to remember the max size.

    string *result = allocate(length + 2);

    result->size = 2;       // Always starts at 2 for the two bytes used by the size field

    return result;
}

void expand_string(string **str, uint16 length) {
    // The struct only keep track of how long the current contents are, not how much is allocated.
    // If the current contents are long enough, there is no need to allocate more memory.

    if (length <= (*str)->size - 2)
        return;

    // We're assuming the existing memory allocation wasn't big enough to hold length.
    // If it was big enough then we're wasting cycles.

    string *longer = empty_string(length);

    longer->size = (*str)->size;

    copy_string(*str, 0, length, longer, 0);

    // Free the existing string, we don't need it any more, update the str pointer

    free((void **) str);

    *str = longer;
}

string *string_from_cstring(char data[]) {
    // Figure out how long the C string is

    uint32 size = 0;

    while (data[size] != 0x00)
        size++;

    // Size = length of string, add two to cover the size field in the string struct

    size += 2;

    // Ensure it will fit in our allocation budget

    if (size >= 0xFFFF)
        panic_string_too_big();

    string *result = allocate(size);

    result->size = size;

    // Copy the string's bytes (we didn't count the terminating null)

    copy_memory((void *) data, (void *) &result->data, size - 2);

    return result;
}

string *append_strings(string *one, string *two) {
    // Figure out how big things will be. Total sizes of the two minus two.
    // One and two both have 2 bytes to hold a size, but that's two more than we'll need.

    uint32 size = (uint32) one->size + (uint32) two->size - 2;

    // Ensure it will fit in our allocation budget

    if (size >= 0x0000FFFF)
        panic_string_too_big();

    // Allocate, setup, and copy

    string *result = allocate(size);

    result->size = size;

    // Copy the string data (size field - 2 = string length)

    copy_string(one, 0, one->size - 2, result, 0);
    copy_string(two, 0, two->size - 2, result, one->size - 2);

    return result;
}

string *substring(string *src, uint16 start, uint16 length) {
    // Ensure it will fit in our allocation budget

    if (length >= 0xFFFF - 2)
        panic_string_too_big();

    // Allocate it and copy the requsted data in

    string *result = empty_string(length + 2);

    result->size = length + 2;

    copy_string(src, start, length, result, 0);

    return result;
}

void copy_string(string *src, uint16 src_start, uint16 length, string *dest, uint16 dest_start) {
    // Make sure it will fit

    if ((uint32) dest_start + (uint32) length > dest->size - 2)
        panic_string_out_of_range();

    // Copy the requested data across

    copy_memory((void *) ((uint64) &src->data + src_start), (void *) ((uint64) &dest->data + dest_start), length);
}
