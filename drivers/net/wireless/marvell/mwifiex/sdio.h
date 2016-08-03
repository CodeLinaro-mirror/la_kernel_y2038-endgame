/*
 * Marvell Wireless LAN device driver: SDIO specific definitions
 *
 * Copyright (C) 2011-2014, Marvell International Ltd.
 *
 * This software file (the "File") is distributed by Marvell International
 * Ltd. under the terms of the GNU General Public License Version 2, June 1991
 * (the "License").  You may use, redistribute and/or modify this File in
 * accordance with the terms and conditions of the License, a copy of which
 * is available by writing to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA or on the
 * worldwide web at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 *
 * THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE
 * ARE EXPRESSLY DISCLAIMED.  The License provides additional details about
 * this warranty disclaimer.
 */

#ifndef	_MWIFIEX_SDIO_H
#define	_MWIFIEX_SDIO_H


#include <linux/completion.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/card.h>
#include <linux/mmc/host.h>

#include "main.h"

/* data structure for SDIO MPA TX */
struct mwifiex_sdio_mpa_tx {
	/* multiport tx aggregation buffer pointer */
	u8 *buf;
	u32 buf_len;
	u32 pkt_cnt;
	u32 ports;
	u16 start_port;
	u8 enabled;
	u32 buf_size;
	u32 pkt_aggr_limit;
};

struct mwifiex_sdio_mpa_rx {
	u8 *buf;
	u32 buf_len;
	u32 pkt_cnt;
	u32 ports;
	u16 start_port;

	struct sk_buff **skb_arr;
	u32 *len_arr;

	u8 enabled;
	u32 buf_size;
	u32 pkt_aggr_limit;
};

struct sdio_mmc_card {
	struct sdio_func *func;
	struct mwifiex_adapter *adapter;

	struct completion fw_done;
	const char *firmware;
	const struct mwifiex_sdio_card_reg *reg;
	u8 max_ports;
	u8 mp_agg_pkt_limit;
	u16 tx_buf_size;
	u32 mp_tx_agg_buf_size;
	u32 mp_rx_agg_buf_size;

	u32 mp_rd_bitmap;
	u32 mp_wr_bitmap;

	u16 mp_end_port;
	u32 mp_data_port_mask;

	u8 curr_rd_port;
	u8 curr_wr_port;

	u8 *mp_regs;
	bool supports_sdio_new_mode;
	bool has_control_mask;
	bool can_dump_fw;
	bool fw_dump_enh;
	bool can_auto_tdls;
	bool can_ext_scan;

	struct mwifiex_sdio_mpa_tx mpa_tx;
	struct mwifiex_sdio_mpa_rx mpa_rx;

	struct work_struct work;
	unsigned long work_flags;
};

#endif /* _MWIFIEX_SDIO_H */
