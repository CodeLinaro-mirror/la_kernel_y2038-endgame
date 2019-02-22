// SPDX-License-Identifier: GPL-2.0
/*
 * Fast user context implementation of clock_gettime, gettimeofday, and time.
 *
 * Copyright 2019 ARM Limited
 * Copyright 2006 Andi Kleen, SUSE Labs.
 * 32 Bit compat layer by Stefani Seibold <stefani@seibold.net>
 *  sponsored by Rohde & Schwarz GmbH & Co. KG Munich/Germany
 */
#include <linux/time.h>
#include <linux/types.h>

#include "../../../../lib/vdso/gettimeofday.c"

extern int __vdso_clock_gettime(clockid_t clock, struct __vdso_timespec *ts);
extern int __vdso_gettimeofday(struct __vdso_timeval *tv, struct timezone *tz);
extern time_t __vdso_time(time_t *t);
extern int __vdso_clock_getres(clockid_t clock, struct __vdso_timespec *res);

notrace int __vdso_clock_gettime(clockid_t clock, struct __vdso_timespec *ts)
{
	return __cvdso_clock_gettime(clock, ts);
}

int clock_gettime(clockid_t, struct __vdso_timespec *)
	__attribute__((weak, alias("__vdso_clock_gettime")));

notrace int __vdso_gettimeofday(struct __vdso_timeval *tv,
				struct timezone *tz)
{
	return __cvdso_gettimeofday(tv, tz);
}
int gettimeofday(struct __vdso_timeval *, struct timezone *)
	__attribute__((weak, alias("__vdso_gettimeofday")));

notrace time_t __vdso_time(time_t *t)
{
	return __cvdso_time(t);
}
time_t time(time_t *t)
	__attribute__((weak, alias("__vdso_time")));

notrace int __vdso_clock_getres(clockid_t clock,
				struct __vdso_timespec *res)
{
	return __cvdso_clock_getres(clock, res);
}
int clock_getres(clockid_t, struct __vdso_timespec *)
	__attribute__((weak, alias("__vdso_clock_getres")));
