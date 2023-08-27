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

clear_bss:	
	cbz		w2, setup_done		// If amount left is 0, we've cleared it all
	
	str		xzr, [x1], #8		// Store 0 at the current address, move it forward 8 bytes
	sub		w2, w2, #8			// Amount left is now 8 bytes left 
	
	cbnz	w2, clear_bss
	
setup_done:
	wfe
	b		setup_done
	
	