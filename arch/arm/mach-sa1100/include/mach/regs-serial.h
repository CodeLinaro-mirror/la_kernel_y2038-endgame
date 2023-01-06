/* SPDX-License-Identifier: GPL-2.0 */
/*
 * SA-1100 Universal Asynchronous Receiver/Transmitter (UART) control registers
 *
 *	Author  	Copyright (c) Marc A. Viredaz, 1998
 *	        	DEC Western Research Laboratory, Palo Alto, CA
 *	Date    	January 1998 (April 1997)
 */
#ifndef __MACH_SA1100_REG_SERIAL_H
#define __MACH_SA1100_REG_SERIAL_H

#include <mach/hardware.h>
#include <mach/bitfield.h>

/*
 * Registers
 *    Ser1UTCR0 	Serial port 1 Universal Asynchronous
 *              	Receiver/Transmitter (UART) Control Register 0
 *              	(read/write).
 *    Ser1UTCR1 	Serial port 1 Universal Asynchronous
 *              	Receiver/Transmitter (UART) Control Register 1
 *              	(read/write).
 *    Ser1UTCR2 	Serial port 1 Universal Asynchronous
 *              	Receiver/Transmitter (UART) Control Register 2
 *              	(read/write).
 *    Ser1UTCR3 	Serial port 1 Universal Asynchronous
 *              	Receiver/Transmitter (UART) Control Register 3
 *              	(read/write).
 *    Ser1UTDR  	Serial port 1 Universal Asynchronous
 *              	Receiver/Transmitter (UART) Data Register
 *              	(read/write).
 *    Ser1UTSR0 	Serial port 1 Universal Asynchronous
 *              	Receiver/Transmitter (UART) Status Register 0
 *              	(read/write).
 *    Ser1UTSR1 	Serial port 1 Universal Asynchronous
 *              	Receiver/Transmitter (UART) Status Register 1 (read).
 *
 *    Ser2UTCR0 	Serial port 2 Universal Asynchronous
 *              	Receiver/Transmitter (UART) Control Register 0
 *              	(read/write).
 *    Ser2UTCR1 	Serial port 2 Universal Asynchronous
 *              	Receiver/Transmitter (UART) Control Register 1
 *              	(read/write).
 *    Ser2UTCR2 	Serial port 2 Universal Asynchronous
 *              	Receiver/Transmitter (UART) Control Register 2
 *              	(read/write).
 *    Ser2UTCR3 	Serial port 2 Universal Asynchronous
 *              	Receiver/Transmitter (UART) Control Register 3
 *              	(read/write).
 *    Ser2UTCR4 	Serial port 2 Universal Asynchronous
 *              	Receiver/Transmitter (UART) Control Register 4
 *              	(read/write).
 *    Ser2UTDR  	Serial port 2 Universal Asynchronous
 *              	Receiver/Transmitter (UART) Data Register
 *              	(read/write).
 *    Ser2UTSR0 	Serial port 2 Universal Asynchronous
 *              	Receiver/Transmitter (UART) Status Register 0
 *              	(read/write).
 *    Ser2UTSR1 	Serial port 2 Universal Asynchronous
 *              	Receiver/Transmitter (UART) Status Register 1 (read).
 *
 *    Ser3UTCR0 	Serial port 3 Universal Asynchronous
 *              	Receiver/Transmitter (UART) Control Register 0
 *              	(read/write).
 *    Ser3UTCR1 	Serial port 3 Universal Asynchronous
 *              	Receiver/Transmitter (UART) Control Register 1
 *              	(read/write).
 *    Ser3UTCR2 	Serial port 3 Universal Asynchronous
 *              	Receiver/Transmitter (UART) Control Register 2
 *              	(read/write).
 *    Ser3UTCR3 	Serial port 3 Universal Asynchronous
 *              	Receiver/Transmitter (UART) Control Register 3
 *              	(read/write).
 *    Ser3UTDR  	Serial port 3 Universal Asynchronous
 *              	Receiver/Transmitter (UART) Data Register
 *              	(read/write).
 *    Ser3UTSR0 	Serial port 3 Universal Asynchronous
 *              	Receiver/Transmitter (UART) Status Register 0
 *              	(read/write).
 *    Ser3UTSR1 	Serial port 3 Universal Asynchronous
 *              	Receiver/Transmitter (UART) Status Register 1 (read).
 *
 * Clocks
 *    fxtl, Txtl	Frequency, period of the system crystal (3.6864 MHz
 *              	or 3.5795 MHz).
 *    fua, Tua  	Frequency, period of the UART communication.
 */

#define _UTCR0(Nb)	__REG(0x80010000 + ((Nb) - 1)*0x00020000)  /* UART Control Reg. 0 [1..3] */
#define _UTCR1(Nb)	__REG(0x80010004 + ((Nb) - 1)*0x00020000)  /* UART Control Reg. 1 [1..3] */
#define _UTCR2(Nb)	__REG(0x80010008 + ((Nb) - 1)*0x00020000)  /* UART Control Reg. 2 [1..3] */
#define _UTCR3(Nb)	__REG(0x8001000C + ((Nb) - 1)*0x00020000)  /* UART Control Reg. 3 [1..3] */
#define _UTCR4(Nb)	__REG(0x80010010 + ((Nb) - 1)*0x00020000)  /* UART Control Reg. 4 [2] */
#define _UTDR(Nb)	__REG(0x80010014 + ((Nb) - 1)*0x00020000)  /* UART Data Reg. [1..3] */
#define _UTSR0(Nb)	__REG(0x8001001C + ((Nb) - 1)*0x00020000)  /* UART Status Reg. 0 [1..3] */
#define _UTSR1(Nb)	__REG(0x80010020 + ((Nb) - 1)*0x00020000)  /* UART Status Reg. 1 [1..3] */

#define Ser1UTCR0	_UTCR0 (1)	/* Ser. port 1 UART Control Reg. 0 */
#define Ser1UTCR1	_UTCR1 (1)	/* Ser. port 1 UART Control Reg. 1 */
#define Ser1UTCR2	_UTCR2 (1)	/* Ser. port 1 UART Control Reg. 2 */
#define Ser1UTCR3	_UTCR3 (1)	/* Ser. port 1 UART Control Reg. 3 */
#define Ser1UTDR	_UTDR (1)	/* Ser. port 1 UART Data Reg.      */
#define Ser1UTSR0	_UTSR0 (1)	/* Ser. port 1 UART Status Reg. 0  */
#define Ser1UTSR1	_UTSR1 (1)	/* Ser. port 1 UART Status Reg. 1  */

#define Ser2UTCR0	_UTCR0 (2)	/* Ser. port 2 UART Control Reg. 0 */
#define Ser2UTCR1	_UTCR1 (2)	/* Ser. port 2 UART Control Reg. 1 */
#define Ser2UTCR2	_UTCR2 (2)	/* Ser. port 2 UART Control Reg. 2 */
#define Ser2UTCR3	_UTCR3 (2)	/* Ser. port 2 UART Control Reg. 3 */
#define Ser2UTCR4	_UTCR4 (2)	/* Ser. port 2 UART Control Reg. 4 */
#define Ser2UTDR	_UTDR (2)	/* Ser. port 2 UART Data Reg.      */
#define Ser2UTSR0	_UTSR0 (2)	/* Ser. port 2 UART Status Reg. 0  */
#define Ser2UTSR1	_UTSR1 (2)	/* Ser. port 2 UART Status Reg. 1  */

#define Ser3UTCR0	_UTCR0 (3)	/* Ser. port 3 UART Control Reg. 0 */
#define Ser3UTCR1	_UTCR1 (3)	/* Ser. port 3 UART Control Reg. 1 */
#define Ser3UTCR2	_UTCR2 (3)	/* Ser. port 3 UART Control Reg. 2 */
#define Ser3UTCR3	_UTCR3 (3)	/* Ser. port 3 UART Control Reg. 3 */
#define Ser3UTDR	_UTDR (3)	/* Ser. port 3 UART Data Reg.      */
#define Ser3UTSR0	_UTSR0 (3)	/* Ser. port 3 UART Status Reg. 0  */
#define Ser3UTSR1	_UTSR1 (3)	/* Ser. port 3 UART Status Reg. 1  */

/* Those are still used in some places */
#define _Ser1UTCR0	__PREG(Ser1UTCR0)
#define _Ser2UTCR0	__PREG(Ser2UTCR0)
#define _Ser3UTCR0	__PREG(Ser3UTCR0)

/* Register offsets */
#define UTCR0		0x00
#define UTCR1		0x04
#define UTCR2		0x08
#define UTCR3		0x0c
#define UTDR		0x14
#define UTSR0		0x1c
#define UTSR1		0x20

#define UTCR0_PE	0x00000001	/* Parity Enable                   */
#define UTCR0_OES	0x00000002	/* Odd/Even parity Select          */
#define UTCR0_OddPar	(UTCR0_OES*0)	/*  Odd Parity                     */
#define UTCR0_EvenPar	(UTCR0_OES*1)	/*  Even Parity                    */
#define UTCR0_SBS	0x00000004	/* Stop Bit Select                 */
#define UTCR0_1StpBit	(UTCR0_SBS*0)	/*  1 Stop Bit per frame           */
#define UTCR0_2StpBit	(UTCR0_SBS*1)	/*  2 Stop Bits per frame          */
#define UTCR0_DSS	0x00000008	/* Data Size Select                */
#define UTCR0_7BitData	(UTCR0_DSS*0)	/*  7-Bit Data                     */
#define UTCR0_8BitData	(UTCR0_DSS*1)	/*  8-Bit Data                     */
#define UTCR0_SCE	0x00000010	/* Sample Clock Enable             */
                	        	/* (ser. port 1: GPIO [18],        */
                	        	/* ser. port 3: GPIO [20])         */
#define UTCR0_RCE	0x00000020	/* Receive Clock Edge select       */
#define UTCR0_RcRsEdg	(UTCR0_RCE*0)	/*  Receive clock Rising-Edge      */
#define UTCR0_RcFlEdg	(UTCR0_RCE*1)	/*  Receive clock Falling-Edge     */
#define UTCR0_TCE	0x00000040	/* Transmit Clock Edge select      */
#define UTCR0_TrRsEdg	(UTCR0_TCE*0)	/*  Transmit clock Rising-Edge     */
#define UTCR0_TrFlEdg	(UTCR0_TCE*1)	/*  Transmit clock Falling-Edge    */
#define UTCR0_Ser2IrDA	        	/* Ser. port 2 IrDA settings       */ \
                	(UTCR0_1StpBit + UTCR0_8BitData)

#define UTCR1_BRD	Fld (4, 0)	/* Baud Rate Divisor/16 - 1 [11:8] */
#define UTCR2_BRD	Fld (8, 0)	/* Baud Rate Divisor/16 - 1  [7:0] */
                	        	/* fua = fxtl/(16*(BRD[11:0] + 1)) */
                	        	/* Tua = 16*(BRD [11:0] + 1)*Txtl  */
#define UTCR1_BdRtDiv(Div)      	/*  Baud Rate Divisor [16..65536]  */ \
                	(((Div) - 16)/16 >> FSize (UTCR2_BRD) << \
                	 FShft (UTCR1_BRD))
#define UTCR2_BdRtDiv(Div)      	/*  Baud Rate Divisor [16..65536]  */ \
                	(((Div) - 16)/16 & FAlnMsk (UTCR2_BRD) << \
                	 FShft (UTCR2_BRD))
                	        	/*  fua = fxtl/(16*Floor (Div/16)) */
                	        	/*  Tua = 16*Floor (Div/16)*Txtl   */
#define UTCR1_CeilBdRtDiv(Div)  	/*  Ceil. of BdRtDiv [16..65536]   */ \
                	(((Div) - 1)/16 >> FSize (UTCR2_BRD) << \
                	 FShft (UTCR1_BRD))
#define UTCR2_CeilBdRtDiv(Div)  	/*  Ceil. of BdRtDiv [16..65536]   */ \
                	(((Div) - 1)/16 & FAlnMsk (UTCR2_BRD) << \
                	 FShft (UTCR2_BRD))
                	        	/*  fua = fxtl/(16*Ceil (Div/16))  */
                	        	/*  Tua = 16*Ceil (Div/16)*Txtl    */

#define UTCR3_RXE	0x00000001	/* Receive Enable                  */
#define UTCR3_TXE	0x00000002	/* Transmit Enable                 */
#define UTCR3_BRK	0x00000004	/* BReaK mode                      */
#define UTCR3_RIE	0x00000008	/* Receive FIFO 1/3-to-2/3-full or */
                	        	/* more Interrupt Enable           */
#define UTCR3_TIE	0x00000010	/* Transmit FIFO 1/2-full or less  */
                	        	/* Interrupt Enable                */
#define UTCR3_LBM	0x00000020	/* Look-Back Mode                  */
#define UTCR3_Ser2IrDA	        	/* Ser. port 2 IrDA settings (RIE, */ \
                	        	/* TIE, LBM can be set or cleared) */ \
                	(UTCR3_RXE + UTCR3_TXE)

#define UTCR4_HSE	0x00000001	/* Hewlett-Packard Serial InfraRed */
                	        	/* (HP-SIR) modulation Enable      */
#define UTCR4_NRZ	(UTCR4_HSE*0)	/*  Non-Return to Zero modulation  */
#define UTCR4_HPSIR	(UTCR4_HSE*1)	/*  HP-SIR modulation              */
#define UTCR4_LPM	0x00000002	/* Low-Power Mode                  */
#define UTCR4_Z3_16Bit	(UTCR4_LPM*0)	/*  Zero pulse = 3/16 Bit time     */
#define UTCR4_Z1_6us	(UTCR4_LPM*1)	/*  Zero pulse = 1.6 us            */

#define UTDR_DATA	Fld (8, 0)	/* receive/transmit DATA FIFOs     */
#if 0           	        	/* Hidden receive FIFO bits        */
#define UTDR_PRE	0x00000100	/*  receive PaRity Error (read)    */
#define UTDR_FRE	0x00000200	/*  receive FRaming Error (read)   */
#define UTDR_ROR	0x00000400	/*  Receive FIFO Over-Run (read)   */
#endif /* 0 */

#define UTSR0_TFS	0x00000001	/* Transmit FIFO 1/2-full or less  */
                	        	/* Service request (read)          */
#define UTSR0_RFS	0x00000002	/* Receive FIFO 1/3-to-2/3-full or */
                	        	/* more Service request (read)     */
#define UTSR0_RID	0x00000004	/* Receiver IDle                   */
#define UTSR0_RBB	0x00000008	/* Receive Beginning of Break      */
#define UTSR0_REB	0x00000010	/* Receive End of Break            */
#define UTSR0_EIF	0x00000020	/* Error In FIFO (read)            */

#define UTSR1_TBY	0x00000001	/* Transmitter BusY (read)         */
#define UTSR1_RNE	0x00000002	/* Receive FIFO Not Empty (read)   */
#define UTSR1_TNF	0x00000004	/* Transmit FIFO Not Full (read)   */
#define UTSR1_PRE	0x00000008	/* receive PaRity Error (read)     */
#define UTSR1_FRE	0x00000010	/* receive FRaming Error (read)    */
#define UTSR1_ROR	0x00000020	/* Receive FIFO Over-Run (read)    */

#endif
