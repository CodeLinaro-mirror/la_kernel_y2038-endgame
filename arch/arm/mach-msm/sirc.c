/* Copyright (c) 2008-2009, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include <linux/io.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <asm/irq.h>

#include "msm_iomap-8x50.h"
#include "sirc.h"

#define SPSS_SIRC_INT_SELECT          (MSM_SIRC_BASE + 0x00)
#define SPSS_SIRC_INT_ENABLE          (MSM_SIRC_BASE + 0x04)
#define SPSS_SIRC_INT_ENABLE_CLEAR    (MSM_SIRC_BASE + 0x08)
#define SPSS_SIRC_INT_ENABLE_SET      (MSM_SIRC_BASE + 0x0C)
#define SPSS_SIRC_INT_TYPE            (MSM_SIRC_BASE + 0x10)
#define SPSS_SIRC_INT_POLARITY        (MSM_SIRC_BASE + 0x14)
#define SPSS_SIRC_SECURITY            (MSM_SIRC_BASE + 0x18)
#define SPSS_SIRC_IRQ_STATUS          (MSM_SIRC_BASE + 0x1C)
#define SPSS_SIRC_IRQ1_STATUS         (MSM_SIRC_BASE + 0x20)
#define SPSS_SIRC_RAW_STATUS          (MSM_SIRC_BASE + 0x24)
#define SPSS_SIRC_INT_CLEAR           (MSM_SIRC_BASE + 0x28)
#define SPSS_SIRC_SOFT_INT            (MSM_SIRC_BASE + 0x2C)

static unsigned int int_enable;
static unsigned int wake_enable;
static int first_sirc_irq;
static int nr_sirc_irqs;
static int sirc_mask;

static struct sirc_regs_t sirc_regs = {
	.int_enable       = SPSS_SIRC_INT_ENABLE,
	.int_enable_clear = SPSS_SIRC_INT_ENABLE_CLEAR,
	.int_enable_set   = SPSS_SIRC_INT_ENABLE_SET,
	.int_type         = SPSS_SIRC_INT_TYPE,
	.int_polarity     = SPSS_SIRC_INT_POLARITY,
	.int_clear        = SPSS_SIRC_INT_CLEAR,
};

static struct sirc_cascade_regs sirc_reg_table[] = {
	{
		.int_status  = SPSS_SIRC_IRQ_STATUS,
	}
};

/* Mask off the given interrupt. Keep the int_enable mask in sync with
   the enable reg, so it can be restored after power collapse. */
static void sirc_irq_mask(struct irq_data *d)
{
	unsigned int mask;

	mask = 1 << (d->irq - first_sirc_irq);
	writel(mask, sirc_regs.int_enable_clear);
	int_enable &= ~mask;
	return;
}

/* Unmask the given interrupt. Keep the int_enable mask in sync with
   the enable reg, so it can be restored after power collapse. */
static void sirc_irq_unmask(struct irq_data *d)
{
	unsigned int mask;

	mask = 1 << (d->irq - first_sirc_irq);
	writel(mask, sirc_regs.int_enable_set);
	int_enable |= mask;
	return;
}

static void sirc_irq_ack(struct irq_data *d)
{
	unsigned int mask;

	mask = 1 << (d->irq - first_sirc_irq);
	writel(mask, sirc_regs.int_clear);
	return;
}

static int sirc_irq_set_wake(struct irq_data *d, unsigned int on)
{
	unsigned int mask;

	/* Used to set the interrupt enable mask during power collapse. */
	mask = 1 << (d->irq - first_sirc_irq);
	if (on)
		wake_enable |= mask;
	else
		wake_enable &= ~mask;

	return 0;
}

static int sirc_irq_set_type(struct irq_data *d, unsigned int flow_type)
{
	unsigned int mask;
	unsigned int val;

	mask = 1 << (d->irq - first_sirc_irq);
	val = readl(sirc_regs.int_polarity);

	if (flow_type & (IRQF_TRIGGER_LOW | IRQF_TRIGGER_FALLING))
		val |= mask;
	else
		val &= ~mask;

	writel(val, sirc_regs.int_polarity);

	val = readl(sirc_regs.int_type);
	if (flow_type & (IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING)) {
		val |= mask;
		__irq_set_handler_locked(d->irq, handle_edge_irq);
	} else {
		val &= ~mask;
		__irq_set_handler_locked(d->irq, handle_level_irq);
	}

	writel(val, sirc_regs.int_type);

	return 0;
}

/* Finds the pending interrupt on the passed cascade irq and redrives it */
static void sirc_irq_handler(unsigned int irq, struct irq_desc *desc)
{
	unsigned int reg = 0;
	unsigned int sirq;
	unsigned int status;

	while ((reg < ARRAY_SIZE(sirc_reg_table)) &&
		(sirc_reg_table[reg].cascade_irq != irq))
		reg++;

	status = readl(sirc_reg_table[reg].int_status);
	status &= sirc_mask;
	if (status == 0)
		return;

	for (sirq = 0;
	     (sirq < nr_sirc_irqs) && ((status & (1U << sirq)) == 0);
	     sirq++)
		;
	generic_handle_irq(sirq+first_sirc_irq);

	desc->irq_data.chip->irq_ack(&desc->irq_data);
}

static struct irq_chip sirc_irq_chip = {
	.name          = "sirc",
	.irq_ack       = sirc_irq_ack,
	.irq_mask      = sirc_irq_mask,
	.irq_unmask    = sirc_irq_unmask,
	.irq_set_wake  = sirc_irq_set_wake,
	.irq_set_type  = sirc_irq_set_type,
};

void __init msm_init_sirc(int first, int count, int cascade)
{
	int i;

	int_enable = 0;
	wake_enable = 0;
	first_sirc_irq = first;
	nr_sirc_irqs = count;
	sirc_mask = (1 << count) - 1;
	sirc_reg_table[0].cascade_irq = cascade;

	for (i = first; i < first + count; i++) {
		irq_set_chip_and_handler(i, &sirc_irq_chip, handle_edge_irq);
		set_irq_flags(i, IRQF_VALID);
	}

	for (i = 0; i < ARRAY_SIZE(sirc_reg_table); i++) {
		irq_set_chained_handler(sirc_reg_table[i].cascade_irq,
					sirc_irq_handler);
		irq_set_irq_wake(sirc_reg_table[i].cascade_irq, 1);
	}
	return;
}

