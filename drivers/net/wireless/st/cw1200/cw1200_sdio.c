// SPDX-License-Identifier: GPL-2.0-only
/*
 * Mac80211 SDIO driver for ST-Ericsson CW1200 device
 *
 * Copyright (c) 2010, ST-Ericsson
 * Author: Dmitry Tarnyagin <dmitry.tarnyagin@lockless.no>
 */

#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/gpio/consumer.h>
#include <linux/delay.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/sdio_ids.h>
#include <net/mac80211.h>

#include "cw1200.h"
#include "hwbus.h"
#include "hwio.h"

MODULE_AUTHOR("Dmitry Tarnyagin <dmitry.tarnyagin@lockless.no>");
MODULE_DESCRIPTION("mac80211 ST-Ericsson CW1200 SDIO driver");
MODULE_LICENSE("GPL");

#define SDIO_BLOCK_SIZE (512)

struct hwbus_priv {
	struct sdio_func	*func;
	struct cw1200_common	*core;
};

static const struct sdio_device_id cw1200_sdio_ids[] = {
	{ SDIO_DEVICE(SDIO_VENDOR_ID_STE, SDIO_DEVICE_ID_STE_CW1200) },
	{ /* end: all zeroes */			},
};
MODULE_DEVICE_TABLE(sdio, cw1200_sdio_ids);

/* hwbus_ops implemetation */

static int cw1200_sdio_memcpy_fromio(struct hwbus_priv *self,
				     unsigned int addr,
				     void *dst, int count)
{
	return sdio_memcpy_fromio(self->func, dst, addr, count);
}

static int cw1200_sdio_memcpy_toio(struct hwbus_priv *self,
				   unsigned int addr,
				   const void *src, int count)
{
	return sdio_memcpy_toio(self->func, addr, (void *)src, count);
}

static void cw1200_sdio_lock(struct hwbus_priv *self)
{
	sdio_claim_host(self->func);
}

static void cw1200_sdio_unlock(struct hwbus_priv *self)
{
	sdio_release_host(self->func);
}

static void cw1200_sdio_irq_handler(struct sdio_func *func)
{
	struct hwbus_priv *self = sdio_get_drvdata(func);

	/* note:  sdio_host already claimed here. */
	if (self->core)
		cw1200_irq_handler(self->core);
}

static int cw1200_sdio_irq_subscribe(struct hwbus_priv *self)
{
	int ret = 0;

	pr_debug("SW IRQ subscribe\n");
	sdio_claim_host(self->func);
	ret = sdio_claim_irq(self->func, cw1200_sdio_irq_handler);
	sdio_release_host(self->func);
	return ret;
}

static int cw1200_sdio_irq_unsubscribe(struct hwbus_priv *self)
{
	int ret = 0;

	pr_debug("SW IRQ unsubscribe\n");

	sdio_claim_host(self->func);
	ret = sdio_release_irq(self->func);
	sdio_release_host(self->func);

	return ret;
}

/* Like the rest of the driver, this only supports one device per system */
static struct gpio_desc *cw1200_reset;
static struct gpio_desc *cw1200_powerup;

static int cw1200_sdio_off(void)
{
	if (cw1200_reset) {
		gpiod_set_value(cw1200_reset, 0);
		msleep(30); /* Min is 2 * CLK32K cycles */
	}

	return 0;
}

static int cw1200_sdio_on(void)
{
	/* Ensure I/Os are pulled low (reset is active low) */
	cw1200_reset = devm_gpiod_get_optional(NULL, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(cw1200_reset)) {
		pr_err("could not get CW1200 SDIO reset GPIO\n");
		return PTR_ERR(cw1200_reset);
	}
	gpiod_set_consumer_name(cw1200_reset, "cw1200_wlan_reset");
	cw1200_powerup = devm_gpiod_get_optional(NULL, "powerup", GPIOD_OUT_LOW);
	if (IS_ERR(cw1200_powerup)) {
		pr_err("could not get CW1200 SDIO powerup GPIO\n");
		return PTR_ERR(cw1200_powerup);
	}
	gpiod_set_consumer_name(cw1200_powerup, "cw1200_wlan_powerup");

	if (cw1200_reset || cw1200_powerup)
		msleep(10); /* Settle time? */

	/* Enable POWERUP signal */
	if (cw1200_powerup) {
		gpiod_set_value(cw1200_powerup, 1);
		msleep(250); /* or more..? */
	}
	/* Deassert RSTn signal, note active low */
	if (cw1200_reset) {
		gpiod_set_value(cw1200_reset, 0);
		msleep(50); /* Or more..? */
	}
	return 0;
}

static size_t cw1200_sdio_align_size(struct hwbus_priv *self, size_t size)
{
	return sdio_align_size(self->func, size);
}

static int cw1200_sdio_pm(struct hwbus_priv *self, bool suspend)
{
	return 0;
}

static const struct hwbus_ops cw1200_sdio_hwbus_ops = {
	.hwbus_memcpy_fromio	= cw1200_sdio_memcpy_fromio,
	.hwbus_memcpy_toio	= cw1200_sdio_memcpy_toio,
	.lock			= cw1200_sdio_lock,
	.unlock			= cw1200_sdio_unlock,
	.align_size		= cw1200_sdio_align_size,
	.power_mgmt		= cw1200_sdio_pm,
};

/* Probe Function to be called by SDIO stack when device is discovered */
static int cw1200_sdio_probe(struct sdio_func *func,
			     const struct sdio_device_id *id)
{
	struct hwbus_priv *self;
	int status;

	pr_info("cw1200_wlan_sdio: Probe called\n");

	/* We are only able to handle the wlan function */
	if (func->num != 0x01)
		return -ENODEV;

	self = kzalloc_obj(*self);
	if (!self) {
		pr_err("Can't allocate SDIO hwbus_priv.\n");
		return -ENOMEM;
	}

	func->card->quirks |= MMC_QUIRK_LENIENT_FN0;

	self->func = func;
	sdio_set_drvdata(func, self);
	sdio_claim_host(func);
	sdio_enable_func(func);
	sdio_release_host(func);

	status = cw1200_sdio_irq_subscribe(self);

	/*
	 * defaults for sagrad 109xevk board, missing a DT binding
	 * to get proper settings
	 */
	status = cw1200_core_probe(&cw1200_sdio_hwbus_ops,
				   self, &func->dev, &self->core,
				   38400, NULL,
				   "sdd_sagrad_1091_1098.bin",
				   false);
	if (status) {
		cw1200_sdio_irq_unsubscribe(self);
		sdio_claim_host(func);
		sdio_disable_func(func);
		sdio_release_host(func);
		sdio_set_drvdata(func, NULL);
		kfree(self);
	}

	return status;
}

/* Disconnect Function to be called by SDIO stack when
 * device is disconnected
 */
static void cw1200_sdio_disconnect(struct sdio_func *func)
{
	struct hwbus_priv *self = sdio_get_drvdata(func);

	if (self) {
		cw1200_sdio_irq_unsubscribe(self);
		if (self->core) {
			cw1200_core_release(self->core);
			self->core = NULL;
		}
		sdio_claim_host(func);
		sdio_disable_func(func);
		sdio_release_host(func);
		sdio_set_drvdata(func, NULL);
		kfree(self);
	}
}

#ifdef CONFIG_PM
static int cw1200_sdio_suspend(struct device *dev)
{
	int ret;
	struct sdio_func *func = dev_to_sdio_func(dev);
	struct hwbus_priv *self = sdio_get_drvdata(func);

	if (!cw1200_can_suspend(self->core))
		return -EAGAIN;

	/* Notify SDIO that CW1200 will remain powered during suspend */
	ret = sdio_set_host_pm_flags(func, MMC_PM_KEEP_POWER);
	if (ret)
		pr_err("Error setting SDIO pm flags: %i\n", ret);

	return ret;
}

static int cw1200_sdio_resume(struct device *dev)
{
	return 0;
}

static const struct dev_pm_ops cw1200_pm_ops = {
	.suspend = cw1200_sdio_suspend,
	.resume = cw1200_sdio_resume,
};
#endif

static struct sdio_driver sdio_driver = {
	.name		= "cw1200_wlan_sdio",
	.id_table	= cw1200_sdio_ids,
	.probe		= cw1200_sdio_probe,
	.remove		= cw1200_sdio_disconnect,
#ifdef CONFIG_PM
	.drv = {
		.pm = &cw1200_pm_ops,
	}
#endif
};

/* Init Module function -> Called by insmod */
static int __init cw1200_sdio_init(void)
{
	int ret;

	if (cw1200_sdio_on()) {
		ret = -1;
		goto err;
	}

	ret = sdio_register_driver(&sdio_driver);
	if (ret)
		goto err;

	return 0;

err:
	cw1200_sdio_off();
	return ret;
}

/* Called at Driver Unloading */
static void __exit cw1200_sdio_exit(void)
{
	sdio_unregister_driver(&sdio_driver);
	cw1200_sdio_off();
}


module_init(cw1200_sdio_init);
module_exit(cw1200_sdio_exit);
