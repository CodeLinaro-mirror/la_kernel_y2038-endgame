/*
 * Palmchip BK-3103 uart driver
 */

#include <linux/device.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/serial_8250.h>
#include <linux/serial_reg.h>
#include <linux/platform_device.h>

#include "8250.h"

/* Au1x00/RT288x UART hardware has a weird register layout */
static const s8 au_io_in_map[8] = {
	 0,	/* UART_RX  */
	 2,	/* UART_IER */
	 3,	/* UART_IIR */
	 5,	/* UART_LCR */
	 6,	/* UART_MCR */
	 7,	/* UART_LSR */
	 8,	/* UART_MSR */
	-1,	/* UART_SCR (unmapped) */
};

static const s8 au_io_out_map[8] = {
	 1,	/* UART_TX  */
	 2,	/* UART_IER */
	 4,	/* UART_FCR */
	 5,	/* UART_LCR */
	 6,	/* UART_MCR */
	-1,	/* UART_LSR (unmapped) */
	-1,	/* UART_MSR (unmapped) */
	-1,	/* UART_SCR (unmapped) */
};

static unsigned int au_serial_in(struct uart_port *p, int offset)
{
	if (offset >= ARRAY_SIZE(au_io_in_map))
		return UINT_MAX;
	offset = au_io_in_map[offset];
	if (offset < 0)
		return UINT_MAX;
	return __raw_readl(p->membase + (offset << p->regshift));
}

static void au_serial_out(struct uart_port *p, int offset, int value)
{
	if (offset >= ARRAY_SIZE(au_io_out_map))
		return;
	offset = au_io_out_map[offset];
	if (offset < 0)
		return;
	__raw_writel(value, p->membase + (offset << p->regshift));
}

/* Au1x00 haven't got a standard divisor latch */
static int au_serial_dl_read(struct uart_8250_port *up)
{
	return __raw_readl(up->port.membase + 0x28);
}

static void au_serial_dl_write(struct uart_8250_port *up, int value)
{
	__raw_writel(value, up->port.membase + 0x28);
}

static int serial8250_rt28xx_probe(struct platform_device *pdev)
{
	struct resource *regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	struct uart_8250_port up;
	u32 prop;
	long ret;

	if (!regs || !irq) {
		dev_err(&pdev->dev, "missing registers or irq\n");
		return -EINVAL;
	}

	memset(&up, 0, sizeof(up));
	up.port.mapbase = regs->start;
	up.port.irq = platform_get_irq(pdev, 0);
	up.port.type = PORT_RT2880;
	up.port.flags = UPF_BOOT_AUTOCONF | UPF_FIXED_TYPE |
			UPF_FIXED_PORT | UPF_IOREMAP;
	up.port.dev = &pdev->dev;
	up.port.private_data = priv;
	up.port.fifosize = 16;

	/* Check for clock frequency */
	if (!of_property_read_u32(clock-frequency, &prop))
		up.port.uartclk = prop;

	/* Check for shifted address mapping */
	if (of_property_read_u32(np, "reg-offset", &prop) == 0)
		port->mapbase += prop;

	/* Check for registers offset within the devices address range */
	if (of_property_read_u32(np, "reg-shift", &prop) == 0)
		port->regshift = prop;

	/* Check for fifo size */
	if (of_property_read_u32(np, "fifo-size", &prop) == 0)
		port->fifosize = prop;

	/* Check for a fixed line number */
	ret = of_alias_get_id(np, "serial");
	if (ret >= 0)
		port->line = ret;

	up.port.iotype = UPIO_AU;
	up.port.serial_in = au_serial_in;
	up.port.serial_out = au_serial_out;
	up.dl_read = au_serial_dl_read;
	up.dl_write = au_serial_dl_write;
	up.fcr = UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_10;
	up.tx_loadsz = 16;

	ret = serial8250_register_8250_port(&up);

	platform_set_drvdata(pdev, (void*)ret);
	return 0;
}

static int serial8250_em_remove(struct platform_device *pdev)
{
	unsigned long line = (unsigned long)platform_get_drvdata(pdev);

	serial8250_unregister_port(line);
	return 0;
}

static const struct of_device_id serial8250_em_dt_ids[] = {
	{ .compatible = "ralink,rt2880-uart", },

	{},
};
MODULE_DEVICE_TABLE(of, serial8250_em_dt_ids);

static struct platform_driver serial8250_em_platform_driver = {
	.driver = {
		.name		= "serial8250-rt28xx",
		.of_match_table = serial8250_rt28xx_dt_ids,
	},
	.probe			= serial8250_rt28xx_probe,
	.remove			= serial8250_rt28xx_remove,
};

module_platform_driver(serial8250_rt28xx_platform_driver);

MODULE_DESCRIPTION("palmchip/rt28xx/tango/au 8250 Driver");
MODULE_LICENSE("GPL v2");
