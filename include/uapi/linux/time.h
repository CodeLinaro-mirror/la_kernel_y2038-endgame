#ifndef _UAPI_LINUX_TIME_H
#define _UAPI_LINUX_TIME_H

#include <linux/types.h>

/*
 * time_t, timespec and timeval are not safe to use beyond
 * 2038 on 32-bit systems, and should be phased out for
 * in-kernel uses as well as interfaces to user space.
 *
 * Inside of the kernel, we can use e.g. inode_time,
 * ktime_t or timespec64, as appropriate.
 *
 * In the long run, we have to stop making these definitions
 * visibile to user headers, so libc can define its own
 * 64-bit types.
 */
#ifndef _STRUCT_TIMESPEC
#define _STRUCT_TIMESPEC
struct timespec {
	__kernel_time_t	tv_sec;			/* seconds */
	long		tv_nsec;		/* nanoseconds */
};
#endif

struct timeval {
	__kernel_time_t		tv_sec;		/* seconds */
	__kernel_suseconds_t	tv_usec;	/* microseconds */
};

/*
 * __kernel_timespec64 is the general type to be used for
 * new user space interfaces passing a time argument.
 * 64-bit nanoseconds is a bit silly, but the advantage is
 * that it is compatible with the native 'struct timespec'
 * on 64-bit user space. This simplifies the compat code.
 */
struct __kernel_timespec64 {
	long long tv_sec;
	long long tv_nsec;
};

/*
 * As interfaces get moved over from time_t, timeval and timespec
 * to __kernel_timespec64, we have to provide backwards compatibility
 * interfaces. These can use __kernel_timespec32. Other types will
 * be needed as required.
 * The compat syscalls and ioctls can also migrate from compat_timespec
 * to __kernel_timespec32 in order to share the implementation with
 * the native 32-bit legacy handlers.
 */
struct __kernel_timespec32 {
	int	tv_sec;
	int	tv_nsec;
};

/* timezone is safe for use beyond 2038 */
struct timezone {
	int	tz_minuteswest;	/* minutes west of Greenwich */
	int	tz_dsttime;	/* type of dst correction */
};


/*
 * Names of the interval timers, and structure
 * defining a timer setting:
 */
#define	ITIMER_REAL		0
#define	ITIMER_VIRTUAL		1
#define	ITIMER_PROF		2

struct itimerspec {
	struct timespec it_interval;	/* timer period */
	struct timespec it_value;	/* timer expiration */
};

struct itimerval {
	struct timeval it_interval;	/* timer interval */
	struct timeval it_value;	/* current value */
};

struct __kernel_itimerspec64 {
	struct __kernel_timespec64 it_interval;
	struct __kernel_timespec64 ir_value;
};

/*
 * The IDs of the various system clocks (for POSIX.1b interval timers):
 */
#define CLOCK_REALTIME			0
#define CLOCK_MONOTONIC			1
#define CLOCK_PROCESS_CPUTIME_ID	2
#define CLOCK_THREAD_CPUTIME_ID		3
#define CLOCK_MONOTONIC_RAW		4
#define CLOCK_REALTIME_COARSE		5
#define CLOCK_MONOTONIC_COARSE		6
#define CLOCK_BOOTTIME			7
#define CLOCK_REALTIME_ALARM		8
#define CLOCK_BOOTTIME_ALARM		9
#define CLOCK_SGI_CYCLE			10	/* Hardware specific */
#define CLOCK_TAI			11

#define MAX_CLOCKS			16
#define CLOCKS_MASK			(CLOCK_REALTIME | CLOCK_MONOTONIC)
#define CLOCKS_MONO			CLOCK_MONOTONIC

/*
 * The various flags for setting POSIX.1b interval timers:
 */
#define TIMER_ABSTIME			0x01

#endif /* _UAPI_LINUX_TIME_H */
