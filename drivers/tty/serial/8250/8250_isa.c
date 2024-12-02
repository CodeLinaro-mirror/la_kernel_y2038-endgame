// SPDX-License-Identifier: GPL-2.0+
/*
 *  Legacy platform driver for ISA 8250/16550-type serial ports
 *
 *  Supports:
 *	      ISA-compatible 8250/16550 ports
 *	      PNP/ACPI 8250/16550 ports
 *	      "early" uart devices
 */
#include <linux/array_size.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>

#include <linux/serial_8250.h>

#ifdef CONFIG_SPARC
#include <linux/sunserialcore.h>
#endif

#include "8250.h"

/*
 * Configuration:
 * share_irqs:     Whether we pass IRQF_SHARED to request_irq().
 *                 This option is unsafe when used on edge-triggered interrupts.
 * skip_txen_test: Force skip of txen test at init time.
 */
static unsigned int share_irqs = SERIAL8250_SHARE_IRQS;
unsigned int skip_txen_test;

#include <asm/serial.h>

/*
 * old_serial_port tells us about built-in ports that have no
 * standard enumeration mechanism. Platforms that can find all
 * serial ports via mechanisms like ACPI or PCI need not supply it.
 */

/* Standard COM flags (except for COM4, because of the 8514 problem) */
#ifdef CONFIG_SERIAL_8250_DETECT_IRQ
#define STD_COMX_FLAGS (UPF_BOOT_AUTOCONF |	UPF_SKIP_TEST	| UPF_AUTO_IRQ)
#define STD_COM4_FLAGS (UPF_BOOT_AUTOCONF |	0		| UPF_AUTO_IRQ)
#else
#define STD_COMX_FLAGS (UPF_BOOT_AUTOCONF |	UPF_SKIP_TEST	| 0		)
#define STD_COM4_FLAGS (UPF_BOOT_AUTOCONF |	0		| 0		)
#endif

static struct uart_port old_serial_port[] __initdata = {
#ifdef CONFIG_SERIAL_8250_ISA
	{ .iotype = UPIO_PORT, .uartclk = 1843200, .iobase = 0x3F8, .irq = 4, .flags = STD_COMX_FLAGS },
	{ .iotype = UPIO_PORT, .uartclk = 1843200, .iobase = 0x2F8, .irq = 3, .flags = STD_COMX_FLAGS },
	{ .iotype = UPIO_PORT, .uartclk = 1843200, .iobase = 0x3E8, .irq = 4, .flags = STD_COMX_FLAGS },
	{ .iotype = UPIO_PORT, .uartclk = 1843200, .iobase = 0x2E8, .irq = 3, .flags = STD_COM4_FLAGS },
#endif
};

serial8250_isa_config_fn serial8250_isa_config;
void serial8250_set_isa_configurator(serial8250_isa_config_fn v)
{
	serial8250_isa_config = v;
}
EXPORT_SYMBOL(serial8250_set_isa_configurator);

/*
 * early_serial_setup - early registration for 8250 ports
 *
 * Setup an 8250 port structure prior to console initialisation.  Use
 * after console initialisation will cause undefined behaviour.
 */
int __init early_serial_setup(struct uart_port *port)
{
	struct uart_port *p;

	if (port->line >= UART_NR || nr_uarts == 0)
		return -ENODEV;

	serial8250_setup_ports();
	p = &serial8250_get_port(port->line)->port;
	p->iobase       = port->iobase;
	p->membase      = port->membase;
	p->irq          = port->irq;
	p->irqflags     = port->irqflags;
	p->uartclk      = port->uartclk;
	p->fifosize     = port->fifosize;
	p->regshift     = port->regshift;
	p->iotype       = port->iotype;
	p->flags        = port->flags;
	p->mapbase      = port->mapbase;
	p->mapsize      = port->mapsize;
	p->private_data = port->private_data;
	p->type		= port->type;
	p->line		= port->line;

	serial8250_set_defaults(up_to_u8250p(p));

	if (port->serial_in)
		p->serial_in = port->serial_in;
	if (port->serial_out)
		p->serial_out = port->serial_out;
	if (port->handle_irq)
		p->handle_irq = port->handle_irq;

	return 0;
}

void __init serial8250_isa_init_ports(void)
{
	int i;

	if (nr_uarts > UART_NR)
		nr_uarts = UART_NR;

	for (i = 0; i < ARRAY_SIZE(old_serial_port) && i < nr_uarts; i++) {
		struct uart_8250_port *up = serial8250_get_port(i);

		old_serial_port[i].line = i;
		old_serial_port[i].irqflags = share_irqs ? IRQF_SHARED : 0;

		early_serial_setup(&old_serial_port[i]);

		serial8250_set_defaults(up);

		/* Allow Intel CE4100 and jailhouse to override defaults */
		if (serial8250_isa_config != NULL)
			serial8250_isa_config(i, &up->port, &up->capabilities);
	}
}

/*
 * This "device" covers _all_ ISA 8250-compatible serial devices listed
 * in the old_serial_port[] table as well as those from early_serial_setup().
 */
struct platform_device *serial8250_isa_devs;
static int serial8250_probe(struct platform_device *pdev)
{
	int i;

	for (i = 0; i < nr_uarts; i++) {
		struct uart_8250_port *up = serial8250_get_port(i);

		if (up->port.type == PORT_8250_CIR)
			continue;

		if (up->port.dev)
			continue;

		up->port.dev = &pdev->dev;

		if (uart_console_registered(&up->port) && up->port.dev)
			pm_runtime_get_sync(up->port.dev);

		serial8250_apply_quirks(up);
		uart_add_one_port(&serial8250_reg, &up->port);
	}

	return 0;
}

/*
 * Remove serial ports registered against a platform device.
 */
static void serial8250_remove(struct platform_device *dev)
{
	int i;

	for (i = 0; i < nr_uarts; i++) {
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
			uart_suspend_port(&serial8250_reg, &up->port);
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

static struct platform_driver serial8250_isa_driver = {
	.probe		= serial8250_probe,
	.remove_new	= serial8250_remove,
	.suspend	= serial8250_suspend,
	.resume		= serial8250_resume,
	.driver		= {
		.name	= "serial8250-isa",
	},
};

int __init serial8250_isa_init(void)
{
	int ret;

	/*
	 * pre-populate the usual ISA devices on x86, to ensure the
	 * expected order between them.
	 */
	if (nr_uarts > 0 && !IS_ENABLED(CONFIG_SERIAL_8250_CONSOLE))
		serial8250_isa_init_ports();

	pr_info("Serial: 8250/16550 driver, %d ports, IRQ sharing %s\n",
		nr_uarts, str_enabled_disabled(share_irqs));

	/*
	 * The PnP driver finds the actually present devices on
	 * most PCs, usually through ACPI. This overrides the devices
	 * from serial8250_isa_init_ports().
	 */
	ret = serial8250_pnp_init();
	if (ret)
		goto out;

	serial8250_isa_devs = platform_device_alloc("serial8250-isa", PLAT8250_DEV_LEGACY);
	if (!serial8250_isa_devs) {
		ret = -ENOMEM;
		goto unreg_pnp;
	}

	ret = platform_device_add(serial8250_isa_devs);
	if (ret) {
		platform_device_put(serial8250_isa_devs);
		goto unreg_pnp;
	}

	ret = platform_driver_register(&serial8250_isa_driver);
	if (ret == 0)
		return 0;

	platform_device_unregister(serial8250_isa_devs);
unreg_pnp:
	serial8250_pnp_exit();
out:
	return ret;
}

void __exit serial8250_isa_exit(void)
{
	platform_driver_unregister(&serial8250_isa_driver);
	platform_device_unregister(serial8250_isa_devs);

	serial8250_pnp_exit();
}

module_param_hw(share_irqs, uint, other, 0644);
MODULE_PARM_DESC(share_irqs, "Share IRQs with other non-8250/16x50 devices (unsafe)");

/*
 * Overriding this parameter changes the preset ISA style uarts
 * that can be set up with setserial. Since Linux-6.5, this is no
 * longer required to ports from DT or platform_data.
 */
unsigned int nr_uarts = ARRAY_SIZE(old_serial_port);
module_param(nr_uarts, uint, 0644);
MODULE_PARM_DESC(nr_uarts, "Maximum number of ISA style UARTs supported. (1-"
		 __MODULE_STRING(CONFIG_SERIAL_8250_NR_UARTS) ")");

module_param(skip_txen_test, uint, 0644);
MODULE_PARM_DESC(skip_txen_test, "Skip checking for the TXEN bug at init time");

#ifdef CONFIG_SERIAL_8250_DEPRECATED_OPTIONS
#ifndef MODULE
/*
 * This module was renamed to 8250_core in 3.7. Keep the old "8250" name
 * working as well for the module options so we don't break people. We
 * need to keep the names identical and the convenient macros will happily
 * refuse to let us do that by failing the build with redefinition errors
 * of global variables. So we stick them inside a dummy function to avoid
 * those conflicts. The options still get parsed, and the redefined
 * MODULE_PARAM_PREFIX lets us keep the "8250." syntax alive.
 *
 * This is hacky.  I'm sorry.
 */
static void __used s8250_options(void)
{
#undef MODULE_PARAM_PREFIX
#define MODULE_PARAM_PREFIX "8250_core."

	module_param_cb(share_irqs, &param_ops_uint, &share_irqs, 0644);
	module_param_cb(nr_uarts, &param_ops_uint, &nr_uarts, 0644);
	module_param_cb(skip_txen_test, &param_ops_uint, &skip_txen_test, 0644);
}
#else
MODULE_ALIAS("8250_core");
#endif
#endif
