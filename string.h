#include "types.h"

#ifndef __string_h__
#define	__string_h__

// We use a variation on Pascal style strings. Instead of one byte for length,
// we use two bytes for the size of the entire struct (characters used + the size field).

typedef struct __attribute__((__packed__)) {
	uint16 size;        // Size of the USED struct, 2 bytes for size + length of used data
	uint8* data;
} string;

string *empty_string(uint16 length);
void expand_string(string **str, uint16 length);
string *string_from_cstring(char data[]);
string *append_strings(string *one, string *two);
string *substring(string *src, uint16 start, uint16 length);
void copy_string(string *src, uint16 src_start, uint16 length, string *dest, uint16 dest_start);

#endif
