#ifndef __mailbox_h__
#define	__mailbox_h__

#define MAILBOX_CHANNEL_POWER							0
#define MAILBOX_CHANNEL_FRAMEBUFFER						1
#define MAILBOX_CHANNEL_VIRTUAL_UART					2
#define MAILBOX_CHANNEL_VIDE_CORE_HOST_INTERFACE_QUEUE	3
#define MAILBOX_CHANNEL_LEDS							4
#define MAILBOX_CHANNEL_BUTTONS							5
#define MAILBOX_CHANNEL_TOUCH_SCREEN					6
#define MAILBOX_CHANNEL_COUNTER							7
#define MAILBOX_CHANNEL_PROPERTY_TAGS					8

#define MAILBOX_RESPONSE								0x80000000
#define MAILBOX_REQUEST									0
#define MAILBOX_TAG_GET_SERIAL							0x10004
#define MAILBOX_TAG_LAST								0

extern unsigned int mailbox_data[36];

extern int mailbox_call(char channel);

#endif
