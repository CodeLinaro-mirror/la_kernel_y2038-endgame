/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/linkage.h>
#include <asm/assembler.h>
/*
 * Function: v6_early_abort
 *
 * Params  : r2 = pt_regs
 *	   : r4 = aborted context pc
 *	   : r5 = aborted context psr
 *
 * Returns : r4 - r11, r13 preserved
 *
 * Purpose : obtain information about current aborted instruction.
 * Note: we read user space.  This means we might cause a data
 * abort here if the I-TLB and D-TLB aren't seeing the same
 * picture.  Unfortunately, this does happen.  We live with it.
 */
	.arch	armv6k
	.align	5
ENTRY(v6_early_abort)
	mrc	p15, 0, r1, c5, c0, 0		@ get FSR
	mrc	p15, 0, r0, c6, c0, 0		@ get FAR
	uaccess_disable ip			@ disable userspace access
	b	do_DataAbort
