#include "types.h"

#ifndef __pstring_h__
#define	__pstring_h__

typedef struct __attribute__((__packed__)) {
	uint16 size;        // Including this field, so length is size - 2
	uint8* data;
} pstring;

pstring *empty_pstring(uint16 length);
pstring *pstring_from_cstring(char data[]);
pstring *append_pstrings(pstring *one, pstring *two);
pstring *sub_pstring(pstring *src, uint16 start, uint16 length);
void update_pstring(pstring *src, uint16 src_start, uint16 length, pstring *dest, uint16 dest_start);

#endif
