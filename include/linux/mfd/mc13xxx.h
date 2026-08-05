/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright 2009-2010 Pengutronix
 * Uwe Kleine-Koenig <u.kleine-koenig@pengutronix.de>
 */
#ifndef __LINUX_MFD_MC13XXX_H
#define __LINUX_MFD_MC13XXX_H

#include <linux/interrupt.h>

struct mc13xxx;

void mc13xxx_lock(struct mc13xxx *mc13xxx);
void mc13xxx_unlock(struct mc13xxx *mc13xxx);

int mc13xxx_reg_read(struct mc13xxx *mc13xxx, unsigned int offset, u32 *val);
int mc13xxx_reg_write(struct mc13xxx *mc13xxx, unsigned int offset, u32 val);
int mc13xxx_reg_rmw(struct mc13xxx *mc13xxx, unsigned int offset,
		u32 mask, u32 val);

int mc13xxx_irq_request(struct mc13xxx *mc13xxx, int irq,
		irq_handler_t handler, const char *name, void *dev);
int mc13xxx_irq_free(struct mc13xxx *mc13xxx, int irq, void *dev);

int mc13xxx_irq_status(struct mc13xxx *mc13xxx, int irq,
		int *enabled, int *pending);

int mc13xxx_get_flags(struct mc13xxx *mc13xxx);

int mc13xxx_adc_do_conversion(struct mc13xxx *mc13xxx,
		unsigned int mode, unsigned int channel,
		u8 ato, bool atox, unsigned int *sample);

static inline int mc13xxx_irq_request_nounmask(struct mc13xxx *mc13xxx, int irq,
					       irq_handler_t handler,
					       const char *name, void *dev)
{
	return mc13xxx_irq_request(mc13xxx, irq, handler, name, dev);
}

int mc13xxx_irq_mask(struct mc13xxx *mc13xxx, int irq);
int mc13xxx_irq_unmask(struct mc13xxx *mc13xxx, int irq);

#define MC13783_AUDIO_RX0	36
#define MC13783_AUDIO_RX1	37
#define MC13783_AUDIO_TX	38
#define MC13783_SSI_NETWORK	39
#define MC13783_AUDIO_CODEC	40
#define MC13783_AUDIO_DAC	41

#define MC13XXX_IRQ_ADCDONE	0
#define MC13XXX_IRQ_ADCBISDONE	1
#define MC13XXX_IRQ_TS		2
#define MC13XXX_IRQ_CHGDET	6
#define MC13XXX_IRQ_CHGREV	8
#define MC13XXX_IRQ_CHGSHORT	9
#define MC13XXX_IRQ_CCCV	10
#define MC13XXX_IRQ_CHGCURR	11
#define MC13XXX_IRQ_BPON	12
#define MC13XXX_IRQ_LOBATL	13
#define MC13XXX_IRQ_LOBATH	14
#define MC13XXX_IRQ_1HZ		24
#define MC13XXX_IRQ_TODA	25
#define MC13XXX_IRQ_SYSRST	30
#define MC13XXX_IRQ_RTCRST	31
#define MC13XXX_IRQ_PC		32
#define MC13XXX_IRQ_WARM	33
#define MC13XXX_IRQ_MEMHLD	34
#define MC13XXX_IRQ_THWARNL	36
#define MC13XXX_IRQ_THWARNH	37
#define MC13XXX_IRQ_CLK		38

#define MC13783_TS_ATO_FIRST	false
#define MC13783_TS_ATO_EACH	true

enum mc13783_ssi_port {
	MC13783_SSI1_PORT,
	MC13783_SSI2_PORT,
};

#define MC13XXX_USE_TOUCHSCREEN	(1 << 0)
#define MC13XXX_USE_CODEC	(1 << 1)
#define MC13XXX_USE_ADC		(1 << 2)
#define MC13XXX_USE_RTC		(1 << 3)

#define MC13XXX_ADC_MODE_TS		1
#define MC13XXX_ADC_MODE_SINGLE_CHAN	2
#define MC13XXX_ADC_MODE_MULT_CHAN	3

#define MC13XXX_ADC0		43
#define MC13XXX_ADC0_LICELLCON		(1 << 0)
#define MC13XXX_ADC0_CHRGICON		(1 << 1)
#define MC13XXX_ADC0_BATICON		(1 << 2)
#define MC13XXX_ADC0_ADIN7SEL_DIE	(1 << 4)
#define MC13XXX_ADC0_ADIN7SEL_UID	(2 << 4)
#define MC13XXX_ADC0_ADREFEN		(1 << 10)
#define MC13XXX_ADC0_TSMOD0		(1 << 12)
#define MC13XXX_ADC0_TSMOD1		(1 << 13)
#define MC13XXX_ADC0_TSMOD2		(1 << 14)
#define MC13XXX_ADC0_CHRGRAWDIV		(1 << 15)
#define MC13XXX_ADC0_ADINC1		(1 << 16)
#define MC13XXX_ADC0_ADINC2		(1 << 17)

#define MC13XXX_ADC0_TSMOD_MASK		(MC13XXX_ADC0_TSMOD0 | \
					MC13XXX_ADC0_TSMOD1 | \
					MC13XXX_ADC0_TSMOD2)

#define MC13XXX_ADC0_CONFIG_MASK	(MC13XXX_ADC0_TSMOD_MASK | \
					MC13XXX_ADC0_LICELLCON | \
					MC13XXX_ADC0_CHRGICON | \
					MC13XXX_ADC0_BATICON)

#endif /* ifndef __LINUX_MFD_MC13XXX_H */
