.include "gpio.h"

.equ VIDEOCORE_MAILBOX,		(MMIO_BASE + 0x0000B880)

.equ MAILBOX_READ,			(VIDEOCORE_MAILBOX + 0x0)
.equ MAILBOX_POLL,			(VIDEOCORE_MAILBOX + 0x10)
.equ MAILBOX_SENDER,		(VIDEOCORE_MAILBOX + 0x14)
.equ MAILBOX_STATUS,		(VIDEOCORE_MAILBOX + 0x18)
.equ MAILBOX_CONFIG,		(VIDEOCORE_MAILBOX + 0x1C)
.equ MAILBOX_WRITE,			(VIDEOCORE_MAILBOX + 0x20)

.equ MAILBOX_FULL,		0x80000000
.equ MAILBOX_EMPTY,		0x40000000

.data
.align 16

mailbox_data: .fill 36 * 4

.text

.global mailbox_data
.global mailbox_call

// Make a call to the defined mailbox (mailbox in w0, smashes r1, r2)

mailbox_call:
	// We tell the video core a single address. It's (top 28 bytes of data address) | (channel number in w0)
	
	ldr		w1, mailbox_data							// Already 16 byte aligned, the bottom 4 bits are 0
	add		w0, w0, w1									// Add that to the channel number

	ldr		x2, =MAILBOX_STATUS
	
wait_for_space:

	ldr		w1, [x2]									// Get the status
	and		w1, w1, #MAILBOX_FULL
	cbnz	w1, wait_for_space							// If it's full, loop again

	// We can write w0 (address | channel) into the mailbox write address
	
	ldr		x2, =MAILBOX_WRITE
	str		w0, [x2]
	
	ldr		x2, =MAILBOX_STATUS
	
wait_for_read:

	ldr		w1, [x2]									// Get the status
	and		w1, w1, #MAILBOX_EMPTY
	cbnz	w1, wait_for_read							// If it's not empty, loop again

	// Something's been read, was it us? If we were successful w0 will be in [MAILBOX_READ]
	
	ldr		x2, =MAILBOX_READ
	ldr		w1, [x2]
	cmp		w0, w1										// Did we get our address back?
	
	cset	w0, eq										// Set w0 = 1 (success) if equal, else 0 (failure)
	
	ret

