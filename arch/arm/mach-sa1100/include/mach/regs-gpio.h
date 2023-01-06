/* SPDX-License-Identifier: GPL-2.0 */
/*
 * SA-1100 General-Purpose Input/Output (GPIO) control registers
 *
 *	Author  	Copyright (c) Marc A. Viredaz, 1998
 *	        	DEC Western Research Laboratory, Palo Alto, CA
 *	Date    	January 1998 (April 1997)
 */
#ifndef __MACH_SA1100_REG_GPIO_H
#define __MACH_SA1100_REG_GPIO_H

#include <mach/hardware.h>
#include <mach/bitfield.h>

/*
 * General-Purpose Input/Output (GPIO) control registers
 *
 * Registers
 *    GPLR      	General-Purpose Input/Output (GPIO) Pin Level
 *              	Register (read).
 *    GPDR      	General-Purpose Input/Output (GPIO) Pin Direction
 *              	Register (read/write).
 *    GPSR      	General-Purpose Input/Output (GPIO) Pin output Set
 *              	Register (write).
 *    GPCR      	General-Purpose Input/Output (GPIO) Pin output Clear
 *              	Register (write).
 *    GRER      	General-Purpose Input/Output (GPIO) Rising-Edge
 *              	detect Register (read/write).
 *    GFER      	General-Purpose Input/Output (GPIO) Falling-Edge
 *              	detect Register (read/write).
 *    GEDR      	General-Purpose Input/Output (GPIO) Edge Detect
 *              	status Register (read/write).
 *    GAFR      	General-Purpose Input/Output (GPIO) Alternate
 *              	Function Register (read/write).
 *
 * Clock
 *    fcpu, Tcpu	Frequency, period of the CPU core clock (CCLK).
 */

#define GPLR		__REG(0x90040000)  /* GPIO Pin Level Reg.             */
#define GPDR		__REG(0x90040004)  /* GPIO Pin Direction Reg.         */
#define GPSR		__REG(0x90040008)  /* GPIO Pin output Set Reg.        */
#define GPCR		__REG(0x9004000C)  /* GPIO Pin output Clear Reg.      */
#define GRER		__REG(0x90040010)  /* GPIO Rising-Edge detect Reg.    */
#define GFER		__REG(0x90040014)  /* GPIO Falling-Edge detect Reg.   */
#define GEDR		__REG(0x90040018)  /* GPIO Edge Detect status Reg.    */
#define GAFR		__REG(0x9004001C)  /* GPIO Alternate Function Reg.    */

#define GPIO_MIN	(0)
#define GPIO_MAX	(27)

#define GPIO_GPIO(Nb)	        	/* GPIO [0..27]                    */ \
                	(0x00000001 << (Nb))
#define GPIO_GPIO0	GPIO_GPIO (0)	/* GPIO  [0]                       */
#define GPIO_GPIO1	GPIO_GPIO (1)	/* GPIO  [1]                       */
#define GPIO_GPIO2	GPIO_GPIO (2)	/* GPIO  [2]                       */
#define GPIO_GPIO3	GPIO_GPIO (3)	/* GPIO  [3]                       */
#define GPIO_GPIO4	GPIO_GPIO (4)	/* GPIO  [4]                       */
#define GPIO_GPIO5	GPIO_GPIO (5)	/* GPIO  [5]                       */
#define GPIO_GPIO6	GPIO_GPIO (6)	/* GPIO  [6]                       */
#define GPIO_GPIO7	GPIO_GPIO (7)	/* GPIO  [7]                       */
#define GPIO_GPIO8	GPIO_GPIO (8)	/* GPIO  [8]                       */
#define GPIO_GPIO9	GPIO_GPIO (9)	/* GPIO  [9]                       */
#define GPIO_GPIO10	GPIO_GPIO (10)	/* GPIO [10]                       */
#define GPIO_GPIO11	GPIO_GPIO (11)	/* GPIO [11]                       */
#define GPIO_GPIO12	GPIO_GPIO (12)	/* GPIO [12]                       */
#define GPIO_GPIO13	GPIO_GPIO (13)	/* GPIO [13]                       */
#define GPIO_GPIO14	GPIO_GPIO (14)	/* GPIO [14]                       */
#define GPIO_GPIO15	GPIO_GPIO (15)	/* GPIO [15]                       */
#define GPIO_GPIO16	GPIO_GPIO (16)	/* GPIO [16]                       */
#define GPIO_GPIO17	GPIO_GPIO (17)	/* GPIO [17]                       */
#define GPIO_GPIO18	GPIO_GPIO (18)	/* GPIO [18]                       */
#define GPIO_GPIO19	GPIO_GPIO (19)	/* GPIO [19]                       */
#define GPIO_GPIO20	GPIO_GPIO (20)	/* GPIO [20]                       */
#define GPIO_GPIO21	GPIO_GPIO (21)	/* GPIO [21]                       */
#define GPIO_GPIO22	GPIO_GPIO (22)	/* GPIO [22]                       */
#define GPIO_GPIO23	GPIO_GPIO (23)	/* GPIO [23]                       */
#define GPIO_GPIO24	GPIO_GPIO (24)	/* GPIO [24]                       */
#define GPIO_GPIO25	GPIO_GPIO (25)	/* GPIO [25]                       */
#define GPIO_GPIO26	GPIO_GPIO (26)	/* GPIO [26]                       */
#define GPIO_GPIO27	GPIO_GPIO (27)	/* GPIO [27]                       */

#define GPIO_LDD(Nb)	        	/* LCD Data [8..15] (O)            */ \
                	GPIO_GPIO ((Nb) - 6)
#define GPIO_LDD8	GPIO_LDD (8)	/* LCD Data  [8] (O)               */
#define GPIO_LDD9	GPIO_LDD (9)	/* LCD Data  [9] (O)               */
#define GPIO_LDD10	GPIO_LDD (10)	/* LCD Data [10] (O)               */
#define GPIO_LDD11	GPIO_LDD (11)	/* LCD Data [11] (O)               */
#define GPIO_LDD12	GPIO_LDD (12)	/* LCD Data [12] (O)               */
#define GPIO_LDD13	GPIO_LDD (13)	/* LCD Data [13] (O)               */
#define GPIO_LDD14	GPIO_LDD (14)	/* LCD Data [14] (O)               */
#define GPIO_LDD15	GPIO_LDD (15)	/* LCD Data [15] (O)               */
                	        	/* ser. port 4:                    */
#define GPIO_SSP_TXD	GPIO_GPIO (10)	/*  SSP Transmit Data (O)          */
#define GPIO_SSP_RXD	GPIO_GPIO (11)	/*  SSP Receive Data (I)           */
#define GPIO_SSP_SCLK	GPIO_GPIO (12)	/*  SSP Sample CLocK (O)           */
#define GPIO_SSP_SFRM	GPIO_GPIO (13)	/*  SSP Sample FRaMe (O)           */
                	        	/* ser. port 1:                    */
#define GPIO_UART_TXD	GPIO_GPIO (14)	/*  UART Transmit Data (O)         */
#define GPIO_UART_RXD	GPIO_GPIO (15)	/*  UART Receive Data (I)          */
#define GPIO_SDLC_SCLK	GPIO_GPIO (16)	/*  SDLC Sample CLocK (I/O)        */
#define GPIO_SDLC_AAF	GPIO_GPIO (17)	/*  SDLC Abort After Frame (O)     */
#define GPIO_UART_SCLK1	GPIO_GPIO (18)	/*  UART Sample CLocK 1 (I)        */
                	        	/* ser. port 4:                    */
#define GPIO_SSP_CLK	GPIO_GPIO (19)	/*  SSP external CLocK (I)         */
                	        	/* ser. port 3:                    */
#define GPIO_UART_SCLK3	GPIO_GPIO (20)	/*  UART Sample CLocK 3 (I)        */
                	        	/* ser. port 4:                    */
#define GPIO_MCP_CLK	GPIO_GPIO (21)	/*  MCP CLocK (I)                  */
                	        	/* test controller:                */
#define GPIO_TIC_ACK	GPIO_GPIO (21)	/*  TIC ACKnowledge (O)            */
#define GPIO_MBGNT	GPIO_GPIO (21)	/*  Memory Bus GraNT (O)           */
#define GPIO_TREQA	GPIO_GPIO (22)	/*  TIC REQuest A (I)              */
#define GPIO_MBREQ	GPIO_GPIO (22)	/*  Memory Bus REQuest (I)         */
#define GPIO_TREQB	GPIO_GPIO (23)	/*  TIC REQuest B (I)              */
#define GPIO_1Hz	GPIO_GPIO (25)	/* 1 Hz clock (O)                  */
#define GPIO_RCLK	GPIO_GPIO (26)	/* internal (R) CLocK (O, fcpu/2)  */
#define GPIO_32_768kHz	GPIO_GPIO (27)	/* 32.768 kHz clock (O, RTC)       */

#define GPDR_In 	0       	/* Input                           */
#define GPDR_Out	1       	/* Output                          */

#endif
