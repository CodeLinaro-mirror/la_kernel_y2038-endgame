// SPDX-License-Identifier: GPL-2.0-only
#ifndef _IXP4XX_ETH_H
#define _IXP4XX_ETH_H

#include <linux/types.h>

struct eth_regs {
	u32 tx_control[2], __res1[2];		/* 000 */
	u32 rx_control[2], __res2[2];		/* 010 */
	u32 random_seed, __res3[3];		/* 020 */
	u32 partial_empty_threshold, __res4;	/* 030 */
	u32 partial_full_threshold, __res5;	/* 038 */
	u32 tx_start_bytes, __res6[3];		/* 040 */
	u32 tx_deferral, rx_deferral, __res7[2];/* 050 */
	u32 tx_2part_deferral[2], __res8[2];	/* 060 */
	u32 slot_time, __res9[3];		/* 070 */
	u32 mdio_command[4];			/* 080 */
	u32 mdio_status[4];			/* 090 */
	u32 mcast_mask[6], __res10[2];		/* 0A0 */
	u32 mcast_addr[6], __res11[2];		/* 0C0 */
	u32 int_clock_threshold, __res12[3];	/* 0E0 */
	u32 hw_addr[6], __res13[61];		/* 0F0 */
	u32 core_control;			/* 1FC */
};

/* Core Control Register */
#define CORE_RESET		0x01
#define CORE_RX_FIFO_FLUSH	0x02
#define CORE_TX_FIFO_FLUSH	0x04
#define CORE_SEND_JAM		0x08
#define CORE_MDC_EN		0x10 /* MDIO using NPE-B ETH-0 only */

#define DEFAULT_TX_CNTRL0	(TX_CNTRL0_TX_EN | TX_CNTRL0_RETRY |	\
				 TX_CNTRL0_PAD_EN | TX_CNTRL0_APPEND_FCS | \
				 TX_CNTRL0_2DEFER)
#define DEFAULT_RX_CNTRL0	RX_CNTRL0_RX_EN
#define DEFAULT_CORE_CNTRL	CORE_MDC_EN

#define IXP4XX_MDIO_BUS_ID "ixp4xx-eth-0"

#endif
