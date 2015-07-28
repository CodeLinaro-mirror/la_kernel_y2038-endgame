#ifndef _LINUX_UTIME_H
#define _LINUX_UTIME_H

#include <linux/types.h>

#if !defined(__KERNEL__) || defined(__KERNEL_COMPAT_TIME__)
struct utimbuf {
	__kernel_time_t actime;
	__kernel_time_t modtime;
};
#endif

#endif
