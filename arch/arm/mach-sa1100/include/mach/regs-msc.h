/* SPDX-License-Identifier: GPL-2.0 */
/*
 * SA-1100 Static memory control registers
 *
 *	Author  	Copyright (c) Marc A. Viredaz, 1998
 *	        	DEC Western Research Laboratory, Palo Alto, CA
 *	Date    	January 1998 (April 1997)
 */
#ifndef __MACH_SA1100_REG_MSC_H
#define __MACH_SA1100_REG_MSC_H

#include <mach/hardware.h>
#include <mach/bitfield.h>

/*
 * Registers
 *    MSC0      	Memory system: Static memory Control register 0
 *              	(read/write).
 *    MSC1      	Memory system: Static memory Control register 1
 *              	(read/write).
 *
 * Clocks
 *    fcpu, Tcpu	Frequency, period of the CPU core clock (CCLK).
 *    fmem, Tmem	Frequency, period of the memory clock (fmem = fcpu/2).
 */

#define MSC0		__REG(0xa0000010)  /* Static memory Control reg. 0 */
#define MSC1		__REG(0xa0000014)  /* Static memory Control reg. 1 */
#define MSC2		__REG(0xa000002c)  /* Static memory Control reg. 2, not contiguous   */

#define MSC_Bnk(Nb)	        	/* static memory Bank [0..3]       */ \
                	Fld (16, ((Nb) Modulo 2)*16)
#define MSC0_Bnk0	MSC_Bnk (0)	/* static memory Bank 0            */
#define MSC0_Bnk1	MSC_Bnk (1)	/* static memory Bank 1            */
#define MSC1_Bnk2	MSC_Bnk (2)	/* static memory Bank 2            */
#define MSC1_Bnk3	MSC_Bnk (3)	/* static memory Bank 3            */

#define MSC_RT  	Fld (2, 0)	/* ROM/static memory Type          */
#define MSC_NonBrst	        	/*  Non-Burst static memory        */ \
                	(0 << FShft (MSC_RT))
#define MSC_SRAM	        	/*  32-bit byte-writable SRAM      */ \
                	(1 << FShft (MSC_RT))
#define MSC_Brst4	        	/*  Burst-of-4 static memory       */ \
                	(2 << FShft (MSC_RT))
#define MSC_Brst8	        	/*  Burst-of-8 static memory       */ \
                	(3 << FShft (MSC_RT))
#define MSC_RBW 	0x0004  	/* ROM/static memory Bus Width     */
#define MSC_32BitStMem	(MSC_RBW*0)	/*  32-Bit Static Memory           */
#define MSC_16BitStMem	(MSC_RBW*1)	/*  16-Bit Static Memory           */
#define MSC_RDF 	Fld (5, 3)	/* ROM/static memory read Delay    */
                	        	/* First access - 1(.5) [Tmem]     */
#define MSC_1stRdAcc(Tcpu)      	/*  1st Read Access time (burst    */ \
                	        	/*  static memory) [3..65 Tcpu]    */ \
                	((((Tcpu) - 3)/2) << FShft (MSC_RDF))
#define MSC_Ceil1stRdAcc(Tcpu)  	/*  Ceil. of 1stRdAcc [3..65 Tcpu] */ \
                	((((Tcpu) - 2)/2) << FShft (MSC_RDF))
#define MSC_RdAcc(Tcpu)	        	/*  Read Access time (non-burst    */ \
                	        	/*  static memory) [2..64 Tcpu]    */ \
                	((((Tcpu) - 2)/2) << FShft (MSC_RDF))
#define MSC_CeilRdAcc(Tcpu)     	/*  Ceil. of RdAcc [2..64 Tcpu]    */ \
                	((((Tcpu) - 1)/2) << FShft (MSC_RDF))
#define MSC_RDN 	Fld (5, 8)	/* ROM/static memory read Delay    */
                	        	/* Next access - 1 [Tmem]          */
#define MSC_NxtRdAcc(Tcpu)      	/*  Next Read Access time (burst   */ \
                	        	/*  static memory) [2..64 Tcpu]    */ \
                	((((Tcpu) - 2)/2) << FShft (MSC_RDN))
#define MSC_CeilNxtRdAcc(Tcpu)  	/*  Ceil. of NxtRdAcc [2..64 Tcpu] */ \
                	((((Tcpu) - 1)/2) << FShft (MSC_RDN))
#define MSC_WrAcc(Tcpu)	        	/*  Write Access time (non-burst   */ \
                	        	/*  static memory) [2..64 Tcpu]    */ \
                	((((Tcpu) - 2)/2) << FShft (MSC_RDN))
#define MSC_CeilWrAcc(Tcpu)     	/*  Ceil. of WrAcc [2..64 Tcpu]    */ \
                	((((Tcpu) - 1)/2) << FShft (MSC_RDN))
#define MSC_RRR 	Fld (3, 13)	/* ROM/static memory RecoveRy      */
                	        	/* time/2 [Tmem]                   */
#define MSC_Rec(Tcpu)	        	/*  Recovery time [0..28 Tcpu]     */ \
                	(((Tcpu)/4) << FShft (MSC_RRR))
#define MSC_CeilRec(Tcpu)       	/*  Ceil. of Rec [0..28 Tcpu]      */ \
                	((((Tcpu) + 3)/4) << FShft (MSC_RRR))

#endif
