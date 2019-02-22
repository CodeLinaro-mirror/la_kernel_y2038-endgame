// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright (C) 2001 Andrea Arcangeli <andrea@suse.de> SuSE
 *  Copyright 2003 Andi Kleen, SuSE Labs.
 *
 *  Modified for x86 32 bit architecture by
 *  Stefani Seibold <stefani@seibold.net>
 *  sponsored by Rohde & Schwarz GmbH & Co. KG Munich/Germany
 *
 *  Thanks to hpa@transmeta.com for some useful hint.
 *  Special thanks to Ingo Molnar for his early experience with
 *  a different vsyscall implementation for Linux/IA32 and for the name.
 *
 */

#include <linux/timekeeper_internal.h>
#include <asm/vgtod.h>
#include <asm/vvar.h>

int vclocks_used __read_mostly;

DEFINE_VVAR(struct vdso_data, vdso_data);

void update_vsyscall_tz(void)
{
	vdso_data.tz_minuteswest = sys_tz.tz_minuteswest;
	vdso_data.tz_dsttime = sys_tz.tz_dsttime;
}

void update_vsyscall(struct timekeeper *tk)
{
	int vclock_mode = tk->tkr_mono.clock->archdata.vclock_mode;
	struct vdso_data *vdata = &vdso_data;
	struct vdso_timestamp *vdso_ts;
	u64 nsec;

	/* Mark the new vclock used. */
	BUILD_BUG_ON(VCLOCK_MAX >= 32);
	WRITE_ONCE(vclocks_used, READ_ONCE(vclocks_used) | (1 << vclock_mode));

	vdso_write_begin(vdata);

	/* copy vsyscall data */
	vdata->clock_mode	= vclock_mode;
	vdata->cycle_last	= tk->tkr_mono.cycle_last;
	vdata->cs[CLOCKSOURCE_MONO].mask
				= tk->tkr_mono.mask;
	vdata->cs[CLOCKSOURCE_MONO].mult
				= tk->tkr_mono.mult;
	vdata->cs[CLOCKSOURCE_MONO].shift
				= tk->tkr_mono.shift;
	vdata->cs[CLOCKSOURCE_RAW].mask
				= tk->tkr_raw.mask;
	vdata->cs[CLOCKSOURCE_RAW].mult
				= tk->tkr_raw.mult;
	vdata->cs[CLOCKSOURCE_RAW].shift
				= tk->tkr_raw.shift;
	/* CLOCK_REALTIME */
	vdso_ts			= &vdata->basetime[CLOCK_REALTIME];
	vdso_ts->sec		= tk->xtime_sec;
	vdso_ts->nsec		= tk->tkr_mono.xtime_nsec;
	/* CLOCK_MONOTONIC */
	vdso_ts			= &vdata->basetime[CLOCK_MONOTONIC];
	vdso_ts->sec		= tk->xtime_sec +
					tk->wall_to_monotonic.tv_sec;
	nsec			= tk->tkr_mono.xtime_nsec;
	nsec			= nsec +
				  ((u64)tk->wall_to_monotonic.tv_nsec <<
				   tk->tkr_mono.shift);
	while (nsec >= (((u64)NSEC_PER_SEC) << tk->tkr_mono.shift)) {
		nsec = nsec -
			(((u64)NSEC_PER_SEC) << tk->tkr_mono.shift);
		vdso_ts->sec++;
	}
	vdso_ts->nsec		= nsec;
	/* CLOCK_MONOTONIC_RAW */
	vdso_ts			= &vdata->basetime[CLOCK_MONOTONIC_RAW];
	vdso_ts->sec		= tk->raw_sec;
	vdso_ts->nsec		= tk->tkr_raw.xtime_nsec;
	/* CLOCK_BOOTTIME */
	vdso_ts			= &vdata->basetime[CLOCK_BOOTTIME];
	vdso_ts->sec		= tk->xtime_sec +
					tk->wall_to_monotonic.tv_sec;
	nsec			= tk->tkr_mono.xtime_nsec;
	nsec			= nsec +
				  ((u64)(tk->wall_to_monotonic.tv_nsec +
				   ktime_to_ns(tk->offs_boot)) <<
				   tk->tkr_mono.shift);
	while (nsec >= (((u64)NSEC_PER_SEC) << tk->tkr_mono.shift)) {
		nsec = nsec -
			(((u64)NSEC_PER_SEC) << tk->tkr_mono.shift);
		vdso_ts->sec++;
	}
	vdso_ts->nsec		= nsec;
	/* CLOCK_TAI */
	vdso_ts			= &vdata->basetime[CLOCK_TAI];
	vdso_ts->sec		= tk->xtime_sec + (s64)tk->tai_offset;
	vdso_ts->nsec		= tk->tkr_mono.xtime_nsec;
	/* CLOCK_REALTIME_COARSE */
	vdso_ts			= &vdata->basetime[CLOCK_REALTIME_COARSE];
	vdso_ts->sec		= tk->xtime_sec;
	vdso_ts->nsec		= tk->tkr_mono.xtime_nsec >> tk->tkr_mono.shift;
	/* CLOCK_MONOTONIC_COARSE */
	vdso_ts			= &vdata->basetime[CLOCK_MONOTONIC_COARSE];
	vdso_ts->sec		= tk->xtime_sec + tk->wall_to_monotonic.tv_sec;
	nsec			= tk->tkr_mono.xtime_nsec >> tk->tkr_mono.shift;
	nsec			= nsec + tk->wall_to_monotonic.tv_nsec;
	while (nsec >= NSEC_PER_SEC) {
		nsec = nsec - NSEC_PER_SEC;
		vdso_ts->sec++;
	}
	vdso_ts->nsec		= nsec;

	vdso_write_end(vdata);
}
