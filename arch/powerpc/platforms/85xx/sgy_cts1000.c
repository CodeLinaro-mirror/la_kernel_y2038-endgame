// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Servergy CTS-1000 Setup
 *
 * Maintained by Ben Collins <ben.c@servergy.com>
 *
 * Copyright 2012 by Servergy, Inc.
 */

#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/workqueue.h>
#include <linux/reboot.h>
#include <linux/interrupt.h>

#include <asm/machdep.h>

struct gpio_desc *halt_gpio;

static const struct of_device_id child_match[] = {
	{
		.compatible = "sgy,gpio-halt",
	},
	{},
};

static void gpio_halt_wfn(struct work_struct *work)
{
	/* Likely wont return */
	orderly_poweroff(true);
}
static DECLARE_WORK(gpio_halt_wq, gpio_halt_wfn);

static void __noreturn gpio_halt_cb(void)
{
	if (!halt_gpio)
		panic("No reset GPIO information was provided in DT\n");

	printk(KERN_INFO "gpio-halt: triggering GPIO.\n");

	/* Probably wont return */
	gpiod_set_value(halt_gpio, 0);

	panic("Halt failed\n");
}

/* This IRQ means someone pressed the power button and it is waiting for us
 * to handle the shutdown/poweroff. */
static irqreturn_t gpio_halt_irq(int irq, void *__data)
{
	printk(KERN_INFO "gpio-halt: shutdown due to power button IRQ.\n");
	schedule_work(&gpio_halt_wq);

        return IRQ_HANDLED;
};

static int gpio_halt_probe(struct platform_device *pdev)
{
	enum of_gpio_flags flags;
	struct device_node *node = pdev->dev.of_node;
	struct device_node *halt_node;
	int gpio, err, irq;
	int trigger;

	if (!node)
		return -ENODEV;

	/* If there's no matching child, this isn't really an error */
	halt_node = of_find_matching_node(node, child_match);
	if (!halt_node)
		return 0;

	halt_gpio = devm_fwnode_gpiod_get_index(&pdev->dev,
						of_node_to_fwnode(halt_node),
						NULL, 0, GPIOD_OUT_HIGH,
						"gpio-halt");
	if (IS_ERR(halt_gpio)) {
		printk(KERN_ERR "gpio-halt: error requesting GPIO\n");
		halt_gpio = NULL;
		return PTR_ERR(halt_gpio);
	}

	/* Now get the IRQ which tells us when the power button is hit */
	irq = irq_of_parse_and_map(halt_node, 0);
	err = devm_request_irq(irq, gpio_halt_irq, IRQF_TRIGGER_RISING |
			  IRQF_TRIGGER_FALLING, "gpio-halt", halt_node);
	if (err) {
		printk(KERN_ERR "gpio-halt: error requesting IRQ %d for "
		       "GPIO %d.\n", irq, gpio);
		halt_gpio = NULL;
		return err;
	}

	/* Register our halt function */
	ppc_md.halt = gpio_halt_cb;
	pm_power_off = gpio_halt_cb;

	printk(KERN_INFO "gpio-halt: registered GPIO, %d irq).\n", irq);

	return 0;
}

static int gpio_halt_remove(struct platform_device *pdev)
{
	ppc_md.halt = NULL;
	pm_power_off = NULL;
	halt_gpio = NULL;

	return 0;
}

static const struct of_device_id gpio_halt_match[] = {
	/* We match on the gpio bus itself and scan the children since they
	 * wont be matched against us. We know the bus wont match until it
	 * has been registered too. */
	{
		.compatible = "fsl,qoriq-gpio",
	},
	{},
};
MODULE_DEVICE_TABLE(of, gpio_halt_match);

static struct platform_driver gpio_halt_driver = {
	.driver = {
		.name		= "gpio-halt",
		.of_match_table = gpio_halt_match,
	},
	.probe		= gpio_halt_probe,
	.remove		= gpio_halt_remove,
};

module_platform_driver(gpio_halt_driver);

MODULE_DESCRIPTION("Driver to support GPIO triggered system halt for Servergy CTS-1000 Systems.");
MODULE_VERSION("1.0");
MODULE_AUTHOR("Ben Collins <ben.c@servergy.com>");
MODULE_LICENSE("GPL");
