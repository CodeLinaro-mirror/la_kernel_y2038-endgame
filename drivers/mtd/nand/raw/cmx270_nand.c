// SPDX-License-Identifier: GPL-2.0-only
/*
 *  Copyright (C) 2006 Compulab, Ltd.
 *  Mike Rapoport <mike@compulab.co.il>
 *
 *  Derived from drivers/mtd/nand/h1910.c (removed in v3.10)
 *       Copyright (C) 2002 Marius Gröger (mag@sysgo.de)
 *       Copyright (c) 2001 Thomas Gleixner (gleixner@autronix.de)
 *
 *  Overview:
 *   This is a device driver for the NAND flash device found on the
 *   CM-X270 board.
 */

#include <linux/mtd/rawnand.h>
#include <linux/mtd/partitions.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/soc/pxa/cpu.h>
#include <linux/platform_device.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/mach-types.h>

static int gpio_nand_cs;
static int gpio_nand_rb;

/* MTD structure for CM-X270 board */
static struct mtd_info *cmx270_nand_mtd;

/* remaped IO address of the device */
static void __iomem *cmx270_nand_io;

/*
 * Define static partitions for flash device
 */
static const struct mtd_partition partition_info[] = {
	[0] = {
		.name	= "cmx270-0",
		.offset	= 0,
		.size	= MTDPART_SIZ_FULL
	}
};
#define NUM_PARTITIONS (ARRAY_SIZE(partition_info))

static u_char cmx270_read_byte(struct nand_chip *this)
{
	return (readl(this->legacy.IO_ADDR_R) >> 16);
}

static void cmx270_write_buf(struct nand_chip *this, const u_char *buf,
			     int len)
{
	int i;

	for (i=0; i<len; i++)
		writel((*buf++ << 16), this->legacy.IO_ADDR_W);
}

static void cmx270_read_buf(struct nand_chip *this, u_char *buf, int len)
{
	int i;

	for (i=0; i<len; i++)
		*buf++ = readl(this->legacy.IO_ADDR_R) >> 16;
}

static inline void nand_cs_on(void)
{
	gpio_set_value(gpio_nand_cs, 0);
}

static void nand_cs_off(void)
{
	dsb();

	gpio_set_value(gpio_nand_cs, 1);
}

/*
 *	hardware specific access to control-lines
 */
static void cmx270_hwcontrol(struct nand_chip *this, int dat,
			     unsigned int ctrl)
{
	unsigned int nandaddr = (unsigned int)this->legacy.IO_ADDR_W;

	dsb();

	if (ctrl & NAND_CTRL_CHANGE) {
		if ( ctrl & NAND_ALE )
			nandaddr |=  (1 << 3);
		else
			nandaddr &= ~(1 << 3);
		if ( ctrl & NAND_CLE )
			nandaddr |=  (1 << 2);
		else
			nandaddr &= ~(1 << 2);
		if ( ctrl & NAND_NCE )
			nand_cs_on();
		else
			nand_cs_off();
	}

	dsb();
	this->legacy.IO_ADDR_W = (void __iomem*)nandaddr;
	if (dat != NAND_CMD_NONE)
		writel((dat << 16), this->legacy.IO_ADDR_W);

	dsb();
}

/*
 *	read device ready pin
 */
static int cmx270_device_ready(struct nand_chip *this)
{
	dsb();

	return (gpio_get_value(gpio_nand_rb));
}

/*
 * Main initialization routine
 */
static int cmx270_probe(struct platform_device *pdev)
{
	const int *gpios = pdev->dev.platform_data;
	struct nand_chip *this;
	int ret;

	if (!(machine_is_armcore() && cpu_is_pxa27x()))
		return -ENODEV;

	if (!gpios) {
		dev_warn(&pdev->dev, "missing platform data");
		return -ENODEV;
	}

	gpio_nand_cs = gpios[0];
	gpio_nand_rb = gpios[1];

	ret = gpio_request(gpio_nand_cs, "NAND CS");
	if (ret) {
		pr_warn("CM-X270: failed to request NAND CS gpio\n");
		return ret;
	}

	gpio_direction_output(gpio_nand_cs, 1);

	ret = gpio_request(gpio_nand_rb, "NAND R/B");
	if (ret) {
		pr_warn("CM-X270: failed to request NAND R/B gpio\n");
		goto err_gpio_request;
	}

	gpio_direction_input(gpio_nand_rb);

	/* Allocate memory for MTD device structure and private data */
	this = kzalloc(sizeof(struct nand_chip), GFP_KERNEL);
	if (!this) {
		ret = -ENOMEM;
		goto err_kzalloc;
	}

	cmx270_nand_io = devm_platform_ioremap_resource(pdev, 0);
	if (!cmx270_nand_io) {
		pr_debug("Unable to ioremap NAND device\n");
		ret = -EINVAL;
		goto err_ioremap;
	}

	cmx270_nand_mtd = nand_to_mtd(this);

	/* Link the private data with the MTD structure */
	cmx270_nand_mtd->owner = THIS_MODULE;

	/* insert callbacks */
	this->legacy.IO_ADDR_R = cmx270_nand_io;
	this->legacy.IO_ADDR_W = cmx270_nand_io;
	this->legacy.cmd_ctrl = cmx270_hwcontrol;
	this->legacy.dev_ready = cmx270_device_ready;

	/* 15 us command delay time */
	this->legacy.chip_delay = 20;
	this->ecc.mode = NAND_ECC_SOFT;
	this->ecc.algo = NAND_ECC_HAMMING;

	/* read/write functions */
	this->legacy.read_byte = cmx270_read_byte;
	this->legacy.read_buf = cmx270_read_buf;
	this->legacy.write_buf = cmx270_write_buf;

	/* Scan to find existence of the device */
	ret = nand_scan(this, 1);
	if (ret) {
		pr_notice("No NAND device\n");
		goto err_scan;
	}

	/* Register the partitions */
	ret = mtd_device_register(cmx270_nand_mtd, partition_info,
				  NUM_PARTITIONS);
	if (ret)
		goto err_scan;

	/* Return happy */
	return 0;

err_scan:
err_ioremap:
	kfree(this);
err_kzalloc:
	gpio_free(gpio_nand_rb);
err_gpio_request:
	gpio_free(gpio_nand_cs);

	return ret;

}

/*
 * Clean up routine
 */
static int cmx270_remove(struct platform_device *pdev)
{
	/* Release resources, unregister device */
	nand_release(mtd_to_nand(cmx270_nand_mtd));

	gpio_free(gpio_nand_rb);
	gpio_free(gpio_nand_cs);

	kfree(mtd_to_nand(cmx270_nand_mtd));

	return 0;
}

static struct platform_driver cmx270_nand_driver = {
	.driver.name = "cmx270-nand",
	.probe = cmx270_probe,
	.remove = cmx270_remove,
};
module_platform_driver(cmx270_nand_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mike Rapoport <mike@compulab.co.il>");
MODULE_DESCRIPTION("NAND flash driver for Compulab CM-X270 Module");
