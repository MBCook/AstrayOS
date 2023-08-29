#include "pstring.h"

#ifndef __uart_h__
#define	__uart_h__

extern void uart_init();
extern void uart_send_char(char c);
extern char uart_receive_char(char c);
extern void uart_send_string(pstring *s);
extern void uart_send_word_in_hex(unsigned int i, unsigned int showPrefix);

#endif
