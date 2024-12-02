// SPDX-License-Identifier: GPL-2.0+
/*
 *  Universal/legacy platform driver for 8250/16550-type serial ports
 *
 *  Supports:
 *	      ISA-compatible 8250/16550 ports
 *	      ACPI 8250/16550 ports
 *	      PNP 8250/16550 ports
 *	      "serial8250" platform devices
 */
#include <linux/acpi.h>
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
unsigned int share_irqs = SERIAL8250_SHARE_IRQS;
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
 * Generic 16550A platform devices
 */
static int serial8250_probe_acpi(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct uart_8250_port uart = { };
	struct resource *regs;
	unsigned char iotype;
	int ret, line;

	regs = platform_get_mem_or_io(pdev, 0);
	if (!regs)
		return dev_err_probe(dev, -EINVAL, "no registers defined\n");

	switch (resource_type(regs)) {
	case IORESOURCE_IO:
		uart.port.iobase = regs->start;
		iotype = UPIO_PORT;
		break;
	case IORESOURCE_MEM:
		uart.port.mapbase = regs->start;
		uart.port.mapsize = resource_size(regs);
		uart.port.flags = UPF_IOREMAP;
		iotype = UPIO_MEM;
		break;
	default:
		return -EINVAL;
	}

	/* default clock frequency */
	uart.port.uartclk = 1843200;
	uart.port.type = PORT_16550A;
	uart.port.dev = &pdev->dev;
	uart.port.flags |= UPF_SKIP_TEST | UPF_BOOT_AUTOCONF;

	ret = uart_read_and_validate_port_properties(&uart.port);
	/* no interrupt -> fall back to polling */
	if (ret == -ENXIO)
		ret = 0;
	if (ret)
		return ret;

	/*
	 * The previous call may not set iotype correctly when reg-io-width
	 * property is absent and it doesn't support IO port resource.
	 */
	uart.port.iotype = iotype;

	line = serial8250_register_8250_port(&uart);
	if (line < 0)
		return line;

	return 0;
}

static int serial8250_probe_platform(struct platform_device *dev, struct plat_serial8250_port *p)
{
	struct uart_8250_port uart;
	int ret, i, irqflag = 0;

	memset(&uart, 0, sizeof(uart));

	for (i = 0; p && p->flags != 0; p++, i++) {
		uart.port.iobase	= p->iobase;
		uart.port.membase	= p->membase;
		uart.port.irq		= p->irq;
		uart.port.irqflags	= p->irqflags;
		uart.port.uartclk	= p->uartclk;
		uart.port.regshift	= p->regshift;
		uart.port.iotype	= p->iotype;
		uart.port.flags		= p->flags;
		uart.port.mapbase	= p->mapbase;
		uart.port.mapsize	= p->mapsize;
		uart.port.hub6		= p->hub6;
		uart.port.has_sysrq	= p->has_sysrq;
		uart.port.private_data	= p->private_data;
		uart.port.type		= p->type;
		uart.bugs		= p->bugs;
		uart.port.serial_in	= p->serial_in;
		uart.port.serial_out	= p->serial_out;
		uart.dl_read		= p->dl_read;
		uart.dl_write		= p->dl_write;
		uart.port.handle_irq	= p->handle_irq;
		uart.port.handle_break	= p->handle_break;
		uart.port.set_termios	= p->set_termios;
		uart.port.set_ldisc	= p->set_ldisc;
		uart.port.get_mctrl	= p->get_mctrl;
		uart.port.pm		= p->pm;
		uart.port.dev		= &dev->dev;
		uart.port.irqflags	|= irqflag;
		ret = serial8250_register_8250_port(&uart);
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
 * This "device" covers _all_ ISA 8250-compatible serial devices listed
 * in the old_serial_port[] table as well as those from early_serial_setup().
 */
struct platform_device *serial8250_isa_devs;
static int serial8250_register_ports(struct uart_driver *drv)
{
	int i;

	for (i = 0; i < nr_uarts; i++) {
		struct uart_8250_port *up = serial8250_get_port(i);

		if (up->port.type == PORT_8250_CIR)
			continue;

		if (up->port.dev)
			continue;

		up->port.dev = &serial8250_isa_devs->dev;

		if (uart_console_registered(&up->port) && up->port.dev)
			pm_runtime_get_sync(up->port.dev);

		serial8250_apply_quirks(up);
		uart_add_one_port(drv, &up->port);
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

	/*
	 * Any device not claimed by the PnP driver gets assigned
	 * to serial8250_isa_devs here. This also includes any
	 * platform specific ones that came through early_serial_setup().
	 */
	return serial8250_register_ports(&serial8250_reg);
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

static int __init serial8250_init(void)
{
	int ret;

	serial8250_setup_ports();

	/*
	 * pre-populate the usual ISA devices on x86, to ensure the
	 * expected order between them.
	 */
	if (nr_uarts > 0 && !IS_ENABLED(CONFIG_SERIAL_8250_CONSOLE))
		serial8250_isa_init_ports();

	pr_info("Serial: 8250/16550 driver, %d ports, IRQ sharing %s\n",
		nr_uarts, str_enabled_disabled(share_irqs));

#ifdef CONFIG_SPARC
	ret = sunserial_register_minors(&serial8250_reg, UART_NR);
#else
	serial8250_reg.nr = UART_NR;
	ret = uart_register_driver(&serial8250_reg);
#endif
	if (ret)
		goto out;

	/*
	 * The PnP driver finds the actually present devices on
	 * most PCs, usually through ACPI. This overrides the devices
	 * from serial8250_isa_init_ports().
	 */
	ret = serial8250_pnp_init();
	if (ret)
		goto unreg_uart_drv;

	serial8250_isa_devs = platform_device_alloc("serial8250", PLAT8250_DEV_LEGACY);
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
unreg_uart_drv:
#ifdef CONFIG_SPARC
	sunserial_unregister_minors(&serial8250_reg, UART_NR);
#else
	uart_unregister_driver(&serial8250_reg);
#endif
out:
	return ret;
}
module_init(serial8250_init);

static void __exit serial8250_exit(void)
{
	platform_driver_unregister(&serial8250_isa_driver);
	platform_device_unregister(serial8250_isa_devs);

	serial8250_pnp_exit();

#ifdef CONFIG_SPARC
	sunserial_unregister_minors(&serial8250_reg, UART_NR);
#else
	uart_unregister_driver(&serial8250_reg);
#endif
}
module_exit(serial8250_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Generic 8250/16x50 serial platform driver");

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

MODULE_ALIAS_CHARDEV_MAJOR(TTY_MAJOR);

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
