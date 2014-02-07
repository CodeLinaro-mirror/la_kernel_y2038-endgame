/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
 * Author: Thomas Abraham <thomas.ab@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file contains the utility functions to register the cpu clocks
 * for samsung platforms.
*/

#include <linux/errno.h>
#include "clk.h"

#define SRC_CPU			0x0
#define STAT_CPU		0x200
#define DIV_CPU0		0x300
#define DIV_CPU1		0x304
#define DIV_STAT_CPU0		0x400
#define DIV_STAT_CPU1		0x404

#define MAX_DIV			8

#define EXYNOS4210_ARM_DIV1(base) ((readl(base + DIV_CPU0) & 0xf) + 1)
#define EXYNOS4210_ARM_DIV2(base) (((readl(base + DIV_CPU0) >> 28) & 0xf) + 1)

#define EXYNOS4210_DIV_CPU0(d5, d4, d3, d2, d1, d0)			\
		((d5 << 24) | (d4 << 20) | (d3 << 16) | (d2 << 12) |	\
		 (d1 << 8) | (d0 <<  4))
#define EXYNOS4210_DIV_CPU1(d2, d1, d0)					\
		((d2 << 8) | (d1 << 4) | (d0 << 0))

#define EXYNOS4210_DIV1_HPM_MASK	((0x7 << 0) | (0x7 << 4))
#define EXYNOS4210_MUX_HPM_MASK		(1 << 20)

/**
 * struct exynos4210_armclk_data: config data to setup exynos4210 cpu clocks.
 * @prate:	frequency of the parent clock.
 * @div0:	value to be programmed in the div_cpu0 register.
 * @div1:	value to be programmed in the div_cpu1 register.
 *
 * This structure holds the divider configuration data for divider clocks
 * belonging to the CMU_CPU clock domain. The parent frequency at which these
 * divider values are vaild is specified in @prate.
 */
struct exynos4210_armclk_data {
	unsigned long	prate;
	unsigned int	div0;
	unsigned int	div1;
};

/**
 * struct samsung_cpuclk: information about clock supplied to a CPU core.
 * @hw:		handle between ccf and cpu clock.
 * @alt_parent:	alternate parent clock to use when switching the speed
 *		of the primary parent clock.
 * @ctrl_base:	base address of the clock controller.
 * @offset:	offset from the ctrl_base address where the cpu clock div/mux
 *		registers can be accessed.
 * @clk_nb:	clock notifier registered for changes in clock speed of the
 *		primary parent clock.
 * @data:	optional data which the acutal instantiation of this clock
 *		can use.
 */
struct samsung_cpuclk {
	struct clk_hw		hw;
	struct clk		*alt_parent;
	void __iomem		*ctrl_base;
	unsigned long		offset;
	struct notifier_block	clk_nb;
	void			*data;
};

#define to_samsung_cpuclk_hw(hw) container_of(hw, struct samsung_cpuclk, hw)
#define to_samsung_cpuclk_nb(nb) container_of(nb, struct samsung_cpuclk, clk_nb)

/**
 * struct samsung_cpuclk_soc_data: soc specific data for cpu clocks.
 * @parser:	pointer to a function that can parse SoC specific data.
 * @ops:	clock operations to be used for this clock.
 * @offset:	optional offset from base of clock controller register base, to
 *		be used when accessing clock controller registers related to the
 *		cpu clock.
 * @clk_cb:	the clock notifier callback to be called for changes in the
 *		clock rate of the primary parent clock.
 *
 * This structure provides SoC specific data for ARM clocks. Based on
 * the compatible value of the clock controller node, the value of the
 * fields in this structure can be populated.
 */
struct samsung_cpuclk_soc_data {
	int (*parser)(struct device_node *, void **);
	const struct clk_ops *ops;
	unsigned int offset;
	int (*clk_cb)(struct notifier_block *nb, unsigned long evt, void *data);
};

/* common round rate callback useable for all types of cpu clocks */
static long samsung_cpuclk_round_rate(struct clk_hw *hw,
			unsigned long drate, unsigned long *prate)
{
	struct clk *parent = __clk_get_parent(hw->clk);
	unsigned long max_prate = __clk_round_rate(parent, UINT_MAX);
	unsigned long t_prate, best_div = 1;
	unsigned long delta, min_delta = UINT_MAX;

	do {
		t_prate = __clk_round_rate(parent, drate * best_div);
		delta = drate - (t_prate / best_div);
		if (delta < min_delta) {
			*prate = t_prate;
			min_delta = delta;
		}
		if (!delta)
			break;
		best_div++;
	} while ((drate * best_div) < max_prate && best_div <= MAX_DIV);

	return t_prate / best_div;
}

static unsigned long _calc_div(unsigned long prate, unsigned long drate)
{
	unsigned long div = prate / drate;

	WARN_ON(div >= MAX_DIV);
	return (!(prate % drate)) ? div-- : div;
}

/* helper function to register a cpu clock */
static int __init samsung_cpuclk_register(unsigned int lookup_id,
		const char *name, const char **parents,
		unsigned int num_parents, void __iomem *base,
		const struct samsung_cpuclk_soc_data *soc_data,
		struct device_node *np, const struct clk_ops *ops)
{
	struct samsung_cpuclk *cpuclk;
	struct clk_init_data init;
	struct clk *clk;
	int ret;

	cpuclk = kzalloc(sizeof(*cpuclk), GFP_KERNEL);
	if (!cpuclk) {
		pr_err("%s: could not allocate memory for %s clock\n",
					__func__, name);
		return -ENOMEM;
	}

	init.name = name;
	init.flags = CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT;
	init.parent_names = parents;
	init.num_parents = 1;
	init.ops = ops;

	cpuclk->hw.init = &init;
	cpuclk->ctrl_base = base;

	if (soc_data && soc_data->parser) {
		ret = soc_data->parser(np, &cpuclk->data);
		if (ret) {
			pr_err("%s: error %d in parsing %s clock data",
					__func__, ret, name);
			ret = -EINVAL;
			goto free_cpuclk;
		}
		cpuclk->offset = soc_data->offset;
		init.ops = soc_data->ops;
	}

	if (soc_data && soc_data->clk_cb) {
		cpuclk->clk_nb.notifier_call = soc_data->clk_cb;
		if (clk_notifier_register(__clk_lookup(parents[0]),
				&cpuclk->clk_nb)) {
			pr_err("%s: failed to register clock notifier for %s\n",
					__func__, name);
			goto free_cpuclk_data;
		}
	}

	if (num_parents == 2) {
		cpuclk->alt_parent = __clk_lookup(parents[1]);
		if (!cpuclk->alt_parent) {
			pr_err("%s: could not lookup alternate parent %s\n",
					__func__, parents[1]);
			ret = -EINVAL;
			goto free_cpuclk_data;
		}
	}

	clk = clk_register(NULL, &cpuclk->hw);
	if (IS_ERR(clk)) {
		pr_err("%s: could not register cpuclk %s\n", __func__,	name);
		ret = PTR_ERR(clk);
		goto free_cpuclk_data;
	}

	samsung_clk_add_lookup(clk, lookup_id);
	return 0;

free_cpuclk_data:
	kfree(cpuclk->data);
free_cpuclk:
	kfree(cpuclk);
	return ret;
}

static inline void _exynos4210_set_armclk_div(void __iomem *base,
			unsigned long div)
{
	writel((readl(base + DIV_CPU0) & ~0xf) | div, base + DIV_CPU0);
	while (readl(base + DIV_STAT_CPU0) != 0)
		;
}

static unsigned long exynos4210_armclk_recalc_rate(struct clk_hw *hw,
				unsigned long parent_rate)
{
	struct samsung_cpuclk *armclk = to_samsung_cpuclk_hw(hw);
	void __iomem *base = armclk->ctrl_base + armclk->offset;

	return parent_rate / EXYNOS4210_ARM_DIV1(base) /
				EXYNOS4210_ARM_DIV2(base);
}

/*
 * This clock notifier is called when the frequency of the parent clock
 * of armclk is to be changed. This notifier handles the setting up all
 * the divider clocks, remux to temporary parent and handling the safe
 * frequency levels when using temporary parent.
 */
static int exynos4210_armclk_notifier_cb(struct notifier_block *nb,
				unsigned long event, void *data)
{
	struct clk_notifier_data *ndata = data;
	struct samsung_cpuclk *armclk = to_samsung_cpuclk_nb(nb);
	struct exynos4210_armclk_data *armclk_data;
	unsigned long alt_prate, alt_div, div0, div1, mux_reg;
	void __iomem *base;
	bool need_safe_freq;

	armclk_data = armclk->data;
	base = armclk->ctrl_base + armclk->offset;
	alt_prate = clk_get_rate(armclk->alt_parent);

	if (event == POST_RATE_CHANGE)
		goto e4210_armclk_post_rate_change;

	/* pre-rate change. find out the divider values to use for clock data */
	while (armclk_data->prate != ndata->new_rate) {
		if (armclk_data->prate == 0)
			return NOTIFY_BAD;
		armclk_data++;
	}

	div0 = armclk_data->div0;
	div1 = armclk_data->div1;
	if (readl(base + SRC_CPU) & EXYNOS4210_MUX_HPM_MASK) {
		div1 = readl(base + DIV_CPU1) & EXYNOS4210_DIV1_HPM_MASK;
		div1 |= ((armclk_data->div1) & ~EXYNOS4210_DIV1_HPM_MASK);
	}

	/*
	 * if the new and old parent clock speed is less than the clock speed
	 * of the alternate parent, then it should be ensured that at no point
	 * the armclk speed is more than the old_prate until the dividers are
	 * set.
	 */
	need_safe_freq = ndata->old_rate < alt_prate &&
				ndata->new_rate < alt_prate;
	if (need_safe_freq) {
		alt_div = _calc_div(alt_prate, ndata->old_rate);
		_exynos4210_set_armclk_div(base, alt_div);
		div0 |= alt_div;
	}

	mux_reg = readl(base + SRC_CPU);
	writel(mux_reg | (1 << 16), base + SRC_CPU);
	while (((readl(base + STAT_CPU) >> 16) & 0x7) != 2)
		;

	writel(div0, base + DIV_CPU0);
	while (readl(base + DIV_STAT_CPU0) != 0)
		;
	writel(div1, base + DIV_CPU1);
	while (readl(base + DIV_STAT_CPU1) != 0)
		;
	return NOTIFY_OK;

e4210_armclk_post_rate_change:
	/* post-rate change event, re-mux back to primary parent */
	mux_reg = readl(base + SRC_CPU);
	writel(mux_reg & ~(1 << 16), base + SRC_CPU);
	while (((readl(base + STAT_CPU) >> 16) & 0x7) != 1)
			;

	return NOTIFY_OK;
}

static int exynos4210_armclk_set_rate(struct clk_hw *hw, unsigned long drate,
					unsigned long prate)
{
	struct samsung_cpuclk *armclk = to_samsung_cpuclk_hw(hw);
	void __iomem *base = armclk->ctrl_base + armclk->offset;
	unsigned long div;

	div = drate < prate ? _calc_div(prate, drate) : 0;
	_exynos4210_set_armclk_div(base, div);
	return 0;
}

static const struct clk_ops exynos4210_armclk_clk_ops = {
	.recalc_rate = exynos4210_armclk_recalc_rate,
	.round_rate = samsung_cpuclk_round_rate,
	.set_rate = exynos4210_armclk_set_rate,
};

/*
 * parse divider configuration data from dt for all the cpu clock domain
 * clocks in exynos4210 and compatible SoC's.
 */
static int __init exynos4210_armclk_parser(struct device_node *np, void **data)
{
	struct exynos4210_armclk_data *tdata;
	unsigned long cfg[10], row, col;
	const struct property *prop;
	const __be32 *val;
	u32 cells;
	int ret;

	if (of_property_read_u32(np, "samsung,armclk-cells", &cells))
		return -EINVAL;
	prop = of_find_property(np, "samsung,armclk-divider-table", NULL);
	if (!prop)
		return -EINVAL;
	if (!prop->value)
		return -EINVAL;
	if ((prop->length / sizeof(u32)) % cells)
		return -EINVAL;
	row = ((prop->length / sizeof(u32)) / cells) + 1;

	*data = kzalloc(sizeof(*tdata) * row, GFP_KERNEL);
	if (!*data)
		ret = -ENOMEM;
	tdata = *data;

	val = prop->value;
	for (; row > 1; row--, tdata++) {
		for (col = 0; col < cells; col++)
			cfg[col] = be32_to_cpup(val++);

		tdata->prate = cfg[0] * 1000;
		tdata->div0 = EXYNOS4210_DIV_CPU0(cfg[6], cfg[5], cfg[4],
						cfg[3], cfg[2], cfg[1]);
		tdata->div1 = cells == 10 ?
				EXYNOS4210_DIV_CPU1(cfg[9], cfg[8], cfg[7]) :
				EXYNOS4210_DIV_CPU1(0, cfg[8], cfg[7]);
	}
	tdata->prate = 0;
	return 0;
}

static const struct samsung_cpuclk_soc_data exynos4210_cpuclk_soc_data = {
	.parser = exynos4210_armclk_parser,
	.ops = &exynos4210_armclk_clk_ops,
	.offset = 0x14200,
	.clk_cb = exynos4210_armclk_notifier_cb,
};

static const struct samsung_cpuclk_soc_data exynos5250_cpuclk_soc_data = {
	.parser = exynos4210_armclk_parser,
	.ops = &exynos4210_armclk_clk_ops,
	.offset = 0x200,
	.clk_cb = exynos4210_armclk_notifier_cb,
};

static const struct of_device_id samsung_clock_ids_armclk[] = {
	{ .compatible = "samsung,exynos4210-clock",
			.data = &exynos4210_cpuclk_soc_data, },
	{ .compatible = "samsung,exynos4412-clock",
			.data = &exynos4210_cpuclk_soc_data, },
	{ .compatible = "samsung,exynos5250-clock",
			.data = &exynos5250_cpuclk_soc_data, },
	{ },
};

/**
 * samsung_register_arm_clock: register arm clock with ccf.
 * @lookup_id: armclk clock output id for the clock controller.
 * @parent: name of the parent clock for armclk.
 * @base: base address of the clock controller from which armclk is generated.
 * @np: device tree node pointer of the clock controller (optional).
 * @ops: clock ops for this clock (optional)
 */
int __init samsung_register_arm_clock(unsigned int lookup_id,
		const char **parent_names, unsigned int num_parents,
		void __iomem *base, struct device_node *np, struct clk_ops *ops)
{
	const struct of_device_id *match;
	const struct samsung_cpuclk_soc_data *data = NULL;

	if (np) {
		match = of_match_node(samsung_clock_ids_armclk, np);
		data = match ? match->data : NULL;
	}

	return samsung_cpuclk_register(lookup_id, "armclk", parent_names,
			num_parents, base, data, np, ops);
}
