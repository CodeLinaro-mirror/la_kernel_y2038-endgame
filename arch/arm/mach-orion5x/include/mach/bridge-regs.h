/*
 * arch/arm/mach-orion5x/include/mach/bridge-regs.h
 *
 * Orion CPU Bridge Registers
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef __ASM_ARCH_BRIDGE_REGS_H
#define __ASM_ARCH_BRIDGE_REGS_H

#include <mach/orion5x.h>

#define CPU_CONTROL		(BRIDGE_VIRT_BASE + 0x0104)

#define RSTOUTn_MASK		(BRIDGE_VIRT_BASE + 0x0108)
#define RSTOUTn_MASK_PHYS	(BRIDGE_PHYS_BASE + 0x0108)
#define SOFT_RESET_OUT_EN	0x00000004

#define SYSTEM_SOFT_RESET	(BRIDGE_VIRT_BASE + 0x010c)
#define SOFT_RESET		0x00000001

#define BRIDGE_INT_TIMER1_CLR	(~0x0004)

#define IRQ_VIRT_BASE		(BRIDGE_VIRT_BASE + 0x0200)
#define MAIN_IRQ_CAUSE		(BRIDGE_VIRT_BASE + 0x0200)
#define MAIN_IRQ_MASK		(BRIDGE_VIRT_BASE + 0x0204)

#define TIMER_VIRT_BASE		(BRIDGE_VIRT_BASE + 0x0300)
#define TIMER_PHYS_BASE		(BRIDGE_PHYS_BASE + 0x0300)

#endif
