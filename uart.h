#include "string.h"
#include "types.h"

#ifndef __uart_h__
#define	__uart_h__

// Initializes UART 1 so we can send/receive data
extern void uart_init();

// Sends a single character
extern void uart_send_char(char c);

// Receives a single character
extern char uart_receive_char();

// Sends a full string
extern void uart_send_string(string *s);

// Receives a string into the destination, up to max_length bytes long or a newline character is seen
// Echo_on controls if characters are echoed back while being typed or no output is shown
extern void uart_receive_string(string *dest, uint16 max_length, bool echo_on);

// Sends a 32 bit number out in hex, show_prefix indicates if we also display the '0x' prefix
extern void uart_send_word_in_hex(uint32 i, bool show_prefix);

#endif
