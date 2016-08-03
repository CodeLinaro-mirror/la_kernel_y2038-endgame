/*
 * This file contains definitions for mwifiex USB interface driver.
 *
 * Copyright (C) 2012-2014, Marvell International Ltd.
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

#ifndef _MWIFIEX_USB_H
#define _MWIFIEX_USB_H

#include <linux/completion.h>
#include <linux/usb.h>

#define MWIFIEX_TX_DATA_PORT	2
#define MWIFIEX_TX_DATA_URB	6
#define MWIFIEX_RX_DATA_URB	6

struct urb_context {
	struct mwifiex_adapter *adapter;
	struct sk_buff *skb;
	struct urb *urb;
	u8 ep;
};

struct usb_tx_data_port {
	u8 tx_data_ep;
	u8 block_status;
	atomic_t tx_data_urb_pending;
	int tx_data_ix;
	struct urb_context tx_data_list[MWIFIEX_TX_DATA_URB];
};

struct usb_card_rec {
	struct mwifiex_adapter *adapter;
	struct usb_device *udev;
	struct usb_interface *intf;
	struct completion fw_done;
	u8 rx_cmd_ep;
	struct urb_context rx_cmd;
	atomic_t rx_cmd_urb_pending;
	struct urb_context rx_data_list[MWIFIEX_RX_DATA_URB];
	u8 usb_boot_state;
	u8 rx_data_ep;
	atomic_t rx_data_urb_pending;
	u8 tx_cmd_ep;
	atomic_t tx_cmd_urb_pending;
	int bulk_out_maxpktsize;
	struct urb_context tx_cmd;
	u8 mc_resync_flag;
	struct usb_tx_data_port port[MWIFIEX_TX_DATA_PORT];
};

#endif /*_MWIFIEX_USB_H */
