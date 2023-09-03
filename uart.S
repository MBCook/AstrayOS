.include "gpio.h"
.include "aux.h"

.global uart_init
.global uart_send_char
.global uart_receive_char
.global uart_receive_string
.global uart_send_string
.global uart_send_word_in_hex

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
	
first_loop:	
	sub		x0, x0, 1
	cbnz	x0, first_loop
	
	// Assert clock on lines 14 and 15
	
	ldr		x2, =GPIO_PIN_PULLUP_DOWN_CLOCK0
	mov		w1, (1 << 14) | (1 << 15)
	str		w1, [x2]
	
	// Wait at least 150 cycles again

	mov		x0, 150

second_loop:	
	sub		x0, x0, 1
	cbnz	x0, second_loop
	
	// Remove the clock line assert (x2 hasn't changed)

	str		wzr, [x2]
	
	// Enable the transmit/receive pins
	
	ldr		x0, =AUX_MINI_UART_EXTRA_CONTROL
	mov		w1, #3
	str		w1, [x0]
	
	// Done
	
	ret

// Send a character (w0 holds the character, smashes r1 and r2)

uart_send_char:
	ldr		x1, =AUX_MINI_UART_LINE_STATUS			// Get the UART status

uart_send_wait:	
	ldr		w2, [x1]
	and		w2, w2, #0x20							// Can it accept a byte? If so bit 5 is set
	cbz		w2, uart_send_wait
	
	ldr		x1, =AUX_MINI_UART_IO_DATA				// Data register
	strb	w0, [x1]								// Send it
	
	ret 

// Gets a character (w0 holds the character returned, smashes r1 and r2)

uart_receive_char:
	ldr		x1, =AUX_MINI_UART_LINE_STATUS			// Get the UART status

uart_receive_wait:	
	ldr		w2, [x1]
	and		w2, w2, #0x01							// Does it have a byte? If so bit 0 is set
	cbz		w2, uart_receive_wait
	
	ldr		x1, =AUX_MINI_UART_IO_DATA				// Data register
	ldrb	w0, [x1]								// Load it

	ret	

// Receive a string until we get a \n or \r (x0 holds struct, x1 is max string length, w2 is echo on, smashes r2 - r6)
// Returns bytes used in x0

uart_receive_string:
    mov     x3, x0                                  // Save the address of the struct
    mov     w4, w1                                  // Save the max size allocated for us to use
    add     w4, w4, #2                              // Add the size of the size field
    mov     w5, #2                                  // How many bytes of the structure we've used so far
    mov     w6, w2                                  // Save if we need to echo characters

uart_receive_get_char:
    stp		x29, x30, [sp, #-0x10]!					// Call uart_send_char
    mov		x29, sp
    bl		uart_receive_char
    mov		sp, x29
    ldp		x29, x30, [sp], #0x10

    cbz     w6, post_echo                           // Check if echo is on

    stp		x29, x30, [sp, #-0x10]!					// They asked for echo
    mov		x29, sp
    bl		uart_send_char
    mov		sp, x29
    ldp		x29, x30, [sp], #0x10

post_echo:
    // Return if found an end of line character

    cmp     w0, '\n'
    b.eq    uart_receive_string_done
    cmp     w0, '\r'
    b.eq    uart_receive_string_done

    // Store the character, increment how many bytes we've used

    strb    w0, [x3, w5, UXTW]                      // Store at x3 + w5 unsigned bytes
    add     w5, w5, #1

    // Ensure we have space left

    cmp     w4, w5
    b.ne    uart_receive_get_char

uart_receive_string_done:
    strh    w5, [x3]                                // Update the size of the struct
    ret

// Send a pascal string (x0 holds address, 2 bytes for length, smashes r1, r2, r3, and r4)

uart_send_string:
	mov		x3, x0									// Save the address (use x3 so uart_send_char can use x0)
	ldrh	w4, [x3], #2							// Load the size (first 16 bits w4 points at), move address past it
	sub     w4, w4, #2                              // Subtract the two bytes used by the size
	cbz		w4, uart_send_done						// Make sure there is stuff to send

uart_send_loop:
	ldrb	w0, [x3], #1							// Load up the next byte to send then increment x3

	stp		x29, x30, [sp, #-0x10]!					// Call uart_send_char
	mov		x29, sp
	bl		uart_send_char
	mov		sp, x29
	ldp		x29, x30, [sp], #0x10
	
	sub		w4, w4, 1								// Count down the number of bytes left
	cbnz	w4, uart_send_loop						// If there is more to send, loop around
	
uart_send_done:
	ret
	
// Send a word as hex (w0 holds the word, w1 indicates if we show '0x', smashes r1, r2, r3, and r4)

uart_send_word_in_hex:
	mov		w3, w0									// Save the number
	mov		w4, #32									// Size of bit shift needed to get a nibble + 4
	
	cbz		w1, send_nibble							// If w1 is 0, don't show '0x'
	
	stp		x29, x30, [sp, #-0x10]!					// Send '0x' first
	mov		x29, sp
	
	mov		w0, '0'
	bl		uart_send_char
	mov		w0, 'x'
	bl		uart_send_char	
	
	mov		sp, x29
	ldp		x29, x30, [sp], #0x10
	
send_nibble:
	cbz		w4, uart_send_word_in_hex_done			// If there are 0 bits left, we're done
	
	sub		w4, w4, 4								// Adjust the shift size so we can get our next nibble
	ror		w0, w3, w4								// Shift down to what we want
	and		w0, w0, #0xF							// Only keep the last nibble
	add		w0, w0, #0x30							// Turn the number into a digit character
	
	cmp		w0, #0x39								// If the value is 0x39 or less, it's a digit, we're done
	b.le	char_ready
	
	add		w0, w0, #7								// It's a letter, add 7 to get a capital hex digit

char_ready:	
	stp		x29, x30, [sp, #-0x10]!					// Send the character
	mov		x29, sp
	bl		uart_send_char
	mov		sp, x29
	ldp		x29, x30, [sp], #0x10
	
	b		send_nibble								// Loop to the top

uart_send_word_in_hex_done:
	ret
	