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

static void output_unsigned_integer(uint8 **buffer, uint64 number, uint16 * buffer_index, uint16 *buffer_size) {
    // Let's count up how many digits we'll need

    uint8 needed = 0;

    for (uint64 working = number; working > 0; working = working / 10)
        needed++;

    if (needed == 0)
        needed = 1;     // The number was 0, we still need to show the zero

    // Allocate a buffer we can use as a stack of digits

    uint8 *stack = allocate(needed);

    uint64 working = number;

    for (uint8 i = 0; i < needed; i++) {
        stack[i] = working % 10 + '0';
        working = working / 10;
    }

    // Make sure that will fit in our destination

    if (*buffer_index + needed >= *buffer_size) {
        // Double the buffer size

        *buffer_size = *buffer_size * 2;
        reallocate((void **) buffer, *buffer_size);
    }

    // Now we can copy our digits into the buffer

    for (uint8 i = needed; i > 0; i--) {
        (*buffer)[(*buffer_index)++] = stack[i - 1];
    }

    free((void **) &stack);
}

static uint8 bytes_to_display(uint64 number) {
    // Figure out how many bytes to show as hex or binary

    if (number >> 32 > 0) {
        // Something in the top 32 bits (out of 64) set, so 8 bytes
        return 8;
    } else if (number >> 16 > 0) {
        // Something in the next 16 bits (top two bytes of 32) set, so 4 bytes
        return 4;
    } else if (number >> 8 > 0) {
        // Something in the next 8 bits (top byte of 16) set, so 2 bytes
        return 2;
    } else {
        // It's a byte or zero which we always show as one byte
        return 1;
    }
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

    // Reallocate memory (it knows if the block is big enough

    reallocate((void **) str, length + 2);
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

string *format_string(string *format_string, ...) {
    __builtin_va_list arguments;
    __builtin_va_start(arguments, format_string);

    // Setup a buffer we can work in. We'll start with our minimum allocate size for simplicity.

    uint16 buffer_size = MINIMUM_ALLOCATION_BYTES;
    uint16 buffer_index = 2;                // Buffer will become a string, reserve space for the size field

    // Make working with out place in the format easy

    uint8 *format = (uint8 *) &format_string->data;
    uint16 format_index = 0;

    // The buffer we're working in

    uint8 *buffer = allocate(buffer_size);

    while(format_index < format_string->size - 2) {
        // Ensure the buffer is big enough

        if (buffer_index + 1 >= buffer_size) {
            // Double the buffer size

            buffer_size = buffer_size * 2;
            reallocate((void **) &buffer, buffer_size);
        }

        // See what character is next

        if (format[format_index] != '%') {
            // It's normal, copy straight across
            buffer[buffer_index++] = format[format_index++];
        } else {
            // We're doing a format string, so get the next character

            format_index += 1;  // To eat up the % sign used for escaping format specifiers

            uint8 specifier = format[format_index++];

            if (specifier == '%') {
                // Just an escaped percent sign, stick it in the buffer

                buffer[buffer_index++] = '%';
            } else if (specifier == 'c') {
                // Just a character, it too goes straight in

                int thing = __builtin_va_arg(arguments, int);

                buffer[buffer_index++] = (char) thing;
            } else if (specifier == 'u') {
                // Integer, non-negative

                uint64 number = __builtin_va_arg(arguments, uint64);

                output_unsigned_integer(&buffer, number, &buffer_index, &buffer_size);
            } else if (specifier == 'd') {
                // 32 bit integer, possibly-negative

                int64 number = __builtin_va_arg(arguments, int32);

                if (number < 0) {
                    buffer[buffer_index++] = '-';
                    number = 0 - number;
                }

                output_unsigned_integer(&buffer, (uint64) number, &buffer_index, &buffer_size);
            } else if (specifier == 'D') {
                // 64 bit integer, possibly-negative

                int64 number = __builtin_va_arg(arguments, int64);

                if (number < 0) {
                    buffer[buffer_index++] = '-';
                    number = 0 - number;
                }

                output_unsigned_integer(&buffer, (uint64) number, &buffer_index, &buffer_size);
            } else if (specifier == 'x') {
                uint64 number = __builtin_va_arg(arguments, uint64);
                uint8 bytes = bytes_to_display(number);

                uint8 chars = 2 + bytes * 2;     // Two characters for 0x, two for each byte (one per nibble)

                if (buffer_index + chars > buffer_size) {
                    // Double the buffer size
                    buffer_size = buffer_size * 2;
                    reallocate((void **) &buffer, buffer_size);
                }

                buffer[buffer_index++] = '0';
                buffer[buffer_index++] = 'x';

                // Convert each byte in order, high to low
                for (int b = bytes - 1; b >= 0; b--) {
                    uint8 byte = (uint8) (number >> 8 * b);     // Get the byte we care about
                    uint8 highNibble = byte >> 4;
                    uint8 lowNibble = byte & 0x0F;
                    buffer[buffer_index++] = highNibble + '0' + (highNibble > 9 ? 7 : 0);   // 7 adjusts '9' + 1 -> 'A'
                    buffer[buffer_index++] = lowNibble + '0' + (lowNibble > 9 ? 7 : 0);
                }
            } else if (specifier == 'b') {
                uint64 number = __builtin_va_arg(arguments, uint64);
                uint8 bytes = bytes_to_display(number);

                uint8 chars = 2 + bytes * 8;    // Two characters for 0b, one for each bit

                if (buffer_index + chars > buffer_size) {
                    // Double the buffer size
                    buffer_size = buffer_size * 2;
                    reallocate((void **) &buffer, buffer_size);
                }

                buffer[buffer_index++] = '0';
                buffer[buffer_index++] = 'b';

                // Convert each byte in order, high to low
                for (int b = bytes - 1; b >= 0; b--) {
                    uint8 byte = (uint8) (number >> 8 * b);     // Get the byte we care about

                    for (int bit = 7; bit >= 0; bit--)
                        buffer[buffer_index++] = ((byte & (1 << bit)) >> bit) + '0';
                }
            } else if (specifier == 's') {
                 string *s = __builtin_va_arg(arguments, void *);

                 while (s->size - 2 + buffer_index > buffer_size) {
                    // Double the buffer size until it's big enough (while loop in case the string is HUGE)
                    buffer_size = buffer_size * 2;
                    reallocate((void **) &buffer, buffer_size);
                 }

                 copy_string(s, 0, s->size - 2, (string *) buffer, buffer_index - 2);

                 buffer_index += s->size - 2;
            }
        }
    }

    // We're done, return the now correct buffer

    __builtin_va_end(arguments);

    ((string *) buffer)->size = buffer_index;

    return (string *) buffer;
}
