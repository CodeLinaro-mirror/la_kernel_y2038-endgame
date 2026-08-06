/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *  OMAP GPMC (General Purpose Memory Controller) defines
 */

#define GPMC_CONFIG_WP		0x00000005

/* IRQ numbers in GPMC IRQ domain for legacy boot use */
#define GPMC_IRQ_FIFOEVENTENABLE	0
#define GPMC_IRQ_COUNT_EVENT		1

/**
 * gpmc_nand_ops - Interface between NAND and GPMC
 * @nand_write_buffer_empty: get the NAND write buffer empty status.
 */
struct gpmc_nand_ops {
	bool (*nand_writebuffer_empty)(void);
};

struct gpmc_nand_regs;

struct gpmc_onenand_info {
	bool sync_read;
	bool sync_write;
	int burst_len;
};

#if IS_ENABLED(CONFIG_OMAP_GPMC)
struct gpmc_nand_ops *gpmc_omap_get_nand_ops(struct gpmc_nand_regs *regs,
					     int cs);
/**
 * gpmc_omap_onenand_set_timings - set optimized sync timings.
 * @cs:      Chip Select Region
 * @freq:    Chip frequency
 * @latency: Burst latency cycle count
 * @info:    Structure describing parameters used
 *
 * Sets optimized timings for the @cs region based on @freq and @latency.
 * Updates the @info structure based on the GPMC settings.
 */
int gpmc_omap_onenand_set_timings(struct device *dev, int cs, int freq,
				  int latency,
				  struct gpmc_onenand_info *info);

#else
static inline struct gpmc_nand_ops *gpmc_omap_get_nand_ops(struct gpmc_nand_regs *regs,
							   int cs)
{
	return NULL;
}

static inline
int gpmc_omap_onenand_set_timings(struct device *dev, int cs, int freq,
				  int latency,
				  struct gpmc_onenand_info *info)
{
	return -EINVAL;
}
#endif /* CONFIG_OMAP_GPMC */
