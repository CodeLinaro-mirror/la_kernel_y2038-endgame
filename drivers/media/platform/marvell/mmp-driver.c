// SPDX-License-Identifier: GPL-2.0-only
/*
 * Support for the camera device found on Marvell MMP processors; known
 * to work with the Armada 610 as used in the OLPC 1.75 system.
 *
 * Copyright 2011 Jonathan Corbet <corbet@lwn.net>
 * Copyright 2018 Lubomir Rintel <lkundrak@v3.sk>
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/videodev2.h>
#include <media/v4l2-device.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/io.h>
#include <linux/list.h>
#include <linux/pm.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>

#include "mcam-core.h"

MODULE_ALIAS("platform:mmp-camera");
MODULE_AUTHOR("Jonathan Corbet <corbet@lwn.net>");
MODULE_DESCRIPTION("Support for the camera device found on Marvell MMP processors");
MODULE_LICENSE("GPL");

static char *mcam_clks[] = {"axi", "func", "phy"};

struct mmp_camera {
	struct platform_device *pdev;
	struct mcam_camera mcam;
	struct list_head devlist;
	struct clk *mipi_clk;
	int irq;
};

static inline struct mmp_camera *mcam_to_cam(struct mcam_camera *mcam)
{
	return container_of(mcam, struct mmp_camera, mcam);
}

static irqreturn_t mmpcam_irq(int irq, void *data)
{
	struct mcam_camera *mcam = data;
	unsigned int irqs, handled;

	spin_lock(&mcam->dev_lock);
	irqs = mcam_reg_read(mcam, REG_IRQSTAT);
	handled = mccic_irq(mcam, irqs);
	spin_unlock(&mcam->dev_lock);
	return IRQ_RETVAL(handled);
}

static void mcam_init_clk(struct mcam_camera *mcam)
{
	unsigned int i;

	for (i = 0; i < NR_MCAM_CLK; i++) {
		if (mcam_clks[i] != NULL) {
			/* Some clks are not necessary on some boards
			 * We still try to run even it fails getting clk
			 */
			mcam->clk[i] = devm_clk_get(mcam->dev, mcam_clks[i]);
			if (IS_ERR(mcam->clk[i]))
				dev_warn(mcam->dev, "Could not get clk: %s\n",
						mcam_clks[i]);
		}
	}
}

static int mmpcam_probe(struct platform_device *pdev)
{
	struct mmp_camera *cam;
	struct mcam_camera *mcam;
	struct resource *res;
	struct fwnode_handle *ep;
	struct v4l2_async_connection *asd;
	int ret;

	cam = devm_kzalloc(&pdev->dev, sizeof(*cam), GFP_KERNEL);
	if (cam == NULL)
		return -ENOMEM;
	platform_set_drvdata(pdev, cam);
	cam->pdev = pdev;
	INIT_LIST_HEAD(&cam->devlist);

	mcam = &cam->mcam;
	mcam->dev = &pdev->dev;

	/*
	 * These are values that used to be hardcoded in mcam-core and
	 * work well on a OLPC XO 1.75 with a parallel bus sensor.
	 * If it turns out other setups make sense, the values should
	 * be obtained from the device tree.
	 */
	mcam->mclk_src = 3;
	mcam->mclk_div = 2;
	mcam->mipi_enabled = false;
	mcam->chip_id = MCAM_ARMADA610;
	mcam->buffer_mode = B_DMA_sg;
	strscpy(mcam->bus_info, "platform:mmp-camera", sizeof(mcam->bus_info));
	spin_lock_init(&mcam->dev_lock);
	/*
	 * Get our I/O memory.
	 */
	mcam->regs = devm_platform_get_and_ioremap_resource(pdev, 0, &res);
	if (IS_ERR(mcam->regs))
		return PTR_ERR(mcam->regs);
	mcam->regs_size = resource_size(res);

	mcam_init_clk(mcam);

	/*
	 * Register with V4L.
	 */

	ret = v4l2_device_register(mcam->dev, &mcam->v4l2_dev);
	if (ret)
		return ret;

	/*
	 * Create a match of the sensor against its OF node.
	 */
	ep = fwnode_graph_get_next_endpoint(of_fwnode_handle(pdev->dev.of_node),
					    NULL);
	if (!ep) {
		ret = -ENODEV;
		goto out_v4l2_device_unregister;
	}

	v4l2_async_nf_init(&mcam->notifier, &mcam->v4l2_dev);

	asd = v4l2_async_nf_add_fwnode_remote(&mcam->notifier, ep,
					      struct v4l2_async_connection);
	fwnode_handle_put(ep);
	if (IS_ERR(asd)) {
		ret = PTR_ERR(asd);
		goto out_v4l2_device_unregister;
	}

	/*
	 * Register the device with the core.
	 */
	ret = mccic_register(mcam);
	if (ret)
		goto out_v4l2_device_unregister;

	/*
	 * Add OF clock provider.
	 */
	ret = devm_of_clk_add_hw_provider(&pdev->dev, of_clk_hw_simple_get,
					  &mcam->mclk_hw);
	if (ret) {
		dev_err(&pdev->dev, "can't add DT clock provider\n");
		goto out;
	}

	/*
	 * Finally, set up our IRQ now that the core is ready to
	 * deal with it.
	 */
	ret = platform_get_irq(pdev, 0);
	if (ret < 0)
		goto out;
	cam->irq = ret;
	ret = devm_request_irq(&pdev->dev, cam->irq, mmpcam_irq, IRQF_SHARED,
					"mmp-camera", mcam);
	if (ret)
		goto out;

	pm_runtime_enable(&pdev->dev);
	return 0;
out:
	mccic_shutdown(mcam);
out_v4l2_device_unregister:
	v4l2_device_unregister(&mcam->v4l2_dev);

	return ret;
}

static void mmpcam_remove(struct platform_device *pdev)
{
	struct mmp_camera *cam = platform_get_drvdata(pdev);
	struct mcam_camera *mcam = &cam->mcam;

	mccic_shutdown(mcam);
	v4l2_device_unregister(&mcam->v4l2_dev);
	pm_runtime_force_suspend(mcam->dev);
}

/*
 * Suspend/resume support.
 */

static int __maybe_unused mmpcam_runtime_resume(struct device *dev)
{
	struct mmp_camera *cam = dev_get_drvdata(dev);
	struct mcam_camera *mcam = &cam->mcam;
	unsigned int i;

	for (i = 0; i < NR_MCAM_CLK; i++) {
		if (!IS_ERR(mcam->clk[i]))
			clk_prepare_enable(mcam->clk[i]);
	}

	return 0;
}

static int __maybe_unused mmpcam_runtime_suspend(struct device *dev)
{
	struct mmp_camera *cam = dev_get_drvdata(dev);
	struct mcam_camera *mcam = &cam->mcam;
	int i;

	for (i = NR_MCAM_CLK - 1; i >= 0; i--) {
		if (!IS_ERR(mcam->clk[i]))
			clk_disable_unprepare(mcam->clk[i]);
	}

	return 0;
}

static int __maybe_unused mmpcam_suspend(struct device *dev)
{
	struct mmp_camera *cam = dev_get_drvdata(dev);

	if (!pm_runtime_suspended(dev))
		mccic_suspend(&cam->mcam);
	return 0;
}

static int __maybe_unused mmpcam_resume(struct device *dev)
{
	struct mmp_camera *cam = dev_get_drvdata(dev);

	if (!pm_runtime_suspended(dev))
		return mccic_resume(&cam->mcam);
	return 0;
}

static const struct dev_pm_ops mmpcam_pm_ops = {
	SET_RUNTIME_PM_OPS(mmpcam_runtime_suspend, mmpcam_runtime_resume, NULL)
	SET_SYSTEM_SLEEP_PM_OPS(mmpcam_suspend, mmpcam_resume)
};

static const struct of_device_id mmpcam_of_match[] = {
	{ .compatible = "marvell,mmp2-ccic", },
	{},
};
MODULE_DEVICE_TABLE(of, mmpcam_of_match);

static struct platform_driver mmpcam_driver = {
	.probe		= mmpcam_probe,
	.remove		= mmpcam_remove,
	.driver = {
		.name	= "mmp-camera",
		.of_match_table = mmpcam_of_match,
		.pm = &mmpcam_pm_ops,
	}
};

module_platform_driver(mmpcam_driver);
