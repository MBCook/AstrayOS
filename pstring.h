#ifndef __pstring_h__
#define	__pstring_h__

typedef struct PString {
	unsigned int length;
	char data[];
} pstring;

#endif
