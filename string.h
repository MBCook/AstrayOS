#include "types.h"

#ifndef __string_h__
#define	__string_h__

// We use a variation on Pascal style strings. Instead of one byte for length,
// we use two bytes for the size of the entire struct (characters used + the size field).

typedef struct __attribute__((__packed__)) {
	uint16 size;        // Size of the USED struct, 2 bytes for size + length of used data
	uint8* data;
} string;

// Reserve an empty string guaranteed to be able to hold length UTF-8 bytes
string *empty_string(uint16 length);

// Ensures the string can hold length UTF-8 bytes, allocating a new string/copying/and freeing the old if necessary
void expand_string(string **str, uint16 length);

// Creates a string from a C style string that we may get from an external source
string *string_from_cstring(char data[]);

// Returns a new string that is a concatenation of the two passed in
string *append_strings(string *one, string *two);

// Returns a new string that is a substring starting at position start made up of length bytes
string *substring(string *src, uint16 start, uint16 length);

// Copies selected bytes from one string to the other starting at the given destination position
void copy_string(string *src, uint16 src_start, uint16 length, string *dest, uint16 dest_start);

#endif
