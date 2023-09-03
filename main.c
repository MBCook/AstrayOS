#import "uart.h"
#import "mailbox.h"
#import "memory.h"
#import "string.h"

void main() {
	uart_init();

    uart_send_char('\n');

	init_memory_pools();

	string *str = string_from_cstring("Hello!\n");

	uart_send_string(str);

    free((void **) &str);

	// Get the board's serial number with a mailbox call

    mailbox_data[0] = 8 * 4;					// Length of message in bytes (8 ints, which are 4 bytes each)
    mailbox_data[1] = MAILBOX_REQUEST;			// We're sending a request

    mailbox_data[2] = MAILBOX_TAG_GET_SERIAL;	// We want the serial number
    mailbox_data[3] = 8;						// Buffer size
    mailbox_data[4] = 0;						// Response size (we pre-set 0, the GPU will set the real response size)
    mailbox_data[5] = 0;						// Our buffer will start empty, two ints = 8 bytes
    mailbox_data[6] = 0;

    mailbox_data[7] = MAILBOX_TAG_LAST;			// We're done sending tags

	if (mailbox_call(MAILBOX_CHANNEL_PROPERTY_TAGS)) {
	    str = string_from_cstring("Serial number is: ");

        uart_send_string(str);

        free((void **) &str);

		uart_send_word_in_hex(mailbox_data[6], true);
		uart_send_word_in_hex(mailbox_data[5], false);

		uart_send_char('\n');
    } else {
		str = string_from_cstring("Unable to get the serial number.");

        uart_send_string(str);

        free((void **) &str);
    }

    uart_send_char('\n');

    // Echo back strings they type

    str = empty_string(60);

    string *used = string_from_cstring("Bytes used: ");

    while (true) {
        // Get a string up to 60 characters, echoing characters as they type

        uint16 used_bytes = uart_receive_string(str, 60, true);

        uart_send_char('\n');

        // Echo it back

        uart_send_string(str);

        uart_send_char('\n');

        // Print out how many bytes were used (characters + 2)

        uart_send_string(used);

        uart_send_word_in_hex(used_bytes, true);

        uart_send_char('\n');
    }

    while (true) {};
}
