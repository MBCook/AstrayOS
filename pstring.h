#include "types.h"

#ifndef __pstring_h__
#define	__pstring_h__

typedef struct __attribute__((__packed__)) {
	uint16 length;
	uint8 data[];
} pstring;

#endif
