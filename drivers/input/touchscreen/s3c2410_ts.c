/*
 * Samsung S3C24XX touchscreen driver and ADC core
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the term of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Copyright 2004 Arnaud Patard <arnaud.patard@rtp-net.org>
 * Copyright 2008 Ben Dooks <ben-linux@fluff.org>
 * Copyright 2008-2009 Simtec Electronics <linux@simtec.co.uk>
 *
 * Additional work by Herbert Pötzl <herbert@13thfloor.at> and
 * Harald Welte <laforge@openmoko.org>
 */

#include <linux/err.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/platform_data/touchscreen-s3c2410.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/regulator/consumer.h>
#include <linux/sched.h>
#include <linux/slab.h>

#define S3C2410_ADCREG(x) (x)

#define S3C2410_ADCCON	   S3C2410_ADCREG(0x00)
#define S3C2410_ADCTSC	   S3C2410_ADCREG(0x04)
#define S3C2410_ADCDLY	   S3C2410_ADCREG(0x08)
#define S3C2410_ADCDAT0	   S3C2410_ADCREG(0x0C)
#define S3C2410_ADCDAT1	   S3C2410_ADCREG(0x10)
#define S3C64XX_ADCUPDN		S3C2410_ADCREG(0x14)
#define S3C2443_ADCMUX		S3C2410_ADCREG(0x18)
#define S3C64XX_ADCCLRINT	S3C2410_ADCREG(0x18)
#define S5P_ADCMUX		S3C2410_ADCREG(0x1C)
#define S3C64XX_ADCCLRINTPNDNUP	S3C2410_ADCREG(0x20)

/* ADCCON Register Bits */
#define S3C64XX_ADCCON_RESSEL		(1<<16)
#define S3C2410_ADCCON_ECFLG		(1<<15)
#define S3C2410_ADCCON_PRSCEN		(1<<14)
#define S3C2410_ADCCON_PRSCVL(x)	(((x)&0xFF)<<6)
#define S3C2410_ADCCON_PRSCVLMASK	(0xFF<<6)
#define S3C2410_ADCCON_SELMUX(x)	(((x)&0x7)<<3)
#define S3C2410_ADCCON_MUXMASK		(0x7<<3)
#define S3C2416_ADCCON_RESSEL		(1 << 3)
#define S3C2410_ADCCON_STDBM		(1<<2)
#define S3C2410_ADCCON_READ_START	(1<<1)
#define S3C2410_ADCCON_ENABLE_START	(1<<0)
#define S3C2410_ADCCON_STARTMASK	(0x3<<0)


/* ADCTSC Register Bits */
#define S3C2443_ADCTSC_UD_SEN		(1 << 8)
#define S3C2410_ADCTSC_YM_SEN		(1<<7)
#define S3C2410_ADCTSC_YP_SEN		(1<<6)
#define S3C2410_ADCTSC_XM_SEN		(1<<5)
#define S3C2410_ADCTSC_XP_SEN		(1<<4)
#define S3C2410_ADCTSC_PULL_UP_DISABLE	(1<<3)
#define S3C2410_ADCTSC_AUTO_PST		(1<<2)
#define S3C2410_ADCTSC_XY_PST(x)	(((x)&0x3)<<0)

/* ADCDAT0 Bits */
#define S3C2410_ADCDAT0_UPDOWN		(1<<15)
#define S3C2410_ADCDAT0_AUTO_PST	(1<<14)
#define S3C2410_ADCDAT0_XY_PST		(0x3<<12)
#define S3C2410_ADCDAT0_XPDATA_MASK	(0x03FF)

/* ADCDAT1 Bits */
#define S3C2410_ADCDAT1_UPDOWN		(1<<15)
#define S3C2410_ADCDAT1_AUTO_PST	(1<<14)
#define S3C2410_ADCDAT1_XY_PST		(0x3<<12)
#define S3C2410_ADCDAT1_YPDATA_MASK	(0x03FF)

#define TSC_SLEEP  (S3C2410_ADCTSC_PULL_UP_DISABLE | S3C2410_ADCTSC_XY_PST(0))

#define INT_DOWN	(0)
#define INT_UP		(1 << 8)

#define WAIT4INT	(S3C2410_ADCTSC_YM_SEN | \
			 S3C2410_ADCTSC_YP_SEN | \
			 S3C2410_ADCTSC_XP_SEN | \
			 S3C2410_ADCTSC_XY_PST(3))

#define AUTOPST		(S3C2410_ADCTSC_YM_SEN | \
			 S3C2410_ADCTSC_YP_SEN | \
			 S3C2410_ADCTSC_XP_SEN | \
			 S3C2410_ADCTSC_AUTO_PST | \
			 S3C2410_ADCTSC_XY_PST(0))

#define FEAT_PEN_IRQ	(1 << 0)	/* HAS ADCCLRINTPNDNUP */

/* Per-touchscreen data. */

/**
 * struct s3c2410ts - driver touchscreen state.
 * @client: The ADC client we registered with the core driver.
 * @dev: The device we are bound to.
 * @input: The input device we registered with the input subsystem.
 * @clock: The clock for the adc.
 * @io: Pointer to the IO base.
 * @xp: The accumulated X position data.
 * @yp: The accumulated Y position data.
 * @irq_tc: The interrupt number for pen up/down interrupt
 * @count: The number of samples collected.
 * @shift: The log2 of the maximum count to read in one go.
 * @features: The features supported by the TSADC MOdule.
 */
struct s3c2410ts {
	struct s3c_adc_client *client;
	struct device *dev;
	struct input_dev *input;
	struct clk *clock;
	void __iomem *io;
	unsigned long xp;
	unsigned long yp;
	int irq_tc;
	int count;
	int shift;
	int features;
};

static struct s3c2410ts ts;
static void touch_timer_fire(unsigned long data);
static DEFINE_TIMER(touch_timer, touch_timer_fire, 0, 0);

/* This driver is designed to control the usage of the ADC block between
 * the touchscreen and any other drivers that may need to use it, such as
 * the hwmon driver.
 *
 * Priority will be given to the touchscreen driver, but as this itself is
 * rate limited it should not starve other requests which are processed in
 * order that they are received.
 *
 * Each user registers to get a client block which uniquely identifies it
 * and stores information such as the necessary functions to callback when
 * action is required.
 */

enum s3c_cpu_type {
	TYPE_ADCV1, /* S3C24XX */
	TYPE_ADCV11, /* S3C2443 */
	TYPE_ADCV12, /* S3C2416, S3C2450 */
	TYPE_ADCV2, /* S3C64XX */
	TYPE_ADCV3, /* S5PV210, S5PC110, EXYNOS4210 */
};

struct s3c_adc_client {
	struct platform_device	*pdev;
	struct list_head	 pend;
	wait_queue_head_t	*wait;

	unsigned int		 nr_samples;
	int			 result;
	unsigned char		 is_ts;
	unsigned char		 channel;
};

struct adc_device {
	struct platform_device	*pdev;
	struct platform_device	*owner;
	struct clk		*clk;
	struct s3c_adc_client	*cur;
	struct s3c_adc_client	*ts_pend;
	void __iomem		*regs;
	spinlock_t		 lock;

	unsigned int		 prescale;

	int			 irq;
	struct regulator	*vdd;
};

static struct adc_device *adc_dev;

static LIST_HEAD(adc_pending);	/* protected by adc_device.lock */

#define adc_dbg(_adc, msg...) dev_dbg(&(_adc)->pdev->dev, msg)

static inline void s3c_adc_convert(struct adc_device *adc)
{
	unsigned con = readl(adc->regs + S3C2410_ADCCON);

	con |= S3C2410_ADCCON_ENABLE_START;
	writel(con, adc->regs + S3C2410_ADCCON);
}

static inline void s3c_adc_select(struct adc_device *adc,
				  struct s3c_adc_client *client)
{
	unsigned con = readl(adc->regs + S3C2410_ADCCON);
	enum s3c_cpu_type cpu = platform_get_device_id(adc->pdev)->driver_data;

	if (client->is_ts)
		writel(S3C2410_ADCTSC_PULL_UP_DISABLE | AUTOPST,
		       ts.io + S3C2410_ADCTSC);

	if (cpu == TYPE_ADCV1 || cpu == TYPE_ADCV2)
		con &= ~S3C2410_ADCCON_MUXMASK;
	con &= ~S3C2410_ADCCON_STDBM;
	con &= ~S3C2410_ADCCON_STARTMASK;

	if (!client->is_ts) {
		if (cpu == TYPE_ADCV3)
			writel(client->channel & 0xf, adc->regs + S5P_ADCMUX);
		else if (cpu == TYPE_ADCV11 || cpu == TYPE_ADCV12)
			writel(client->channel & 0xf,
						adc->regs + S3C2443_ADCMUX);
		else
			con |= S3C2410_ADCCON_SELMUX(client->channel);
	}

	writel(con, adc->regs + S3C2410_ADCCON);
}

static void s3c_adc_dbgshow(struct adc_device *adc)
{
	adc_dbg(adc, "CON=%08x, TSC=%08x, DLY=%08x\n",
		readl(adc->regs + S3C2410_ADCCON),
		readl(adc->regs + S3C2410_ADCTSC),
		readl(adc->regs + S3C2410_ADCDLY));
}

static void s3c_adc_try(struct adc_device *adc)
{
	struct s3c_adc_client *next = adc->ts_pend;

	if (!next && !list_empty(&adc_pending)) {
		next = list_first_entry(&adc_pending,
					struct s3c_adc_client, pend);
		list_del(&next->pend);
	} else
		adc->ts_pend = NULL;

	if (next) {
		adc_dbg(adc, "new client is %p\n", next);
		adc->cur = next;
		s3c_adc_select(adc, next);
		s3c_adc_convert(adc);
		s3c_adc_dbgshow(adc);
	}
}

int s3c_adc_start(struct s3c_adc_client *client,
		  unsigned int channel, unsigned int nr_samples)
{
	struct adc_device *adc = adc_dev;
	unsigned long flags;

	if (!adc) {
		printk(KERN_ERR "%s: failed to find adc\n", __func__);
		return -EINVAL;
	}

	spin_lock_irqsave(&adc->lock, flags);

	if (client->is_ts && adc->ts_pend) {
		spin_unlock_irqrestore(&adc->lock, flags);
		return -EAGAIN;
	}

	client->channel = channel;
	client->nr_samples = nr_samples;

	if (client->is_ts)
		adc->ts_pend = client;
	else
		list_add_tail(&client->pend, &adc_pending);

	if (!adc->cur)
		s3c_adc_try(adc);

	spin_unlock_irqrestore(&adc->lock, flags);

	return 0;
}

int s3c_adc_read(struct s3c_adc_client *client, unsigned int ch)
{
	DECLARE_WAIT_QUEUE_HEAD_ONSTACK(wake);
	int ret;

	client->wait = &wake;
	client->result = -1;

	ret = s3c_adc_start(client, ch, 1);
	if (ret < 0)
		goto err;

	ret = wait_event_timeout(wake, client->result >= 0, HZ / 2);
	if (client->result < 0) {
		ret = -ETIMEDOUT;
		goto err;
	}

	return client->result;

err:
	return ret;
}
EXPORT_SYMBOL_GPL(s3c_adc_read);

struct s3c_adc_client *s3c_adc_register(struct platform_device *pdev,
					void (*select)(struct s3c_adc_client *client,
						       unsigned int selected),
					void (*conv)(struct s3c_adc_client *client,
						     unsigned d0, unsigned d1,
						     unsigned *samples_left),
					unsigned int is_ts)
{
	struct s3c_adc_client *client;

	WARN_ON(!pdev);

	if (!pdev)
		return ERR_PTR(-EINVAL);

	client = kzalloc(sizeof(struct s3c_adc_client), GFP_KERNEL);
	if (!client) {
		dev_err(&pdev->dev, "no memory for adc client\n");
		return ERR_PTR(-ENOMEM);
	}

	client->pdev = pdev;
	client->is_ts = is_ts;

	return client;
}
EXPORT_SYMBOL_GPL(s3c_adc_register);

void s3c_adc_release(struct s3c_adc_client *client)
{
	unsigned long flags;

	spin_lock_irqsave(&adc_dev->lock, flags);

	/* We should really check that nothing is in progress. */
	if (adc_dev->cur == client)
		adc_dev->cur = NULL;
	if (adc_dev->ts_pend == client)
		adc_dev->ts_pend = NULL;
	else {
		struct list_head *p, *n;
		struct s3c_adc_client *tmp;

		list_for_each_safe(p, n, &adc_pending) {
			tmp = list_entry(p, struct s3c_adc_client, pend);
			if (tmp == client)
				list_del(&tmp->pend);
		}
	}

	if (adc_dev->cur == NULL)
		s3c_adc_try(adc_dev);

	spin_unlock_irqrestore(&adc_dev->lock, flags);
	kfree(client);
}
EXPORT_SYMBOL_GPL(s3c_adc_release);

static irqreturn_t s3c_adc_irq(int irq, void *pw)
{
	struct adc_device *adc = pw;
	struct s3c_adc_client *client = adc->cur;
	enum s3c_cpu_type cpu = platform_get_device_id(adc->pdev)->driver_data;
	unsigned data0, data1;

	if (!client) {
		dev_warn(&adc->pdev->dev, "%s: no adc pending\n", __func__);
		goto exit;
	}

	data0 = readl(adc->regs + S3C2410_ADCDAT0);
	data1 = readl(adc->regs + S3C2410_ADCDAT1);
	adc_dbg(adc, "read %d: 0x%04x, 0x%04x\n", client->nr_samples, data0, data1);

	client->nr_samples--;

	if (cpu == TYPE_ADCV1 || cpu == TYPE_ADCV11) {
		data0 &= 0x3ff;
		data1 &= 0x3ff;
	} else {
		/* S3C2416/S3C64XX/S5P ADC resolution is 12-bit */
		data0 &= 0xfff;
		data1 &= 0xfff;
	}

	if (client->is_ts) {
		ts.xp += data0;
		ts.yp += data1;

		ts.count++;

		/* From tests, it seems that it is unlikely to get a pen-up
		 * event during the conversion process which means we can
		 * ignore any pen-up events with less than the requisite
		 * count done.
		 *
		 * In several thousand conversions, no pen-ups where detected
		 * before count completed.
		 */
	} else {
		client->result = data0;
		wake_up(client->wait);
	}

	if (client->nr_samples > 0) {
		/* fire another conversion for this */

		if (client->is_ts)
			writel(S3C2410_ADCTSC_PULL_UP_DISABLE | AUTOPST,
			       ts.io + S3C2410_ADCTSC);
		s3c_adc_convert(adc);
	} else {
		spin_lock(&adc->lock);
		if (client->is_ts) {
			mod_timer(&touch_timer, jiffies+1);
			writel(WAIT4INT | INT_UP, ts.io + S3C2410_ADCTSC);
		}
		adc->cur = NULL;

		s3c_adc_try(adc);
		spin_unlock(&adc->lock);
	}

exit:
	if (cpu == TYPE_ADCV2 || cpu == TYPE_ADCV3) {
		/* Clear ADC interrupt */
		writel(0, adc->regs + S3C64XX_ADCCLRINT);
	}
	return IRQ_HANDLED;
}

static int s3c_adc_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct adc_device *adc;
	struct resource *regs;
	enum s3c_cpu_type cpu = platform_get_device_id(pdev)->driver_data;
	int ret;
	unsigned tmp;

	adc = devm_kzalloc(dev, sizeof(struct adc_device), GFP_KERNEL);
	if (adc == NULL) {
		dev_err(dev, "failed to allocate adc_device\n");
		return -ENOMEM;
	}

	spin_lock_init(&adc->lock);

	adc->pdev = pdev;
	adc->prescale = S3C2410_ADCCON_PRSCVL(49);

	adc->vdd = devm_regulator_get(dev, "vdd");
	if (IS_ERR(adc->vdd)) {
		dev_err(dev, "operating without regulator \"vdd\" .\n");
		return PTR_ERR(adc->vdd);
	}

	adc->irq = platform_get_irq(pdev, 1);
	if (adc->irq <= 0) {
		dev_err(dev, "failed to get adc irq\n");
		return -ENOENT;
	}

	ret = devm_request_irq(dev, adc->irq, s3c_adc_irq, 0, dev_name(dev),
				adc);
	if (ret < 0) {
		dev_err(dev, "failed to attach adc irq\n");
		return ret;
	}

	adc->clk = devm_clk_get(dev, "adc");
	if (IS_ERR(adc->clk)) {
		dev_err(dev, "failed to get adc clock\n");
		return PTR_ERR(adc->clk);
	}

	regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	adc->regs = devm_ioremap_resource(dev, regs);
	if (IS_ERR(adc->regs))
		return PTR_ERR(adc->regs);

	ret = regulator_enable(adc->vdd);
	if (ret)
		return ret;

	clk_enable(adc->clk);

	tmp = adc->prescale | S3C2410_ADCCON_PRSCEN;

	/* Enable 12-bit ADC resolution */
	if (cpu == TYPE_ADCV12)
		tmp |= S3C2416_ADCCON_RESSEL;
	if (cpu == TYPE_ADCV2 || cpu == TYPE_ADCV3)
		tmp |= S3C64XX_ADCCON_RESSEL;

	writel(tmp, adc->regs + S3C2410_ADCCON);

	dev_info(dev, "attached adc driver\n");

	platform_set_drvdata(pdev, adc);
	adc_dev = adc;

	return 0;
}

static int s3c_adc_remove(struct platform_device *pdev)
{
	struct adc_device *adc = platform_get_drvdata(pdev);

	clk_disable(adc->clk);
	regulator_disable(adc->vdd);

	return 0;
}

#ifdef CONFIG_PM
static int s3c_adc_suspend(struct device *dev)
{
	struct platform_device *pdev = container_of(dev,
			struct platform_device, dev);
	struct adc_device *adc = platform_get_drvdata(pdev);
	unsigned long flags;
	u32 con;

	spin_lock_irqsave(&adc->lock, flags);

	con = readl(adc->regs + S3C2410_ADCCON);
	con |= S3C2410_ADCCON_STDBM;
	writel(con, adc->regs + S3C2410_ADCCON);

	disable_irq(adc->irq);
	spin_unlock_irqrestore(&adc->lock, flags);
	clk_disable(adc->clk);
	regulator_disable(adc->vdd);

	return 0;
}

static int s3c_adc_resume(struct device *dev)
{
	struct platform_device *pdev = container_of(dev,
			struct platform_device, dev);
	struct adc_device *adc = platform_get_drvdata(pdev);
	enum s3c_cpu_type cpu = platform_get_device_id(pdev)->driver_data;
	int ret;
	unsigned long tmp;

	ret = regulator_enable(adc->vdd);
	if (ret)
		return ret;
	clk_enable(adc->clk);
	enable_irq(adc->irq);

	tmp = adc->prescale | S3C2410_ADCCON_PRSCEN;

	/* Enable 12-bit ADC resolution */
	if (cpu == TYPE_ADCV12)
		tmp |= S3C2416_ADCCON_RESSEL;
	if (cpu == TYPE_ADCV2 || cpu == TYPE_ADCV3)
		tmp |= S3C64XX_ADCCON_RESSEL;

	writel(tmp, adc->regs + S3C2410_ADCCON);

	return 0;
}

#else
#define s3c_adc_suspend NULL
#define s3c_adc_resume NULL
#endif

static struct platform_device_id s3c_adc_driver_ids[] = {
	{
		.name           = "s3c24xx-adc",
		.driver_data    = TYPE_ADCV1,
	}, {
		.name		= "s3c2443-adc",
		.driver_data	= TYPE_ADCV11,
	}, {
		.name		= "s3c2416-adc",
		.driver_data	= TYPE_ADCV12,
	}, {
		.name           = "s3c64xx-adc",
		.driver_data    = TYPE_ADCV2,
	}, {
		.name		= "samsung-adc-v3",
		.driver_data	= TYPE_ADCV3,
	},
	{ }
};
MODULE_DEVICE_TABLE(platform, s3c_adc_driver_ids);

static const struct dev_pm_ops adc_pm_ops = {
	.suspend	= s3c_adc_suspend,
	.resume		= s3c_adc_resume,
};

static struct platform_driver s3c_adc_driver = {
	.id_table	= s3c_adc_driver_ids,
	.driver		= {
		.name	= "s3c-adc",
		.pm	= &adc_pm_ops,
	},
	.probe		= s3c_adc_probe,
	.remove		= s3c_adc_remove,
};

/**
 * get_down - return the down state of the pen
 * @data0: The data read from ADCDAT0 register.
 * @data1: The data read from ADCDAT1 register.
 *
 * Return non-zero if both readings show that the pen is down.
 */
static inline bool get_down(unsigned long data0, unsigned long data1)
{
	/* returns true if both data values show stylus down */
	return (!(data0 & S3C2410_ADCDAT0_UPDOWN) &&
		!(data1 & S3C2410_ADCDAT0_UPDOWN));
}

static void touch_timer_fire(unsigned long data)
{
	unsigned long data0;
	unsigned long data1;
	bool down;

	data0 = readl(ts.io + S3C2410_ADCDAT0);
	data1 = readl(ts.io + S3C2410_ADCDAT1);

	down = get_down(data0, data1);

	if (down) {
		if (ts.count == (1 << ts.shift)) {
			ts.xp >>= ts.shift;
			ts.yp >>= ts.shift;

			dev_dbg(ts.dev, "%s: X=%lu, Y=%lu, count=%d\n",
				__func__, ts.xp, ts.yp, ts.count);

			input_report_abs(ts.input, ABS_X, ts.xp);
			input_report_abs(ts.input, ABS_Y, ts.yp);

			input_report_key(ts.input, BTN_TOUCH, 1);
			input_sync(ts.input);

			ts.xp = 0;
			ts.yp = 0;
			ts.count = 0;
		}

		s3c_adc_start(ts.client, 0, 1 << ts.shift);
	} else {
		ts.xp = 0;
		ts.yp = 0;
		ts.count = 0;

		input_report_key(ts.input, BTN_TOUCH, 0);
		input_sync(ts.input);

		writel(WAIT4INT | INT_DOWN, ts.io + S3C2410_ADCTSC);
	}
}

/**
 * stylus_irq - touchscreen stylus event interrupt
 * @irq: The interrupt number
 * @dev_id: The device ID.
 *
 * Called when the IRQ_TC is fired for a pen up or down event.
 */
static irqreturn_t stylus_irq(int irq, void *dev_id)
{
	unsigned long data0;
	unsigned long data1;
	bool down;

	data0 = readl(ts.io + S3C2410_ADCDAT0);
	data1 = readl(ts.io + S3C2410_ADCDAT1);

	down = get_down(data0, data1);

	/* TODO we should never get an interrupt with down set while
	 * the timer is running, but maybe we ought to verify that the
	 * timer isn't running anyways. */

	if (down)
		s3c_adc_start(ts.client, 0, 1 << ts.shift);
	else
		dev_dbg(ts.dev, "%s: count=%d\n", __func__, ts.count);

	if (ts.features & FEAT_PEN_IRQ) {
		/* Clear pen down/up interrupt */
		writel(0x0, ts.io + S3C64XX_ADCCLRINTPNDNUP);
	}

	return IRQ_HANDLED;
}

/**
 * s3c2410ts_probe - device core probe entry point
 * @pdev: The device we are being bound to.
 *
 * Initialise, find and allocate any resources we need to run and then
 * register with the ADC and input systems.
 */
static int s3c2410ts_probe(struct platform_device *pdev)
{
	struct s3c2410_ts_mach_info *info;
	struct device *dev = &pdev->dev;
	struct input_dev *input_dev;
	struct resource *res;
	int ret = -EINVAL;

	/* Initialise input stuff */
	memset(&ts, 0, sizeof(struct s3c2410ts));

	ts.dev = dev;

	info = dev_get_platdata(&pdev->dev);
	if (!info) {
		dev_err(dev, "no platform data, cannot attach\n");
		return -EINVAL;
	}

	dev_dbg(dev, "initialising touchscreen\n");

	ts.clock = clk_get(dev, "adc");
	if (IS_ERR(ts.clock)) {
		dev_err(dev, "cannot get adc clock source\n");
		return -ENOENT;
	}

	clk_prepare_enable(ts.clock);
	dev_dbg(dev, "got and enabled clocks\n");

	ts.irq_tc = ret = platform_get_irq(pdev, 0);
	if (ret < 0) {
		dev_err(dev, "no resource for interrupt\n");
		goto err_clk;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(dev, "no resource for registers\n");
		ret = -ENOENT;
		goto err_clk;
	}

	ts.io = ioremap(res->start, resource_size(res));
	if (ts.io == NULL) {
		dev_err(dev, "cannot map registers\n");
		ret = -ENOMEM;
		goto err_clk;
	}

	/* inititalise the gpio */
	if (info->cfg_gpio)
		info->cfg_gpio(to_platform_device(ts.dev));

	ts.client = s3c_adc_register(pdev, NULL, NULL, 1);
	if (IS_ERR(ts.client)) {
		dev_err(dev, "failed to register adc client\n");
		ret = PTR_ERR(ts.client);
		goto err_iomap;
	}

	/* Initialise registers */
	if ((info->delay & 0xffff) > 0)
		writel(info->delay & 0xffff, ts.io + S3C2410_ADCDLY);

	writel(WAIT4INT | INT_DOWN, ts.io + S3C2410_ADCTSC);

	input_dev = input_allocate_device();
	if (!input_dev) {
		dev_err(dev, "Unable to allocate the input device !!\n");
		ret = -ENOMEM;
		goto err_iomap;
	}

	ts.input = input_dev;
	ts.input->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
	ts.input->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
	input_set_abs_params(ts.input, ABS_X, 0, 0x3FF, 0, 0);
	input_set_abs_params(ts.input, ABS_Y, 0, 0x3FF, 0, 0);

	ts.input->name = "S3C24XX TouchScreen";
	ts.input->id.bustype = BUS_HOST;
	ts.input->id.vendor = 0xDEAD;
	ts.input->id.product = 0xBEEF;
	ts.input->id.version = 0x0102;

	ts.shift = info->oversampling_shift;
	ts.features = platform_get_device_id(pdev)->driver_data;

	ret = request_irq(ts.irq_tc, stylus_irq, 0,
			  "s3c2410_ts_pen", ts.input);
	if (ret) {
		dev_err(dev, "cannot get TC interrupt\n");
		goto err_inputdev;
	}

	dev_info(dev, "driver attached, registering input device\n");

	/* All went ok, so register to the input system */
	ret = input_register_device(ts.input);
	if (ret < 0) {
		dev_err(dev, "failed to register input device\n");
		ret = -EIO;
		goto err_tcirq;
	}

	return 0;

 err_tcirq:
	free_irq(ts.irq_tc, ts.input);
 err_inputdev:
	input_free_device(ts.input);
 err_iomap:
	iounmap(ts.io);
 err_clk:
	del_timer_sync(&touch_timer);
	clk_put(ts.clock);
	return ret;
}

/**
 * s3c2410ts_remove - device core removal entry point
 * @pdev: The device we are being removed from.
 *
 * Free up our state ready to be removed.
 */
static int s3c2410ts_remove(struct platform_device *pdev)
{
	free_irq(ts.irq_tc, ts.input);
	del_timer_sync(&touch_timer);

	clk_disable_unprepare(ts.clock);
	clk_put(ts.clock);

	input_unregister_device(ts.input);
	iounmap(ts.io);

	return 0;
}

#ifdef CONFIG_PM
static int s3c2410ts_suspend(struct device *dev)
{
	writel(TSC_SLEEP, ts.io + S3C2410_ADCTSC);
	disable_irq(ts.irq_tc);
	clk_disable(ts.clock);

	return 0;
}

static int s3c2410ts_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct s3c2410_ts_mach_info *info = dev_get_platdata(&pdev->dev);

	clk_enable(ts.clock);
	enable_irq(ts.irq_tc);

	/* Initialise registers */
	if ((info->delay & 0xffff) > 0)
		writel(info->delay & 0xffff, ts.io + S3C2410_ADCDLY);

	writel(WAIT4INT | INT_DOWN, ts.io + S3C2410_ADCTSC);

	return 0;
}

static const struct dev_pm_ops s3c_ts_pmops = {
	.suspend	= s3c2410ts_suspend,
	.resume		= s3c2410ts_resume,
};
#endif

static struct platform_device_id s3cts_driver_ids[] = {
	{ "s3c2410-ts", 0 },
	{ "s3c2440-ts", 0 },
	{ "s3c64xx-ts", FEAT_PEN_IRQ },
	{ }
};
MODULE_DEVICE_TABLE(platform, s3cts_driver_ids);

static struct platform_driver s3c_ts_driver = {
	.driver         = {
		.name   = "samsung-ts",
#ifdef CONFIG_PM
		.pm	= &s3c_ts_pmops,
#endif
	},
	.id_table	= s3cts_driver_ids,
	.probe		= s3c2410ts_probe,
	.remove		= s3c2410ts_remove,
};

static int __init adcts_init(void)
{
	int ret;

	ret = platform_driver_register(&s3c_adc_driver);
	if (ret) {
		printk(KERN_ERR "%s: failed to add adc driver\n", __func__);
		return ret;
	}

	ret = platform_driver_register(&s3c_ts_driver);
	if (ret)
		platform_driver_unregister(&s3c_adc_driver);

	return ret;
}
module_init(adcts_init);

static void __exit adcts_exit(void)
{
	platform_driver_unregister(&s3c_ts_driver);
	platform_driver_unregister(&s3c_adc_driver);
}
module_exit(adcts_exit);

MODULE_AUTHOR("Arnaud Patard <arnaud.patard@rtp-net.org>, "
	      "Ben Dooks <ben@simtec.co.uk>, "
	      "Simtec Electronics <linux@simtec.co.uk>");
MODULE_DESCRIPTION("S3C24XX Touchscreen driver");
MODULE_LICENSE("GPL v2");
