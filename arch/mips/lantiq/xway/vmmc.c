// SPDX-License-Identifier: GPL-2.0-only
/*
 *
 *  Copyright (C) 2012 John Crispin <john@phrozen.org>
 */

#include <linux/export.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/dma-mapping.h>

#include <lantiq_soc.h>

static unsigned int *cp1_base;

unsigned int *ltq_get_cp1_base(void)
{
	if (!cp1_base)
		panic("no cp1 base was set\n");

	return cp1_base;
}
EXPORT_SYMBOL(ltq_get_cp1_base);

static int vmmc_probe(struct platform_device *pdev)
{
#define CP1_SIZE       (1 << 20)
	int gpio_count;
	dma_addr_t dma;
	struct gpio_desc *gpio;

	cp1_base =
		(void *) CPHYSADDR(dma_alloc_coherent(&pdev->dev, CP1_SIZE,
						    &dma, GFP_KERNEL));


	gpio_count = of_gpio_count(pdev->dev.of_node);
	while (gpio_count > 0) {
		enum of_gpio_flags flags;
		gpio = gpiod_get_index(&pdev->dev, NULL, --gpio_count,
					GPIOD_OUT_HIGH);
		if (IS_ERR(gpio))
			dev_err(&pdev->dev, "gpio_get failed\n", gpio_count);
		else
			dev_info(&pdev->dev, "requested GPIO %s\n", gpio->name);
	}

	dev_info(&pdev->dev, "reserved %dMB at 0x%p", CP1_SIZE >> 20, cp1_base);

	return 0;
}

static const struct of_device_id vmmc_match[] = {
	{ .compatible = "lantiq,vmmc-xway" },
	{},
};

static struct platform_driver vmmc_driver = {
	.probe = vmmc_probe,
	.driver = {
		.name = "lantiq,vmmc",
		.of_match_table = vmmc_match,
	},
};
builtin_platform_driver(vmmc_driver);
