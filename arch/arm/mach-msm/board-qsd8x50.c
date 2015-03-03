/* Copyright (c) 2009-2011, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/irq.h>
#include <linux/platform_data/mmc-msm_sdcc.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/usb/msm_hsusb.h>
#include <linux/err.h>
#include <linux/clkdev.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/io.h>
#include <asm/setup.h>

#include "clk.h"
#include "common.h"
#include "devices.h"
#include "gpiomux.h"
#include "irqs-8x50.h"
#include "sirc.h"
#include "vreg.h"

static const resource_size_t qsd8x50_surf_smc91x_base __initconst = 0x70000300;
static const unsigned        qsd8x50_surf_smc91x_gpio __initconst = 156;

/* Leave smc91x resources empty here, as we'll fill them in
 * at run-time: they vary from board to board, and the true
 * configuration won't be known until boot.
 */
static struct resource smc91x_resources[] = {
	[0] = {
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device smc91x_device = {
	.name           = "smc91x",
	.id             = 0,
	.num_resources  = ARRAY_SIZE(smc91x_resources),
	.resource       = smc91x_resources,
};

static int __init msm_init_smc91x(void)
{
	if (machine_is_qsd8x50_surf()) {
		smc91x_resources[0].start = qsd8x50_surf_smc91x_base;
		smc91x_resources[0].end   = qsd8x50_surf_smc91x_base + 0xff;
		smc91x_resources[1].start =
			gpio_to_irq(qsd8x50_surf_smc91x_gpio);
		smc91x_resources[1].end   =
			gpio_to_irq(qsd8x50_surf_smc91x_gpio);
		platform_device_register(&smc91x_device);
	}

	return 0;
}
module_init(msm_init_smc91x);

static int hsusb_phy_init_seq[] = {
	0x08, 0x31,	/* Increase HS Driver Amplitude */
	0x20, 0x32,	/* Enable and set Pre-Emphasis Depth to 10% */
	-1
};

static int hsusb_link_clk_reset(struct clk *link_clk, bool assert)
{
	int ret;

	if (assert) {
		ret = clk_reset(link_clk, CLK_RESET_ASSERT);
		if (ret)
			pr_err("usb hs_clk assert failed\n");
	} else {
		ret = clk_reset(link_clk, CLK_RESET_DEASSERT);
		if (ret)
			pr_err("usb hs_clk deassert failed\n");
	}
	return ret;
}

static int hsusb_phy_clk_reset(struct clk *phy_clk)
{
	int ret;

	ret = clk_reset(phy_clk, CLK_RESET_ASSERT);
	if (ret) {
		pr_err("usb phy clk assert failed\n");
		return ret;
	}
	usleep_range(10000, 12000);
	ret = clk_reset(phy_clk, CLK_RESET_DEASSERT);
	if (ret)
		pr_err("usb phy clk deassert failed\n");
	return ret;
}

static struct msm_otg_platform_data msm_otg_pdata = {
	.phy_init_seq		= hsusb_phy_init_seq,
	.mode                   = USB_DR_MODE_PERIPHERAL,
	.otg_control		= OTG_PHY_CONTROL,
	.link_clk_reset		= hsusb_link_clk_reset,
	.phy_clk_reset		= hsusb_phy_clk_reset,
};

static struct platform_device *devices[] __initdata = {
	&msm_clock_8x50,
	&qsd8x50_device_gpio_8x50,
	&qsd8x50_device_uart3,
	&qsd8x50_device_smd,
	&qsd8x50_device_otg,
	&qsd8x50_device_hsusb,
	&qsd8x50_device_hsusb_host,
};

static struct msm_mmc_gpio sdc1_gpio_cfg[] = {
	{51, "sdc1_dat_3"},
	{52, "sdc1_dat_2"},
	{53, "sdc1_dat_1"},
	{54, "sdc1_dat_0"},
	{55, "sdc1_cmd"},
	{56, "sdc1_clk"}
};

static struct vreg *vreg_mmc;
static unsigned long vreg_sts;

static uint32_t msm_sdcc_setup_power(struct device *dv, unsigned int vdd)
{
	int rc = 0;
	struct platform_device *pdev;

	pdev = container_of(dv, struct platform_device, dev);

	if (vdd == 0) {
		if (!vreg_sts)
			return 0;

		clear_bit(pdev->id, &vreg_sts);

		if (!vreg_sts) {
			rc = vreg_disable(vreg_mmc);
			if (rc)
				pr_err("vreg_mmc disable failed for slot "
						"%d: %d\n", pdev->id, rc);
		}
		return 0;
	}

	if (!vreg_sts) {
		rc = vreg_set_level(vreg_mmc, 2900);
		if (rc)
			pr_err("vreg_mmc set level failed for slot %d: %d\n",
					pdev->id, rc);
		rc = vreg_enable(vreg_mmc);
		if (rc)
			pr_err("vreg_mmc enable failed for slot %d: %d\n",
					pdev->id, rc);
	}
	set_bit(pdev->id, &vreg_sts);
	return 0;
}

static void msm_sdcc_clk_reset(struct clk *clk)
{
	int ret;

	ret = clk_reset(clk, CLK_RESET_ASSERT);
	if (ret)
		pr_err("sdcc clock assert failed at %lu Hz with err %d\n",
			clk_get_rate(clk), ret);

	ret = clk_reset(clk, CLK_RESET_DEASSERT);
	if (ret)
	        pr_err("sdcc clock deassert failed at %lu Hz with err %d\n",
			clk_get_rate(clk), ret);
}

static struct msm_mmc_gpio_data sdc1_gpio = {
	.gpio = sdc1_gpio_cfg,
	.size = ARRAY_SIZE(sdc1_gpio_cfg),
};

static struct msm_mmc_platform_data qsd8x50_sdc1_data = {
	.ocr_mask	= MMC_VDD_27_28 | MMC_VDD_28_29,
	.translate_vdd	= msm_sdcc_setup_power,
	.gpio_data	= &sdc1_gpio,
	.clk_reset	= msm_sdcc_clk_reset,
};

static void __init qsd8x50_init_mmc(void)
{
	vreg_mmc = vreg_get(NULL, "gp5");

	if (IS_ERR(vreg_mmc)) {
		pr_err("vreg get for vreg_mmc failed (%ld)\n",
				PTR_ERR(vreg_mmc));
		return;
	}

	qsd8x50_add_sdcc(1, &qsd8x50_sdc1_data, 0, 0);
}

#define SMSM_FAKE_IRQ (0xff)
static uint8_t msm_irq_to_smsm[QSD8X50_NR_IRQS] = {
	[INT_MDDI_EXT] = 1,
	[INT_MDDI_PRI] = 2,
	[INT_MDDI_CLIENT] = 3,
	[INT_USB_OTG] = 4,

	[INT_PWB_I2C] = 5,
	[INT_SDC1_0] = 6,
	[INT_SDC1_1] = 7,
	[INT_SDC2_0] = 8,

	[INT_SDC2_1] = 9,
	[INT_ADSP_A9_A11] = 10,
	[INT_UART1] = 11,
	[INT_UART2] = 12,

	[INT_UART3] = 13,
	[INT_UART1_RX] = 14,
	[INT_UART2_RX] = 15,
	[INT_UART3_RX] = 16,

	[INT_UART1DM_IRQ] = 17,
	[INT_UART1DM_RX] = 18,
	[INT_KEYSENSE] = 19,
	[INT_AD_HSSD] = 20,

	[INT_NAND_WR_ER_DONE] = 21,
	[INT_NAND_OP_DONE] = 22,
	[INT_TCHSCRN1] = 23,
	[INT_TCHSCRN2] = 24,

	[INT_TCHSCRN_SSBI] = 25,
	[INT_USB_HS] = 26,
	[INT_UART2DM_RX] = 27,
	[INT_UART2DM_IRQ] = 28,

	[INT_SDC4_1] = 29,
	[INT_SDC4_0] = 30,
	[INT_SDC3_1] = 31,
	[INT_SDC3_0] = 32,

	/* fake wakeup interrupts */
	[INT_GPIO_GROUP1] = SMSM_FAKE_IRQ,
	[INT_GPIO_GROUP2] = SMSM_FAKE_IRQ,
	[INT_A9_M2A_0] = SMSM_FAKE_IRQ,
	[INT_A9_M2A_1] = SMSM_FAKE_IRQ,
	[INT_A9_M2A_5] = SMSM_FAKE_IRQ,
	[INT_GP_TIMER_EXP] = SMSM_FAKE_IRQ,
	[INT_DEBUG_TIMER_EXP] = SMSM_FAKE_IRQ,
	[INT_ADSP_A11] = SMSM_FAKE_IRQ,
	[INT_SIRC_0] = SMSM_FAKE_IRQ,
	[INT_SIRC_1] = SMSM_FAKE_IRQ,
};

static void __init qsd8x50_init_irq(void)
{
	msm_init_vic(msm_irq_to_smsm, 2);
	msm_init_sirc(FIRST_SIRC_IRQ, NR_SIRC_IRQS_REV_OTHER, INT_SIRC_0);
}

static void __init qsd8x50_init(void)
{
	gpiomux_init(qsd8x50_gpiomux_configs,
		     ARRAY_SIZE(qsd8x50_gpiomux_configs));
	qsd8x50_device_otg.dev.platform_data = &msm_otg_pdata;
	qsd8x50_device_hsusb.dev.parent = &qsd8x50_device_otg.dev;
	qsd8x50_device_hsusb_host.dev.parent = &qsd8x50_device_otg.dev;
	platform_add_devices(devices, ARRAY_SIZE(devices));
	qsd8x50_init_mmc();
}

MACHINE_START(QSD8X50_SURF, "QCT QSD8X50 SURF")
	.atag_offset = 0x100,
	.map_io = msm_map_qsd8x50_io,
	.init_irq = qsd8x50_init_irq,
	.init_machine = qsd8x50_init,
	.init_time	= qsd8x50_timer_init,
MACHINE_END

#ifdef CONFIG_MSM_SOC_REV_A
static void __init qsd8x50_init_irq_rev_a(void)
{
	msm_init_vic(msm_irq_to_smsm, 2);
	msm_init_sirc(FIRST_SIRC_IRQ, NR_SIRC_IRQS_REV_A, INT_SIRC_0);
}

MACHINE_START(QSD8X50A_ST1_5, "QCT QSD8X50A ST1.5")
	.atag_offset = 0x100,
	.nr_irqs = QSD8X50_NR_IRQS,
	.map_io = msm_map_qsd8x50_io_rev_a,
	.init_irq = qsd8x50_init_irq_rev_a,
	.init_machine = qsd8x50_init,
	.init_time	= qsd8x50_timer_init,
MACHINE_END
#endif
