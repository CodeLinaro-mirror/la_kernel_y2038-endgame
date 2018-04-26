/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *  Advanced Linux Sound Architecture - ALSA - Driver
 *  Copyright (c) 1994-2003 by Jaroslav Kysela <perex@perex.cz>,
 *                             Abramo Bagnara <abramo@alsa-project.org>
 */
#ifndef __SOUND_ASOUND_H
#define __SOUND_ASOUND_H

#include <linux/ioctl.h>
#include <linux/time.h>
#include <asm/byteorder.h>

#ifdef  __LITTLE_ENDIAN
#define SNDRV_LITTLE_ENDIAN
#else
#ifdef __BIG_ENDIAN
#define SNDRV_BIG_ENDIAN
#else
#error "Unsupported endian..."
#endif
#endif

#include <uapi/sound/asound.h>


static inline struct snd_monotonic_timestamp
timespec64_to_snd_monotonic_timestamp(struct timespec64 ts)
{
	return (struct snd_monotonic_timestamp) { (u32)ts.tv_sec, ts.tv_nsec };
};

#endif /* __SOUND_ASOUND_H */
