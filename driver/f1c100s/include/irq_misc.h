/*
 * irq_misc.h
 *
 *  Created on: 2018Äê10ÔÂ3ÈÕ
 *      Author: lucy
 */

#ifndef DRIVER_INCLUDE_IRQ_MISC_H_
#define DRIVER_INCLUDE_IRQ_MISC_H_


enum
{
	IRQ_VECTOR			= 0x00,
	IRQ_BASE_ADDR		= 0x04,
	IRQ_NMI_CTRL		= 0x0c,
	IRQ_PEND0			= 0x10,
	IRQ_PEND1			= 0x14,
	IRQ_ENABLE0			= 0x20,
	IRQ_ENABLE1			= 0x24,
	IRQ_MASK0			= 0x30,
	IRQ_MASK1			= 0x34,
	IRQ_RESP0			= 0x40,
	IRQ_RESP1			= 0x44,
	IRQ_FORCE0			= 0x50,
	IRQ_FORCE1			= 0x54,
	IRQ_PRIORITY0		= 0x60,
	IRQ_PRIORITY1		= 0x64,
	IRQ_PRIORITY2		= 0x68,
	IRQ_PRIORITY3		= 0x6c,
};
typedef void (*irq_handle)(int arg);
int request_irq(int irqno,irq_handle callback,int arg);


#define IRQ_ADDR_BASE	(0x01c20400)
#define IRQ_GPIOD_ADDR_BASE	(0x01c20a00)
#define IRQ_GPIOE_ADDR_BASE	(0x01c20a20)
#define IRQ_GPIOF_ADDR_BASE	(0x01c20a40)

#define IRQ_NMI			(0)
#define IRQ_UART0		(1)
#define IRQ_UART1		(2)
#define IRQ_UART2		(3)
#define IRQ_SPDIF		(5)
#define IRQ_CIR			(6)
#define IRQ_I2C0		(7)
#define IRQ_I2C1		(8)
#define IRQ_I2C2		(9)
#define IRQ_SPI0		(10)
#define IRQ_SPI1		(11)
#define IRQ_TIMER0		(13)
#define IRQ_TIMER1		(14)
#define IRQ_TIMER2		(15)
#define IRQ_WDOG		(16)
#define IRQ_RSB			(17)
#define IRQ_DMA			(18)
#define IRQ_TP			(20)
#define IRQ_AUDIO		(21)
#define IRQ_LRADC		(22)
#define IRQ_MMC0		(23)
#define IRQ_MMC1		(24)
#define IRQ_USBOTG		(26)
#define IRQ_TVD			(27)
#define IRQ_TVE			(28)
#define IRQ_LCD			(29)
#define IRQ_DEFE		(30)
#define IRQ_DEBE		(31)
#define IRQ_CSI			(32)
#define IRQ_DEITLA		(33)
#define IRQ_VE			(34)
#define IRQ_I2S			(35)
#define IRQ_GPIOD		(38)
#define IRQ_GPIOE		(39)
#define IRQ_GPIOF		(40)

#define IRQ_GPIOD0		(64)
#define IRQ_GPIOD1		(65)
#define IRQ_GPIOD2		(66)
#define IRQ_GPIOD3		(67)
#define IRQ_GPIOD4		(68)
#define IRQ_GPIOD5		(69)
#define IRQ_GPIOD6		(70)
#define IRQ_GPIOD7		(71)
#define IRQ_GPIOD8		(72)
#define IRQ_GPIOD9		(73)
#define IRQ_GPIOD10		(74)
#define IRQ_GPIOD11		(75)
#define IRQ_GPIOD12		(76)
#define IRQ_GPIOD13		(77)
#define IRQ_GPIOD14		(78)
#define IRQ_GPIOD15		(79)
#define IRQ_GPIOD17		(80)
#define IRQ_GPIOD18		(81)
#define IRQ_GPIOD19		(82)
#define IRQ_GPIOD20		(83)
#define IRQ_GPIOD21		(84)

#define IRQ_GPIOE0		(96)
#define IRQ_GPIOE1		(97)
#define IRQ_GPIOE2		(98)
#define IRQ_GPIOE3		(99)
#define IRQ_GPIOE4		(100)
#define IRQ_GPIOE5		(101)
#define IRQ_GPIOE6		(102)
#define IRQ_GPIOE7		(103)
#define IRQ_GPIOE8		(104)
#define IRQ_GPIOE9		(105)
#define IRQ_GPIOE10		(106)
#define IRQ_GPIOE11		(107)
#define IRQ_GPIOE12		(108)

#define IRQ_GPIOF0		(128)
#define IRQ_GPIOF1		(129)
#define IRQ_GPIOF2		(130)
#define IRQ_GPIOF3		(131)
#define IRQ_GPIOF4		(132)
#define IRQ_GPIOF5		(133)



#endif /* DRIVER_INCLUDE_IRQ_MISC_H_ */
