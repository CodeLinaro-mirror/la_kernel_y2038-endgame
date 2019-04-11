/*
 * arch/arm/mach-ks8695/include/mach/regs-misc.h
 *
 * Copyright (C) 2006 Andrew Victor
 *
 * KS8695 - Miscellaneous Registers
 *
 * This file is licensed under  the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef KS8695_MISC_H
#define KS8695_MISC_H

#define KS8695_MISC_OFFSET	(0xF0000 + 0xEA00)
#define KS8695_MISC_VA		(KS8695_IO_VA + KS8695_MISC_OFFSET)
#define KS8695_MISC_PA		(KS8695_IO_PA + KS8695_MISC_OFFSET)

/*
 * Miscellaneous registers
 */
#define KS8695_DID		(0x00)		/* Device ID */
#define KS8695_RID		(0x04)		/* Revision ID */

/* Device ID Register */
#define DID_ID			(0xffff << 0)	/* Device ID */

/* Revision ID Register */
#define RID_SUBID		(0xf << 4)	/* Sub-Device ID */
#define RID_REVISION		(0xf << 0)	/* Revision ID */

#endif
