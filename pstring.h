#include "types.h"

#ifndef __pstring_h__
#define	__pstring_h__

typedef struct __attribute__((__packed__)) {
	uint16 length;
	uint8* data;
} pstring;

pstring *empty_pstring(uint16 length);
pstring *pstring_from_cstring(char data[]);
pstring *append_pstrings(pstring *one, pstring *two);

#endif
