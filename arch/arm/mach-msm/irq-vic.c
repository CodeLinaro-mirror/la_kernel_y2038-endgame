/*
 * Copyright (C) 2007 Google, Inc.
 * Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/ptrace.h>
#include <linux/timer.h>
#include <linux/irq.h>
#include <linux/io.h>

#include <asm/exception.h>
#include <asm/cacheflush.h>

#include "msm_iomap.h"
#include "smd_private.h"

enum {
	IRQ_DEBUG_SLEEP_INT_TRIGGER = 1U << 0,
	IRQ_DEBUG_SLEEP_INT = 1U << 1,
	IRQ_DEBUG_SLEEP_ABORT = 1U << 2,
	IRQ_DEBUG_SLEEP = 1U << 3,
	IRQ_DEBUG_SLEEP_REQUEST = 1U << 4,
};
static int msm_irq_debug_mask;
module_param_named(debug_mask, msm_irq_debug_mask, int,
		   S_IRUGO | S_IWUSR | S_IWGRP);

#define VIC_REG(off) (MSM_VIC_BASE + (off))
#define VIC_INT_TO_REG_ADDR(base, irq) (base + (irq / 32) * 4)
#define VIC_INT_TO_REG_INDEX(irq) ((irq >> 5) & 3)

#define VIC_INT_SELECT0     VIC_REG(0x0000)  /* 1: FIQ, 0: IRQ */
#define VIC_INT_SELECT1     VIC_REG(0x0004)  /* 1: FIQ, 0: IRQ */
#define VIC_INT_SELECT2     VIC_REG(0x0008)  /* 1: FIQ, 0: IRQ */
#define VIC_INT_SELECT3     VIC_REG(0x000C)  /* 1: FIQ, 0: IRQ */
#define VIC_INT_EN0         VIC_REG(0x0010)
#define VIC_INT_EN1         VIC_REG(0x0014)
#define VIC_INT_EN2         VIC_REG(0x0018)
#define VIC_INT_EN3         VIC_REG(0x001C)
#define VIC_INT_ENCLEAR0    VIC_REG(0x0020)
#define VIC_INT_ENCLEAR1    VIC_REG(0x0024)
#define VIC_INT_ENCLEAR2    VIC_REG(0x0028)
#define VIC_INT_ENCLEAR3    VIC_REG(0x002C)
#define VIC_INT_ENSET0      VIC_REG(0x0030)
#define VIC_INT_ENSET1      VIC_REG(0x0034)
#define VIC_INT_ENSET2      VIC_REG(0x0038)
#define VIC_INT_ENSET3      VIC_REG(0x003C)
#define VIC_INT_TYPE0       VIC_REG(0x0040)  /* 1: EDGE, 0: LEVEL  */
#define VIC_INT_TYPE1       VIC_REG(0x0044)  /* 1: EDGE, 0: LEVEL  */
#define VIC_INT_TYPE2       VIC_REG(0x0048)  /* 1: EDGE, 0: LEVEL  */
#define VIC_INT_TYPE3       VIC_REG(0x004C)  /* 1: EDGE, 0: LEVEL  */
#define VIC_INT_POLARITY0   VIC_REG(0x0050)  /* 1: NEG, 0: POS */
#define VIC_INT_POLARITY1   VIC_REG(0x0054)  /* 1: NEG, 0: POS */
#define VIC_INT_POLARITY2   VIC_REG(0x0058)  /* 1: NEG, 0: POS */
#define VIC_INT_POLARITY3   VIC_REG(0x005C)  /* 1: NEG, 0: POS */
#define VIC_NO_PEND_VAL     VIC_REG(0x0060)

#define VIC_NO_PEND_VAL_FIQ VIC_REG(0x0064)
#define VIC_INT_MASTEREN    VIC_REG(0x0068)  /* 1: IRQ, 2: FIQ     */
#define VIC_CONFIG          VIC_REG(0x006C)  /* 1: USE SC VIC */

#define VIC_IRQ_STATUS0     VIC_REG(0x0080)
#define VIC_IRQ_STATUS1     VIC_REG(0x0084)
#define VIC_IRQ_STATUS2     VIC_REG(0x0088)
#define VIC_IRQ_STATUS3     VIC_REG(0x008C)
#define VIC_FIQ_STATUS0     VIC_REG(0x0090)
#define VIC_FIQ_STATUS1     VIC_REG(0x0094)
#define VIC_FIQ_STATUS2     VIC_REG(0x0098)
#define VIC_FIQ_STATUS3     VIC_REG(0x009C)
#define VIC_RAW_STATUS0     VIC_REG(0x00A0)
#define VIC_RAW_STATUS1     VIC_REG(0x00A4)
#define VIC_RAW_STATUS2     VIC_REG(0x00A8)
#define VIC_RAW_STATUS3     VIC_REG(0x00AC)
#define VIC_INT_CLEAR0      VIC_REG(0x00B0)
#define VIC_INT_CLEAR1      VIC_REG(0x00B4)
#define VIC_INT_CLEAR2      VIC_REG(0x00B8)
#define VIC_INT_CLEAR3      VIC_REG(0x00BC)
#define VIC_SOFTINT0        VIC_REG(0x00C0)
#define VIC_SOFTINT1        VIC_REG(0x00C4)
#define VIC_SOFTINT2        VIC_REG(0x00C8)
#define VIC_SOFTINT3        VIC_REG(0x00CC)
#define VIC_IRQ_VEC_RD      VIC_REG(0x00D0)  /* pending int # */
#define VIC_IRQ_VEC_PEND_RD VIC_REG(0x00D4)  /* pending vector addr */
#define VIC_IRQ_VEC_WR      VIC_REG(0x00D8)

#define VIC_FIQ_VEC_RD      VIC_REG(0x00DC)
#define VIC_FIQ_VEC_PEND_RD VIC_REG(0x00E0)
#define VIC_FIQ_VEC_WR      VIC_REG(0x00E4)
#define VIC_IRQ_IN_SERVICE  VIC_REG(0x00E8)
#define VIC_IRQ_IN_STACK    VIC_REG(0x00EC)
#define VIC_FIQ_IN_SERVICE  VIC_REG(0x00F0)
#define VIC_FIQ_IN_STACK    VIC_REG(0x00F4)
#define VIC_TEST_BUS_SEL    VIC_REG(0x00F8)
#define VIC_IRQ_CTRL_CONFIG VIC_REG(0x00FC)

#define VIC_VECTPRIORITY(n) VIC_REG(0x0200+((n) * 4))
#define VIC_VECTADDR(n)     VIC_REG(0x0400+((n) * 4))

#define VIC_MAX_REGS	    4

static uint32_t msm_irq_smsm_wake_enable[2];
static struct {
	uint32_t int_en[2];
	uint32_t int_type;
	uint32_t int_polarity;
	uint32_t int_select;
} msm_irq_shadow_reg[VIC_MAX_REGS];
static uint32_t msm_irq_idle_disable[VIC_MAX_REGS];

#define SMSM_FAKE_IRQ (0xff)
static uint8_t *msm_irq_to_smsm;

static inline void msm_irq_write_all_regs(void __iomem *base, unsigned int val, int num)
{
	int i;

	for (i = 0; i < num; i++)
		writel(val, base + (i * 4));
}

static void msm_irq_ack(struct irq_data *d)
{
	void __iomem *reg = VIC_INT_TO_REG_ADDR(VIC_INT_CLEAR0, d->irq);
	writel(1 << (d->irq & 31), reg);
}

static void msm_irq_mask(struct irq_data *d)
{
	void __iomem *reg = VIC_INT_TO_REG_ADDR(VIC_INT_ENCLEAR0, d->irq);
	unsigned index = VIC_INT_TO_REG_INDEX(d->irq);
	uint32_t mask = 1UL << (d->irq & 31);
	int smsm_irq = msm_irq_to_smsm[d->irq];

	msm_irq_shadow_reg[index].int_en[0] &= ~mask;
	writel(mask, reg);
	if (smsm_irq == 0)
		msm_irq_idle_disable[index] &= ~mask;
	else {
		mask = 1UL << (smsm_irq - 1);
		msm_irq_smsm_wake_enable[0] &= ~mask;
	}
}

static void msm_irq_unmask(struct irq_data *d)
{
	void __iomem *reg = VIC_INT_TO_REG_ADDR(VIC_INT_ENSET0, d->irq);
	unsigned index = VIC_INT_TO_REG_INDEX(d->irq);
	uint32_t mask = 1UL << (d->irq & 31);
	int smsm_irq = msm_irq_to_smsm[d->irq];

	msm_irq_shadow_reg[index].int_en[0] |= mask;
	writel(mask, reg);

	if (smsm_irq == 0)
		msm_irq_idle_disable[index] |= mask;
	else {
		mask = 1UL << (smsm_irq - 1);
		msm_irq_smsm_wake_enable[0] |= mask;
	}
}

static int msm_irq_set_wake(struct irq_data *d, unsigned int on)
{
	unsigned index = VIC_INT_TO_REG_INDEX(d->irq);
	uint32_t mask = 1UL << (d->irq & 31);
	int smsm_irq = msm_irq_to_smsm[d->irq];

	if (smsm_irq == 0) {
		printk(KERN_ERR "msm_irq_set_wake: bad wakeup irq %d\n", d->irq);
		return -EINVAL;
	}
	if (on)
		msm_irq_shadow_reg[index].int_en[1] |= mask;
	else
		msm_irq_shadow_reg[index].int_en[1] &= ~mask;

	if (smsm_irq == SMSM_FAKE_IRQ)
		return 0;

	mask = 1UL << (smsm_irq - 1);
	if (on)
		msm_irq_smsm_wake_enable[1] |= mask;
	else
		msm_irq_smsm_wake_enable[1] &= ~mask;
	return 0;
}

static int msm_irq_set_type(struct irq_data *d, unsigned int flow_type)
{
	void __iomem *treg = VIC_INT_TO_REG_ADDR(VIC_INT_TYPE0, d->irq);
	void __iomem *preg = VIC_INT_TO_REG_ADDR(VIC_INT_POLARITY0, d->irq);
	unsigned index = VIC_INT_TO_REG_INDEX(d->irq);
	int b = 1 << (d->irq & 31);
	uint32_t polarity;
	uint32_t type;

	polarity = msm_irq_shadow_reg[index].int_polarity;
	if (flow_type & (IRQF_TRIGGER_FALLING | IRQF_TRIGGER_LOW))
		polarity |= b;
	if (flow_type & (IRQF_TRIGGER_RISING | IRQF_TRIGGER_HIGH))
		polarity &= ~b;
	writel(polarity, preg);
	msm_irq_shadow_reg[index].int_polarity = polarity;

	type = msm_irq_shadow_reg[index].int_type;
	if (flow_type & (IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING)) {
		type |= b;
		__irq_set_handler_locked(d->irq, handle_edge_irq);
	}
	if (flow_type & (IRQF_TRIGGER_HIGH | IRQF_TRIGGER_LOW)) {
		type &= ~b;
		__irq_set_handler_locked(d->irq, handle_level_irq);
	}
	writel(type, treg);
	msm_irq_shadow_reg[index].int_type = type;
	return 0;
}

static struct irq_chip msm_irq_chip = {
	.name          = "msm",
	.irq_disable   = msm_irq_mask,
	.irq_ack       = msm_irq_ack,
	.irq_mask      = msm_irq_mask,
	.irq_unmask    = msm_irq_unmask,
	.irq_set_wake  = msm_irq_set_wake,
	.irq_set_type  = msm_irq_set_type,
};

static void __exception_irq_entry msm_vic_handle_irq(struct pt_regs *regs)
{
	int irq;

	do {
		/*
		 * 0xD0 has irq# or old irq# if the irq has been handled
		 * 0xD4 has irq# or -1 if none pending *but* if you just
		 * read 0xD4 you never get the first irq for some reason
		 */
		irq = readl(VIC_IRQ_VEC_RD);
		irq = readl(VIC_IRQ_VEC_PEND_RD);

		if (irq == -1)
			break;

		generic_handle_irq(irq);
	} while (1);
}

void __init msm_init_vic(uint8_t *irq_to_smsm, int nr)
{
	unsigned n;
	msm_irq_to_smsm = irq_to_smsm;

	/* set entry point */
	set_handle_irq(msm_vic_handle_irq);

	/* select level interrupts */
	msm_irq_write_all_regs(VIC_INT_TYPE0, 0, nr / 32);

	/* select highlevel interrupts */
	msm_irq_write_all_regs(VIC_INT_POLARITY0, 0, nr / 32);

	/* select IRQ for all INTs */
	msm_irq_write_all_regs(VIC_INT_SELECT0, 0, nr / 32);

	/* disable all INTs */
	msm_irq_write_all_regs(VIC_INT_EN0, 0, nr / 32);

	/* don't use vic */
	writel(0, VIC_CONFIG);

	/* enable interrupt controller */
	writel(3, VIC_INT_MASTEREN);

	for (n = 0; n < nr; n++) {
		irq_set_chip_and_handler(n, &msm_irq_chip, handle_level_irq);
		set_irq_flags(n, IRQF_VALID);
	}
}
