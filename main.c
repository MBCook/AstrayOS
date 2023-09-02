#import "uart.h"
#import "mailbox.h"
#import "memory.h"

char hello[] = {0, 0, 'H', 'e', 'l', 'l', 'o', '\n'};
char serial[] = {0, 0, 'M', 'y', ' ', 's', 'e', 'r', 'i', 'a', 'l', ':', ' '};
char error[] = {0, 0, 'E', 'r', 'r', 'o', 'r', '?'};

void main() {
	uart_init();

	init_memory_pools();
	
	pstring *str = (pstring *) &hello;
	str->length = sizeof hello;
	
	uart_send_string(str);
	
	// get the board's unique serial number with a mailbox call
    mailbox_data[0] = 8 * 4;					// Length of message in bytes (8 ints, which are 4 bytes each)
    mailbox_data[1] = MAILBOX_REQUEST;			// We're sending a request
    
    mailbox_data[2] = MAILBOX_TAG_GET_SERIAL;	// We want the serial number
    mailbox_data[3] = 8;						// Buffer size
    mailbox_data[4] = 0;						// Response size (we pre-set 0, the GPU will set the real response size)
    mailbox_data[5] = 0;						// Our buffer will start empty, two ints = 8 bytes
    mailbox_data[6] = 0;

    mailbox_data[7] = MAILBOX_TAG_LAST;			// We're done sending tags

	if (mailbox_call(MAILBOX_CHANNEL_PROPERTY_TAGS)) {
		pstring *str = (pstring *) &serial;
		str->length = sizeof serial;
	
		uart_send_string(str);
				
		uart_send_word_in_hex(mailbox_data[6], true);
		uart_send_word_in_hex(mailbox_data[5], false);
		
		uart_send_char('\n');
    } else {
		pstring *str = (pstring *) &error;
		str->length = sizeof error;
    }

    while (true) {};
}
