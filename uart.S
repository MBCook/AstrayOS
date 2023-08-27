.include "gpio.h"

.equ AUX_ENABLE,						(MMIO_BASE + 0x00215004)
.equ AUX_MINI_UART_IO_DATA,				(MMIO_BASE + 0x00215040)
.equ AUX_MINI_UART_INTERRUPT_ENABLE,	(MMIO_BASE + 0x00215044)
.equ AUX_MINI_UART_INTERRUPT_IDENTITY,	(MMIO_BASE + 0x00215048)
.equ AUX_MINI_UART_LINE_CONTROL,		(MMIO_BASE + 0x0021504C)
.equ AUX_MINI_UART_MODEM_CONTROL,		(MMIO_BASE + 0x00215050)
.equ AUX_MINI_UART_LINE_STATUS,			(MMIO_BASE + 0x00215054)
.equ AUX_MINI_UART_MODEM_STATUS,		(MMIO_BASE + 0x00215058)
.equ AUX_MINI_UART_SCRATCH,				(MMIO_BASE + 0x0021505C)
.equ AUX_MINI_UART_EXTRA_CONTROL,		(MMIO_BASE + 0x00215060)
.equ AUX_MINI_UART_EXTRA_STATS,			(MMIO_BASE + 0x00215064)
.equ AUX_MINI_UART_BAUDRATE,			(MMIO_BASE + 0x00215068)

.global uart_init
.global uart_send_char
.global uart_receive_char
.global uart_send_string

// Start up the UART (smashes r0, r1, r2)

uart_init:
	ldr		x0, =AUX_ENABLE							// Set bit 1 to enable the mini UART
	ldr		w1, [x0]
	orr		w1, w1, #1
	str		w1, [x0]

	ldr		x0, =AUX_MINI_UART_EXTRA_CONTROL		// Disable receive/transmit while we work
	str		wzr, [x0]
	
	ldr		x0, =AUX_MINI_UART_LINE_CONTROL			// We want 8 bits (why is bit 2 set?)
	mov		w1, #3
	str		w1, [x0]
	
	ldr		x0, =AUX_MINI_UART_MODEM_CONTROL		// Set clear to send pin high
	str		wzr, [x0]
	
	ldr		x0, =AUX_MINI_UART_INTERRUPT_ENABLE		// Disable interrupts
	str		wzr, [x0]	
	
	ldr		x0, =AUX_MINI_UART_LINE_CONTROL			// Let us mess with baud rate, mark line break
	mov		w1, #0xC6
	str		w1, [x0]

	ldr		x0, =AUX_MINI_UART_BAUDRATE				// 115,200 baud
	mov		w1, #0x10E
	str		w1, [x0]

	ldr		x0, =GPIO_FUNCTION_SEL_0
	ldr		w1, [x0]								// Get the current GPIO pin configuration
	and		w1, w1, #0xFFFC0FFF						// Mask off bits for function select 4 & 5
	add		w1, w1, #0x00012000						// Set function select 4 & 5 to alternate function 5
	str		w1, [x0]

	ldr		x0, =GPIO_PIN_PULLUP_DOWN_ENABLE		// Disable pull up/down on all pins
	str		wzr, [x0]
	
	// Wait at least 150 cycles (it will be more than that)

	mov		x0, 150
	bl		uart_init_wait_loop
	
	// Assert clock on lines 14 and 15
	
	ldr		x2, =GPIO_PIN_PULLUP_DOWN_CLOCK0
	mov		w1, (1 << 14) | (1 << 15)
	str		w1, [x2]
	
	// Wait at least 150 cycles again

	mov		x0, 150
	bl		uart_init_wait_loop
	
	// Remove the clock line assert (x2 hasn't changed)

	str		wzr, [x2]
	
	// Enable the transmit/receive pins
	
	ldr		x0, =AUX_MINI_UART_EXTRA_CONTROL
	mov		w1, #3
	str		w1, [x0]
	
	// Done
	
	ret

uart_init_wait_loop:
	sub		x0, x0, 1
	cbnz	x0, uart_init_wait_loop
	ret

// Send a character (w0 holds the character, smashes r1 and r2)

uart_send_char:
	ldr		x1, =AUX_MINI_UART_LINE_STATUS			// Get the UART status

uart_send_wait:	
	ldr		w2, [x1]
	and		w2, w2, #0x20							// Can it accept a byte?
	cbnz	w2, uart_send_wait
	
	ldr		x1, =AUX_MINI_UART_IO_DATA				// Data register
	strb	w0, [x1]								// Send it
	
	ret 

// Gets a character (w0 holds the character returned, smashes r1 and r2)

uart_receive_char:
	ldr		x1, =AUX_MINI_UART_LINE_STATUS			// Get the UART status

uart_receive_wait:	
	ldr		w2, [x1]
	and		w2, w2, #0x01							// Does it have a byte?
	cbnz	w2, uart_receive_wait
	
	ldr		x1, =AUX_MINI_UART_IO_DATA				// Data register
	ldrb	w0, [x0]								// Load it
	
	ret	

// Send a pascal string (x0 holds address, four bytes for length, smashes r1, r2, r3, and r4)

uart_send_string:
	mov		x3, x0									// Save the address
	ldr		w4, [x3], #4							// Load the length, move the address past it
	cbz		w4, uart_send_done						// Make sure there is stuff to send

uart_send_loop:
	ldrb	w0, [x3], #1							// Load up the next byte to send then increment x3
	
	cmp		w0, #'\n'								// If we don't find a LF, send it as normal
	b.ne	uart_send_normal
	
	mov		w0, #'\r'								// We found a LF, send a CR first
	bl		uart_send_char
	mov		w0, #'\n'								// Put the LF back to send
	
uart_send_normal:

	bl		uart_send_char							// Send the char
	sub		w4, w4, 1								// Count down the number of bytes left
	cbnz	w4, uart_send_loop						// If there is more to send, loop around
	
uart_send_done:
	ret
	