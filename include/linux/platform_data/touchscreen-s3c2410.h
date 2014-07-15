/*
 * Copyright (c) 2005 Arnaud Patard <arnaud.patard@rtp-net.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __TOUCHSCREEN_S3C2410_H
#define __TOUCHSCREEN_S3C2410_H

struct s3c_adc_client;
struct platform_device;

extern int s3c_adc_read(struct s3c_adc_client *client, unsigned int ch);

extern struct s3c_adc_client *
	s3c_adc_register(struct platform_device *pdev,
			 void (*select)(struct s3c_adc_client *client,
					unsigned selected),
			 void (*conv)(struct s3c_adc_client *client,
				      unsigned d0, unsigned d1,
				      unsigned *samples_left),
			 unsigned int is_ts);

extern void s3c_adc_release(struct s3c_adc_client *client);


struct s3c2410_ts_mach_info {
	int delay;
	int presc;
	int oversampling_shift;
	void (*cfg_gpio)(struct platform_device *dev);
};

extern void s3c24xx_ts_set_platdata(struct s3c2410_ts_mach_info *);

/* defined by architecture to configure gpio */
extern void s3c24xx_ts_cfg_gpio(struct platform_device *dev);

#endif /*__TOUCHSCREEN_S3C2410_H */
