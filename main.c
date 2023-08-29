#import "uart.h"

void main() {
	uart_init();
	
	char bytes[] = {0, 0, 0, 0, 'H', 'e', 'l', 'l', 'o', '\n'};
	pstring *str = (pstring *) &bytes;
	str->length = 6;
	
	uart_send_string(str);

    int i = 0;
    
    while(1) {
    	i++;
    }
}
