/*
 * include/linux/platform_data/media/si4713.h
 *
 * Board related data definitions for Si4713 i2c device driver.
 *
 * Copyright (c) 2009 Nokia Corporation
 * Contact: Eduardo Valentin <eduardo.valentin@nokia.com>
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 *
 */

#ifndef SI4713_H
#define SI4713_H

/*
 * Structure to query for Received Noise Level (RNL).
 */
struct si4713_rnl {
	__u32 index;		/* modulator index */
	__u32 frequency;	/* frequency to perform rnl measurement */
	__s32 rnl;		/* result of measurement in dBuV */
	__u32 reserved[4];	/* drivers and apps must init this to 0 */
};

/*
 * This is the ioctl number to query for rnl. Users must pass a
 * struct si4713_rnl pointer specifying desired frequency in 'frequency' field
 * following driver capabilities (i.e V4L2_TUNER_CAP_LOW).
 * Driver must return measured value in the same structure, filling 'rnl' field.
 */
#define SI4713_IOC_MEASURE_RNL	_IOWR('V', BASE_VIDIOC_PRIVATE + 0, \
						struct si4713_rnl)

#endif /* ifndef SI4713_H*/
