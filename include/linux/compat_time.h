/*
 * These are the type definitions for the architecture specific
 * syscall compatibility layer for 32-bit time_t.
 *
 * The emulation is intended both for 32-bit architectures that have
 * been converted to 64-bit __kernel_time64_t as well as  64-bit
 * architectures with 32-bit user space.
 */
#ifndef __LINUX_COMPAT_TIME_H
#define __LINUX_COMPAT_TIME_H

#include <linux/types.h>
#include <linux/time.h>
#include <linux/compiler.h>

typedef s32	compat_int_t;
typedef u32	compat_uint_t;
typedef s32	compat_long_t;
typedef u32	compat_ulong_t;
typedef u32	compat_size_t;
typedef s32	compat_ssize_t;
typedef s32	compat_time_t;
typedef	u32	compat_aio_context_t;
typedef u32	compat_uptr_t;
typedef __kernel_pid_t compat_pid_t;

struct compat_timespec {
	compat_time_t	tv_sec;
	s32		tv_nsec;
};

struct compat_timeval {
	compat_time_t	tv_sec;
	s32		tv_usec;
};

struct compat_itimerspec {
	struct compat_timespec it_interval;
	struct compat_timespec it_value;
};

struct compat_utimbuf {
	compat_time_t		actime;
	compat_time_t		modtime;
};

struct compat_itimerval {
	struct compat_timeval	it_interval;
	struct compat_timeval	it_value;
};

struct compat_timex {
	compat_uint_t modes;
	compat_long_t offset;
	compat_long_t freq;
	compat_long_t maxerror;
	compat_long_t esterror;
	compat_int_t status;
	compat_long_t constant;
	compat_long_t precision;
	compat_long_t tolerance;
	struct compat_timeval time;
	compat_long_t tick;
	compat_long_t ppsfreq;
	compat_long_t jitter;
	compat_int_t shift;
	compat_long_t stabil;
	compat_long_t jitcnt;
	compat_long_t calcnt;
	compat_long_t errcnt;
	compat_long_t stbcnt;
	compat_int_t tai;

	compat_int_t:32; compat_int_t:32; compat_int_t:32; compat_int_t:32;
	compat_int_t:32; compat_int_t:32; compat_int_t:32; compat_int_t:32;
	compat_int_t:32; compat_int_t:32; compat_int_t:32;
};


/*
 * These functions operate on 32- or 64-bit specs depending on
 * COMPAT_USE_64BIT_TIME, hence the void user pointer arguments.
 */
extern int compat_get_timespec(struct timespec *, const void __user *);
extern int compat_put_timespec(const struct timespec *, void __user *);
extern int compat_get_timeval(struct timeval *, const void __user *);
extern int compat_put_timeval(const struct timeval *, void __user *);
extern int compat_get_timespec64(struct timespec64 *ts, const void __user *uts);
extern int compat_put_timespec64(const struct timespec64 *ts, void __user *uts);
struct compat_timex;
struct __kernel_timex;
extern int compat_get_timex(struct __kernel_timex *txc, struct compat_timex __user *utp);
extern int compat_put_timex(struct compat_timex __user *utp, struct __kernel_timex *txc);

/*
 * This function convert a timespec if necessary and returns a *user
 * space* pointer.  If no conversion is necessary, it returns the
 * initial pointer.  NULL is a legitimate argument and will always
 * output NULL.
 */
extern int compat_convert_timespec(struct timespec __user **,
				   const void __user *);

struct compat_rusage {
	struct compat_timeval ru_utime;
	struct compat_timeval ru_stime;
	compat_long_t	ru_maxrss;
	compat_long_t	ru_ixrss;
	compat_long_t	ru_idrss;
	compat_long_t	ru_isrss;
	compat_long_t	ru_minflt;
	compat_long_t	ru_majflt;
	compat_long_t	ru_nswap;
	compat_long_t	ru_inblock;
	compat_long_t	ru_oublock;
	compat_long_t	ru_msgsnd;
	compat_long_t	ru_msgrcv;
	compat_long_t	ru_nsignals;
	compat_long_t	ru_nvcsw;
	compat_long_t	ru_nivcsw;
};

struct __kernel_rusage;
extern int put_compat_rusage(const struct __kernel_rusage *,
			     struct compat_rusage __user *);

static inline int compat_timeval_compare(struct compat_timeval *lhs,
					struct compat_timeval *rhs)
{
	if (lhs->tv_sec < rhs->tv_sec)
		return -1;
	if (lhs->tv_sec > rhs->tv_sec)
		return 1;
	return lhs->tv_usec - rhs->tv_usec;
}

static inline int compat_timespec_compare(struct compat_timespec *lhs,
					struct compat_timespec *rhs)
{
	if (lhs->tv_sec < rhs->tv_sec)
		return -1;
	if (lhs->tv_sec > rhs->tv_sec)
		return 1;
	return lhs->tv_nsec - rhs->tv_nsec;
}

extern int get_compat_itimerspec(struct itimerspec *dst,
				 const struct compat_itimerspec __user *src);
extern int put_compat_itimerspec(struct compat_itimerspec __user *dst,
				 const struct itimerspec *src);


#endif
