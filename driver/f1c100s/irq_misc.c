#include "irq_misc.h"
#include <arm32.h>
#include <io.h>
typedef struct {
	irq_handle irq_hdl_proc;
	int tag;
}irq_handle_stu;
static irq_handle_stu f1c100s_irq_vector[160]={0};
static void irq_f1c100s_enable(int type)
{
		int gpio_irq;
		int offset;
		if(type < 64)
		{
			gpio_irq = 0;
			offset = type;
		}
		else if(type < 96)
		{
			gpio_irq = 1;
			offset = IRQ_GPIOD;
		}
		else if(type < 128)
		{
			gpio_irq = 2;
			offset = IRQ_GPIOE;
		}
		else
		{
			gpio_irq = 3;
			offset = IRQ_GPIOF;
		}

		int irq =  offset;
		unsigned int val;
		val = read32(IRQ_ADDR_BASE + IRQ_ENABLE0 + (irq / 32) * 4);
		val |= 1 << (irq % 32);
		write32(IRQ_ADDR_BASE + IRQ_ENABLE0 + (irq / 32) * 4, val);

		val = read32(IRQ_ADDR_BASE + IRQ_MASK0 + (irq / 32) * 4);
		val &= ~(1 << (irq % 32));
		write32(IRQ_ADDR_BASE+ IRQ_MASK0 + (irq / 32) * 4, val);

		if(gpio_irq)
		{

		}
}

//static void irq_f1c100s_disable(int offset)
//{
//	//struct irq_f1c100s_pdata_t * pdat = (struct irq_f1c100s_pdata_t *)chip->priv;
//	int irq = chip->base + offset;
//	u32_t val;
//
//	val = read32(pdat->virt + IRQ_ENABLE0 + (irq / 32) * 4);
//	val &= ~(1 << (irq % 32));
//	write32(pdat->virt + IRQ_ENABLE0 + (irq / 32) * 4, val);
//
//	val = read32(pdat->virt + IRQ_MASK0 + (irq / 32) * 4);
//	val |= 1 << (irq % 32);
//	write32(pdat->virt + IRQ_MASK0 + (irq / 32) * 4, val);
//}
void default_irq_proc(int arg)
{
	lprintf("no callback function for irq :%d\n",arg);
}
int request_irq(int irqno,irq_handle callback,int arg)
{
	int ret = 0;
	arm32_interrupt_disable();//
	irq_f1c100s_enable(irqno);
	if(!callback)
	{
		f1c100s_irq_vector[irqno].tag = irqno;
		f1c100s_irq_vector[irqno].irq_hdl_proc = default_irq_proc;
	}
	else
	{
		f1c100s_irq_vector[irqno].tag = arg;
		f1c100s_irq_vector[irqno].irq_hdl_proc = callback;
	}
	arm32_interrupt_enable();//
	return ret;
}
void interrupt_handle_exception(void * regs)
{
//	struct device_t * pos, * n;
//	struct irqchip_t * chip;
//
//	list_for_each_entry_safe(pos, n, &__device_head[DEVICE_TYPE_IRQCHIP], head)
//	{
//		chip = (struct irqchip_t *)(pos->priv);
//		if(chip->dispatch)
//			chip->dispatch(chip);
//	}
	unsigned int irq, hwirq;
		hwirq = read32(IRQ_ADDR_BASE + IRQ_VECTOR) >> 2;
		while (hwirq != 0)
		{
			irq =  hwirq;
			//handle_IRQ(irq, regs);
			if(irq < 36 )
			{
				if(f1c100s_irq_vector[irq].irq_hdl_proc)
				{
					f1c100s_irq_vector[irq].irq_hdl_proc(irq);
				}
				else
				{
					lprintf("no irq:%d handle proc \n",irq);
				}
			}
			else if(irq == IRQ_GPIOD)
			{

			}
			else if(irq == IRQ_GPIOE)
			{

			}
			else if(irq == IRQ_GPIOF)
			{

			}
			hwirq = read32(IRQ_ADDR_BASE + IRQ_VECTOR) >> 2;
		}
}
