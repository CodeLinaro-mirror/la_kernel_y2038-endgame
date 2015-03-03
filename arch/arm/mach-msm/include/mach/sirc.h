/* Copyright (c) 2008-2009, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __ASM_ARCH_MSM_SIRC_H
#define __ASM_ARCH_MSM_SIRC_H

struct sirc_regs_t {
	void    *int_enable;
	void    *int_enable_clear;
	void    *int_enable_set;
	void    *int_type;
	void    *int_polarity;
	void    *int_clear;
};

struct sirc_cascade_regs {
	void    *int_status;
	unsigned int    cascade_irq;
};

void msm_init_sirc(int first, int nr_irqs, int cascade);
void msm_sirc_enter_sleep(void);
void msm_sirc_exit_sleep(void);

#endif
