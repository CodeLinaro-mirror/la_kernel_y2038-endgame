// SPDX-License-Identifier: GPL-2.0-only
/*
 * arch/arm/mach-iop32x/irq.c
 *
 * Generic IOP32X IRQ handling functionality
 *
 * Author: Rory Bolt <rorybolt@pacbell.net>
 * Copyright (C) 2002 Rory Bolt
 */

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <asm/mach/irq.h>
#include <asm/irq.h>
#include <asm/mach-types.h>
#include <asm/exception.h>
#include <asm/traps.h>
#include <asm/ptrace.h>

#include "hardware.h"
#include "iop3xx.h"

u32 iop32x_cp6_enabled;

static void enable_cp6(void)
{
	u32 temp;

        /* enable cp6 access */
        asm volatile (
		"mrc	p15, 0, %0, c15, c1, 0\n\t"
		"orr	%0, %0, #(1 << 6)\n\t"
		"mcr	p15, 0, %0, c15, c1, 0\n\t"
		: "=r"(temp));

	WRITE_ONCE(iop32x_cp6_enabled, 1);
}

static void wait_cp6(void)
{
	u32 temp;

	asm volatile(
		"mrc	p15, 0, %0, c15, c1, 0	\n\t"
		"mov	%0, %0			\n\t"
		"sub	pc, pc, #4 @ cp_wait" : "=r" (temp));
}

static int cp6_trap(struct pt_regs *regs, unsigned int instr)
{
	enable_cp6();

	return 0;
}

/* permit kernel space cp6 access
 * deny user space cp6 access
 */
static struct undef_hook cp6_hook = {
	.instr_mask     = 0x0f000ff0,
	.instr_val      = 0x0e000610,
	.cpsr_mask      = MODE_MASK,
	.cpsr_val       = SVC_MODE,
	.fn             = cp6_trap,
};

void __init iop_init_cp6_handler(void)
{
	register_undef_hook(&cp6_hook);
}
static u32 iop32x_mask;

static void intctl_write(u32 val)
{
	asm volatile("mcr p6, 0, %0, c0, c0, 0" : : "r" (val));
}

static void intstr_write(u32 val)
{
	asm volatile("mcr p6, 0, %0, c4, c0, 0" : : "r" (val));
}

static u32 iintsrc_read(void)
{
	u32 val;

	asm volatile("mrc p6, 0, %0, c8, c0, 0" : "=r" (val));

	return val;
}

static void
iop32x_irq_mask(struct irq_data *d)
{
	iop32x_mask &= ~(1 << d->irq);
	intctl_write(iop32x_mask);
}

static void
iop32x_irq_unmask(struct irq_data *d)
{
	iop32x_mask |= 1 << d->irq;
	intctl_write(iop32x_mask);
}

static struct irq_chip ext_chip = {
	.name		= "IOP32x",
	.irq_ack	= iop32x_irq_mask,
	.irq_mask	= iop32x_irq_mask,
	.irq_unmask	= iop32x_irq_unmask,
};

static asmlinkage void
__exception_irq_entry iop32x_handle_irq(struct pt_regs *regs)
{
	u32 stat;

	if (!READ_ONCE(iop32x_cp6_enabled)) {
		enable_cp6();
		wait_cp6();
	}

	stat = iintsrc_read();

	if (stat) {
		handle_IRQ(__fls(stat), regs);
		return;
	}
}

void __init iop32x_init_irq(void)
{
	int i;

	iop_init_cp6_handler();

	intctl_write(0);
	intstr_write(0);
	if (machine_is_glantank() ||
	    machine_is_iq80321() ||
	    machine_is_iq31244() ||
	    machine_is_n2100() ||
	    machine_is_em7210())
		*IOP3XX_PCIIRSR = 0x0f;

	for (i = 0; i < NR_IRQS; i++) {
		irq_set_chip_and_handler(i, &ext_chip, handle_level_irq);
		irq_clear_status_flags(i, IRQ_NOREQUEST | IRQ_NOPROBE);
	}

	set_handle_irq(iop32x_handle_irq);
}
