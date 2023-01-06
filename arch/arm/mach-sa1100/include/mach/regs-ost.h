/* SPDX-License-Identifier: GPL-2.0 */
/*
 * SA-1100  Operating System (OS) timer control registers
 *
 *	Author  	Copyright (c) Marc A. Viredaz, 1998
 *	        	DEC Western Research Laboratory, Palo Alto, CA
 *	Date    	January 1998 (April 1997)
 */
#ifndef __MACH_SA1100_REG_OST_H
#define __MACH_SA1100_REG_OST_H

#include <mach/hardware.h>

/*
 * Registers
 *    OSMR0     	Operating System (OS) timer Match Register 0
 *              	(read/write).
 *    OSMR1     	Operating System (OS) timer Match Register 1
 *              	(read/write).
 *    OSMR2     	Operating System (OS) timer Match Register 2
 *              	(read/write).
 *    OSMR3     	Operating System (OS) timer Match Register 3
 *              	(read/write).
 *    OSCR      	Operating System (OS) timer Counter Register
 *              	(read/write).
 *    OSSR      	Operating System (OS) timer Status Register
 *              	(read/write).
 *    OWER      	Operating System (OS) timer Watch-dog Enable Register
 *              	(read/write).
 *    OIER      	Operating System (OS) timer Interrupt Enable Register
 *              	(read/write).
 */

#define OSMR0  		io_p2v(0x90000000)  /* OS timer Match Reg. 0 */
#define OSMR1  		io_p2v(0x90000004)  /* OS timer Match Reg. 1 */
#define OSMR2  		io_p2v(0x90000008)  /* OS timer Match Reg. 2 */
#define OSMR3  		io_p2v(0x9000000c)  /* OS timer Match Reg. 3 */
#define OSCR   		io_p2v(0x90000010)  /* OS timer Counter Reg. */
#define OSSR   		io_p2v(0x90000014)  /* OS timer Status Reg. */
#define OWER   		io_p2v(0x90000018)  /* OS timer Watch-dog Enable Reg. */
#define OIER  	 	io_p2v(0x9000001C)  /* OS timer Interrupt Enable Reg. */

#define OSSR_M(Nb)	        	/* Match detected [0..3]           */ \
                	(0x00000001 << (Nb))
#define OSSR_M0 	OSSR_M (0)	/* Match detected 0                */
#define OSSR_M1 	OSSR_M (1)	/* Match detected 1                */
#define OSSR_M2 	OSSR_M (2)	/* Match detected 2                */
#define OSSR_M3 	OSSR_M (3)	/* Match detected 3                */

#define OWER_WME	0x00000001	/* Watch-dog Match Enable          */
                	        	/* (set only)                      */

#define OIER_E(Nb)	        	/* match interrupt Enable [0..3]   */ \
                	(0x00000001 << (Nb))
#define OIER_E0 	OIER_E (0)	/* match interrupt Enable 0        */
#define OIER_E1 	OIER_E (1)	/* match interrupt Enable 1        */
#define OIER_E2 	OIER_E (2)	/* match interrupt Enable 2        */
#define OIER_E3 	OIER_E (3)	/* match interrupt Enable 3        */

#endif
