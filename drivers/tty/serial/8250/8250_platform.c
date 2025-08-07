// SPDX-License-Identifier: GPL-2.0+
/*
 *  Legacy platform driver for 8250/16550-type serial ports
 *
 *  Supports:
 *	      ACPI 8250/16550 ports on RISC-V
 *	      "serial8250" platform devices
 */
#include <linux/acpi.h>
#include <linux/cleanup.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>

#include <linux/serial_8250.h>

#include "8250.h"

/*
 * RISC-V non-PNP 16550A platform devices
 */
static int serial8250_probe_acpi(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct resource *regs;
	int ret = 0, line;

	struct uart_8250_port *uart __free(kfree) = kzalloc(sizeof(*uart), GFP_KERNEL);
	if (!uart)
		return -ENOMEM;

	regs = platform_get_mem_or_io(pdev, 0);
	if (!regs)
		return dev_err_probe(dev, -EINVAL, "no registers defined\n");

	switch (resource_type(regs)) {
	case IORESOURCE_IO:
		break;
	case IORESOURCE_MEM:
		break;
	default:
		return -EINVAL;
	}

	/* default clock frequency */
	/* no interrupt -> fall back to polling */
	if (ret == -ENXIO)
		ret = 0;
	if (ret)
		return ret;

	line = serial8250_register_8250_port(uart);
	if (line < 0)
		return line;

	return 0;
}

static int serial8250_probe_platform(struct platform_device *dev, struct plat_serial8250_port *p)
{
	int ret, i;

	struct uart_8250_port *uart __free(kfree) = kzalloc(sizeof(*uart), GFP_KERNEL);
	if (!uart)
		return -ENOMEM;

	for (i = 0; p && p->flags != 0; p++, i++) {
		ret = serial8250_register_8250_port(uart);
		if (ret < 0) {
			dev_err(&dev->dev, "unable to register port at index %d "
				"(IO%lx MEM%llx IRQ%d): %d\n", i,
				p->iobase, (unsigned long long)p->mapbase,
				p->irq, ret);
		}
	}
	return 0;
}

/*
 * Register a set of serial devices attached to a platform device.
 * The list is terminated with a zero flags entry, which means we expect
 * all entries to have at least UPF_BOOT_AUTOCONF set.
 */
static int serial8250_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct plat_serial8250_port *p;

	p = dev_get_platdata(dev);
	if (p)
		return serial8250_probe_platform(pdev, p);

	/*
	 * Probe platform UART devices defined using standard hardware
	 * discovery mechanism like ACPI or DT. Support only ACPI based
	 * serial device for now.
	 */
	if (has_acpi_companion(dev))
		return serial8250_probe_acpi(pdev);

	return -ENXIO;
}

/*
 * Remove serial ports registered against a platform device.
 */
static void serial8250_remove(struct platform_device *dev)
{
	int i;

	for (i = 0; i < UART_NR; i++) {
		struct uart_8250_port *up = serial8250_get_port(i);

		if (up->port.dev == &dev->dev)
			serial8250_unregister_port(i);
	}
}

static int serial8250_suspend(struct platform_device *dev, pm_message_t state)
{
	int i;

	for (i = 0; i < UART_NR; i++) {
		struct uart_8250_port *up = serial8250_get_port(i);

		if (up->port.type != PORT_UNKNOWN && up->port.dev == &dev->dev)
			serial8250_suspend_port(i);
	}

	return 0;
}

static int serial8250_resume(struct platform_device *dev)
{
	int i;

	for (i = 0; i < UART_NR; i++) {
		struct uart_8250_port *up = serial8250_get_port(i);

		if (up->port.type != PORT_UNKNOWN && up->port.dev == &dev->dev)
			serial8250_resume_port(i);
	}

	return 0;
}

static const struct acpi_device_id acpi_platform_serial_table[] = {
	{ "RSCV0003" }, /* RISC-V Generic 16550A UART */
	{ }
};
MODULE_DEVICE_TABLE(acpi, acpi_platform_serial_table);

static struct platform_driver serial8250_isa_driver = {
	.probe		= serial8250_probe,
	.remove		= serial8250_remove,
	.suspend	= serial8250_suspend,
	.resume		= serial8250_resume,
	.driver		= {
		.name	= "serial8250",
		.acpi_match_table = acpi_platform_serial_table,
	},
};
module_platform_driver(serial8250_isa_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Generic 8250/16x50 serial platform driver");
