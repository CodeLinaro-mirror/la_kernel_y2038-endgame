#ifndef _LINUX_TIME32_H
#define _LINUX_TIME32_H
/*
 * These are all interfaces based on the old time_t definition
 * that overflows in 2038 on 32-bit architectures. New code
 * should use the replacements based on time64_t and timespec64.
 *
 * Any interfaces in here that become unused as we migrate
 * code to time64_t should get removed.
 */

#include <linux/time64.h>
#include <linux/timex.h>
#ifndef __VDSO32__
//#include <linux/compat.h>
#endif

#include <vdso/time32.h>
#ifdef CONFIG_COMPAT_32BIT_TIME

struct old_itimerspec32 {
	struct old_timespec32 it_interval;
	struct old_timespec32 it_value;
};

struct old_utimbuf32 {
	old_time32_t	actime;
	old_time32_t	modtime;
};

struct old_timex32 {
	u32 modes;
	s32 offset;
	s32 freq;
	s32 maxerror;
	s32 esterror;
	s32 status;
	s32 constant;
	s32 precision;
	s32 tolerance;
	struct old_timeval32 time;
	s32 tick;
	s32 ppsfreq;
	s32 jitter;
	s32 shift;
	s32 stabil;
	s32 jitcnt;
	s32 calcnt;
	s32 errcnt;
	s32 stbcnt;
	s32 tai;

	s32:32; s32:32; s32:32; s32:32;
	s32:32; s32:32; s32:32; s32:32;
	s32:32; s32:32; s32:32;
};

/**
 * ns_to_old_timeval32 - Compat version of ns_to_timeval
 * @nsec:	the nanoseconds value to be converted
 *
 * Returns the old_timeval32 representation of the nsec parameter.
 */
static inline struct old_timeval32 ns_to_old_timeval32(s64 nsec)
{
	struct __kernel_old_timeval tv;
	struct old_timeval32 ctv;

	tv = ns_to_kernel_old_timeval(nsec);
	ctv.tv_sec = tv.tv_sec;
	ctv.tv_usec = tv.tv_usec;

	return ctv;
}

static inline bool have_time32(void)
{
	return true;
}
#else

struct old_timex32;
struct old_itimerspec32;

static inline bool have_time32(void)
{
//	return IS_ENABLED(CONFIG_64BIT) && !in_compat_syscall();
	return false;
}

#endif /* CONFIG_COMPAT_32BIT_TIME */

extern int get_old_timespec32(struct timespec64 *, const void __user *);
extern int put_old_timespec32(const struct timespec64 *, void __user *);
extern int get_old_itimerspec32(struct itimerspec64 *its,
			const struct old_itimerspec32 __user *uits);
extern int put_old_itimerspec32(const struct itimerspec64 *its,
			struct old_itimerspec32 __user *uits);
struct __kernel_timex;
int get_old_timex32(struct __kernel_timex *, const struct old_timex32 __user *);
int put_old_timex32(struct old_timex32 __user *, const struct __kernel_timex *);

#endif
