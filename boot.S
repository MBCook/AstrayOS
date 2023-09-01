.section ".text.boot"

.global _start

_start:
	// Figure out which core we're on
	mrs		x1, mpidr_el1				// Read multiprocessor affinity register
	and		x1, x1, #3					// Keep only the bottom two bits (tells us core between 0 and 3)
	cbz		x1, main_core				// If we're the main core (#0), jump to the label to keep going
	
stop_core:
	wfe									// Non-main cores will wait forever here
	b		stop_core

main_core:
	// Setup the stack above our code's start
	
	ldr		x1, =_start
	mov		sp, x1
	
	// Clear the BSS section, which any C code would expect
	
	ldr		x1, =__bss_start	// Current address
	ldr		w2, =__bss_size		// Amount of bytes to write

clear_bss_double_word:
	cbz		w2, setup_done		// If amount left is 0, we've cleared it all
	cmp     w2, #7              // Check if we're under 8 bytes left
	b.lt    clear_bss_bytes

	str		xzr, [x1], #8		// Store eight bytes of 0s at the current address, move it forward 8 bytes
	sub		w2, w2, #8			// Amount left is now 8 bytes left 
	
	cbnz	w2, clear_bss_double_word

clear_bss_bytes:
    cbz		w2, setup_done		// If amount left is 0, we've cleared it all

    strb	wzr, [x1], #1		// Store one byte of 0 at the current address, move it forward 1 byte
    sub		w2, w2, #1			// Amount left is now 1 byte left

    cbnz	w2, clear_bss_bytes
	
setup_done:
	bl		main				// Run the main function in our main.c file
	b		setup_done			// If it ever returns (it shouldn't), try again
	
	