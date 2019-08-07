// SPDX-License-Identifier: GPL-2.0-only
/*
 *  linux/arch/arm/mach-omap1/clock.c
 *
 *  Copyright (C) 2004 - 2005, 2009-2010 Nokia Corporation
 *  Written by Tuukka Tikkanen <tuukka.tikkanen@elektrobit.com>
 *
 *  Modified to use omap shared clock framework by
 *  Tony Lindgren <tony@atomide.com>
 */
#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <linux/soc/ti/omap1-io.h>

#include <asm/mach-types.h>

#include "hardware.h"
#include "soc.h"
#include "iomap.h"
#include "sram.h"
#include "usb.h"

struct omap1_clk;

struct omap1_clk_lookup {
	u16				cpu;
	struct clk_lookup		lk;
};

#define CLK(dev, con, ck, cp)		\
	{				\
		.cpu = cp,		\
		.lk = {			\
			.clk_hw = (&(ck)->clk_hw),	\
			.dev_id = dev,	\
			.con_id = con,	\
		},			\
	}

/* Platform flags for the clkdev-OMAP integration code */
#define CK_310		(1 << 0)
#define CK_7XX		(1 << 1)	/* 7xx, 850 */
#define CK_1510		(1 << 2)
#define CK_16XX		(1 << 3)	/* 16xx, 17xx, 5912 */
#define CK_1710		(1 << 4)	/* 1710 extra for rate selection */

/**
 * struct clk_ops - some clock function pointers
 * @enable: fn ptr that enables the current clock in hardware
 * @disable: fn ptr that enables the current clock in hardware
 *
 * A "companion" clk is an accompanying clock to the one being queried
 * that must be enabled for the IP module connected to the clock to
 * become accessible by the hardware.  Neither @find_idlest nor
 * block-specific; the hwmod code has been created to handle this, but
 * until hwmod data is ready and drivers have been converted to use PM
 * runtime calls in place of clk_enable()/clk_disable(), @find_idlest and
 * @find_companion must, unfortunately, remain.
 */
struct clk_ops {
	int			(*enable)(struct clk_hw *);
	void			(*disable)(struct clk_hw *);
};

/*
 * struct omap1_clk.flags possibilities
 *
 * XXX document the rest of the clock flags here
 *
 */
#define ENABLE_REG_32BIT	(1 << 0)	/* Use 32-bit access */
#define CLOCK_IDLE_CONTROL	(1 << 1)
#define CLOCK_NO_IDLE_PARENT	(1 << 2)

/**
 * struct omap1_clk - OMAP struct clk
 * @node: list_head connecting this clock into the full clock list
 * @ops: struct clkops * for this clock
 * @name: the name of the clock in the hardware (used in hwmod data and debug)
 * @parent: pointer to this clock's parent struct clk
 * @children: list_head connecting to the child clks' @sibling list_heads
 * @sibling: list_head connecting this clk to its parent clk's @children
 * @rate: current clock rate
 * @enable_reg: register to write to enable the clock (see @enable_bit)
 * @recalc: fn ptr that returns the clock's current rate
 * @set_rate: fn ptr that can change the clock's current rate
 * @round_rate: fn ptr that can round the clock's current rate
 * @init: fn ptr to do clock-specific initialization
 * @enable_bit: bitshift to write to enable/disable the clock (see @enable_reg)
 * @usecount: number of users that have requested this clock to be enabled
 * @fixed_div: when > 0, this clock's rate is its parent's rate / @fixed_div
 * @flags: see "struct clk.flags possibilities" above
 * @rate_offset: bitshift for rate selection bitfield (OMAP1 only)
 *
 * XXX @rate_offset, should probably be removed and OMAP1
 * clock code converted to use clksel.
 *
 * XXX @usecount is poorly named.  It should be "enable_count" or
 * something similar.  "users" in the description refers to kernel
 * code (core code or drivers) that have called clk_enable() and not
 * yet called clk_disable(); the usecount of parent clocks is also
 * incremented by the clock code when clk_enable() is called on child
 * clocks and decremented by the clock code when clk_disable() is
 * called on child clocks.
 *
 * XXX @clkdm, @usecount, @children, @sibling should be marked for
 * internal use only.
 *
 * @children and @sibling are used to optimize parent-to-child clock
 * tree traversals.  (child-to-parent traversals use @parent.)
 *
 * XXX The notion of the clock's current rate probably needs to be
 * separated from the clock's target rate.
 */
struct clk {
	struct clk_hw		*clk_hw;
};

struct clk_hw {
	struct clk		clk;
};

struct omap1_clk {
	struct clk_hw		clk_hw;
	struct list_head	node;
	const struct clk_ops	*ops;
	const char		*name;
	struct omap1_clk	*parent;
	struct list_head	children;
	struct list_head	sibling;	/* node for children */
	unsigned long		rate;
	void __iomem		*enable_reg;
	unsigned long		(*recalc)(struct clk_hw *);
	int			(*set_rate)(struct clk_hw *, unsigned long);
	long			(*round_rate)(struct clk_hw *, unsigned long);
	void			(*init)(struct clk_hw *);
	u8			enable_bit;
	s8			usecount;
	u8			fixed_div;
	u8			flags;
	u8			rate_offset;
#if defined(CONFIG_PM_DEBUG) && defined(CONFIG_DEBUG_FS)
	struct dentry		*dent;	/* For visible tree hierarchy */
#endif
};

#define to_omap1_clk(__hw) container_of((__hw), struct omap1_clk, clk_hw)

struct uart_clk {
	struct omap1_clk clk;
	unsigned long	sysc_addr;
};

/* Provide a method for preventing idling some ARM IDLECT clocks */
struct arm_idlect1_clk {
	struct omap1_clk clk;
	unsigned long	no_idle_count;
	__u8		idlect_shift;
};

/* ARM_CKCTL bit shifts */
#define CKCTL_PERDIV_OFFSET	0
#define CKCTL_LCDDIV_OFFSET	2
#define CKCTL_ARMDIV_OFFSET	4
#define CKCTL_DSPDIV_OFFSET	6
#define CKCTL_TCDIV_OFFSET	8
#define CKCTL_DSPMMUDIV_OFFSET	10
/*#define ARM_TIMXO		12*/
#define EN_DSPCK		13
/*#define ARM_INTHCK_SEL	14*/ /* Divide-by-2 for mpu inth_ck */
/* DSP_CKCTL bit shifts */
#define CKCTL_DSPPERDIV_OFFSET	0

/* ARM_IDLECT2 bit shifts */
#define EN_WDTCK	0
#define EN_XORPCK	1
#define EN_PERCK	2
#define EN_LCDCK	3
#define EN_LBCK		4 /* Not on 1610/1710 */
/*#define EN_HSABCK	5*/
#define EN_APICK	6
#define EN_TIMCK	7
#define DMACK_REQ	8
#define EN_GPIOCK	9 /* Not on 1610/1710 */
/*#define EN_LBFREECK	10*/
#define EN_CKOUT_ARM	11

/* ARM_IDLECT3 bit shifts */
#define EN_OCPI_CK	0
#define EN_TC1_CK	2
#define EN_TC2_CK	4

/* DSP_IDLECT2 bit shifts (0,1,2 are same as for ARM_IDLECT2) */
#define EN_DSPTIMCK	5

/* Various register defines for clock controls scattered around OMAP chip */
#define SDW_MCLK_INV_BIT	2	/* In ULPD_CLKC_CTRL */
#define USB_MCLK_EN_BIT		4	/* In ULPD_CLKC_CTRL */
#define USB_HOST_HHC_UHOST_EN	9	/* In MOD_CONF_CTRL_0 */
#define SWD_ULPD_PLL_CLK_REQ	1	/* In SWD_CLK_DIV_CTRL_SEL */
#define COM_ULPD_PLL_CLK_REQ	1	/* In COM_CLK_DIV_CTRL_SEL */
#define SWD_CLK_DIV_CTRL_SEL	0xfffe0874
#define COM_CLK_DIV_CTRL_SEL	0xfffe0878
#define SOFT_REQ_REG		0xfffe0834
#define SOFT_REQ_REG2		0xfffe0880

extern __u32 arm_idlect1_mask;

/* used for passing SoC type to omap1_{select,round_to}_table_rate() */
static u32 cpu_mask;

/* Some ARM_IDLECT1 bit shifts - used in struct arm_idlect1_clk */
#define IDL_CLKOUT_ARM_SHIFT			12
#define IDLTIM_ARM_SHIFT			9
#define IDLAPI_ARM_SHIFT			8
#define IDLIF_ARM_SHIFT				6
#define IDLLB_ARM_SHIFT				4	/* undocumented? */
#define OMAP1510_IDLLCD_ARM_SHIFT		3	/* undocumented? */
#define IDLPER_ARM_SHIFT			2
#define IDLXORP_ARM_SHIFT			1
#define IDLWDT_ARM_SHIFT			0

/* Some MOD_CONF_CTRL_0 bit shifts - used in struct omap1_clk.enable_bit */
#define CONF_MOD_UART3_CLK_MODE_R		31
#define CONF_MOD_UART2_CLK_MODE_R		30
#define CONF_MOD_UART1_CLK_MODE_R		29
#define CONF_MOD_MMC_SD_CLK_REQ_R		23
#define CONF_MOD_MCBSP3_AUXON			20

/* Some MOD_CONF_CTRL_1 bit shifts - used in struct omap1_clk.enable_bit */
#define CONF_MOD_SOSSI_CLK_EN_R			16

/* Some OTG_SYSCON_2-specific bit fields */
#define OTG_SYSCON_2_UHOST_EN_SHIFT		8

/* Some SOFT_REQ_REG bit fields - used in struct omap1_clk.enable_bit */
#define SOFT_MMC2_DPLL_REQ_SHIFT	13
#define SOFT_MMC_DPLL_REQ_SHIFT		12
#define SOFT_UART3_DPLL_REQ_SHIFT	11
#define SOFT_UART2_DPLL_REQ_SHIFT	10
#define SOFT_UART1_DPLL_REQ_SHIFT	9
#define SOFT_USB_OTG_DPLL_REQ_SHIFT	8
#define SOFT_CAM_DPLL_REQ_SHIFT		7
#define SOFT_COM_MCKO_REQ_SHIFT		6
#define SOFT_PERIPH_REQ_SHIFT		5	/* sys_ck gate for UART2 ? */
#define USB_REQ_EN_SHIFT		4
#define SOFT_USB_REQ_SHIFT		3	/* sys_ck gate for USB host? */
#define SOFT_SDW_REQ_SHIFT		2	/* sys_ck gate for Bluetooth? */
#define SOFT_COM_REQ_SHIFT		1	/* sys_ck gate for com proc? */
#define SOFT_DPLL_REQ_SHIFT		0

__u32 arm_idlect1_mask;
static struct clk_hw *api_ck_p, *ck_dpll1_p, *ck_ref_p;

static LIST_HEAD(clocks);
static DEFINE_MUTEX(clocks_mutex);
static DEFINE_SPINLOCK(clockfw_lock);

/*
 * Omap1 specific clock functions
 */

static unsigned long omap1_uart_recalc(struct clk_hw *clk_hw)
{
	struct omap1_clk *clk = to_omap1_clk(clk_hw);
	unsigned int val = __raw_readl(clk->enable_reg);
	return val & clk->enable_bit ? 48000000 : 12000000;
}

static unsigned long omap1_sossi_recalc(struct clk_hw *clk_hw)
{
	struct omap1_clk *clk = to_omap1_clk(clk_hw);
	u32 div = omap_readl(MOD_CONF_CTRL_1);

	div = (div >> 17) & 0x7;
	div++;

	return clk->parent->rate / div;
}

static void omap1_clk_allow_idle(struct clk_hw *clk_hw)
{
	struct omap1_clk *clk = to_omap1_clk(clk_hw);
	struct arm_idlect1_clk * iclk = container_of(clk, struct arm_idlect1_clk, clk);

	if (!(clk->flags & CLOCK_IDLE_CONTROL))
		return;

	if (iclk->no_idle_count > 0 && !(--iclk->no_idle_count))
		arm_idlect1_mask |= 1 << iclk->idlect_shift;
}

static void omap1_clk_deny_idle(struct clk_hw *clk_hw)
{
	struct omap1_clk *clk = to_omap1_clk(clk_hw);
	struct arm_idlect1_clk * iclk = container_of(clk, struct arm_idlect1_clk, clk);

	if (!(clk->flags & CLOCK_IDLE_CONTROL))
		return;

	if (iclk->no_idle_count++ == 0)
		arm_idlect1_mask &= ~(1 << iclk->idlect_shift);
}

static __u16 verify_ckctl_value(__u16 newval)
{
	/* This function checks for following limitations set
	 * by the hardware (all conditions must be true):
	 * DSPMMU_CK == DSP_CK  or  DSPMMU_CK == DSP_CK/2
	 * ARM_CK >= TC_CK
	 * DSP_CK >= TC_CK
	 * DSPMMU_CK >= TC_CK
	 *
	 * In addition following rules are enforced:
	 * LCD_CK <= TC_CK
	 * ARMPER_CK <= TC_CK
	 *
	 * However, maximum frequencies are not checked for!
	 */
	__u8 per_exp;
	__u8 lcd_exp;
	__u8 arm_exp;
	__u8 dsp_exp;
	__u8 tc_exp;
	__u8 dspmmu_exp;

	per_exp = (newval >> CKCTL_PERDIV_OFFSET) & 3;
	lcd_exp = (newval >> CKCTL_LCDDIV_OFFSET) & 3;
	arm_exp = (newval >> CKCTL_ARMDIV_OFFSET) & 3;
	dsp_exp = (newval >> CKCTL_DSPDIV_OFFSET) & 3;
	tc_exp = (newval >> CKCTL_TCDIV_OFFSET) & 3;
	dspmmu_exp = (newval >> CKCTL_DSPMMUDIV_OFFSET) & 3;

	if (dspmmu_exp < dsp_exp)
		dspmmu_exp = dsp_exp;
	if (dspmmu_exp > dsp_exp+1)
		dspmmu_exp = dsp_exp+1;
	if (tc_exp < arm_exp)
		tc_exp = arm_exp;
	if (tc_exp < dspmmu_exp)
		tc_exp = dspmmu_exp;
	if (tc_exp > lcd_exp)
		lcd_exp = tc_exp;
	if (tc_exp > per_exp)
		per_exp = tc_exp;

	newval &= 0xf000;
	newval |= per_exp << CKCTL_PERDIV_OFFSET;
	newval |= lcd_exp << CKCTL_LCDDIV_OFFSET;
	newval |= arm_exp << CKCTL_ARMDIV_OFFSET;
	newval |= dsp_exp << CKCTL_DSPDIV_OFFSET;
	newval |= tc_exp << CKCTL_TCDIV_OFFSET;
	newval |= dspmmu_exp << CKCTL_DSPMMUDIV_OFFSET;

	return newval;
}

static int calc_dsor_exp(struct omap1_clk *clk, unsigned long rate)
{
	/* Note: If target frequency is too low, this function will return 4,
	 * which is invalid value. Caller must check for this value and act
	 * accordingly.
	 *
	 * Note: This function does not check for following limitations set
	 * by the hardware (all conditions must be true):
	 * DSPMMU_CK == DSP_CK  or  DSPMMU_CK == DSP_CK/2
	 * ARM_CK >= TC_CK
	 * DSP_CK >= TC_CK
	 * DSPMMU_CK >= TC_CK
	 */
	unsigned long realrate;
	struct omap1_clk * parent;
	unsigned  dsor_exp;

	parent = clk->parent;
	if (unlikely(parent == NULL))
		return -EIO;

	realrate = parent->rate;
	for (dsor_exp=0; dsor_exp<4; dsor_exp++) {
		if (realrate <= rate)
			break;

		realrate /= 2;
	}

	return dsor_exp;
}

static unsigned long omap1_ckctl_recalc(struct clk_hw *clk_hw)
{
	struct omap1_clk *clk = to_omap1_clk(clk_hw);
	/* Calculate divisor encoded as 2-bit exponent */
	int dsor = 1 << (3 & (omap_readw(ARM_CKCTL) >> clk->rate_offset));

	return clk->parent->rate / dsor;
}

/*-------------------------------------------------------------------------
 * Omap1 MPU rate table
 *-------------------------------------------------------------------------*/
struct mpu_rate {
	unsigned long		rate;
	unsigned long		xtal;
	unsigned long		pll_rate;
	__u16			ckctl_val;
	__u16			dpllctl_val;
	u32			flags;
};

static struct mpu_rate omap1_rate_table[] = {
	/* MPU MHz, xtal MHz, dpll1 MHz, CKCTL, DPLL_CTL
	 * NOTE: Comment order here is different from bits in CKCTL value:
	 * armdiv, dspdiv, dspmmu, tcdiv, perdiv, lcddiv
	 */
	{ 216000000, 12000000, 216000000, 0x050d, 0x2910, /* 1/1/2/2/2/8 */
			CK_1710 },
	{ 195000000, 13000000, 195000000, 0x050e, 0x2790, /* 1/1/2/2/4/8 */
			CK_7XX },
	{ 192000000, 19200000, 192000000, 0x050f, 0x2510, /* 1/1/2/2/8/8 */
			CK_16XX },
	{ 192000000, 12000000, 192000000, 0x050f, 0x2810, /* 1/1/2/2/8/8 */
			CK_16XX },
	{  96000000, 12000000, 192000000, 0x055f, 0x2810, /* 2/2/2/2/8/8 */
			CK_16XX },
	{  48000000, 12000000, 192000000, 0x0baf, 0x2810, /* 4/4/4/8/8/8 */
			CK_16XX },
	{  24000000, 12000000, 192000000, 0x0fff, 0x2810, /* 8/8/8/8/8/8 */
			CK_16XX },
	{ 182000000, 13000000, 182000000, 0x050e, 0x2710, /* 1/1/2/2/4/8 */
			CK_7XX },
	{ 168000000, 12000000, 168000000, 0x010f, 0x2710, /* 1/1/1/2/8/8 */
			CK_16XX|CK_7XX },
	{ 150000000, 12000000, 150000000, 0x010a, 0x2cb0, /* 1/1/1/2/4/4 */
			CK_1510 },
	{ 120000000, 12000000, 120000000, 0x010a, 0x2510, /* 1/1/1/2/4/4 */
			CK_16XX|CK_1510|CK_310|CK_7XX },
	{  96000000, 12000000,  96000000, 0x0005, 0x2410, /* 1/1/1/1/2/2 */
			CK_16XX|CK_1510|CK_310|CK_7XX },
	{  60000000, 12000000,  60000000, 0x0005, 0x2290, /* 1/1/1/1/2/2 */
			CK_16XX|CK_1510|CK_310|CK_7XX },
	{  30000000, 12000000,  60000000, 0x0555, 0x2290, /* 2/2/2/2/2/2 */
			CK_16XX|CK_1510|CK_310|CK_7XX },
	{ 0, 0, 0, 0, 0 },
};

/* MPU virtual clock functions */
static int omap1_select_table_rate(struct clk_hw *clk_hw, unsigned long rate)
{
	/* Find the highest supported frequency <= rate and switch to it */
	struct mpu_rate * ptr;
	unsigned long ref_rate;

	ref_rate = to_omap1_clk(ck_ref_p)->rate;

	for (ptr = omap1_rate_table; ptr->rate; ptr++) {
		if (!(ptr->flags & cpu_mask))
			continue;

		if (ptr->xtal != ref_rate)
			continue;

		/* Can check only after xtal frequency check */
		if (ptr->rate <= rate)
			break;
	}

	if (!ptr->rate)
		return -EINVAL;

	/*
	 * In most cases we should not need to reprogram DPLL.
	 * Reprogramming the DPLL is tricky, it must be done from SRAM.
	 */
	omap_sram_reprogram_clock(ptr->dpllctl_val, ptr->ckctl_val);

	/* XXX Do we need to recalculate the tree below DPLL1 at this point? */
	to_omap1_clk(ck_dpll1_p)->rate = ptr->pll_rate;

	return 0;
}

static int omap1_clk_set_rate_dsp_domain(struct clk_hw *clk_hw, unsigned long rate)
{
	struct omap1_clk *clk = to_omap1_clk(clk_hw);
	int dsor_exp;
	u16 regval;

	dsor_exp = calc_dsor_exp(clk, rate);
	if (dsor_exp > 3)
		dsor_exp = -EINVAL;
	if (dsor_exp < 0)
		return dsor_exp;

	regval = __raw_readw(DSP_CKCTL);
	regval &= ~(3 << clk->rate_offset);
	regval |= dsor_exp << clk->rate_offset;
	__raw_writew(regval, DSP_CKCTL);
	clk->rate = clk->parent->rate / (1 << dsor_exp);

	return 0;
}

static long omap1_clk_round_rate_ckctl_arm(struct clk_hw *clk_hw, unsigned long rate)
{
	struct omap1_clk *clk = to_omap1_clk(clk_hw);
	int dsor_exp = calc_dsor_exp(clk, rate);
	if (dsor_exp < 0)
		return dsor_exp;
	if (dsor_exp > 3)
		dsor_exp = 3;
	return clk->parent->rate / (1 << dsor_exp);
}

static int omap1_clk_set_rate_ckctl_arm(struct clk_hw *clk_hw, unsigned long rate)
{
	struct omap1_clk *clk = to_omap1_clk(clk_hw);
	int dsor_exp;
	u16 regval;

	dsor_exp = calc_dsor_exp(clk, rate);
	if (dsor_exp > 3)
		dsor_exp = -EINVAL;
	if (dsor_exp < 0)
		return dsor_exp;

	regval = omap_readw(ARM_CKCTL);
	regval &= ~(3 << clk->rate_offset);
	regval |= dsor_exp << clk->rate_offset;
	regval = verify_ckctl_value(regval);
	omap_writew(regval, ARM_CKCTL);
	clk->rate = clk->parent->rate / (1 << dsor_exp);
	return 0;
}

static long omap1_round_to_table_rate(struct clk_hw *clk_hw, unsigned long rate)
{
	/* Find the highest supported frequency <= rate */
	struct mpu_rate * ptr;
	long highest_rate;
	unsigned long ref_rate;

	ref_rate = to_omap1_clk(ck_ref_p)->rate;

	highest_rate = -EINVAL;

	for (ptr = omap1_rate_table; ptr->rate; ptr++) {
		if (!(ptr->flags & cpu_mask))
			continue;

		if (ptr->xtal != ref_rate)
			continue;

		highest_rate = ptr->rate;

		/* Can check only after xtal frequency check */
		if (ptr->rate <= rate)
			break;
	}

	return highest_rate;
}

static unsigned calc_ext_dsor(unsigned long rate)
{
	unsigned dsor;

	/* MCLK and BCLK divisor selection is not linear:
	 * freq = 96MHz / dsor
	 *
	 * RATIO_SEL range: dsor <-> RATIO_SEL
	 * 0..6: (RATIO_SEL+2) <-> (dsor-2)
	 * 6..48:  (8+(RATIO_SEL-6)*2) <-> ((dsor-8)/2+6)
	 * Minimum dsor is 2 and maximum is 96. Odd divisors starting from 9
	 * can not be used.
	 */
	for (dsor = 2; dsor < 96; ++dsor) {
		if ((dsor & 1) && dsor > 8)
			continue;
		if (rate >= 96000000 / dsor)
			break;
	}
	return dsor;
}

/* XXX Only needed on 1510 */
static int omap1_set_uart_rate(struct clk_hw *clk_hw, unsigned long rate)
{
	struct omap1_clk *clk = to_omap1_clk(clk_hw);
	unsigned int val;

	val = __raw_readl(clk->enable_reg);
	if (rate == 12000000)
		val &= ~(1 << clk->enable_bit);
	else if (rate == 48000000)
		val |= (1 << clk->enable_bit);
	else
		return -EINVAL;
	__raw_writel(val, clk->enable_reg);
	clk->rate = rate;

	return 0;
}

/* External clock (MCLK & BCLK) functions */
static int omap1_set_ext_clk_rate(struct clk_hw *clk_hw, unsigned long rate)
{
	struct omap1_clk *clk = to_omap1_clk(clk_hw);
	unsigned dsor;
	__u16 ratio_bits;

	dsor = calc_ext_dsor(rate);
	clk->rate = 96000000 / dsor;
	if (dsor > 8)
		ratio_bits = ((dsor - 8) / 2 + 6) << 2;
	else
		ratio_bits = (dsor - 2) << 2;

	ratio_bits |= __raw_readw(clk->enable_reg) & ~0xfd;
	__raw_writew(ratio_bits, clk->enable_reg);

	return 0;
}

static int omap1_set_sossi_rate(struct clk_hw *clk_hw, unsigned long rate)
{
	struct omap1_clk *clk = to_omap1_clk(clk_hw);
	u32 l;
	int div;
	unsigned long p_rate;

	p_rate = clk->parent->rate;
	/* Round towards slower frequency */
	div = (p_rate + rate - 1) / rate;
	div--;
	if (div < 0 || div > 7)
		return -EINVAL;

	l = omap_readl(MOD_CONF_CTRL_1);
	l &= ~(7 << 17);
	l |= div << 17;
	omap_writel(l, MOD_CONF_CTRL_1);

	clk->rate = p_rate / (div + 1);

	return 0;
}

static long omap1_round_ext_clk_rate(struct clk_hw *clk_hw, unsigned long rate)
{
	return 96000000 / calc_ext_dsor(rate);
}

static void omap1_init_ext_clk(struct clk_hw *clk_hw)
{
	struct omap1_clk *clk = to_omap1_clk(clk_hw);
	unsigned dsor;
	__u16 ratio_bits;

	/* Determine current rate and ensure clock is based on 96MHz APLL */
	ratio_bits = __raw_readw(clk->enable_reg) & ~1;
	__raw_writew(ratio_bits, clk->enable_reg);

	ratio_bits = (ratio_bits & 0xfc) >> 2;
	if (ratio_bits > 6)
		dsor = (ratio_bits - 6) * 2 + 8;
	else
		dsor = ratio_bits + 2;

	clk-> rate = 96000000 / dsor;
}

static void omap1_clk_disable(struct clk_hw *clk_hw)
{
	struct omap1_clk *clk = to_omap1_clk(clk_hw);

	if (clk->usecount > 0 && !(--clk->usecount)) {
		clk->ops->disable(&clk->clk_hw);
		if (likely(clk->parent)) {
			omap1_clk_disable(&clk->parent->clk_hw);
			if (clk->flags & CLOCK_NO_IDLE_PARENT)
				omap1_clk_allow_idle(&clk->parent->clk_hw);
		}
	}
}

static omap1_clk_enable(struct clk_hw *clk_hw)
{
	struct omap1_clk *clk = to_omap1_clk(clk_hw);
	int ret = 0;

	if (clk->usecount++ == 0) {
		if (clk->parent) {
			ret = omap1_clk_enable(&clk->parent->clk_hw);
			if (ret)
				goto err;

			if (clk->flags & CLOCK_NO_IDLE_PARENT)
				omap1_clk_deny_idle(&clk->parent->clk_hw);
		}

		ret = clk->ops->enable(&clk->clk_hw);
		if (ret) {
			if (clk->parent)
				omap1_clk_disable(&clk->parent->clk_hw);
			goto err;
		}
	}
	return ret;

err:
	clk->usecount--;
	return ret;
}

static int omap1_clk_enable_generic(struct clk_hw *clk_hw)
{
	struct omap1_clk *clk = to_omap1_clk(clk_hw);
	__u16 regval16;
	__u32 regval32;

	if (unlikely(clk->enable_reg == NULL)) {
		printk(KERN_ERR "clock.c: Enable for %s without enable code\n",
		       clk->name);
		return -EINVAL;
	}

	if (clk->flags & ENABLE_REG_32BIT) {
		regval32 = __raw_readl(clk->enable_reg);
		regval32 |= (1 << clk->enable_bit);
		__raw_writel(regval32, clk->enable_reg);
	} else {
		regval16 = __raw_readw(clk->enable_reg);
		regval16 |= (1 << clk->enable_bit);
		__raw_writew(regval16, clk->enable_reg);
	}

	return 0;
}

static void omap1_clk_disable_generic(struct clk_hw *clk_hw)
{
	struct omap1_clk *clk = to_omap1_clk(clk_hw);
	__u16 regval16;
	__u32 regval32;

	if (clk->enable_reg == NULL)
		return;

	if (clk->flags & ENABLE_REG_32BIT) {
		regval32 = __raw_readl(clk->enable_reg);
		regval32 &= ~(1 << clk->enable_bit);
		__raw_writel(regval32, clk->enable_reg);
	} else {
		regval16 = __raw_readw(clk->enable_reg);
		regval16 &= ~(1 << clk->enable_bit);
		__raw_writew(regval16, clk->enable_reg);
	}
}

static const struct clk_ops clkops_generic = {
	.enable		= omap1_clk_enable_generic,
	.disable	= omap1_clk_disable_generic,
};

static unsigned long omap1_ckctl_recalc_dsp_domain(struct clk_hw *clk_hw)
{
	struct omap1_clk *clk = to_omap1_clk(clk_hw);
	int dsor;

	/* Calculate divisor encoded as 2-bit exponent
	 *
	 * The clock control bits are in DSP domain,
	 * so api_ck is needed for access.
	 * Note that DSP_CKCTL virt addr = phys addr, so
	 * we must use __raw_readw() instead of omap_readw().
	 */
	omap1_clk_enable(api_ck_p);
	dsor = 1 << (3 & (__raw_readw(DSP_CKCTL) >> clk->rate_offset));
	omap1_clk_disable(api_ck_p);

	return clk->parent->rate / dsor;
}

static int omap1_clk_enable_dsp_domain(struct clk_hw *clk_hw)
{
	struct omap1_clk *clk = to_omap1_clk(clk_hw);
	int retval;

	retval = omap1_clk_enable(api_ck_p);
	if (!retval) {
		retval = omap1_clk_enable_generic(&clk->clk_hw);
		omap1_clk_disable(api_ck_p);
	}

	return retval;
}

static void omap1_clk_disable_dsp_domain(struct clk_hw *clk_hw)
{
	if (omap1_clk_enable(api_ck_p) == 0) {
		omap1_clk_disable_generic(clk_hw);
		omap1_clk_disable(api_ck_p);
	}
}

static const struct clk_ops clkops_dspck = {
	.enable		= omap1_clk_enable_dsp_domain,
	.disable	= omap1_clk_disable_dsp_domain,
};

/* XXX SYSC register handling does not belong in the clock framework */
static int omap1_clk_enable_uart_functional_16xx(struct clk_hw *clk_hw)
{
	struct omap1_clk *clk = to_omap1_clk(clk_hw);
	int ret;
	struct uart_clk *uclk;

	ret = omap1_clk_enable_generic(&clk->clk_hw);
	if (ret == 0) {
		/* Set smart idle acknowledgement mode */
		uclk = container_of(clk, struct uart_clk, clk);
		omap_writeb((omap_readb(uclk->sysc_addr) & ~0x10) | 8,
			    uclk->sysc_addr);
	}

	return ret;
}

/* XXX SYSC register handling does not belong in the clock framework */
static void omap1_clk_disable_uart_functional_16xx(struct clk_hw *clk_hw)
{
	struct omap1_clk *clk = to_omap1_clk(clk_hw);
	struct uart_clk *uclk;

	/* Set force idle acknowledgement mode */
	uclk = container_of(clk, struct uart_clk, clk);
	omap_writeb((omap_readb(uclk->sysc_addr) & ~0x18), uclk->sysc_addr);

	omap1_clk_disable_generic(clk_hw);
}

/* XXX SYSC register handling does not belong in the clock framework */
static const struct clk_ops clkops_uart_16xx = {
	.enable		= omap1_clk_enable_uart_functional_16xx,
	.disable	= omap1_clk_disable_uart_functional_16xx,
};

static long omap1_clk_round_rate(struct clk_hw *clk_hw, unsigned long rate)
{
	struct omap1_clk *clk = to_omap1_clk(clk_hw);

	if (clk->round_rate != NULL)
		return clk->round_rate(clk_hw, rate);

	return clk->rate;
}

static int omap1_clk_set_rate(struct clk_hw *clk_hw, unsigned long rate)
{
	struct omap1_clk *clk = to_omap1_clk(clk_hw);
	int  ret = -EINVAL;

	if (clk->set_rate)
		ret = clk->set_rate(clk_hw, rate);
	return ret;
}

/* Propagate rate to children */
static void propagate_rate(struct omap1_clk *tclk)
{
	struct omap1_clk *clkp;

	list_for_each_entry(clkp, &tclk->children, sibling) {
		if (clkp->recalc)
			clkp->rate = clkp->recalc(&clkp->clk_hw);
		propagate_rate(clkp);
	}
}

/*
 * Omap1 clock reset and init functions
 */

int clk_enable(struct clk *clk)
{
	unsigned long flags;
	int ret;

	if (clk == NULL || IS_ERR(clk))
		return -EINVAL;

	spin_lock_irqsave(&clockfw_lock, flags);
	ret = omap1_clk_enable(clk->clk_hw);
	spin_unlock_irqrestore(&clockfw_lock, flags);

	return ret;
}
EXPORT_SYMBOL(clk_enable);

void clk_disable(struct clk *clk)
{
	unsigned long flags;
	struct omap1_clk *_clk = to_omap1_clk(clk->clk_hw);

	if (clk == NULL || IS_ERR(clk))
		return;

	spin_lock_irqsave(&clockfw_lock, flags);
	if (_clk->usecount == 0) {
		pr_err("Trying disable clock %s with 0 usecount\n",
		       _clk->name);
		WARN_ON(1);
		goto out;
	}

	omap1_clk_disable(clk->clk_hw);

out:
	spin_unlock_irqrestore(&clockfw_lock, flags);
}
EXPORT_SYMBOL(clk_disable);

unsigned long clk_get_rate(struct clk *clk)
{
	unsigned long flags;
	unsigned long ret;

	if (clk == NULL || IS_ERR(clk))
		return 0;

	spin_lock_irqsave(&clockfw_lock, flags);
	ret = to_omap1_clk(clk->clk_hw)->rate;
	spin_unlock_irqrestore(&clockfw_lock, flags);

	return ret;
}
EXPORT_SYMBOL(clk_get_rate);

/*
 * Optional clock functions defined in include/linux/clk.h
 */

long clk_round_rate(struct clk *clk, unsigned long rate)
{
	unsigned long flags;
	long ret;

	if (clk == NULL || IS_ERR(clk))
		return 0;

	spin_lock_irqsave(&clockfw_lock, flags);
	ret = omap1_clk_round_rate(clk->clk_hw, rate);
	spin_unlock_irqrestore(&clockfw_lock, flags);

	return ret;
}
EXPORT_SYMBOL(clk_round_rate);

int clk_set_rate(struct clk *clk, unsigned long rate)
{
	unsigned long flags;
	int ret = -EINVAL;

	if (clk == NULL || IS_ERR(clk))
		return ret;

	spin_lock_irqsave(&clockfw_lock, flags);
	ret = omap1_clk_set_rate(clk->clk_hw, rate);
	if (ret == 0)
		propagate_rate(to_omap1_clk(clk->clk_hw));
	spin_unlock_irqrestore(&clockfw_lock, flags);

	return ret;
}
EXPORT_SYMBOL(clk_set_rate);

int clk_set_parent(struct clk *clk, struct clk *parent)
{
	WARN_ONCE(1, "clk_set_parent() not implemented for OMAP1\n");

	return -EINVAL;
}
EXPORT_SYMBOL(clk_set_parent);

struct clk *clk_get_parent(struct clk *clk)
{
	struct omap1_clk *parent = to_omap1_clk(clk->clk_hw)->parent;

	return &parent->clk_hw.clk;
}
EXPORT_SYMBOL(clk_get_parent);

/*
 * OMAP specific clock functions shared between omap1 and omap2
 */

/* Used for clocks that always have same value as the parent clock */
static unsigned long followparent_recalc(struct clk_hw *clk_hw)
{
	struct omap1_clk *clk = to_omap1_clk(clk_hw);

	return clk->parent->rate;
}

/*
 * Used for clocks that have the same value as the parent clock,
 * divided by some factor
 */
static unsigned long omap_fixed_divisor_recalc(struct clk_hw *clk_hw)
{
	struct omap1_clk *clk = to_omap1_clk(clk_hw);

	WARN_ON(!clk->fixed_div);

	return clk->parent->rate / clk->fixed_div;
}

/**
 * clk_preinit - initialize any fields in the struct omap1_clk before clk init
 * @clk: struct omap1_clk * to initialize
 *
 * Initialize any struct omap1_clk fields needed before normal clk initialization
 * can run.  No return value.
 */
static void clk_preinit(struct omap1_clk *clk)
{
	INIT_LIST_HEAD(&clk->children);
}

static int clk_register(struct device *dev, struct clk_hw *clk_hw)
{
	struct omap1_clk *clk = to_omap1_clk(clk_hw);

	if (clk == NULL || IS_ERR(clk))
		return -EINVAL;

	/*
	 * trap out already registered clocks
	 */
	if (clk->node.next || clk->node.prev)
		return 0;

	clk_hw->clk.clk_hw = clk_hw;

	mutex_lock(&clocks_mutex);
	if (clk->parent)
		list_add(&clk->sibling, &clk->parent->children);

	list_add(&clk->node, &clocks);
	if (clk->init)
		clk->init(&clk->clk_hw);
	mutex_unlock(&clocks_mutex);

	return 0;
}

/*
 * Low level helpers
 */
static int clkll_enable_null(struct clk_hw *clk_hw)
{
	return 0;
}

static void clkll_disable_null(struct clk_hw *clk_hw)
{
}

static const struct clk_ops clkops_null = {
	.enable		= clkll_enable_null,
	.disable	= clkll_disable_null,
};

/*
 * Dummy clock
 *
 * Used for clock aliases that are needed on some OMAPs, but not others
 */
static struct omap1_clk dummy_ck = {
	.name	= "dummy",
	.ops	= &clkops_null,
};

/*
 *
 */

#ifdef CONFIG_OMAP_RESET_CLOCKS
/*
 * Disable any unused clocks left on by the bootloader
 */
static void omap1_clk_disable_unused(struct omap1_clk *clk)
{
	__u32 regval32;

	/* Clocks in the DSP domain need api_ck. Just assume bootloader
	 * has not enabled any DSP clocks */
	if (clk->enable_reg == DSP_IDLECT2) {
		pr_info("Skipping reset check for DSP domain clock \"%s\"\n",
			clk->name);
		return;
	}

	/* Is the clock already disabled? */
	if (clk->flags & ENABLE_REG_32BIT)
		regval32 = __raw_readl(clk->enable_reg);
	else
		regval32 = __raw_readw(clk->enable_reg);

	if ((regval32 & (1 << clk->enable_bit)) == 0)
		return;

	printk(KERN_INFO "Disabling unused clock \"%s\"... ", clk->name);
	clk->ops->disable(&clk->clk_hw);
	printk(" done\n");
}

static int __init clk_disable_unused(void)
{
	struct omap1_clk *ck;
	unsigned long flags;

	pr_info("clock: disabling unused clocks to save power\n");

	spin_lock_irqsave(&clockfw_lock, flags);
	list_for_each_entry(ck, &clocks, node) {
		if (ck->ops == &clkops_null)
			continue;

		if (ck->usecount > 0 || !ck->enable_reg)
			continue;

		omap1_clk_disable_unused(ck);
	}
	spin_unlock_irqrestore(&clockfw_lock, flags);

	return 0;
}
late_initcall(clk_disable_unused);
#endif

#if defined(CONFIG_PM_DEBUG) && defined(CONFIG_DEBUG_FS)
/*
 *	debugfs support to trace clock tree hierarchy and attributes
 */

#include <linux/debugfs.h>
#include <linux/seq_file.h>

static struct dentry *clk_debugfs_root;

static int debug_clock_show(struct seq_file *s, void *unused)
{
	struct omap1_clk *c;
	struct omap1_clk *pa;

	mutex_lock(&clocks_mutex);
	seq_printf(s, "%-30s %-30s %-10s %s\n",
		   "clock-name", "parent-name", "rate", "use-count");

	list_for_each_entry(c, &clocks, node) {
		pa = c->parent;
		seq_printf(s, "%-30s %-30s %-10lu %d\n",
			   c->name, pa ? pa->name : "none", c->rate,
			   c->usecount);
	}
	mutex_unlock(&clocks_mutex);

	return 0;
}

DEFINE_SHOW_ATTRIBUTE(debug_clock);

static void clk_debugfs_register_one(struct omap1_clk *c)
{
	struct dentry *d;
	struct omap1_clk *pa = c->parent;

	d = debugfs_create_dir(c->name, pa ? pa->dent : clk_debugfs_root);
	c->dent = d;

	debugfs_create_u8("usecount", S_IRUGO, c->dent, &c->usecount);
	debugfs_create_ulong("rate", S_IRUGO, c->dent, &c->rate);
	debugfs_create_x8("flags", S_IRUGO, c->dent, &c->flags);
}

static void clk_debugfs_register(struct omap1_clk *c)
{
	struct omap1_clk *pa = c->parent;

	if (pa && !pa->dent)
		clk_debugfs_register(pa);

	if (!c->dent)
		clk_debugfs_register_one(c);
}

static int __init clk_debugfs_init(void)
{
	struct omap1_clk *c;
	struct dentry *d;

	d = debugfs_create_dir("clock", NULL);
	clk_debugfs_root = d;

	list_for_each_entry(c, &clocks, node)
		clk_debugfs_register(c);

	debugfs_create_file("summary", S_IRUGO, d, NULL, &debug_clock_fops);

	return 0;
}
late_initcall(clk_debugfs_init);

#endif /* defined(CONFIG_PM_DEBUG) && defined(CONFIG_DEBUG_FS) */

/*
 * Omap1 clocks
 */

static struct omap1_clk ck_ref = {
	.name		= "ck_ref",
	.ops		= &clkops_null,
	.rate		= 12000000,
};

static struct omap1_clk ck_dpll1 = {
	.name		= "ck_dpll1",
	.ops		= &clkops_null,
	.parent		= &ck_ref,
};

/*
 * FIXME: This clock seems to be necessary but no-one has asked for its
 * activation.  [ FIX: SoSSI, SSR ]
 */
static struct arm_idlect1_clk ck_dpll1out = {
	.clk = {
		.name		= "ck_dpll1out",
		.ops		= &clkops_generic,
		.parent		= &ck_dpll1,
		.flags		= CLOCK_IDLE_CONTROL | ENABLE_REG_32BIT,
		.enable_reg	= OMAP1_IO_ADDRESS(ARM_IDLECT2),
		.enable_bit	= EN_CKOUT_ARM,
		.recalc		= &followparent_recalc,
	},
	.idlect_shift	= IDL_CLKOUT_ARM_SHIFT,
};

static struct omap1_clk sossi_ck = {
	.name		= "ck_sossi",
	.ops		= &clkops_generic,
	.parent		= &ck_dpll1out.clk,
	.flags		= CLOCK_NO_IDLE_PARENT | ENABLE_REG_32BIT,
	.enable_reg	= OMAP1_IO_ADDRESS(MOD_CONF_CTRL_1),
	.enable_bit	= CONF_MOD_SOSSI_CLK_EN_R,
	.recalc		= &omap1_sossi_recalc,
	.set_rate	= &omap1_set_sossi_rate,
};

static struct omap1_clk arm_ck = {
	.name		= "arm_ck",
	.ops		= &clkops_null,
	.parent		= &ck_dpll1,
	.rate_offset	= CKCTL_ARMDIV_OFFSET,
	.recalc		= &omap1_ckctl_recalc,
	.round_rate	= omap1_clk_round_rate_ckctl_arm,
	.set_rate	= omap1_clk_set_rate_ckctl_arm,
};

static struct arm_idlect1_clk armper_ck = {
	.clk = {
		.name		= "armper_ck",
		.ops		= &clkops_generic,
		.parent		= &ck_dpll1,
		.flags		= CLOCK_IDLE_CONTROL,
		.enable_reg	= OMAP1_IO_ADDRESS(ARM_IDLECT2),
		.enable_bit	= EN_PERCK,
		.rate_offset	= CKCTL_PERDIV_OFFSET,
		.recalc		= &omap1_ckctl_recalc,
		.round_rate	= omap1_clk_round_rate_ckctl_arm,
		.set_rate	= omap1_clk_set_rate_ckctl_arm,
	},
	.idlect_shift	= IDLPER_ARM_SHIFT,
};

/*
 * FIXME: This clock seems to be necessary but no-one has asked for its
 * activation.  [ GPIO code for 1510 ]
 */
static struct omap1_clk arm_gpio_ck = {
	.name		= "ick",
	.ops		= &clkops_generic,
	.parent		= &ck_dpll1,
	.enable_reg	= OMAP1_IO_ADDRESS(ARM_IDLECT2),
	.enable_bit	= EN_GPIOCK,
	.recalc		= &followparent_recalc,
};

static struct arm_idlect1_clk armxor_ck = {
	.clk = {
		.name		= "armxor_ck",
		.ops		= &clkops_generic,
		.parent		= &ck_ref,
		.flags		= CLOCK_IDLE_CONTROL,
		.enable_reg	= OMAP1_IO_ADDRESS(ARM_IDLECT2),
		.enable_bit	= EN_XORPCK,
		.recalc		= &followparent_recalc,
	},
	.idlect_shift	= IDLXORP_ARM_SHIFT,
};

static struct arm_idlect1_clk armtim_ck = {
	.clk = {
		.name		= "armtim_ck",
		.ops		= &clkops_generic,
		.parent		= &ck_ref,
		.flags		= CLOCK_IDLE_CONTROL,
		.enable_reg	= OMAP1_IO_ADDRESS(ARM_IDLECT2),
		.enable_bit	= EN_TIMCK,
		.recalc		= &followparent_recalc,
	},
	.idlect_shift	= IDLTIM_ARM_SHIFT,
};

static struct arm_idlect1_clk armwdt_ck = {
	.clk = {
		.name		= "armwdt_ck",
		.ops		= &clkops_generic,
		.parent		= &ck_ref,
		.flags		= CLOCK_IDLE_CONTROL,
		.enable_reg	= OMAP1_IO_ADDRESS(ARM_IDLECT2),
		.enable_bit	= EN_WDTCK,
		.fixed_div	= 14,
		.recalc		= &omap_fixed_divisor_recalc,
	},
	.idlect_shift	= IDLWDT_ARM_SHIFT,
};

static struct omap1_clk arminth_ck16xx = {
	.name		= "arminth_ck",
	.ops		= &clkops_null,
	.parent		= &arm_ck,
	.recalc		= &followparent_recalc,
	/* Note: On 16xx the frequency can be divided by 2 by programming
	 * ARM_CKCTL:ARM_INTHCK_SEL(14) to 1
	 *
	 * 1510 version is in TC clocks.
	 */
};

static struct omap1_clk dsp_ck = {
	.name		= "dsp_ck",
	.ops		= &clkops_generic,
	.parent		= &ck_dpll1,
	.enable_reg	= OMAP1_IO_ADDRESS(ARM_CKCTL),
	.enable_bit	= EN_DSPCK,
	.rate_offset	= CKCTL_DSPDIV_OFFSET,
	.recalc		= &omap1_ckctl_recalc,
	.round_rate	= omap1_clk_round_rate_ckctl_arm,
	.set_rate	= omap1_clk_set_rate_ckctl_arm,
};

static struct omap1_clk dspmmu_ck = {
	.name		= "dspmmu_ck",
	.ops		= &clkops_null,
	.parent		= &ck_dpll1,
	.rate_offset	= CKCTL_DSPMMUDIV_OFFSET,
	.recalc		= &omap1_ckctl_recalc,
	.round_rate	= omap1_clk_round_rate_ckctl_arm,
	.set_rate	= omap1_clk_set_rate_ckctl_arm,
};

static struct omap1_clk dspper_ck = {
	.name		= "dspper_ck",
	.ops		= &clkops_dspck,
	.parent		= &ck_dpll1,
	.enable_reg	= DSP_IDLECT2,
	.enable_bit	= EN_PERCK,
	.rate_offset	= CKCTL_PERDIV_OFFSET,
	.recalc		= &omap1_ckctl_recalc_dsp_domain,
	.round_rate	= omap1_clk_round_rate_ckctl_arm,
	.set_rate	= &omap1_clk_set_rate_dsp_domain,
};

static struct omap1_clk dspxor_ck = {
	.name		= "dspxor_ck",
	.ops		= &clkops_dspck,
	.parent		= &ck_ref,
	.enable_reg	= DSP_IDLECT2,
	.enable_bit	= EN_XORPCK,
	.recalc		= &followparent_recalc,
};

static struct omap1_clk dsptim_ck = {
	.name		= "dsptim_ck",
	.ops		= &clkops_dspck,
	.parent		= &ck_ref,
	.enable_reg	= DSP_IDLECT2,
	.enable_bit	= EN_DSPTIMCK,
	.recalc		= &followparent_recalc,
};

static struct arm_idlect1_clk tc_ck = {
	.clk = {
		.name		= "tc_ck",
		.ops		= &clkops_null,
		.parent		= &ck_dpll1,
		.flags		= CLOCK_IDLE_CONTROL,
		.rate_offset	= CKCTL_TCDIV_OFFSET,
		.recalc		= &omap1_ckctl_recalc,
		.round_rate	= omap1_clk_round_rate_ckctl_arm,
		.set_rate	= omap1_clk_set_rate_ckctl_arm,
	},
	.idlect_shift	= IDLIF_ARM_SHIFT,
};

static struct omap1_clk arminth_ck1510 = {
	.name		= "arminth_ck",
	.ops		= &clkops_null,
	.parent		= &tc_ck.clk,
	.recalc		= &followparent_recalc,
	/* Note: On 1510 the frequency follows TC_CK
	 *
	 * 16xx version is in MPU clocks.
	 */
};

static struct omap1_clk tipb_ck = {
	/* No-idle controlled by "tc_ck" */
	.name		= "tipb_ck",
	.ops		= &clkops_null,
	.parent		= &tc_ck.clk,
	.recalc		= &followparent_recalc,
};

static struct omap1_clk l3_ocpi_ck = {
	/* No-idle controlled by "tc_ck" */
	.name		= "l3_ocpi_ck",
	.ops		= &clkops_generic,
	.parent		= &tc_ck.clk,
	.enable_reg	= OMAP1_IO_ADDRESS(ARM_IDLECT3),
	.enable_bit	= EN_OCPI_CK,
	.recalc		= &followparent_recalc,
};

static struct omap1_clk tc1_ck = {
	.name		= "tc1_ck",
	.ops		= &clkops_generic,
	.parent		= &tc_ck.clk,
	.enable_reg	= OMAP1_IO_ADDRESS(ARM_IDLECT3),
	.enable_bit	= EN_TC1_CK,
	.recalc		= &followparent_recalc,
};

/*
 * FIXME: This clock seems to be necessary but no-one has asked for its
 * activation.  [ pm.c (SRAM), CCP, Camera ]
 */
static struct omap1_clk tc2_ck = {
	.name		= "tc2_ck",
	.ops		= &clkops_generic,
	.parent		= &tc_ck.clk,
	.enable_reg	= OMAP1_IO_ADDRESS(ARM_IDLECT3),
	.enable_bit	= EN_TC2_CK,
	.recalc		= &followparent_recalc,
};

static struct omap1_clk dma_ck = {
	/* No-idle controlled by "tc_ck" */
	.name		= "dma_ck",
	.ops		= &clkops_null,
	.parent		= &tc_ck.clk,
	.recalc		= &followparent_recalc,
};

static struct omap1_clk dma_lcdfree_ck = {
	.name		= "dma_lcdfree_ck",
	.ops		= &clkops_null,
	.parent		= &tc_ck.clk,
	.recalc		= &followparent_recalc,
};

static struct arm_idlect1_clk api_ck = {
	.clk = {
		.name		= "api_ck",
		.ops		= &clkops_generic,
		.parent		= &tc_ck.clk,
		.flags		= CLOCK_IDLE_CONTROL,
		.enable_reg	= OMAP1_IO_ADDRESS(ARM_IDLECT2),
		.enable_bit	= EN_APICK,
		.recalc		= &followparent_recalc,
	},
	.idlect_shift	= IDLAPI_ARM_SHIFT,
};

static struct arm_idlect1_clk lb_ck = {
	.clk = {
		.name		= "lb_ck",
		.ops		= &clkops_generic,
		.parent		= &tc_ck.clk,
		.flags		= CLOCK_IDLE_CONTROL,
		.enable_reg	= OMAP1_IO_ADDRESS(ARM_IDLECT2),
		.enable_bit	= EN_LBCK,
		.recalc		= &followparent_recalc,
	},
	.idlect_shift	= IDLLB_ARM_SHIFT,
};

static struct omap1_clk rhea1_ck = {
	.name		= "rhea1_ck",
	.ops		= &clkops_null,
	.parent		= &tc_ck.clk,
	.recalc		= &followparent_recalc,
};

static struct omap1_clk rhea2_ck = {
	.name		= "rhea2_ck",
	.ops		= &clkops_null,
	.parent		= &tc_ck.clk,
	.recalc		= &followparent_recalc,
};

static struct omap1_clk lcd_ck_16xx = {
	.name		= "lcd_ck",
	.ops		= &clkops_generic,
	.parent		= &ck_dpll1,
	.enable_reg	= OMAP1_IO_ADDRESS(ARM_IDLECT2),
	.enable_bit	= EN_LCDCK,
	.rate_offset	= CKCTL_LCDDIV_OFFSET,
	.recalc		= &omap1_ckctl_recalc,
	.round_rate	= omap1_clk_round_rate_ckctl_arm,
	.set_rate	= omap1_clk_set_rate_ckctl_arm,
};

static struct arm_idlect1_clk lcd_ck_1510 = {
	.clk = {
		.name		= "lcd_ck",
		.ops		= &clkops_generic,
		.parent		= &ck_dpll1,
		.flags		= CLOCK_IDLE_CONTROL,
		.enable_reg	= OMAP1_IO_ADDRESS(ARM_IDLECT2),
		.enable_bit	= EN_LCDCK,
		.rate_offset	= CKCTL_LCDDIV_OFFSET,
		.recalc		= &omap1_ckctl_recalc,
		.round_rate	= omap1_clk_round_rate_ckctl_arm,
		.set_rate	= omap1_clk_set_rate_ckctl_arm,
	},
	.idlect_shift	= OMAP1510_IDLLCD_ARM_SHIFT,
};

/*
 * XXX The enable_bit here is misused - it simply switches between 12MHz
 * and 48MHz.  Reimplement with clksel.
 *
 * XXX does this need SYSC register handling?
 */
static struct omap1_clk uart1_1510 = {
	.name		= "uart1_ck",
	.ops		= &clkops_null,
	/* Direct from ULPD, no real parent */
	.parent		= &armper_ck.clk,
	.rate		= 12000000,
	.flags		= ENABLE_REG_32BIT | CLOCK_NO_IDLE_PARENT,
	.enable_reg	= OMAP1_IO_ADDRESS(MOD_CONF_CTRL_0),
	.enable_bit	= CONF_MOD_UART1_CLK_MODE_R,
	.set_rate	= &omap1_set_uart_rate,
	.recalc		= &omap1_uart_recalc,
};

/*
 * XXX The enable_bit here is misused - it simply switches between 12MHz
 * and 48MHz.  Reimplement with clksel.
 *
 * XXX SYSC register handling does not belong in the clock framework
 */
static struct uart_clk uart1_16xx = {
	.clk	= {
		.name		= "uart1_ck",
		.ops		= &clkops_uart_16xx,
		/* Direct from ULPD, no real parent */
		.parent		= &armper_ck.clk,
		.rate		= 48000000,
		.flags		= ENABLE_REG_32BIT | CLOCK_NO_IDLE_PARENT,
		.enable_reg	= OMAP1_IO_ADDRESS(MOD_CONF_CTRL_0),
		.enable_bit	= CONF_MOD_UART1_CLK_MODE_R,
	},
	.sysc_addr	= 0xfffb0054,
};

/*
 * XXX The enable_bit here is misused - it simply switches between 12MHz
 * and 48MHz.  Reimplement with clksel.
 *
 * XXX does this need SYSC register handling?
 */
static struct omap1_clk uart2_ck = {
	.name		= "uart2_ck",
	.ops		= &clkops_null,
	/* Direct from ULPD, no real parent */
	.parent		= &armper_ck.clk,
	.rate		= 12000000,
	.flags		= ENABLE_REG_32BIT | CLOCK_NO_IDLE_PARENT,
	.enable_reg	= OMAP1_IO_ADDRESS(MOD_CONF_CTRL_0),
	.enable_bit	= CONF_MOD_UART2_CLK_MODE_R,
	.set_rate	= &omap1_set_uart_rate,
	.recalc		= &omap1_uart_recalc,
};

/*
 * XXX The enable_bit here is misused - it simply switches between 12MHz
 * and 48MHz.  Reimplement with clksel.
 *
 * XXX does this need SYSC register handling?
 */
static struct omap1_clk uart3_1510 = {
	.name		= "uart3_ck",
	.ops		= &clkops_null,
	/* Direct from ULPD, no real parent */
	.parent		= &armper_ck.clk,
	.rate		= 12000000,
	.flags		= ENABLE_REG_32BIT | CLOCK_NO_IDLE_PARENT,
	.enable_reg	= OMAP1_IO_ADDRESS(MOD_CONF_CTRL_0),
	.enable_bit	= CONF_MOD_UART3_CLK_MODE_R,
	.set_rate	= &omap1_set_uart_rate,
	.recalc		= &omap1_uart_recalc,
};

/*
 * XXX The enable_bit here is misused - it simply switches between 12MHz
 * and 48MHz.  Reimplement with clksel.
 *
 * XXX SYSC register handling does not belong in the clock framework
 */
static struct uart_clk uart3_16xx = {
	.clk	= {
		.name		= "uart3_ck",
		.ops		= &clkops_uart_16xx,
		/* Direct from ULPD, no real parent */
		.parent		= &armper_ck.clk,
		.rate		= 48000000,
		.flags		= ENABLE_REG_32BIT | CLOCK_NO_IDLE_PARENT,
		.enable_reg	= OMAP1_IO_ADDRESS(MOD_CONF_CTRL_0),
		.enable_bit	= CONF_MOD_UART3_CLK_MODE_R,
	},
	.sysc_addr	= 0xfffb9854,
};

static struct omap1_clk usb_clko = {	/* 6 MHz output on W4_USB_CLKO */
	.name		= "usb_clko",
	.ops		= &clkops_generic,
	/* Direct from ULPD, no parent */
	.rate		= 6000000,
	.flags		= ENABLE_REG_32BIT,
	.enable_reg	= OMAP1_IO_ADDRESS(ULPD_CLOCK_CTRL),
	.enable_bit	= USB_MCLK_EN_BIT,
};

static struct omap1_clk usb_hhc_ck1510 = {
	.name		= "usb_hhc_ck",
	.ops		= &clkops_generic,
	/* Direct from ULPD, no parent */
	.rate		= 48000000, /* Actually 2 clocks, 12MHz and 48MHz */
	.flags		= ENABLE_REG_32BIT,
	.enable_reg	= OMAP1_IO_ADDRESS(MOD_CONF_CTRL_0),
	.enable_bit	= USB_HOST_HHC_UHOST_EN,
};

static struct omap1_clk usb_hhc_ck16xx = {
	.name		= "usb_hhc_ck",
	.ops		= &clkops_generic,
	/* Direct from ULPD, no parent */
	.rate		= 48000000,
	/* OTG_SYSCON_2.OTG_PADEN == 0 (not 1510-compatible) */
	.flags		= ENABLE_REG_32BIT,
	.enable_reg	= OMAP1_IO_ADDRESS(OTG_BASE + 0x08), /* OTG_SYSCON_2 */
	.enable_bit	= OTG_SYSCON_2_UHOST_EN_SHIFT
};

static struct omap1_clk usb_dc_ck = {
	.name		= "usb_dc_ck",
	.ops		= &clkops_generic,
	/* Direct from ULPD, no parent */
	.rate		= 48000000,
	.enable_reg	= OMAP1_IO_ADDRESS(SOFT_REQ_REG),
	.enable_bit	= SOFT_USB_OTG_DPLL_REQ_SHIFT,
};

static struct omap1_clk uart1_7xx = {
	.name		= "uart1_ck",
	.ops		= &clkops_generic,
	/* Direct from ULPD, no parent */
	.rate		= 12000000,
	.enable_reg	= OMAP1_IO_ADDRESS(SOFT_REQ_REG),
	.enable_bit	= 9,
};

static struct omap1_clk uart2_7xx = {
	.name		= "uart2_ck",
	.ops		= &clkops_generic,
	/* Direct from ULPD, no parent */
	.rate		= 12000000,
	.enable_reg	= OMAP1_IO_ADDRESS(SOFT_REQ_REG),
	.enable_bit	= 11,
};

static struct omap1_clk mclk_1510 = {
	.name		= "mclk",
	.ops		= &clkops_generic,
	/* Direct from ULPD, no parent. May be enabled by ext hardware. */
	.rate		= 12000000,
	.enable_reg	= OMAP1_IO_ADDRESS(SOFT_REQ_REG),
	.enable_bit	= SOFT_COM_MCKO_REQ_SHIFT,
};

static struct omap1_clk mclk_16xx = {
	.name		= "mclk",
	.ops		= &clkops_generic,
	/* Direct from ULPD, no parent. May be enabled by ext hardware. */
	.enable_reg	= OMAP1_IO_ADDRESS(COM_CLK_DIV_CTRL_SEL),
	.enable_bit	= COM_ULPD_PLL_CLK_REQ,
	.set_rate	= &omap1_set_ext_clk_rate,
	.round_rate	= &omap1_round_ext_clk_rate,
	.init		= &omap1_init_ext_clk,
};

static struct omap1_clk bclk_1510 = {
	.name		= "bclk",
	.ops		= &clkops_generic,
	/* Direct from ULPD, no parent. May be enabled by ext hardware. */
	.rate		= 12000000,
};

static struct omap1_clk bclk_16xx = {
	.name		= "bclk",
	.ops		= &clkops_generic,
	/* Direct from ULPD, no parent. May be enabled by ext hardware. */
	.enable_reg	= OMAP1_IO_ADDRESS(SWD_CLK_DIV_CTRL_SEL),
	.enable_bit	= SWD_ULPD_PLL_CLK_REQ,
	.set_rate	= &omap1_set_ext_clk_rate,
	.round_rate	= &omap1_round_ext_clk_rate,
	.init		= &omap1_init_ext_clk,
};

static struct omap1_clk mmc1_ck = {
	.name		= "mmc1_ck",
	.ops		= &clkops_generic,
	/* Functional clock is direct from ULPD, interface clock is ARMPER */
	.parent		= &armper_ck.clk,
	.rate		= 48000000,
	.flags		= ENABLE_REG_32BIT | CLOCK_NO_IDLE_PARENT,
	.enable_reg	= OMAP1_IO_ADDRESS(MOD_CONF_CTRL_0),
	.enable_bit	= CONF_MOD_MMC_SD_CLK_REQ_R,
};

/*
 * XXX MOD_CONF_CTRL_0 bit 20 is defined in the 1510 TRM as
 * CONF_MOD_MCBSP3_AUXON ??
 */
static struct omap1_clk mmc2_ck = {
	.name		= "mmc2_ck",
	.ops		= &clkops_generic,
	/* Functional clock is direct from ULPD, interface clock is ARMPER */
	.parent		= &armper_ck.clk,
	.rate		= 48000000,
	.flags		= ENABLE_REG_32BIT | CLOCK_NO_IDLE_PARENT,
	.enable_reg	= OMAP1_IO_ADDRESS(MOD_CONF_CTRL_0),
	.enable_bit	= 20,
};

static struct omap1_clk mmc3_ck = {
	.name		= "mmc3_ck",
	.ops		= &clkops_generic,
	/* Functional clock is direct from ULPD, interface clock is ARMPER */
	.parent		= &armper_ck.clk,
	.rate		= 48000000,
	.flags		= ENABLE_REG_32BIT | CLOCK_NO_IDLE_PARENT,
	.enable_reg	= OMAP1_IO_ADDRESS(SOFT_REQ_REG),
	.enable_bit	= SOFT_MMC_DPLL_REQ_SHIFT,
};

static struct omap1_clk virtual_ck_mpu = {
	.name		= "mpu",
	.ops		= &clkops_null,
	.parent		= &arm_ck, /* Is smarter alias for */
	.recalc		= &followparent_recalc,
	.set_rate	= &omap1_select_table_rate,
	.round_rate	= &omap1_round_to_table_rate,
};

/* virtual functional clock domain for I2C. Just for making sure that ARMXOR_CK
remains active during MPU idle whenever this is enabled */
static struct omap1_clk i2c_fck = {
	.name		= "i2c_fck",
	.ops		= &clkops_null,
	.flags		= CLOCK_NO_IDLE_PARENT,
	.parent		= &armxor_ck.clk,
	.recalc		= &followparent_recalc,
};

static struct omap1_clk i2c_ick = {
	.name		= "i2c_ick",
	.ops		= &clkops_null,
	.flags		= CLOCK_NO_IDLE_PARENT,
	.parent		= &armper_ck.clk,
	.recalc		= &followparent_recalc,
};

/*
 * clkdev integration
 */

static struct omap1_clk_lookup omap_clks[] = {
	/* non-ULPD clocks */
	CLK(NULL,	"ck_ref",	&ck_ref,	CK_16XX | CK_1510 | CK_310 | CK_7XX),
	CLK(NULL,	"ck_dpll1",	&ck_dpll1,	CK_16XX | CK_1510 | CK_310 | CK_7XX),
	/* CK_GEN1 clocks */
	CLK(NULL,	"ck_dpll1out",	&ck_dpll1out.clk, CK_16XX),
	CLK(NULL,	"ck_sossi",	&sossi_ck,	CK_16XX),
	CLK(NULL,	"arm_ck",	&arm_ck,	CK_16XX | CK_1510 | CK_310),
	CLK(NULL,	"armper_ck",	&armper_ck.clk,	CK_16XX | CK_1510 | CK_310),
	CLK("omap_gpio.0", "ick",	&arm_gpio_ck,	CK_1510 | CK_310),
	CLK(NULL,	"armxor_ck",	&armxor_ck.clk,	CK_16XX | CK_1510 | CK_310 | CK_7XX),
	CLK(NULL,	"armtim_ck",	&armtim_ck.clk,	CK_16XX | CK_1510 | CK_310),
	CLK("omap_wdt",	"fck",		&armwdt_ck.clk,	CK_16XX | CK_1510 | CK_310),
	CLK("omap_wdt",	"ick",		&armper_ck.clk,	CK_16XX),
	CLK("omap_wdt", "ick",		&dummy_ck,	CK_1510 | CK_310),
	CLK(NULL,	"arminth_ck",	&arminth_ck1510, CK_1510 | CK_310),
	CLK(NULL,	"arminth_ck",	&arminth_ck16xx, CK_16XX),
	/* CK_GEN2 clocks */
	CLK(NULL,	"dsp_ck",	&dsp_ck,	CK_16XX | CK_1510 | CK_310),
	CLK(NULL,	"dspmmu_ck",	&dspmmu_ck,	CK_16XX | CK_1510 | CK_310),
	CLK(NULL,	"dspper_ck",	&dspper_ck,	CK_16XX | CK_1510 | CK_310),
	CLK(NULL,	"dspxor_ck",	&dspxor_ck,	CK_16XX | CK_1510 | CK_310),
	CLK(NULL,	"dsptim_ck",	&dsptim_ck,	CK_16XX | CK_1510 | CK_310),
	/* CK_GEN3 clocks */
	CLK(NULL,	"tc_ck",	&tc_ck.clk,	CK_16XX | CK_1510 | CK_310 | CK_7XX),
	CLK(NULL,	"tipb_ck",	&tipb_ck,	CK_1510 | CK_310),
	CLK(NULL,	"l3_ocpi_ck",	&l3_ocpi_ck,	CK_16XX | CK_7XX),
	CLK(NULL,	"tc1_ck",	&tc1_ck,	CK_16XX),
	CLK(NULL,	"tc2_ck",	&tc2_ck,	CK_16XX),
	CLK(NULL,	"dma_ck",	&dma_ck,	CK_16XX | CK_1510 | CK_310),
	CLK(NULL,	"dma_lcdfree_ck", &dma_lcdfree_ck, CK_16XX),
	CLK(NULL,	"api_ck",	&api_ck.clk,	CK_16XX | CK_1510 | CK_310 | CK_7XX),
	CLK(NULL,	"lb_ck",	&lb_ck.clk,	CK_1510 | CK_310),
	CLK(NULL,	"rhea1_ck",	&rhea1_ck,	CK_16XX),
	CLK(NULL,	"rhea2_ck",	&rhea2_ck,	CK_16XX),
	CLK(NULL,	"lcd_ck",	&lcd_ck_16xx,	CK_16XX | CK_7XX),
	CLK(NULL,	"lcd_ck",	&lcd_ck_1510.clk, CK_1510 | CK_310),
	/* ULPD clocks */
	CLK(NULL,	"uart1_ck",	&uart1_1510,	CK_1510 | CK_310),
	CLK(NULL,	"uart1_ck",	&uart1_16xx.clk, CK_16XX),
	CLK(NULL,	"uart1_ck",	&uart1_7xx,	CK_7XX),
	CLK(NULL,	"uart2_ck",	&uart2_ck,	CK_16XX | CK_1510 | CK_310),
	CLK(NULL,	"uart2_ck",	&uart2_7xx,	CK_7XX),
	CLK(NULL,	"uart3_ck",	&uart3_1510,	CK_1510 | CK_310),
	CLK(NULL,	"uart3_ck",	&uart3_16xx.clk, CK_16XX),
	CLK(NULL,	"usb_clko",	&usb_clko,	CK_16XX | CK_1510 | CK_310),
	CLK(NULL,	"usb_hhc_ck",	&usb_hhc_ck1510, CK_1510 | CK_310),
	CLK(NULL,	"usb_hhc_ck",	&usb_hhc_ck16xx, CK_16XX),
	CLK(NULL,	"usb_dc_ck",	&usb_dc_ck,	CK_16XX | CK_7XX),
	CLK(NULL,	"mclk",		&mclk_1510,	CK_1510 | CK_310),
	CLK(NULL,	"mclk",		&mclk_16xx,	CK_16XX),
	CLK(NULL,	"bclk",		&bclk_1510,	CK_1510 | CK_310),
	CLK(NULL,	"bclk",		&bclk_16xx,	CK_16XX),
	CLK("mmci-omap.0", "fck",	&mmc1_ck,	CK_16XX | CK_1510 | CK_310),
	CLK("mmci-omap.0", "fck",	&mmc3_ck,	CK_7XX),
	CLK("mmci-omap.0", "ick",	&armper_ck.clk,	CK_16XX | CK_1510 | CK_310 | CK_7XX),
	CLK("mmci-omap.1", "fck",	&mmc2_ck,	CK_16XX),
	CLK("mmci-omap.1", "ick",	&armper_ck.clk,	CK_16XX),
	/* Virtual clocks */
	CLK(NULL,	"mpu",		&virtual_ck_mpu, CK_16XX | CK_1510 | CK_310),
	CLK("omap_i2c.1", "fck",	&i2c_fck,	CK_16XX | CK_1510 | CK_310 | CK_7XX),
	CLK("omap_i2c.1", "ick",	&i2c_ick,	CK_16XX),
	CLK("omap_i2c.1", "ick",	&dummy_ck,	CK_1510 | CK_310 | CK_7XX),
	CLK("omap1_spi100k.1", "fck",	&dummy_ck,	CK_7XX),
	CLK("omap1_spi100k.1", "ick",	&dummy_ck,	CK_7XX),
	CLK("omap1_spi100k.2", "fck",	&dummy_ck,	CK_7XX),
	CLK("omap1_spi100k.2", "ick",	&dummy_ck,	CK_7XX),
	CLK("omap_uwire", "fck",	&armxor_ck.clk,	CK_16XX | CK_1510 | CK_310),
	CLK("omap-mcbsp.1", "ick",	&dspper_ck,	CK_16XX),
	CLK("omap-mcbsp.1", "ick",	&dummy_ck,	CK_1510 | CK_310),
	CLK("omap-mcbsp.2", "ick",	&armper_ck.clk,	CK_16XX),
	CLK("omap-mcbsp.2", "ick",	&dummy_ck,	CK_1510 | CK_310),
	CLK("omap-mcbsp.3", "ick",	&dspper_ck,	CK_16XX),
	CLK("omap-mcbsp.3", "ick",	&dummy_ck,	CK_1510 | CK_310),
	CLK("omap-mcbsp.1", "fck",	&dspxor_ck,	CK_16XX | CK_1510 | CK_310),
	CLK("omap-mcbsp.2", "fck",	&armper_ck.clk,	CK_16XX | CK_1510 | CK_310),
	CLK("omap-mcbsp.3", "fck",	&dspxor_ck,	CK_16XX | CK_1510 | CK_310),
};

/*
 * init
 */

static void __init omap1_show_rates(void)
{
	pr_notice("Clocking rate (xtal/DPLL1/MPU): %ld.%01ld/%ld.%01ld/%ld.%01ld MHz\n",
		  ck_ref.rate / 1000000, (ck_ref.rate / 100000) % 10,
		  ck_dpll1.rate / 1000000, (ck_dpll1.rate / 100000) % 10,
		  arm_ck.rate / 1000000, (arm_ck.rate / 100000) % 10);
}

int __init omap1_clk_init(void)
{
	struct omap1_clk_lookup *c;
	int crystal_type = 0; /* Default 12 MHz */
	u32 reg;

#ifdef CONFIG_DEBUG_LL
	/*
	 * Resets some clocks that may be left on from bootloader,
	 * but leaves serial clocks on.
	 */
	omap_writel(0x3 << 29, MOD_CONF_CTRL_0);
#endif

	/* USB_REQ_EN will be disabled later if necessary (usb_dc_ck) */
	reg = omap_readw(SOFT_REQ_REG) & (1 << 4);
	omap_writew(reg, SOFT_REQ_REG);
	if (!cpu_is_omap15xx())
		omap_writew(0, SOFT_REQ_REG2);

	/* By default all idlect1 clocks are allowed to idle */
	arm_idlect1_mask = ~0;

	for (c = omap_clks; c < omap_clks + ARRAY_SIZE(omap_clks); c++)
		clk_preinit(to_omap1_clk(c->lk.clk_hw));

	cpu_mask = 0;
	if (cpu_is_omap1710())
		cpu_mask |= CK_1710;
	if (cpu_is_omap16xx())
		cpu_mask |= CK_16XX;
	if (cpu_is_omap1510())
		cpu_mask |= CK_1510;
	if (cpu_is_omap7xx())
		cpu_mask |= CK_7XX;
	if (cpu_is_omap310())
		cpu_mask |= CK_310;

	for (c = omap_clks; c < omap_clks + ARRAY_SIZE(omap_clks); c++)
		if (c->cpu & cpu_mask) {
			clkdev_add(&c->lk);
			clk_register(NULL, c->lk.clk_hw);
		}

	/* Pointers to these clocks are needed by code in clock.c */
	api_ck_p = &api_ck.clk.clk_hw;
	ck_dpll1_p = &ck_dpll1.clk_hw;
	ck_ref_p = &ck_ref.clk_hw;

	if (cpu_is_omap7xx())
		ck_ref.rate = 13000000;
	if (cpu_is_omap16xx() && crystal_type == 2)
		ck_ref.rate = 19200000;

	pr_info("Clocks: ARM_SYSST: 0x%04x DPLL_CTL: 0x%04x ARM_CKCTL: 0x%04x\n",
		omap_readw(ARM_SYSST), omap_readw(DPLL_CTL),
		omap_readw(ARM_CKCTL));

	/* We want to be in syncronous scalable mode */
	omap_writew(0x1000, ARM_SYSST);


	/*
	 * Initially use the values set by bootloader. Determine PLL rate and
	 * recalculate dependent clocks as if kernel had changed PLL or
	 * divisors. See also omap1_clk_late_init() that can reprogram dpll1
	 * after the SRAM is initialized.
	 */
	{
		unsigned pll_ctl_val = omap_readw(DPLL_CTL);

		ck_dpll1.rate = ck_ref.rate; /* Base xtal rate */
		if (pll_ctl_val & 0x10) {
			/* PLL enabled, apply multiplier and divisor */
			if (pll_ctl_val & 0xf80)
				ck_dpll1.rate *= (pll_ctl_val & 0xf80) >> 7;
			ck_dpll1.rate /= ((pll_ctl_val & 0x60) >> 5) + 1;
		} else {
			/* PLL disabled, apply bypass divisor */
			switch (pll_ctl_val & 0xc) {
			case 0:
				break;
			case 0x4:
				ck_dpll1.rate /= 2;
				break;
			default:
				ck_dpll1.rate /= 4;
				break;
			}
		}
	}
	propagate_rate(&ck_dpll1);
	/* Cache rates for clocks connected to ck_ref (not dpll1) */
	propagate_rate(&ck_ref);
	omap1_show_rates();
	if (machine_is_omap_perseus2() || machine_is_omap_fsample()) {
		/* Select slicer output as OMAP input clock */
		omap_writew(omap_readw(OMAP7XX_PCC_UPLD_CTRL) & ~0x1,
				OMAP7XX_PCC_UPLD_CTRL);
	}

	/* Amstrad Delta wants BCLK high when inactive */
	if (machine_is_ams_delta())
		omap_writel(omap_readl(ULPD_CLOCK_CTRL) |
				(1 << SDW_MCLK_INV_BIT),
				ULPD_CLOCK_CTRL);

	/* Turn off DSP and ARM_TIMXO. Make sure ARM_INTHCK is not divided */
	/* (on 730, bit 13 must not be cleared) */
	if (cpu_is_omap7xx())
		omap_writew(omap_readw(ARM_CKCTL) & 0x2fff, ARM_CKCTL);
	else
		omap_writew(omap_readw(ARM_CKCTL) & 0x0fff, ARM_CKCTL);

	/* Put DSP/MPUI into reset until needed */
	omap_writew(0, ARM_RSTCT1);
	omap_writew(1, ARM_RSTCT2);
	omap_writew(0x400, ARM_IDLECT1);

	/*
	 * According to OMAP5910 Erratum SYS_DMA_1, bit DMACK_REQ (bit 8)
	 * of the ARM_IDLECT2 register must be set to zero. The power-on
	 * default value of this bit is one.
	 */
	omap_writew(0x0000, ARM_IDLECT2);	/* Turn LCD clock off also */

	/*
	 * Only enable those clocks we will need, let the drivers
	 * enable other clocks as necessary
	 */
	omap1_clk_enable(&armper_ck.clk.clk_hw);
	omap1_clk_enable(&armxor_ck.clk.clk_hw);
	omap1_clk_enable(&armtim_ck.clk.clk_hw); /* This should be done by timer code */

	if (cpu_is_omap15xx())
		omap1_clk_enable(&arm_gpio_ck.clk_hw);

	return 0;
}

#define OMAP1_DPLL1_SANE_VALUE	60000000

void __init omap1_clk_late_init(void)
{
	unsigned long rate = ck_dpll1.rate;

	/* Find the highest supported frequency and enable it */
	if (omap1_select_table_rate(&virtual_ck_mpu.clk_hw, ~0)) {
		pr_err("System frequencies not set, using default. Check your config.\n");
		/*
		 * Reprogramming the DPLL is tricky, it must be done from SRAM.
		 */
		omap_sram_reprogram_clock(0x2290, 0x0005);
		ck_dpll1.rate = OMAP1_DPLL1_SANE_VALUE;
	}
	propagate_rate(&ck_dpll1);
	omap1_show_rates();
	loops_per_jiffy = cpufreq_scale(loops_per_jiffy, rate, ck_dpll1.rate);
}
