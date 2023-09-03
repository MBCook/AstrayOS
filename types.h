#ifndef __types_h__
#define	__types_h__

// Useful constants

#define DOUBLE_WORD_BITS                64
#define true                            1
#define false                           0
#define null                            ((void *) 0)

// Integers of various sizes

typedef unsigned char                   uint8;
typedef signed char                     int8;

typedef unsigned short                  uint16;
typedef signed short                    int16;

typedef unsigned int                    uint32;
typedef signed int                      int32;

typedef unsigned long long              uint64;
typedef signed long long                int64;

// A boolean type

typedef unsigned char                   bool;

#endif
