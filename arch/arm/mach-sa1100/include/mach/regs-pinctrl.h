/* SPDX-License-Identifier: GPL-2.0 */
/*
 * SA-1100 Peripheral Pin Controller (PPC) control registers
 *
 *	Author  	Copyright (c) Marc A. Viredaz, 1998
 *	        	DEC Western Research Laboratory, Palo Alto, CA
 *	Date    	January 1998 (April 1997)
 */
#ifndef __MACH_SA1100_REG_PINCTRL_H
#define __MACH_SA1100_REG_PINCTRL_H

#include <mach/hardware.h>
#include <mach/bitfield.h>

/*
 * Peripheral Pin Controller (PPC) control registers
 *
 * Registers
 *    PPDR      	Peripheral Pin Controller (PPC) Pin Direction
 *              	Register (read/write).
 *    PPSR      	Peripheral Pin Controller (PPC) Pin State Register
 *              	(read/write).
 *    PPAR      	Peripheral Pin Controller (PPC) Pin Assignment
 *              	Register (read/write).
 *    PSDR      	Peripheral Pin Controller (PPC) Sleep-mode pin
 *              	Direction Register (read/write).
 *    PPFR      	Peripheral Pin Controller (PPC) Pin Flag Register
 *              	(read).
 */

#define PPDR		__REG(0x90060000)  /* PPC Pin Direction Reg.          */
#define PPSR		__REG(0x90060004)  /* PPC Pin State Reg.              */
#define PPAR		__REG(0x90060008)  /* PPC Pin Assignment Reg.         */
#define PSDR		__REG(0x9006000C)  /* PPC Sleep-mode pin Direction Reg. */
#define PPFR		__REG(0x90060010)  /* PPC Pin Flag Reg.               */

#define PPC_LDD(Nb)	        	/* LCD Data [0..7]                 */ \
                	(0x00000001 << (Nb))
#define PPC_LDD0	PPC_LDD (0)	/* LCD Data [0]                    */
#define PPC_LDD1	PPC_LDD (1)	/* LCD Data [1]                    */
#define PPC_LDD2	PPC_LDD (2)	/* LCD Data [2]                    */
#define PPC_LDD3	PPC_LDD (3)	/* LCD Data [3]                    */
#define PPC_LDD4	PPC_LDD (4)	/* LCD Data [4]                    */
#define PPC_LDD5	PPC_LDD (5)	/* LCD Data [5]                    */
#define PPC_LDD6	PPC_LDD (6)	/* LCD Data [6]                    */
#define PPC_LDD7	PPC_LDD (7)	/* LCD Data [7]                    */
#define PPC_L_PCLK	0x00000100	/* LCD Pixel CLocK                 */
#define PPC_L_LCLK	0x00000200	/* LCD Line CLocK                  */
#define PPC_L_FCLK	0x00000400	/* LCD Frame CLocK                 */
#define PPC_L_BIAS	0x00000800	/* LCD AC BIAS                     */
                	        	/* ser. port 1:                    */
#define PPC_TXD1	0x00001000	/*  SDLC/UART Transmit Data 1      */
#define PPC_RXD1	0x00002000	/*  SDLC/UART Receive Data 1       */
                	        	/* ser. port 2:                    */
#define PPC_TXD2	0x00004000	/*  IPC Transmit Data 2            */
#define PPC_RXD2	0x00008000	/*  IPC Receive Data 2             */
                	        	/* ser. port 3:                    */
#define PPC_TXD3	0x00010000	/*  UART Transmit Data 3           */
#define PPC_RXD3	0x00020000	/*  UART Receive Data 3            */
                	        	/* ser. port 4:                    */
#define PPC_TXD4	0x00040000	/*  MCP/SSP Transmit Data 4        */
#define PPC_RXD4	0x00080000	/*  MCP/SSP Receive Data 4         */
#define PPC_SCLK	0x00100000	/*  MCP/SSP Sample CLocK           */
#define PPC_SFRM	0x00200000	/*  MCP/SSP Sample FRaMe           */

#define PPDR_In 	0       	/* Input                           */
#define PPDR_Out	1       	/* Output                          */

                	        	/* ser. port 1:                    */
#define PPAR_UPR	0x00001000	/*  UART Pin Reassignment          */
#define PPAR_UARTTR	(PPAR_UPR*0)	/*   UART on TXD_1 & RXD_1         */
#define PPAR_UARTGPIO	(PPAR_UPR*1)	/*   UART on GPIO [14:15]          */
                	        	/* ser. port 4:                    */
#define PPAR_SPR	0x00040000	/*  SSP Pin Reassignment           */
#define PPAR_SSPTRSS	(PPAR_SPR*0)	/*   SSP on TXD_C, RXD_C, SCLK_C,  */
                	        	/*   & SFRM_C                      */
#define PPAR_SSPGPIO	(PPAR_SPR*1)	/*   SSP on GPIO [10:13]           */

#define PSDR_OutL	0       	/* Output Low in sleep mode        */
#define PSDR_Flt	1       	/* Floating (input) in sleep mode  */

#define PPFR_LCD	0x00000001	/* LCD controller                  */
#define PPFR_SP1TX	0x00001000	/* Ser. Port 1 SDLC/UART Transmit  */
#define PPFR_SP1RX	0x00002000	/* Ser. Port 1 SDLC/UART Receive   */
#define PPFR_SP2TX	0x00004000	/* Ser. Port 2 ICP Transmit        */
#define PPFR_SP2RX	0x00008000	/* Ser. Port 2 ICP Receive         */
#define PPFR_SP3TX	0x00010000	/* Ser. Port 3 UART Transmit       */
#define PPFR_SP3RX	0x00020000	/* Ser. Port 3 UART Receive        */
#define PPFR_SP4	0x00040000	/* Ser. Port 4 MCP/SSP             */
#define PPFR_PerEn	0       	/* Peripheral Enabled              */
#define PPFR_PPCEn	1       	/* PPC Enabled                     */

#endif
