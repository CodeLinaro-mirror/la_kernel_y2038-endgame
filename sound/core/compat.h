// SPDX-License-Identifier: GPL-2.0

#ifndef SND_CORE_COMPAT_H
#define SND_CORE_COMPAT_H

struct compat_snd_monotonic_timestamp {
	__u32	tv_sec;
	__u32	tv_nsec;
};

#endif
