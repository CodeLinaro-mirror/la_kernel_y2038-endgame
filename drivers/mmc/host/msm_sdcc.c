/*
 *  linux/drivers/mmc/host/msm_sdcc.c - Qualcomm MSM 7X00A SDCC Driver
 *
 *  Copyright (C) 2007 Google Inc,
 *  Copyright (C) 2003 Deep Blue Solutions, Ltd, All Rights Reserved.
 *  Copyright (C) 2009, Code Aurora Forum. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Based on mmci.c
 *
 * Author: San Mehat (san@android.com)
 *
 */

#include <linux/completion.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/highmem.h>
#include <linux/list.h>
#include <linux/log2.h>
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sdio.h>
#include <linux/clk.h>
#include <linux/scatterlist.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/debugfs.h>
#include <linux/io.h>
#include <linux/memory.h>
#include <linux/gfp.h>
#include <linux/gpio.h>

#include <asm/cacheflush.h>
#include <asm/div64.h>
#include <asm/sizes.h>

#include <linux/platform_data/mmc-msm_sdcc.h>
#include <mach/msm_iomap.h>
#include <mach/clk.h>

/* data mover definitions */

struct msm_dmov_errdata {
	uint32_t flush[6];
};

struct msm_dmov_cmd {
	struct list_head list;
	unsigned int cmdptr;
	void (*complete_func)(struct msm_dmov_cmd *cmd,
			      unsigned int result,
			      struct msm_dmov_errdata *err);
	void (*execute_func)(struct msm_dmov_cmd *cmd);
	void *data;
};

#define DMOV_CMD_LIST         (0 << 29) /* does not work */
#define DMOV_CMD_PTR_LIST     (1 << 29) /* works */
#define DMOV_CMD_INPUT_CFG    (2 << 29) /* untested */
#define DMOV_CMD_OUTPUT_CFG   (3 << 29) /* untested */
#define DMOV_CMD_ADDR(addr)   ((addr) >> 3)

#define DMOV_RSLT_VALID       (1 << 31) /* 0 == host has empties result fifo */
#define DMOV_RSLT_ERROR       (1 << 3)
#define DMOV_RSLT_FLUSH       (1 << 2)
#define DMOV_RSLT_DONE        (1 << 1)  /* top pointer done */
#define DMOV_RSLT_USER        (1 << 0)  /* command with FR force result */

#define DMOV_STATUS_RSLT_COUNT(n)    (((n) >> 29))
#define DMOV_STATUS_CMD_COUNT(n)     (((n) >> 27) & 3)
#define DMOV_STATUS_RSLT_VALID       (1 << 1)
#define DMOV_STATUS_CMD_PTR_RDY      (1 << 0)

#define DMOV_CONFIG_FORCE_TOP_PTR_RSLT (1 << 2)
#define DMOV_CONFIG_FORCE_FLUSH_RSLT   (1 << 1)
#define DMOV_CONFIG_IRQ_EN             (1 << 0)

/* channel assignments */

#define DMOV_NAND_CHAN        7
#define DMOV_NAND_CRCI_CMD    5
#define DMOV_NAND_CRCI_DATA   4

#define DMOV_SDC1_CHAN        8
#define DMOV_SDC1_CRCI        6

#define DMOV_SDC2_CHAN        8
#define DMOV_SDC2_CRCI        7

#define DMOV_TSIF_CHAN        10
#define DMOV_TSIF_CRCI        10

#define DMOV_USB_CHAN         11

/* no client rate control ifc (eg, ram) */
#define DMOV_NONE_CRCI        0


/* If the CMD_PTR register has CMD_PTR_LIST selected, the data mover
 * is going to walk a list of 32bit pointers as described below.  Each
 * pointer points to a *array* of dmov_s, etc structs.  The last pointer
 * in the list is marked with CMD_PTR_LP.  The last struct in each array
 * is marked with CMD_LC (see below).
 */
#define CMD_PTR_ADDR(addr)  ((addr) >> 3)
#define CMD_PTR_LP          (1 << 31) /* last pointer */
#define CMD_PTR_PT          (3 << 29) /* ? */

/* Single Item Mode */
typedef struct {
	unsigned cmd;
	unsigned src;
	unsigned dst;
	unsigned len;
} dmov_s;

/* Scatter/Gather Mode */
typedef struct {
	unsigned cmd;
	unsigned src_dscr;
	unsigned dst_dscr;
	unsigned _reserved;
} dmov_sg;

/* Box mode */
typedef struct {
	uint32_t cmd;
	uint32_t src_row_addr;
	uint32_t dst_row_addr;
	uint32_t src_dst_len;
	uint32_t num_rows;
	uint32_t row_offset;
} dmov_box;

/* bits for the cmd field of the above structures */

#define CMD_LC      (1 << 31)  /* last command */
#define CMD_FR      (1 << 22)  /* force result -- does not work? */
#define CMD_OCU     (1 << 21)  /* other channel unblock */
#define CMD_OCB     (1 << 20)  /* other channel block */
#define CMD_TCB     (1 << 19)  /* ? */
#define CMD_DAH     (1 << 18)  /* destination address hold -- does not work?*/
#define CMD_SAH     (1 << 17)  /* source address hold -- does not work? */

#define CMD_MODE_SINGLE     (0 << 0) /* dmov_s structure used */
#define CMD_MODE_SG         (1 << 0) /* untested */
#define CMD_MODE_IND_SG     (2 << 0) /* untested */
#define CMD_MODE_BOX        (3 << 0) /* untested */

#define CMD_DST_SWAP_BYTES  (1 << 14) /* exchange each byte n with byte n+1 */
#define CMD_DST_SWAP_SHORTS (1 << 15) /* exchange each short n with short n+1 */
#define CMD_DST_SWAP_WORDS  (1 << 16) /* exchange each word n with word n+1 */

#define CMD_SRC_SWAP_BYTES  (1 << 11) /* exchange each byte n with byte n+1 */
#define CMD_SRC_SWAP_SHORTS (1 << 12) /* exchange each short n with short n+1 */
#define CMD_SRC_SWAP_WORDS  (1 << 13) /* exchange each word n with word n+1 */

#define CMD_DST_CRCI(n)     (((n) & 15) << 7)
#define CMD_SRC_CRCI(n)     (((n) & 15) << 3)

#define MSM_DMOV_CHANNEL_COUNT 16

#define DMOV_SD0(off, ch) (MSM_DMOV_BASE + 0x0000 + (off) + ((ch) << 2))
#define DMOV_SD1(off, ch) (MSM_DMOV_BASE + 0x0400 + (off) + ((ch) << 2))
#define DMOV_SD2(off, ch) (MSM_DMOV_BASE + 0x0800 + (off) + ((ch) << 2))
#define DMOV_SD3(off, ch) (MSM_DMOV_BASE + 0x0C00 + (off) + ((ch) << 2))

#if defined(CONFIG_ARCH_MSM7X30)
#define DMOV_SD_AARM DMOV_SD2
#else
#define DMOV_SD_AARM DMOV_SD3
#endif

#define DMOV_CMD_PTR(ch)      DMOV_SD_AARM(0x000, ch)
#define DMOV_RSLT(ch)         DMOV_SD_AARM(0x040, ch)
#define DMOV_FLUSH0(ch)       DMOV_SD_AARM(0x080, ch)
#define DMOV_FLUSH1(ch)       DMOV_SD_AARM(0x0C0, ch)
#define DMOV_FLUSH2(ch)       DMOV_SD_AARM(0x100, ch)
#define DMOV_FLUSH3(ch)       DMOV_SD_AARM(0x140, ch)
#define DMOV_FLUSH4(ch)       DMOV_SD_AARM(0x180, ch)
#define DMOV_FLUSH5(ch)       DMOV_SD_AARM(0x1C0, ch)

#define DMOV_STATUS(ch)       DMOV_SD_AARM(0x200, ch)
#define DMOV_ISR              DMOV_SD_AARM(0x380, 0)

#define DMOV_CONFIG(ch)       DMOV_SD_AARM(0x300, ch)

enum {
	MSM_DMOV_PRINT_ERRORS = 1,
	MSM_DMOV_PRINT_IO = 2,
	MSM_DMOV_PRINT_FLOW = 4
};

static DEFINE_SPINLOCK(msm_dmov_lock);
static struct clk *msm_dmov_clk;
static unsigned int channel_active;
static struct list_head ready_commands[MSM_DMOV_CHANNEL_COUNT];
static struct list_head active_commands[MSM_DMOV_CHANNEL_COUNT];
static const unsigned int msm_dmov_print_mask = MSM_DMOV_PRINT_ERRORS;

#define MSM_DMOV_DPRINTF(mask, format, args...) \
	do { \
		if ((mask) & msm_dmov_print_mask) \
			printk(KERN_ERR format, args); \
	} while (0)
#define PRINT_ERROR(format, args...) \
	MSM_DMOV_DPRINTF(MSM_DMOV_PRINT_ERRORS, format, args);
#define PRINT_IO(format, args...) \
	MSM_DMOV_DPRINTF(MSM_DMOV_PRINT_IO, format, args);
#define PRINT_FLOW(format, args...) \
	MSM_DMOV_DPRINTF(MSM_DMOV_PRINT_FLOW, format, args);

static void msm_dmov_stop_cmd(unsigned id, struct msm_dmov_cmd *cmd, int graceful)
{
	writel((graceful << 31), DMOV_FLUSH0(id));
}

static void msm_dmov_enqueue_cmd(unsigned id, struct msm_dmov_cmd *cmd)
{
	unsigned long irq_flags;
	unsigned int status;

	spin_lock_irqsave(&msm_dmov_lock, irq_flags);
	if (!channel_active)
		clk_enable(msm_dmov_clk);
	dsb();
	status = readl(DMOV_STATUS(id));
	if (list_empty(&ready_commands[id]) &&
		(status & DMOV_STATUS_CMD_PTR_RDY)) {
#if 0
		if (list_empty(&active_commands[id])) {
			PRINT_FLOW("msm_dmov_enqueue_cmd(%d), enable interrupt\n", id);
			writel(DMOV_CONFIG_IRQ_EN, DMOV_CONFIG(id));
		}
#endif
		if (cmd->execute_func)
			cmd->execute_func(cmd);
		PRINT_IO("msm_dmov_enqueue_cmd(%d), start command, status %x\n", id, status);
		list_add_tail(&cmd->list, &active_commands[id]);
		if (!channel_active)
			enable_irq(INT_ADM_AARM);
		channel_active |= 1U << id;
		writel(cmd->cmdptr, DMOV_CMD_PTR(id));
	} else {
		if (!channel_active)
			clk_disable(msm_dmov_clk);
		if (list_empty(&active_commands[id]))
			PRINT_ERROR("msm_dmov_enqueue_cmd(%d), error datamover stalled, status %x\n", id, status);

		PRINT_IO("msm_dmov_enqueue_cmd(%d), enqueue command, status %x\n", id, status);
		list_add_tail(&cmd->list, &ready_commands[id]);
	}
	spin_unlock_irqrestore(&msm_dmov_lock, irq_flags);
}

struct msm_dmov_exec_cmdptr_cmd {
	struct msm_dmov_cmd dmov_cmd;
	struct completion complete;
	unsigned id;
	unsigned int result;
	struct msm_dmov_errdata err;
};

static irqreturn_t msm_datamover_irq_handler(int irq, void *dev_id)
{
	unsigned int int_status, mask, id;
	unsigned long irq_flags;
	unsigned int ch_status;
	unsigned int ch_result;
	struct msm_dmov_cmd *cmd;

	spin_lock_irqsave(&msm_dmov_lock, irq_flags);

	int_status = readl(DMOV_ISR); /* read and clear interrupt */
	PRINT_FLOW("msm_datamover_irq_handler: DMOV_ISR %x\n", int_status);

	while (int_status) {
		mask = int_status & -int_status;
		id = fls(mask) - 1;
		PRINT_FLOW("msm_datamover_irq_handler %08x %08x id %d\n", int_status, mask, id);
		int_status &= ~mask;
		ch_status = readl(DMOV_STATUS(id));
		if (!(ch_status & DMOV_STATUS_RSLT_VALID)) {
			PRINT_FLOW("msm_datamover_irq_handler id %d, result not valid %x\n", id, ch_status);
			continue;
		}
		do {
			ch_result = readl(DMOV_RSLT(id));
			if (list_empty(&active_commands[id])) {
				PRINT_ERROR("msm_datamover_irq_handler id %d, got result "
					"with no active command, status %x, result %x\n",
					id, ch_status, ch_result);
				cmd = NULL;
			} else
				cmd = list_entry(active_commands[id].next, typeof(*cmd), list);
			PRINT_FLOW("msm_datamover_irq_handler id %d, status %x, result %x\n", id, ch_status, ch_result);
			if (ch_result & DMOV_RSLT_DONE) {
				PRINT_FLOW("msm_datamover_irq_handler id %d, status %x\n",
					id, ch_status);
				PRINT_IO("msm_datamover_irq_handler id %d, got result "
					"for %p, result %x\n", id, cmd, ch_result);
				if (cmd) {
					list_del(&cmd->list);
					dsb();
					cmd->complete_func(cmd, ch_result, NULL);
				}
			}
			if (ch_result & DMOV_RSLT_FLUSH) {
				struct msm_dmov_errdata errdata;

				errdata.flush[0] = readl(DMOV_FLUSH0(id));
				errdata.flush[1] = readl(DMOV_FLUSH1(id));
				errdata.flush[2] = readl(DMOV_FLUSH2(id));
				errdata.flush[3] = readl(DMOV_FLUSH3(id));
				errdata.flush[4] = readl(DMOV_FLUSH4(id));
				errdata.flush[5] = readl(DMOV_FLUSH5(id));
				PRINT_FLOW("msm_datamover_irq_handler id %d, status %x\n", id, ch_status);
				PRINT_FLOW("msm_datamover_irq_handler id %d, flush, result %x, flush0 %x\n", id, ch_result, errdata.flush[0]);
				if (cmd) {
					list_del(&cmd->list);
					dsb();
					cmd->complete_func(cmd, ch_result, &errdata);
				}
			}
			if (ch_result & DMOV_RSLT_ERROR) {
				struct msm_dmov_errdata errdata;

				errdata.flush[0] = readl(DMOV_FLUSH0(id));
				errdata.flush[1] = readl(DMOV_FLUSH1(id));
				errdata.flush[2] = readl(DMOV_FLUSH2(id));
				errdata.flush[3] = readl(DMOV_FLUSH3(id));
				errdata.flush[4] = readl(DMOV_FLUSH4(id));
				errdata.flush[5] = readl(DMOV_FLUSH5(id));

				PRINT_ERROR("msm_datamover_irq_handler id %d, status %x\n", id, ch_status);
				PRINT_ERROR("msm_datamover_irq_handler id %d, error, result %x, flush0 %x\n", id, ch_result, errdata.flush[0]);
				if (cmd) {
					list_del(&cmd->list);
					dsb();
					cmd->complete_func(cmd, ch_result, &errdata);
				}
				/* this does not seem to work, once we get an error */
				/* the datamover will no longer accept commands */
				writel(0, DMOV_FLUSH0(id));
			}
			ch_status = readl(DMOV_STATUS(id));
			PRINT_FLOW("msm_datamover_irq_handler id %d, status %x\n", id, ch_status);
			if ((ch_status & DMOV_STATUS_CMD_PTR_RDY) && !list_empty(&ready_commands[id])) {
				cmd = list_entry(ready_commands[id].next, typeof(*cmd), list);
				list_move_tail(&cmd->list, &active_commands[id]);
				if (cmd->execute_func)
					cmd->execute_func(cmd);
				PRINT_FLOW("msm_datamover_irq_handler id %d, start command\n", id);
				writel(cmd->cmdptr, DMOV_CMD_PTR(id));
			}
		} while (ch_status & DMOV_STATUS_RSLT_VALID);
		if (list_empty(&active_commands[id]) && list_empty(&ready_commands[id]))
			channel_active &= ~(1U << id);
		PRINT_FLOW("msm_datamover_irq_handler id %d, status %x\n", id, ch_status);
	}

	if (!channel_active) {
		disable_irq_nosync(INT_ADM_AARM);
		clk_disable(msm_dmov_clk);
	}

	spin_unlock_irqrestore(&msm_dmov_lock, irq_flags);
	return IRQ_HANDLED;
}

static int __init msm_init_datamover(void)
{
	int i;
	int ret;
	struct clk *clk;

	for (i = 0; i < MSM_DMOV_CHANNEL_COUNT; i++) {
		INIT_LIST_HEAD(&ready_commands[i]);
		INIT_LIST_HEAD(&active_commands[i]);
		writel(DMOV_CONFIG_IRQ_EN | DMOV_CONFIG_FORCE_TOP_PTR_RSLT | DMOV_CONFIG_FORCE_FLUSH_RSLT, DMOV_CONFIG(i));
	}
	clk = clk_get(NULL, "adm_clk");
	if (IS_ERR(clk))
		return PTR_ERR(clk);
	clk_prepare(clk);
	msm_dmov_clk = clk;
	ret = request_irq(INT_ADM_AARM, msm_datamover_irq_handler, 0, "msmdatamover", NULL);
	if (ret)
		return ret;
	disable_irq(INT_ADM_AARM);
	return 0;
}
module_init(msm_init_datamover);

/* now the actual SD card driver */

#include "msm_sdcc.h"

#define DRIVER_NAME "msm-sdcc"

#define BUSCLK_PWRSAVE 1
#define BUSCLK_TIMEOUT (HZ)
static unsigned int msmsdcc_fmin = 144000;
static unsigned int msmsdcc_fmax = 50000000;
static unsigned int msmsdcc_4bit = 1;
static unsigned int msmsdcc_pwrsave = 1;
static unsigned int msmsdcc_piopoll = 1;
static unsigned int msmsdcc_sdioirq;

#define PIO_SPINMAX 30
#define CMD_SPINMAX 20


static inline void
msmsdcc_disable_clocks(struct msmsdcc_host *host, int deferr)
{
	WARN_ON(!host->clks_on);

	BUG_ON(host->curr.mrq);

	if (deferr) {
		mod_timer(&host->busclk_timer, jiffies + BUSCLK_TIMEOUT);
	} else {
		del_timer_sync(&host->busclk_timer);
		/* Need to check clks_on again in case the busclk
		 * timer fired
		 */
		if (host->clks_on) {
			clk_disable(host->clk);
			clk_disable(host->pclk);
			host->clks_on = 0;
		}
	}
}

static inline int
msmsdcc_enable_clocks(struct msmsdcc_host *host)
{
	int rc;

	del_timer_sync(&host->busclk_timer);

	if (!host->clks_on) {
		rc = clk_enable(host->pclk);
		if (rc)
			return rc;
		rc = clk_enable(host->clk);
		if (rc) {
			clk_disable(host->pclk);
			return rc;
		}
		udelay(1 + ((3 * USEC_PER_SEC) /
		       (host->clk_rate ? host->clk_rate : msmsdcc_fmin)));
		host->clks_on = 1;
	}
	return 0;
}

static inline unsigned int
msmsdcc_readl(struct msmsdcc_host *host, unsigned int reg)
{
	return readl(host->base + reg);
}

static inline void
msmsdcc_writel(struct msmsdcc_host *host, u32 data, unsigned int reg)
{
	writel(data, host->base + reg);
	/* 3 clk delay required! */
	udelay(1 + ((3 * USEC_PER_SEC) /
	       (host->clk_rate ? host->clk_rate : msmsdcc_fmin)));
}

static void
msmsdcc_start_command(struct msmsdcc_host *host, struct mmc_command *cmd,
		      u32 c);

static void msmsdcc_reset_and_restore(struct msmsdcc_host *host)
{
	u32	mci_clk = 0;
	u32	mci_mask0 = 0;
	int	ret = 0;

	/* Save the controller state */
	mci_clk = readl(host->base + MMCICLOCK);
	mci_mask0 = readl(host->base + MMCIMASK0);

	/* Reset the controller */
	ret = clk_reset(host->clk, CLK_RESET_ASSERT);
	if (ret)
		pr_err("%s: Clock assert failed at %u Hz with err %d\n",
				mmc_hostname(host->mmc), host->clk_rate, ret);

	ret = clk_reset(host->clk, CLK_RESET_DEASSERT);
	if (ret)
		pr_err("%s: Clock deassert failed at %u Hz with err %d\n",
				mmc_hostname(host->mmc), host->clk_rate, ret);

	pr_info("%s: Controller has been re-initialiazed\n",
			mmc_hostname(host->mmc));

	/* Restore the contoller state */
	writel(host->pwr, host->base + MMCIPOWER);
	writel(mci_clk, host->base + MMCICLOCK);
	writel(mci_mask0, host->base + MMCIMASK0);
	ret = clk_set_rate(host->clk, host->clk_rate);
	if (ret)
		pr_err("%s: Failed to set clk rate %u Hz (%d)\n",
				mmc_hostname(host->mmc), host->clk_rate, ret);
}

static void
msmsdcc_request_end(struct msmsdcc_host *host, struct mmc_request *mrq)
{
	BUG_ON(host->curr.data);

	host->curr.mrq = NULL;
	host->curr.cmd = NULL;

	if (mrq->data)
		mrq->data->bytes_xfered = host->curr.data_xfered;
	if (mrq->cmd->error == -ETIMEDOUT)
		mdelay(5);

#if BUSCLK_PWRSAVE
	msmsdcc_disable_clocks(host, 1);
#endif
	/*
	 * Need to drop the host lock here; mmc_request_done may call
	 * back into the driver...
	 */
	spin_unlock(&host->lock);
	mmc_request_done(host->mmc, mrq);
	spin_lock(&host->lock);
}

static void
msmsdcc_stop_data(struct msmsdcc_host *host)
{
	host->curr.data = NULL;
	host->curr.got_dataend = 0;
}

uint32_t msmsdcc_fifo_addr(struct msmsdcc_host *host)
{
	return host->memres->start + MMCIFIFO;
}

static inline void
msmsdcc_start_command_exec(struct msmsdcc_host *host, u32 arg, u32 c) {
       msmsdcc_writel(host, arg, MMCIARGUMENT);
       msmsdcc_writel(host, c, MMCICOMMAND);
}

static void
msmsdcc_dma_exec_func(struct msm_dmov_cmd *cmd)
{
	struct msmsdcc_host *host = (struct msmsdcc_host *)cmd->data;

	msmsdcc_writel(host, host->cmd_timeout, MMCIDATATIMER);
	msmsdcc_writel(host, (unsigned int)host->curr.xfer_size,
		       MMCIDATALENGTH);
	msmsdcc_writel(host, (msmsdcc_readl(host, MMCIMASK0) &
			(~MCI_IRQ_PIO)) | host->cmd_pio_irqmask, MMCIMASK0);
	msmsdcc_writel(host, host->cmd_datactrl, MMCIDATACTRL);

	if (host->cmd_cmd) {
		msmsdcc_start_command_exec(host,
					   (u32) host->cmd_cmd->arg,
					   (u32) host->cmd_c);
	}
	host->dma.active = 1;
}

static void
msmsdcc_dma_complete_tlet(unsigned long data)
{
	struct msmsdcc_host *host = (struct msmsdcc_host *)data;
	unsigned long		flags;
	struct mmc_request	*mrq;
	struct msm_dmov_errdata err;

	spin_lock_irqsave(&host->lock, flags);
	host->dma.active = 0;

	err = host->dma.err;
	mrq = host->curr.mrq;
	BUG_ON(!mrq);
	WARN_ON(!mrq->data);

	if (!(host->dma.result & DMOV_RSLT_VALID)) {
		pr_err("msmsdcc: Invalid DataMover result\n");
		goto out;
	}

	if (host->dma.result & DMOV_RSLT_DONE) {
		host->curr.data_xfered = host->curr.xfer_size;
	} else {
		/* Error or flush  */
		if (host->dma.result & DMOV_RSLT_ERROR)
			pr_err("%s: DMA error (0x%.8x)\n",
			       mmc_hostname(host->mmc), host->dma.result);
		if (host->dma.result & DMOV_RSLT_FLUSH)
			pr_err("%s: DMA channel flushed (0x%.8x)\n",
			       mmc_hostname(host->mmc), host->dma.result);

		pr_err("Flush data: %.8x %.8x %.8x %.8x %.8x %.8x\n",
		       err.flush[0], err.flush[1], err.flush[2],
		       err.flush[3], err.flush[4], err.flush[5]);

		msmsdcc_reset_and_restore(host);
		if (!mrq->data->error)
			mrq->data->error = -EIO;
	}
	dma_unmap_sg(mmc_dev(host->mmc), host->dma.sg, host->dma.num_ents,
		     host->dma.dir);

	host->dma.sg = NULL;
	host->dma.busy = 0;

	if (host->curr.got_dataend || mrq->data->error) {

		/*
		 * If we've already gotten our DATAEND / DATABLKEND
		 * for this request, then complete it through here.
		 */
		msmsdcc_stop_data(host);

		if (!mrq->data->error)
			host->curr.data_xfered = host->curr.xfer_size;
		if (!mrq->data->stop || mrq->cmd->error) {
			host->curr.mrq = NULL;
			host->curr.cmd = NULL;
			mrq->data->bytes_xfered = host->curr.data_xfered;

			spin_unlock_irqrestore(&host->lock, flags);
#if BUSCLK_PWRSAVE
			msmsdcc_disable_clocks(host, 1);
#endif
			mmc_request_done(host->mmc, mrq);
			return;
		} else
			msmsdcc_start_command(host, mrq->data->stop, 0);
	}

out:
	spin_unlock_irqrestore(&host->lock, flags);
	return;
}

static void
msmsdcc_dma_complete_func(struct msm_dmov_cmd *cmd,
			  unsigned int result,
			  struct msm_dmov_errdata *err)
{
	struct msmsdcc_dma_data	*dma_data =
		container_of(cmd, struct msmsdcc_dma_data, hdr);
	struct msmsdcc_host *host = dma_data->host;

	dma_data->result = result;
	if (err)
		memcpy(&dma_data->err, err, sizeof(struct msm_dmov_errdata));

	tasklet_schedule(&host->dma_tlet);
}

static int validate_dma(struct msmsdcc_host *host, struct mmc_data *data)
{
	if (host->dma.channel == -1)
		return -ENOENT;

	if ((data->blksz * data->blocks) < MCI_FIFOSIZE)
		return -EINVAL;
	if ((data->blksz * data->blocks) % MCI_FIFOSIZE)
		return -EINVAL;
	return 0;
}

static int msmsdcc_config_dma(struct msmsdcc_host *host, struct mmc_data *data)
{
	struct msmsdcc_nc_dmadata *nc;
	dmov_box *box;
	uint32_t rows;
	uint32_t crci;
	unsigned int n;
	int i, rc;
	struct scatterlist *sg = data->sg;

	rc = validate_dma(host, data);
	if (rc)
		return rc;

	host->dma.sg = data->sg;
	host->dma.num_ents = data->sg_len;

       BUG_ON(host->dma.num_ents > NR_SG); /* Prevent memory corruption */

	nc = host->dma.nc;

	switch (host->pdev_id) {
	case 1:
		crci = MSMSDCC_CRCI_SDC1;
		break;
	case 2:
		crci = MSMSDCC_CRCI_SDC2;
		break;
	case 3:
		crci = MSMSDCC_CRCI_SDC3;
		break;
	case 4:
		crci = MSMSDCC_CRCI_SDC4;
		break;
	default:
		host->dma.sg = NULL;
		host->dma.num_ents = 0;
		return -ENOENT;
	}

	if (data->flags & MMC_DATA_READ)
		host->dma.dir = DMA_FROM_DEVICE;
	else
		host->dma.dir = DMA_TO_DEVICE;

	host->curr.user_pages = 0;

	box = &nc->cmd[0];

	/* location of command block must be 64 bit aligned */
	BUG_ON(host->dma.cmd_busaddr & 0x07);

	nc->cmdptr = (host->dma.cmd_busaddr >> 3) | CMD_PTR_LP;
	host->dma.hdr.cmdptr = DMOV_CMD_PTR_LIST |
			       DMOV_CMD_ADDR(host->dma.cmdptr_busaddr);
	host->dma.hdr.complete_func = msmsdcc_dma_complete_func;

	n = dma_map_sg(mmc_dev(host->mmc), host->dma.sg,
			host->dma.num_ents, host->dma.dir);
	if (n == 0) {
		pr_err("%s: Unable to map in all sg elements\n",
			mmc_hostname(host->mmc));
		host->dma.sg = NULL;
		host->dma.num_ents = 0;
		return -ENOMEM;
	}

	for_each_sg(host->dma.sg, sg, n, i) {

		box->cmd = CMD_MODE_BOX;

		if (i == n - 1)
			box->cmd |= CMD_LC;
		rows = (sg_dma_len(sg) % MCI_FIFOSIZE) ?
			(sg_dma_len(sg) / MCI_FIFOSIZE) + 1 :
			(sg_dma_len(sg) / MCI_FIFOSIZE) ;

		if (data->flags & MMC_DATA_READ) {
			box->src_row_addr = msmsdcc_fifo_addr(host);
			box->dst_row_addr = sg_dma_address(sg);

			box->src_dst_len = (MCI_FIFOSIZE << 16) |
					   (MCI_FIFOSIZE);
			box->row_offset = MCI_FIFOSIZE;

			box->num_rows = rows * ((1 << 16) + 1);
			box->cmd |= CMD_SRC_CRCI(crci);
		} else {
			box->src_row_addr = sg_dma_address(sg);
			box->dst_row_addr = msmsdcc_fifo_addr(host);

			box->src_dst_len = (MCI_FIFOSIZE << 16) |
					   (MCI_FIFOSIZE);
			box->row_offset = (MCI_FIFOSIZE << 16);

			box->num_rows = rows * ((1 << 16) + 1);
			box->cmd |= CMD_DST_CRCI(crci);
		}
		box++;
	}

	return 0;
}

static int
snoop_cccr_abort(struct mmc_command *cmd)
{
	if ((cmd->opcode == 52) &&
	    (cmd->arg & 0x80000000) &&
	    (((cmd->arg >> 9) & 0x1ffff) == SDIO_CCCR_ABORT))
		return 1;
	return 0;
}

static void
msmsdcc_start_command_deferred(struct msmsdcc_host *host,
				struct mmc_command *cmd, u32 *c)
{
	*c |= (cmd->opcode | MCI_CPSM_ENABLE);

	if (cmd->flags & MMC_RSP_PRESENT) {
		if (cmd->flags & MMC_RSP_136)
			*c |= MCI_CPSM_LONGRSP;
		*c |= MCI_CPSM_RESPONSE;
	}

	if (/*interrupt*/0)
		*c |= MCI_CPSM_INTERRUPT;

	if ((((cmd->opcode == 17) || (cmd->opcode == 18))  ||
	     ((cmd->opcode == 24) || (cmd->opcode == 25))) ||
	      (cmd->opcode == 53))
		*c |= MCI_CSPM_DATCMD;

	if (host->prog_scan && (cmd->opcode == 12)) {
		*c |= MCI_CPSM_PROGENA;
		host->prog_enable = true;
	}

	if (cmd == cmd->mrq->stop)
		*c |= MCI_CSPM_MCIABORT;

	if (snoop_cccr_abort(cmd))
		*c |= MCI_CSPM_MCIABORT;

	if (host->curr.cmd != NULL) {
		pr_err("%s: Overlapping command requests\n",
			mmc_hostname(host->mmc));
	}
	host->curr.cmd = cmd;
}

static void
msmsdcc_start_data(struct msmsdcc_host *host, struct mmc_data *data,
			struct mmc_command *cmd, u32 c)
{
	unsigned int datactrl, timeout;
	unsigned long long clks;
	unsigned int pio_irqmask = 0;

	host->curr.data = data;
	host->curr.xfer_size = data->blksz * data->blocks;
	host->curr.xfer_remain = host->curr.xfer_size;
	host->curr.data_xfered = 0;
	host->curr.got_dataend = 0;

	memset(&host->pio, 0, sizeof(host->pio));

	datactrl = MCI_DPSM_ENABLE | (data->blksz << 4);

	if (!msmsdcc_config_dma(host, data))
		datactrl |= MCI_DPSM_DMAENABLE;
	else {
		host->pio.sg = data->sg;
		host->pio.sg_len = data->sg_len;
		host->pio.sg_off = 0;

		if (data->flags & MMC_DATA_READ) {
			pio_irqmask = MCI_RXFIFOHALFFULLMASK;
			if (host->curr.xfer_remain < MCI_FIFOSIZE)
				pio_irqmask |= MCI_RXDATAAVLBLMASK;
		} else
			pio_irqmask = MCI_TXFIFOHALFEMPTYMASK;
	}

	if (data->flags & MMC_DATA_READ)
		datactrl |= MCI_DPSM_DIRECTION;

	clks = (unsigned long long)data->timeout_ns * host->clk_rate;
	do_div(clks, NSEC_PER_SEC);
	timeout = data->timeout_clks + (unsigned int)clks*2 ;

	if (datactrl & MCI_DPSM_DMAENABLE) {
		/* Save parameters for the exec function */
		host->cmd_timeout = timeout;
		host->cmd_pio_irqmask = pio_irqmask;
		host->cmd_datactrl = datactrl;
		host->cmd_cmd = cmd;

		host->dma.hdr.execute_func = msmsdcc_dma_exec_func;
		host->dma.hdr.data = (void *)host;
		host->dma.busy = 1;

		if (cmd) {
			msmsdcc_start_command_deferred(host, cmd, &c);
			host->cmd_c = c;
		}
		msm_dmov_enqueue_cmd(host->dma.channel, &host->dma.hdr);
		if (data->flags & MMC_DATA_WRITE)
			host->prog_scan = true;
	} else {
		msmsdcc_writel(host, timeout, MMCIDATATIMER);

		msmsdcc_writel(host, host->curr.xfer_size, MMCIDATALENGTH);

		msmsdcc_writel(host, (msmsdcc_readl(host, MMCIMASK0) &
				(~MCI_IRQ_PIO)) | pio_irqmask, MMCIMASK0);

		msmsdcc_writel(host, datactrl, MMCIDATACTRL);

		if (cmd) {
			/* Daisy-chain the command if requested */
			msmsdcc_start_command(host, cmd, c);
		}
	}
}

static void
msmsdcc_start_command(struct msmsdcc_host *host, struct mmc_command *cmd, u32 c)
{
	if (cmd == cmd->mrq->stop)
		c |= MCI_CSPM_MCIABORT;

	host->stats.cmds++;

	msmsdcc_start_command_deferred(host, cmd, &c);
	msmsdcc_start_command_exec(host, cmd->arg, c);
}

static void
msmsdcc_data_err(struct msmsdcc_host *host, struct mmc_data *data,
		 unsigned int status)
{
	if (status & MCI_DATACRCFAIL) {
		pr_err("%s: Data CRC error\n", mmc_hostname(host->mmc));
		pr_err("%s: opcode 0x%.8x\n", __func__,
		       data->mrq->cmd->opcode);
		pr_err("%s: blksz %d, blocks %d\n", __func__,
		       data->blksz, data->blocks);
		data->error = -EILSEQ;
	} else if (status & MCI_DATATIMEOUT) {
		pr_err("%s: Data timeout\n", mmc_hostname(host->mmc));
		data->error = -ETIMEDOUT;
	} else if (status & MCI_RXOVERRUN) {
		pr_err("%s: RX overrun\n", mmc_hostname(host->mmc));
		data->error = -EIO;
	} else if (status & MCI_TXUNDERRUN) {
		pr_err("%s: TX underrun\n", mmc_hostname(host->mmc));
		data->error = -EIO;
	} else {
		pr_err("%s: Unknown error (0x%.8x)\n",
		       mmc_hostname(host->mmc), status);
		data->error = -EIO;
	}
}


static int
msmsdcc_pio_read(struct msmsdcc_host *host, char *buffer, unsigned int remain)
{
	uint32_t	*ptr = (uint32_t *) buffer;
	int		count = 0;

	if (remain % 4)
		remain = ((remain >> 2) + 1) << 2;

	while (msmsdcc_readl(host, MMCISTATUS) & MCI_RXDATAAVLBL) {
		*ptr = msmsdcc_readl(host, MMCIFIFO + (count % MCI_FIFOSIZE));
		ptr++;
		count += sizeof(uint32_t);

		remain -=  sizeof(uint32_t);
		if (remain == 0)
			break;
	}
	return count;
}

static int
msmsdcc_pio_write(struct msmsdcc_host *host, char *buffer,
		  unsigned int remain, u32 status)
{
	void __iomem *base = host->base;
	char *ptr = buffer;

	do {
		unsigned int count, maxcnt, sz;

		maxcnt = status & MCI_TXFIFOEMPTY ? MCI_FIFOSIZE :
						    MCI_FIFOHALFSIZE;
		count = min(remain, maxcnt);

		sz = count % 4 ? (count >> 2) + 1 : (count >> 2);
		writesl(base + MMCIFIFO, ptr, sz);
		ptr += count;
		remain -= count;

		if (remain == 0)
			break;

		status = msmsdcc_readl(host, MMCISTATUS);
	} while (status & MCI_TXFIFOHALFEMPTY);

	return ptr - buffer;
}

static int
msmsdcc_spin_on_status(struct msmsdcc_host *host, uint32_t mask, int maxspin)
{
	while (maxspin) {
		if ((msmsdcc_readl(host, MMCISTATUS) & mask))
			return 0;
		udelay(1);
		--maxspin;
	}
	return -ETIMEDOUT;
}

static irqreturn_t
msmsdcc_pio_irq(int irq, void *dev_id)
{
	struct msmsdcc_host	*host = dev_id;
	uint32_t		status;
	u32 mci_mask0;

	status = msmsdcc_readl(host, MMCISTATUS);
	mci_mask0 = msmsdcc_readl(host, MMCIMASK0);

	if (((mci_mask0 & status) & MCI_IRQ_PIO) == 0)
		return IRQ_NONE;

	do {
		unsigned long flags;
		unsigned int remain, len;
		char *buffer;

		if (!(status & (MCI_TXFIFOHALFEMPTY | MCI_RXDATAAVLBL))) {
			if (host->curr.xfer_remain == 0 || !msmsdcc_piopoll)
				break;

			if (msmsdcc_spin_on_status(host,
						   (MCI_TXFIFOHALFEMPTY |
						   MCI_RXDATAAVLBL),
						   PIO_SPINMAX)) {
				break;
			}
		}

		/* Map the current scatter buffer */
		local_irq_save(flags);
		buffer = kmap_atomic(sg_page(host->pio.sg))
				     + host->pio.sg->offset;
		buffer += host->pio.sg_off;
		remain = host->pio.sg->length - host->pio.sg_off;
		len = 0;
		if (status & MCI_RXACTIVE)
			len = msmsdcc_pio_read(host, buffer, remain);
		if (status & MCI_TXACTIVE)
			len = msmsdcc_pio_write(host, buffer, remain, status);

		/* Unmap the buffer */
		kunmap_atomic(buffer);
		local_irq_restore(flags);

		host->pio.sg_off += len;
		host->curr.xfer_remain -= len;
		host->curr.data_xfered += len;
		remain -= len;

		if (remain == 0) {
			/* This sg page is full - do some housekeeping */
			if (status & MCI_RXACTIVE && host->curr.user_pages)
				flush_dcache_page(sg_page(host->pio.sg));

			if (!--host->pio.sg_len) {
				memset(&host->pio, 0, sizeof(host->pio));
				break;
			}

			/* Advance to next sg */
			host->pio.sg++;
			host->pio.sg_off = 0;
		}

		status = msmsdcc_readl(host, MMCISTATUS);
	} while (1);

	if (status & MCI_RXACTIVE && host->curr.xfer_remain < MCI_FIFOSIZE)
		msmsdcc_writel(host, (mci_mask0 & (~MCI_IRQ_PIO)) |
					MCI_RXDATAAVLBLMASK, MMCIMASK0);

	if (!host->curr.xfer_remain)
		msmsdcc_writel(host, (mci_mask0 & (~MCI_IRQ_PIO)) | 0,
					MMCIMASK0);

	return IRQ_HANDLED;
}

static void msmsdcc_do_cmdirq(struct msmsdcc_host *host, uint32_t status)
{
	struct mmc_command *cmd = host->curr.cmd;

	host->curr.cmd = NULL;
	cmd->resp[0] = msmsdcc_readl(host, MMCIRESPONSE0);
	cmd->resp[1] = msmsdcc_readl(host, MMCIRESPONSE1);
	cmd->resp[2] = msmsdcc_readl(host, MMCIRESPONSE2);
	cmd->resp[3] = msmsdcc_readl(host, MMCIRESPONSE3);

	if (status & MCI_CMDTIMEOUT) {
		cmd->error = -ETIMEDOUT;
	} else if (status & MCI_CMDCRCFAIL &&
		   cmd->flags & MMC_RSP_CRC) {
		pr_err("%s: Command CRC error\n", mmc_hostname(host->mmc));
		cmd->error = -EILSEQ;
	}

	if (!cmd->data || cmd->error) {
		if (host->curr.data && host->dma.sg)
			msm_dmov_stop_cmd(host->dma.channel,
					  &host->dma.hdr, 0);
		else if (host->curr.data) { /* Non DMA */
			msmsdcc_reset_and_restore(host);
			msmsdcc_stop_data(host);
			msmsdcc_request_end(host, cmd->mrq);
		} else { /* host->data == NULL */
			if (!cmd->error && host->prog_enable) {
				if (status & MCI_PROGDONE) {
					host->prog_scan = false;
					host->prog_enable = false;
					msmsdcc_request_end(host, cmd->mrq);
				} else {
					host->curr.cmd = cmd;
				}
			} else {
				if (host->prog_enable) {
					host->prog_scan = false;
					host->prog_enable = false;
				}
				msmsdcc_request_end(host, cmd->mrq);
			}
		}
	} else if (cmd->data)
		if (!(cmd->data->flags & MMC_DATA_READ))
			msmsdcc_start_data(host, cmd->data,
						NULL, 0);
}

static void
msmsdcc_handle_irq_data(struct msmsdcc_host *host, u32 status,
			void __iomem *base)
{
	struct mmc_data *data = host->curr.data;

	if (status & (MCI_CMDSENT | MCI_CMDRESPEND | MCI_CMDCRCFAIL |
			MCI_CMDTIMEOUT | MCI_PROGDONE) && host->curr.cmd) {
		msmsdcc_do_cmdirq(host, status);
	}

	if (!data)
		return;

	/* Check for data errors */
	if (status & (MCI_DATACRCFAIL | MCI_DATATIMEOUT |
		      MCI_TXUNDERRUN | MCI_RXOVERRUN)) {
		msmsdcc_data_err(host, data, status);
		host->curr.data_xfered = 0;
		if (host->dma.sg)
			msm_dmov_stop_cmd(host->dma.channel,
					  &host->dma.hdr, 0);
		else {
			msmsdcc_reset_and_restore(host);
			if (host->curr.data)
				msmsdcc_stop_data(host);
			if (!data->stop)
				msmsdcc_request_end(host, data->mrq);
			else
				msmsdcc_start_command(host, data->stop, 0);
		}
	}

	/* Check for data done */
	if (!host->curr.got_dataend && (status & MCI_DATAEND))
		host->curr.got_dataend = 1;

	/*
	 * If DMA is still in progress, we complete via the completion handler
	 */
	if (host->curr.got_dataend && !host->dma.busy) {
		/*
		 * There appears to be an issue in the controller where
		 * if you request a small block transfer (< fifo size),
		 * you may get your DATAEND/DATABLKEND irq without the
		 * PIO data irq.
		 *
		 * Check to see if there is still data to be read,
		 * and simulate a PIO irq.
		 */
		if (readl(base + MMCISTATUS) & MCI_RXDATAAVLBL)
			msmsdcc_pio_irq(1, host);

		msmsdcc_stop_data(host);
		if (!data->error)
			host->curr.data_xfered = host->curr.xfer_size;

		if (!data->stop)
			msmsdcc_request_end(host, data->mrq);
		else
			msmsdcc_start_command(host, data->stop, 0);
	}
}

static irqreturn_t
msmsdcc_irq(int irq, void *dev_id)
{
	struct msmsdcc_host	*host = dev_id;
	void __iomem		*base = host->base;
	u32			status;
	int			ret = 0;
	int			cardint = 0;

	spin_lock(&host->lock);

	do {
		status = msmsdcc_readl(host, MMCISTATUS);
		status &= msmsdcc_readl(host, MMCIMASK0);
		if ((status & (~MCI_IRQ_PIO)) == 0)
			break;
		msmsdcc_writel(host, status, MMCICLEAR);

		if (status & MCI_SDIOINTR)
			status &= ~MCI_SDIOINTR;

		if (!status)
			break;

		msmsdcc_handle_irq_data(host, status, base);

		if (status & MCI_SDIOINTOPER) {
			cardint = 1;
			status &= ~MCI_SDIOINTOPER;
		}
		ret = 1;
	} while (status);

	spin_unlock(&host->lock);

	/*
	 * We have to delay handling the card interrupt as it calls
	 * back into the driver.
	 */
	if (cardint)
		mmc_signal_sdio_irq(host->mmc);

	return IRQ_RETVAL(ret);
}

static void
msmsdcc_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct msmsdcc_host *host = mmc_priv(mmc);
	unsigned long flags;

	WARN_ON(host->curr.mrq != NULL);
	WARN_ON(host->pwr == 0);

	spin_lock_irqsave(&host->lock, flags);

	host->stats.reqs++;

	if (host->eject) {
		if (mrq->data && !(mrq->data->flags & MMC_DATA_READ)) {
			mrq->cmd->error = 0;
			mrq->data->bytes_xfered = mrq->data->blksz *
						  mrq->data->blocks;
		} else
			mrq->cmd->error = -ENOMEDIUM;

		spin_unlock_irqrestore(&host->lock, flags);
		mmc_request_done(mmc, mrq);
		return;
	}

	msmsdcc_enable_clocks(host);

	host->curr.mrq = mrq;

	if (mrq->data && mrq->data->flags & MMC_DATA_READ)
		/* Queue/read data, daisy-chain command when data starts */
		msmsdcc_start_data(host, mrq->data, mrq->cmd, 0);
	else
		msmsdcc_start_command(host, mrq->cmd, 0);

	if (host->cmdpoll && !msmsdcc_spin_on_status(host,
				MCI_CMDRESPEND|MCI_CMDCRCFAIL|MCI_CMDTIMEOUT,
				CMD_SPINMAX)) {
		uint32_t status = msmsdcc_readl(host, MMCISTATUS);
		msmsdcc_do_cmdirq(host, status);
		msmsdcc_writel(host,
			       MCI_CMDRESPEND | MCI_CMDCRCFAIL | MCI_CMDTIMEOUT,
			       MMCICLEAR);
		host->stats.cmdpoll_hits++;
	} else {
		host->stats.cmdpoll_misses++;
	}
	spin_unlock_irqrestore(&host->lock, flags);
}

static void msmsdcc_setup_gpio(struct msmsdcc_host *host, bool enable)
{
	struct msm_mmc_gpio_data *curr;
	int i, rc = 0;

	if (!host->plat->gpio_data || host->gpio_config_status == enable)
		return;

	curr = host->plat->gpio_data;
	for (i = 0; i < curr->size; i++) {
		if (enable) {
			rc = gpio_request(curr->gpio[i].no,
						curr->gpio[i].name);
			if (rc) {
				pr_err("%s: gpio_request(%d, %s) failed %d\n",
					mmc_hostname(host->mmc),
					curr->gpio[i].no,
					curr->gpio[i].name, rc);
				goto free_gpios;
			}
		} else {
			gpio_free(curr->gpio[i].no);
		}
	}
	host->gpio_config_status = enable;
	return;

free_gpios:
	for (; i >= 0; i--)
		gpio_free(curr->gpio[i].no);
}

static void
msmsdcc_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
	struct msmsdcc_host *host = mmc_priv(mmc);
	u32 clk = 0, pwr = 0;
	int rc;
	unsigned long flags;

	spin_lock_irqsave(&host->lock, flags);

	msmsdcc_enable_clocks(host);

	spin_unlock_irqrestore(&host->lock, flags);

	if (ios->clock) {
		if (ios->clock != host->clk_rate) {
			rc = clk_set_rate(host->clk, ios->clock);
			if (rc < 0)
				pr_err("%s: Error setting clock rate (%d)\n",
				       mmc_hostname(host->mmc), rc);
			else
				host->clk_rate = ios->clock;
		}
		clk |= MCI_CLK_ENABLE;
	}

	if (ios->bus_width == MMC_BUS_WIDTH_4)
		clk |= (2 << 10); /* Set WIDEBUS */

	if (ios->clock > 400000 && msmsdcc_pwrsave)
		clk |= (1 << 9); /* PWRSAVE */

	clk |= (1 << 12); /* FLOW_ENA */
	clk |= (1 << 15); /* feedback clock */

	if (host->plat->translate_vdd)
		pwr |= host->plat->translate_vdd(mmc_dev(mmc), ios->vdd);

	switch (ios->power_mode) {
	case MMC_POWER_OFF:
		msmsdcc_setup_gpio(host, false);
		break;
	case MMC_POWER_UP:
		pwr |= MCI_PWR_UP;
		msmsdcc_setup_gpio(host, true);
		break;
	case MMC_POWER_ON:
		pwr |= MCI_PWR_ON;
		break;
	}

	if (ios->bus_mode == MMC_BUSMODE_OPENDRAIN)
		pwr |= MCI_OD;

	msmsdcc_writel(host, clk, MMCICLOCK);

	if (host->pwr != pwr) {
		host->pwr = pwr;
		msmsdcc_writel(host, pwr, MMCIPOWER);
	}
#if BUSCLK_PWRSAVE
	spin_lock_irqsave(&host->lock, flags);
	msmsdcc_disable_clocks(host, 1);
	spin_unlock_irqrestore(&host->lock, flags);
#endif
}

static void msmsdcc_enable_sdio_irq(struct mmc_host *mmc, int enable)
{
	struct msmsdcc_host *host = mmc_priv(mmc);
	unsigned long flags;
	u32 status;

	spin_lock_irqsave(&host->lock, flags);
	if (msmsdcc_sdioirq == 1) {
		status = msmsdcc_readl(host, MMCIMASK0);
		if (enable)
			status |= MCI_SDIOINTOPERMASK;
		else
			status &= ~MCI_SDIOINTOPERMASK;
		host->saved_irq0mask = status;
		msmsdcc_writel(host, status, MMCIMASK0);
	}
	spin_unlock_irqrestore(&host->lock, flags);
}

static void msmsdcc_init_card(struct mmc_host *mmc, struct mmc_card *card)
{
	struct msmsdcc_host *host = mmc_priv(mmc);

	if (host->plat->init_card)
		host->plat->init_card(card);
}

static const struct mmc_host_ops msmsdcc_ops = {
	.request	= msmsdcc_request,
	.set_ios	= msmsdcc_set_ios,
	.enable_sdio_irq = msmsdcc_enable_sdio_irq,
	.init_card	= msmsdcc_init_card,
};

static void
msmsdcc_check_status(unsigned long data)
{
	struct msmsdcc_host *host = (struct msmsdcc_host *)data;
	unsigned int status;

	if (!host->plat->status) {
		mmc_detect_change(host->mmc, 0);
		goto out;
	}

	status = host->plat->status(mmc_dev(host->mmc));
	host->eject = !status;
	if (status ^ host->oldstat) {
		pr_info("%s: Slot status change detected (%d -> %d)\n",
			mmc_hostname(host->mmc), host->oldstat, status);
		if (status)
			mmc_detect_change(host->mmc, (5 * HZ) / 2);
		else
			mmc_detect_change(host->mmc, 0);
	}

	host->oldstat = status;

out:
	if (host->timer.function)
		mod_timer(&host->timer, jiffies + HZ);
}

static irqreturn_t
msmsdcc_platform_status_irq(int irq, void *dev_id)
{
	struct msmsdcc_host *host = dev_id;

	pr_debug("%s: %d\n", __func__, irq);
	msmsdcc_check_status((unsigned long) host);
	return IRQ_HANDLED;
}

static void
msmsdcc_status_notify_cb(int card_present, void *dev_id)
{
	struct msmsdcc_host *host = dev_id;

	pr_debug("%s: card_present %d\n", mmc_hostname(host->mmc),
	       card_present);
	msmsdcc_check_status((unsigned long) host);
}

static void
msmsdcc_busclk_expired(unsigned long _data)
{
	struct msmsdcc_host	*host = (struct msmsdcc_host *) _data;

	if (host->clks_on)
		msmsdcc_disable_clocks(host, 0);
}

static int
msmsdcc_init_dma(struct msmsdcc_host *host)
{
	memset(&host->dma, 0, sizeof(struct msmsdcc_dma_data));
	host->dma.host = host;
	host->dma.channel = -1;

	if (!host->dmares)
		return -ENODEV;

	host->dma.nc = dma_alloc_coherent(NULL,
					  sizeof(struct msmsdcc_nc_dmadata),
					  &host->dma.nc_busaddr,
					  GFP_KERNEL);
	if (host->dma.nc == NULL) {
		pr_err("Unable to allocate DMA buffer\n");
		return -ENOMEM;
	}
	memset(host->dma.nc, 0x00, sizeof(struct msmsdcc_nc_dmadata));
	host->dma.cmd_busaddr = host->dma.nc_busaddr;
	host->dma.cmdptr_busaddr = host->dma.nc_busaddr +
				offsetof(struct msmsdcc_nc_dmadata, cmdptr);
	host->dma.channel = host->dmares->start;

	return 0;
}

static int
msmsdcc_probe(struct platform_device *pdev)
{
	struct msm_mmc_platform_data *plat = pdev->dev.platform_data;
	struct msmsdcc_host *host;
	struct mmc_host *mmc;
	struct resource *cmd_irqres = NULL;
	struct resource *stat_irqres = NULL;
	struct resource *memres = NULL;
	struct resource *dmares = NULL;
	int ret;

	/* must have platform data */
	if (!plat) {
		pr_err("%s: Platform data not available\n", __func__);
		ret = -EINVAL;
		goto out;
	}

	if (pdev->id < 1 || pdev->id > 4)
		return -EINVAL;

	if (pdev->resource == NULL || pdev->num_resources < 2) {
		pr_err("%s: Invalid resource\n", __func__);
		return -ENXIO;
	}

	memres = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	dmares = platform_get_resource(pdev, IORESOURCE_DMA, 0);
	cmd_irqres = platform_get_resource_byname(pdev, IORESOURCE_IRQ,
						  "cmd_irq");
	stat_irqres = platform_get_resource_byname(pdev, IORESOURCE_IRQ,
						   "status_irq");

	if (!cmd_irqres || !memres) {
		pr_err("%s: Invalid resource\n", __func__);
		return -ENXIO;
	}

	/*
	 * Setup our host structure
	 */

	mmc = mmc_alloc_host(sizeof(struct msmsdcc_host), &pdev->dev);
	if (!mmc) {
		ret = -ENOMEM;
		goto out;
	}

	host = mmc_priv(mmc);
	host->pdev_id = pdev->id;
	host->plat = plat;
	host->mmc = mmc;
	host->curr.cmd = NULL;
	init_timer(&host->busclk_timer);
	host->busclk_timer.data = (unsigned long) host;
	host->busclk_timer.function = msmsdcc_busclk_expired;


	host->cmdpoll = 1;

	host->base = ioremap(memres->start, PAGE_SIZE);
	if (!host->base) {
		ret = -ENOMEM;
		goto host_free;
	}

	host->cmd_irqres = cmd_irqres;
	host->memres = memres;
	host->dmares = dmares;
	spin_lock_init(&host->lock);

	tasklet_init(&host->dma_tlet, msmsdcc_dma_complete_tlet,
			(unsigned long)host);

	/*
	 * Setup DMA
	 */
	if (host->dmares) {
		ret = msmsdcc_init_dma(host);
		if (ret)
			goto ioremap_free;
	} else {
		host->dma.channel = -1;
	}

	/* Get our clocks */
	host->pclk = clk_get(&pdev->dev, "sdc_pclk");
	if (IS_ERR(host->pclk)) {
		ret = PTR_ERR(host->pclk);
		goto dma_free;
	}

	host->clk = clk_get(&pdev->dev, "sdc_clk");
	if (IS_ERR(host->clk)) {
		ret = PTR_ERR(host->clk);
		goto pclk_put;
	}

	ret = clk_set_rate(host->clk, msmsdcc_fmin);
	if (ret) {
		pr_err("%s: Clock rate set failed (%d)\n", __func__, ret);
		goto clk_put;
	}

	ret = clk_prepare(host->pclk);
	if (ret)
		goto clk_put;

	ret = clk_prepare(host->clk);
	if (ret)
		goto clk_unprepare_p;

	/* Enable clocks */
	ret = msmsdcc_enable_clocks(host);
	if (ret)
		goto clk_unprepare;

	host->pclk_rate = clk_get_rate(host->pclk);
	host->clk_rate = clk_get_rate(host->clk);

	/*
	 * Setup MMC host structure
	 */
	mmc->ops = &msmsdcc_ops;
	mmc->f_min = msmsdcc_fmin;
	mmc->f_max = msmsdcc_fmax;
	mmc->ocr_avail = plat->ocr_mask;

	if (msmsdcc_4bit)
		mmc->caps |= MMC_CAP_4_BIT_DATA;
	if (msmsdcc_sdioirq)
		mmc->caps |= MMC_CAP_SDIO_IRQ;
	mmc->caps |= MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED;

	mmc->max_segs = NR_SG;
	mmc->max_blk_size = 4096;	/* MCI_DATA_CTL BLOCKSIZE up to 4096 */
	mmc->max_blk_count = 65536;

	mmc->max_req_size = 33554432;	/* MCI_DATA_LENGTH is 25 bits */
	mmc->max_seg_size = mmc->max_req_size;

	msmsdcc_writel(host, 0, MMCIMASK0);
	msmsdcc_writel(host, 0x5e007ff, MMCICLEAR);

	msmsdcc_writel(host, MCI_IRQENABLE, MMCIMASK0);
	host->saved_irq0mask = MCI_IRQENABLE;

	/*
	 * Setup card detect change
	 */

	memset(&host->timer, 0, sizeof(host->timer));

	if (stat_irqres && !(stat_irqres->flags & IORESOURCE_DISABLED)) {
		unsigned long irqflags = IRQF_SHARED |
			(stat_irqres->flags & IRQF_TRIGGER_MASK);

		host->stat_irq = stat_irqres->start;
		ret = request_irq(host->stat_irq,
				  msmsdcc_platform_status_irq,
				  irqflags,
				  DRIVER_NAME " (slot)",
				  host);
		if (ret) {
			pr_err("%s: Unable to get slot IRQ %d (%d)\n",
			       mmc_hostname(mmc), host->stat_irq, ret);
			goto clk_disable;
		}
	} else if (plat->register_status_notify) {
		plat->register_status_notify(msmsdcc_status_notify_cb, host);
	} else if (!plat->status)
		pr_err("%s: No card detect facilities available\n",
		       mmc_hostname(mmc));
	else {
		init_timer(&host->timer);
		host->timer.data = (unsigned long)host;
		host->timer.function = msmsdcc_check_status;
		host->timer.expires = jiffies + HZ;
		add_timer(&host->timer);
	}

	if (plat->status) {
		host->oldstat = host->plat->status(mmc_dev(host->mmc));
		host->eject = !host->oldstat;
	}

	ret = request_irq(cmd_irqres->start, msmsdcc_irq, IRQF_SHARED,
			  DRIVER_NAME " (cmd)", host);
	if (ret)
		goto stat_irq_free;

	ret = request_irq(cmd_irqres->start, msmsdcc_pio_irq, IRQF_SHARED,
			  DRIVER_NAME " (pio)", host);
	if (ret)
		goto cmd_irq_free;

	platform_set_drvdata(pdev, mmc);
	mmc_add_host(mmc);

	pr_info("%s: Qualcomm MSM SDCC at 0x%016llx irq %d,%d dma %d\n",
		mmc_hostname(mmc), (unsigned long long)memres->start,
		(unsigned int) cmd_irqres->start,
		(unsigned int) host->stat_irq, host->dma.channel);
	pr_info("%s: 4 bit data mode %s\n", mmc_hostname(mmc),
		(mmc->caps & MMC_CAP_4_BIT_DATA ? "enabled" : "disabled"));
	pr_info("%s: MMC clock %u -> %u Hz, PCLK %u Hz\n",
		mmc_hostname(mmc), msmsdcc_fmin, msmsdcc_fmax, host->pclk_rate);
	pr_info("%s: Slot eject status = %d\n", mmc_hostname(mmc), host->eject);
	pr_info("%s: Power save feature enable = %d\n",
		mmc_hostname(mmc), msmsdcc_pwrsave);

	if (host->dma.channel != -1) {
		pr_info("%s: DM non-cached buffer at %p, dma_addr 0x%.8x\n",
			mmc_hostname(mmc), host->dma.nc, host->dma.nc_busaddr);
		pr_info("%s: DM cmd busaddr 0x%.8x, cmdptr busaddr 0x%.8x\n",
			mmc_hostname(mmc), host->dma.cmd_busaddr,
			host->dma.cmdptr_busaddr);
	} else
		pr_info("%s: PIO transfer enabled\n", mmc_hostname(mmc));
	if (host->timer.function)
		pr_info("%s: Polling status mode enabled\n", mmc_hostname(mmc));

	return 0;
 cmd_irq_free:
	free_irq(cmd_irqres->start, host);
 stat_irq_free:
	if (host->stat_irq)
		free_irq(host->stat_irq, host);
 clk_disable:
	msmsdcc_disable_clocks(host, 0);
 clk_unprepare:
	clk_unprepare(host->clk);
 clk_unprepare_p:
	clk_unprepare(host->pclk);
 clk_put:
	clk_put(host->clk);
 pclk_put:
	clk_put(host->pclk);
dma_free:
	if (host->dmares)
		dma_free_coherent(NULL, sizeof(struct msmsdcc_nc_dmadata),
					host->dma.nc, host->dma.nc_busaddr);
ioremap_free:
	tasklet_kill(&host->dma_tlet);
	iounmap(host->base);
 host_free:
	mmc_free_host(mmc);
 out:
	return ret;
}

#ifdef CONFIG_PM
static int
msmsdcc_suspend(struct platform_device *dev, pm_message_t state)
{
	struct mmc_host *mmc = platform_get_drvdata(dev);

	if (mmc) {
		struct msmsdcc_host *host = mmc_priv(mmc);

		if (host->stat_irq)
			disable_irq(host->stat_irq);

		msmsdcc_writel(host, 0, MMCIMASK0);
		if (host->clks_on)
			msmsdcc_disable_clocks(host, 0);
	}
	return 0;
}

static int
msmsdcc_resume(struct platform_device *dev)
{
	struct mmc_host *mmc = platform_get_drvdata(dev);

	if (mmc) {
		struct msmsdcc_host *host = mmc_priv(mmc);

		msmsdcc_enable_clocks(host);

		msmsdcc_writel(host, host->saved_irq0mask, MMCIMASK0);

		if (host->stat_irq)
			enable_irq(host->stat_irq);
#if BUSCLK_PWRSAVE
		msmsdcc_disable_clocks(host, 1);
#endif
	}
	return 0;
}
#else
#define msmsdcc_suspend	0
#define msmsdcc_resume 0
#endif

static struct platform_driver msmsdcc_driver = {
	.probe		= msmsdcc_probe,
	.suspend	= msmsdcc_suspend,
	.resume		= msmsdcc_resume,
	.driver		= {
		.name	= "msm_sdcc",
	},
};

module_platform_driver(msmsdcc_driver);

MODULE_DESCRIPTION("Qualcomm MSM 7X00A Multimedia Card Interface driver");
MODULE_LICENSE("GPL");
