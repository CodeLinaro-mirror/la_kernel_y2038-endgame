/* @file mwifiex_pcie.h
 *
 * @brief This file contains definitions for PCI-E interface.
 * driver.
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

#ifndef	_MWIFIEX_PCIE_H
#define	_MWIFIEX_PCIE_H

#include    <linux/completion.h>
#include    <linux/pci.h>
#include    <linux/interrupt.h>

#include    "decl.h"
#include    "main.h"

#define PCIE8766_DEFAULT_FW_NAME "mrvl/pcie8766_uapsta.bin"
#define PCIE8897_DEFAULT_FW_NAME "mrvl/pcie8897_uapsta.bin"
#define PCIE8897_A0_FW_NAME "mrvl/pcie8897_uapsta_a0.bin"
#define PCIE8897_B0_FW_NAME "mrvl/pcie8897_uapsta.bin"
#define PCIEUART8997_FW_NAME_V4 "mrvl/pcieuart8997_combo_v4.bin"
#define PCIEUSB8997_FW_NAME_V4 "mrvl/pcieusb8997_combo_v4.bin"
#define PCIE8997_DEFAULT_WIFIFW_NAME "mrvl/pcie8997_wlan_v4.bin"

#define PCIE_VENDOR_ID_MARVELL              (0x11ab)
#define PCIE_VENDOR_ID_V2_MARVELL           (0x1b4b)
#define PCIE_DEVICE_ID_MARVELL_88W8766P		(0x2b30)
#define PCIE_DEVICE_ID_MARVELL_88W8897		(0x2b38)
#define PCIE_DEVICE_ID_MARVELL_88W8997		(0x2b42)

#define PCIE8897_A0	0x1100
#define PCIE8897_B0	0x1200
#define PCIE8997_A0	0x10
#define PCIE8997_A1	0x11
#define CHIP_VER_PCIEUART	0x3
#define CHIP_MAGIC_VALUE	0x24

/* Constants for Buffer Descriptor (BD) rings */
#define MWIFIEX_MAX_TXRX_BD			0x20
#define MWIFIEX_TXBD_MASK			0x3F
#define MWIFIEX_RXBD_MASK			0x3F

#define MWIFIEX_MAX_EVT_BD			0x08
#define MWIFIEX_EVTBD_MASK			0x0f

struct mwifiex_pcie_device {
	const struct mwifiex_pcie_card_reg *reg;
	u16 blksz_fw_dl;
	u16 tx_buf_size;
	bool can_dump_fw;
	struct memory_type_mapping *mem_type_mapping_tbl;
	u8 num_mem_types;
	bool can_ext_scan;
};

#define MWIFIEX_NUM_MSIX_VECTORS   4

struct mwifiex_msix_context {
	struct pci_dev *dev;
	u16 msg_id;
};

struct pcie_service_card {
	struct pci_dev *dev;
	struct mwifiex_adapter *adapter;
	struct mwifiex_pcie_device pcie;
	struct completion fw_done;

	u8 txbd_flush;
	u32 txbd_wrptr;
	u32 txbd_rdptr;
	u32 txbd_ring_size;
	u8 *txbd_ring_vbase;
	dma_addr_t txbd_ring_pbase;
	void *txbd_ring[MWIFIEX_MAX_TXRX_BD];
	struct sk_buff *tx_buf_list[MWIFIEX_MAX_TXRX_BD];

	u32 rxbd_wrptr;
	u32 rxbd_rdptr;
	u32 rxbd_ring_size;
	u8 *rxbd_ring_vbase;
	dma_addr_t rxbd_ring_pbase;
	void *rxbd_ring[MWIFIEX_MAX_TXRX_BD];
	struct sk_buff *rx_buf_list[MWIFIEX_MAX_TXRX_BD];

	u32 evtbd_wrptr;
	u32 evtbd_rdptr;
	u32 evtbd_ring_size;
	u8 *evtbd_ring_vbase;
	dma_addr_t evtbd_ring_pbase;
	void *evtbd_ring[MWIFIEX_MAX_EVT_BD];
	struct sk_buff *evt_buf_list[MWIFIEX_MAX_EVT_BD];

	struct sk_buff *cmd_buf;
	struct sk_buff *cmdrsp_buf;
	u8 *sleep_cookie_vbase;
	dma_addr_t sleep_cookie_pbase;
	void __iomem *pci_mmap;
	void __iomem *pci_mmap1;
	int msi_enable;
	int msix_enable;
#ifdef CONFIG_PCI
	struct msix_entry msix_entries[MWIFIEX_NUM_MSIX_VECTORS];
#endif
	struct mwifiex_msix_context msix_ctx[MWIFIEX_NUM_MSIX_VECTORS];
	struct mwifiex_msix_context share_irq_ctx;
	struct work_struct work;
	unsigned long work_flags;
};

#endif /* _MWIFIEX_PCIE_H */
