#ifdef __ASSEMBLY__

.equ MMIO_BASE,							0x3F000000

.equ GPIO_FUNCTION_SEL_0,				(MMIO_BASE + 0x00200000)
.equ GPIO_FUNCTION_SEL_1,				(MMIO_BASE + 0x00200004)
.equ GPIO_FUNCTION_SEL_2,				(MMIO_BASE + 0x00200008)
.equ GPIO_FUNCTION_SEL_3,				(MMIO_BASE + 0x0020000C)
.equ GPIO_FUNCTION_SEL_4,				(MMIO_BASE + 0x00200010)
.equ GPIO_FUNCTION_SEL_5,				(MMIO_BASE + 0x00200014)
.equ GPIO_PIN_OUTPUT_SET_0,				(MMIO_BASE + 0x0020001C)
.equ GPIO_PIN_OUTPUT_SET_1,				(MMIO_BASE + 0x00200020)
.equ GPIO_PIN_OUTPUT_CLEAR_0,			(MMIO_BASE + 0x00200028)
.equ GPIO_PIN_OUTPUT_CLEAR_1,			(MMIO_BASE + 0x0020002C)
.equ GPIO_PIN_LEVEL_0,					(MMIO_BASE + 0x00200034)
.equ GPIO_PIN_LEVEL_1,					(MMIO_BASE + 0x00200038)
.equ GPIO_PIN_EVENT_DETECT_STATUS_0,	(MMIO_BASE + 0x00200040)
.equ GPIO_PIN_EVENT_DETECT_STATUS_1,	(MMIO_BASE + 0x00200044)
.equ GPIO_PIN_HIGH_DETECT_ENABLE_0,		(MMIO_BASE + 0x00200064)
.equ GPIO_PIN_HIGH_DETECT_ENABLE_1,		(MMIO_BASE + 0x00200068)
.equ GPIO_PIN_PULLUP_DOWN_ENABLE,		(MMIO_BASE + 0x00200094)
.equ GPIO_PIN_PULLUP_DOWN_CLOCK0,		(MMIO_BASE + 0x00200098)
.equ GPIO_PIN_PULLUP_DOWN_CLOCK1,		(MMIO_BASE + 0x0020009C)

#endif
#ifndef __ASSEMBLY__

#define MMIO_BASE       						0x3F000000

#define GPIO_FUNCTION_SEL_0         			((volatile unsigned int*) (MMIO_BASE + 0x00200000))
#define GPIO_FUNCTION_SEL_1         			((volatile unsigned int*) (MMIO_BASE + 0x00200004))
#define GPIO_FUNCTION_SEL_2         			((volatile unsigned int*) (MMIO_BASE + 0x00200008))
#define GPIO_FUNCTION_SEL_3         			((volatile unsigned int*) (MMIO_BASE + 0x0020000C))
#define GPIO_FUNCTION_SEL_4         			((volatile unsigned int*) (MMIO_BASE + 0x00200010))
#define GPIO_FUNCTION_SEL_5         			((volatile unsigned int*) (MMIO_BASE + 0x00200014))
#define GPIO_PIN_OUTPUT_SET_0          			((volatile unsigned int*) (MMIO_BASE + 0x0020001C))
#define GPIO_PIN_OUTPUT_SET_1          			((volatile unsigned int*) (MMIO_BASE + 0x00200020))
#define GPIO_PIN_OUTPUT_CLEAR_0          		((volatile unsigned int*) (MMIO_BASE + 0x00200028))
#define GPIO_PIN_OUTPUT_CLEAR_1          		((volatile unsigned int*) (MMIO_BASE + 0x0020002C))
#define GPIO_PIN_LEVEL_0          				((volatile unsigned int*) (MMIO_BASE + 0x00200034))
#define GPIO_PIN_LEVEL_1          				((volatile unsigned int*) (MMIO_BASE + 0x00200038))
#define GPIO_PIN_EVENT_DETECT_STATUS_0          ((volatile unsigned int*) (MMIO_BASE + 0x00200040))
#define GPIO_PIN_EVENT_DETECT_STATUS_1          ((volatile unsigned int*) (MMIO_BASE + 0x00200044))
#define GPIO_PIN_HIGH_DETECT_ENABLE_0          	((volatile unsigned int*) (MMIO_BASE + 0x00200064))
#define GPIO_PIN_HIGH_DETECT_ENABLE_1         	((volatile unsigned int*) (MMIO_BASE + 0x00200068))
#define GPIO_PIN_PULLUP_DOWN_ENABLE           	((volatile unsigned int*) (MMIO_BASE + 0x00200094))
#define GPIO_PIN_PULLUP_DOWN_CLOCK0       		((volatile unsigned int*) (MMIO_BASE + 0x00200098))
#define GPIO_PIN_PULLUP_DOWN_CLOCK1       		((volatile unsigned int*) (MMIO_BASE + 0x0020009C))

#endif
