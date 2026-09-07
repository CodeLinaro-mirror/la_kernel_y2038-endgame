// SPDX-License-Identifier: GPL-2.0
/*
 * drivers/usb/host/ehci-orion.c
 *
 * Tzachi Perelstein <tzachi@marvell.com>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/mbus.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/usb.h>
#include <linux/usb/hcd.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>

#include "ehci.h"

#define rdl(off)	readl_relaxed(hcd->regs + (off))
#define wrl(off, val)	writel_relaxed((val), hcd->regs + (off))

#define USB_CMD			0x140
#define   USB_CMD_RUN		BIT(0)
#define   USB_CMD_RESET		BIT(1)
#define USB_MODE		0x1a8
#define   USB_MODE_MASK		GENMASK(1, 0)
#define   USB_MODE_DEVICE	0x2
#define   USB_MODE_HOST		0x3
#define   USB_MODE_SDIS		BIT(4)
#define USB_CAUSE		0x310
#define USB_MASK		0x314
#define USB_WINDOW_CTRL(i)	(0x320 + ((i) << 4))
#define USB_WINDOW_BASE(i)	(0x324 + ((i) << 4))
#define USB_IPG			0x360
#define USB_PHY_PWR_CTRL	0x400
#define USB_PHY_TX_CTRL		0x420
#define USB_PHY_RX_CTRL		0x430
#define USB_PHY_IVREF_CTRL	0x440
#define USB_PHY_TST_GRP_CTRL	0x450

#define USB_SBUSCFG		0x90

/* BAWR = BARD = 3 : Align read/write bursts packets larger than 128 bytes */
#define USB_SBUSCFG_BAWR_ALIGN_128B	(0x3 << 6)
#define USB_SBUSCFG_BARD_ALIGN_128B	(0x3 << 3)
/* AHBBRST = 3	   : Align AHB Burst to INCR16 (64 bytes) */
#define USB_SBUSCFG_AHBBRST_INCR16	(0x3 << 0)

#define USB_SBUSCFG_DEF_VAL (USB_SBUSCFG_BAWR_ALIGN_128B	\
			     | USB_SBUSCFG_BARD_ALIGN_128B	\
			     | USB_SBUSCFG_AHBBRST_INCR16)

#define DRIVER_DESC "EHCI orion driver"

#define hcd_to_orion_priv(h) ((struct orion_ehci_hcd *)hcd_to_ehci(h)->priv)

struct orion_ehci_hcd {
	struct clk *clk;
};

static struct hc_driver __read_mostly ehci_orion_hc_driver;

/*
 * Legacy DMA mask is 32 bit.
 * AC5 has the DDR starting at 8GB, hence it requires
 * a larger (34-bit) DMA mask, in order for DMA allocations
 * to succeed:
 */
static const u64 dma_mask_orion =	DMA_BIT_MASK(32);
static const u64 dma_mask_ac5 =		DMA_BIT_MASK(34);

static void
ehci_orion_conf_mbus_windows(struct usb_hcd *hcd,
			     const struct mbus_dram_target_info *dram)
{
	int i;

	for (i = 0; i < 4; i++) {
		wrl(USB_WINDOW_CTRL(i), 0);
		wrl(USB_WINDOW_BASE(i), 0);
	}

	for (i = 0; i < dram->num_cs; i++) {
		const struct mbus_dram_window *cs = dram->cs + i;

		wrl(USB_WINDOW_CTRL(i), ((cs->size - 1) & 0xffff0000) |
					(cs->mbus_attr << 8) |
					(dram->mbus_dram_target_id << 4) | 1);
		wrl(USB_WINDOW_BASE(i), cs->base);
	}
}

static int ehci_orion_drv_reset(struct usb_hcd *hcd)
{
	struct device *dev = hcd->self.controller;
	int ret;

	ret = ehci_setup(hcd);
	if (ret)
		return ret;

	/*
	 * For SoC without hlock, need to program sbuscfg value to guarantee
	 * AHB master's burst would not overrun or underrun FIFO.
	 *
	 * sbuscfg reg has to be set after usb controller reset, otherwise
	 * the value would be override to 0.
	 */
	if (of_device_is_compatible(dev->of_node, "marvell,armada-3700-ehci"))
		wrl(USB_SBUSCFG, USB_SBUSCFG_DEF_VAL);

	return ret;
}

static int __maybe_unused ehci_orion_drv_suspend(struct device *dev)
{
	struct usb_hcd *hcd = dev_get_drvdata(dev);

	return ehci_suspend(hcd, device_may_wakeup(dev));
}

static int __maybe_unused ehci_orion_drv_resume(struct device *dev)
{
	struct usb_hcd *hcd = dev_get_drvdata(dev);

	return ehci_resume(hcd, false);
}

static SIMPLE_DEV_PM_OPS(ehci_orion_pm_ops, ehci_orion_drv_suspend,
			 ehci_orion_drv_resume);

static const struct ehci_driver_overrides orion_overrides __initconst = {
	.extra_priv_size =	sizeof(struct orion_ehci_hcd),
	.reset = ehci_orion_drv_reset,
};

static int ehci_orion_drv_probe(struct platform_device *pdev)
{
	const struct mbus_dram_target_info *dram;
	struct resource *res;
	struct usb_hcd *hcd;
	struct ehci_hcd *ehci;
	void __iomem *regs;
	int irq, err;
	struct orion_ehci_hcd *priv;
	u64 *dma_mask_ptr;

	if (usb_disabled())
		return -ENODEV;

	pr_debug("Initializing Orion-SoC USB Host Controller\n");

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		err = irq;
		goto err;
	}

	/*
	 * Right now device-tree probed devices don't get dma_mask
	 * set. Since shared usb code relies on it, set it here for
	 * now. Once we have dma capability bindings this can go away.
	 */
	dma_mask_ptr = (u64 *)of_device_get_match_data(&pdev->dev);
	err = dma_coerce_mask_and_coherent(&pdev->dev, *dma_mask_ptr);
	if (err)
		goto err;

	regs = devm_platform_get_and_ioremap_resource(pdev, 0, &res);
	if (IS_ERR(regs)) {
		err = PTR_ERR(regs);
		goto err;
	}

	hcd = usb_create_hcd(&ehci_orion_hc_driver,
			&pdev->dev, dev_name(&pdev->dev));
	if (!hcd) {
		err = -ENOMEM;
		goto err;
	}

	hcd->rsrc_start = res->start;
	hcd->rsrc_len = resource_size(res);
	hcd->regs = regs;

	ehci = hcd_to_ehci(hcd);
	ehci->caps = hcd->regs + 0x100;
	hcd->has_tt = 1;

	priv = hcd_to_orion_priv(hcd);
	/*
	 * Not all platforms can gate the clock, so it is not an error if
	 * the clock does not exists.
	 */
	priv->clk = devm_clk_get(&pdev->dev, NULL);
	if (!IS_ERR(priv->clk)) {
		err = clk_prepare_enable(priv->clk);
		if (err)
			goto err_put_hcd;
	}

	/*
	 * (Re-)program MBUS remapping windows if we are asked to.
	 */
	dram = mv_mbus_dram_info();
	if (dram)
		ehci_orion_conf_mbus_windows(hcd, dram);

	err = usb_add_hcd(hcd, irq, IRQF_SHARED);
	if (err)
		goto err_dis_clk;

	device_wakeup_enable(hcd->self.controller);
	return 0;

err_dis_clk:
	if (!IS_ERR(priv->clk))
		clk_disable_unprepare(priv->clk);
err_put_hcd:
	usb_put_hcd(hcd);
err:
	dev_err(&pdev->dev, "init %s fail, %d\n",
		dev_name(&pdev->dev), err);

	return err;
}

static void ehci_orion_drv_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);
	struct orion_ehci_hcd *priv = hcd_to_orion_priv(hcd);

	usb_remove_hcd(hcd);

	if (!IS_ERR(priv->clk))
		clk_disable_unprepare(priv->clk);

	usb_put_hcd(hcd);
}

static const struct of_device_id ehci_orion_dt_ids[] = {
	{ .compatible = "marvell,orion-ehci", .data = &dma_mask_orion},
	{ .compatible = "marvell,armada-3700-ehci", .data = &dma_mask_orion},
	{ .compatible = "marvell,ac5-ehci", .data = &dma_mask_ac5},
	{},
};
MODULE_DEVICE_TABLE(of, ehci_orion_dt_ids);

static struct platform_driver ehci_orion_driver = {
	.probe		= ehci_orion_drv_probe,
	.remove		= ehci_orion_drv_remove,
	.shutdown	= usb_hcd_platform_shutdown,
	.driver = {
		.name	= "orion-ehci",
		.of_match_table = ehci_orion_dt_ids,
		.pm = &ehci_orion_pm_ops,
	},
};

static int __init ehci_orion_init(void)
{
	if (usb_disabled())
		return -ENODEV;

	ehci_init_driver(&ehci_orion_hc_driver, &orion_overrides);
	return platform_driver_register(&ehci_orion_driver);
}
module_init(ehci_orion_init);

static void __exit ehci_orion_cleanup(void)
{
	platform_driver_unregister(&ehci_orion_driver);
}
module_exit(ehci_orion_cleanup);

MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_ALIAS("platform:orion-ehci");
MODULE_AUTHOR("Tzachi Perelstein");
MODULE_LICENSE("GPL v2");
