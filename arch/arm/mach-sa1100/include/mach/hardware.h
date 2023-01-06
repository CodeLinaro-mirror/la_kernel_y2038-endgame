/* SPDX-License-Identifier: GPL-2.0 */
/*
 * arch/arm/mach-sa1100/include/mach/hardware.h
 *
 * Copyright (C) 1998 Nicolas Pitre <nico@fluxnic.net>
 *
 * This file contains the hardware definitions for SA1100 architecture
 *
 * 2000/05/23 John Dorsey <john+@cs.cmu.edu>
 *      Definitions for SA1111 added.
 */

#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H


#define UNCACHEABLE_ADDR	0xfa050000	/* ICIP */


/*
 * SA1100 internal I/O mappings
 *
 * We have the following mapping:
 *      phys            virt
 *      80000000        f8000000
 *      90000000        fa000000
 *      a0000000        fc000000
 *      b0000000        fe000000
 */

#define VIO_BASE        0xf8000000	/* virtual start of IO space */
#define VIO_SHIFT       3		/* x = IO space shrink power */
#define PIO_START       0x80000000	/* physical start of IO space */

#define io_p2v( x )             \
   IOMEM( (((x)&0x00ffffff) | (((x)&0x30000000)>>VIO_SHIFT)) + VIO_BASE )
#define io_v2p( x )             \
   ( (((x)&0x00ffffff) | (((x)&(0x30000000>>VIO_SHIFT))<<VIO_SHIFT)) + PIO_START )

#define __MREG(x)	IOMEM(io_p2v(x))

#ifndef __ASSEMBLY__

# define __REG(x)	(*((volatile unsigned long __iomem *)io_p2v(x)))
# define __PREG(x)	(io_v2p((unsigned long)&(x)))

#else

# define __REG(x)	io_p2v(x)
# define __PREG(x)	io_v2p(x)

#endif


/*
 * SA1100 CS line to physical address
 */
#define SA1100_CS0_PHYS	0x00000000
#define SA1100_CS1_PHYS	0x08000000
#define SA1100_CS2_PHYS	0x10000000
#define SA1100_CS3_PHYS	0x18000000
#define SA1100_CS4_PHYS	0x40000000
#define SA1100_CS5_PHYS	0x48000000

#include "SA-1100.h"

#endif  /* _ASM_ARCH_HARDWARE_H */
