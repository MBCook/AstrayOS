#include "pstring.h"
#include "types.h"

#ifndef __uart_h__
#define	__uart_h__

extern void uart_init();
extern void uart_send_char(char c);
extern char uart_receive_char();
extern void uart_send_string(pstring *s);
extern uint16 uart_receive_string(pstring *s, uint16 length, bool echo_on);
extern void uart_send_word_in_hex(uint32 i, uint32 showPrefix);

#endif
