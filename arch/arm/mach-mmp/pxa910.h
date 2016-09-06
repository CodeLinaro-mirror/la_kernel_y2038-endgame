#ifndef __ASM_MACH_PXA910_H
#define __ASM_MACH_PXA910_H

extern void pxa910_timer_init(void);
extern void __init icu_init_irq(void);
extern void __init pxa910_init_irq(void);

#include <linux/i2c.h>
#include <linux/i2c/pxa-i2c.h>
#include <linux/platform_data/mtd-nand-pxa3xx.h>
#include <video/mmp_disp.h>

#include "devices.h"

extern struct pxa_device_desc pxa910_device_uart1;
extern struct pxa_device_desc pxa910_device_uart2;
extern struct pxa_device_desc pxa910_device_twsi0;
extern struct pxa_device_desc pxa910_device_twsi1;
extern struct pxa_device_desc pxa910_device_nand;
extern struct platform_device pxa168_device_u2o;
extern struct platform_device pxa168_device_u2ootg;
extern struct platform_device pxa168_device_u2oehci;
extern struct pxa_device_desc pxa910_device_disp;
extern struct platform_device pxa910_device_gpio;
extern struct platform_device pxa910_device_rtc;

static inline int pxa910_add_uart(int id)
{
	struct pxa_device_desc *d = NULL;

	switch (id) {
	case 1: d = &pxa910_device_uart1; break;
	case 2: d = &pxa910_device_uart2; break;
	}

	if (d == NULL)
		return -EINVAL;

	return pxa_register_device(d, NULL, 0);
}

static inline int pxa910_add_twsi(int id, struct i2c_pxa_platform_data *data,
				  struct i2c_board_info *info, unsigned size)
{
	struct pxa_device_desc *d = NULL;
	int ret;

	switch (id) {
	case 0: d = &pxa910_device_twsi0; break;
	case 1: d = &pxa910_device_twsi1; break;
	default:
		return -EINVAL;
	}

	ret = i2c_register_board_info(id, info, size);
	if (ret)
		return ret;

	return pxa_register_device(d, data, sizeof(*data));
}

static inline int pxa910_add_pwm(int id)
{
	struct resource res = DEFINE_RES_MEM(0xd401a000ul + 0x400 * id, 0x10);
	struct platform_device *dev;

	dev = platform_device_register_simple("pxa910-pwm", id, &res, 1);

	return PTR_ERR_OR_ZERO(dev);
}

static inline int pxa910_add_nand(struct pxa3xx_nand_platform_data *info)
{
	return pxa_register_device(&pxa910_device_nand, info, sizeof(*info));
}

static inline int pxa910_add_fb(struct mmp_buffer_driver_mach_info *fb,
				struct mmp_mach_panel_info *panel)
{
	struct platform_device *dev;

	dev = platform_device_register_data(NULL, "mmp-fb", -1,
					    fb, sizeof(*fb));
	if (!IS_ERR(dev))
		dev = platform_device_register_data(NULL, "tpo-hvga", -1,
						    panel, sizeof(*panel));

	return PTR_ERR_OR_ZERO(dev);
}

#endif /* __ASM_MACH_PXA910_H */
