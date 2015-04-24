/*
 *  arch/arm/include/asm/unistd.h
 *
 *  Copyright (C) 2001-2005 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Please forward _all_ changes to this file to rmk@arm.linux.org.uk,
 * no matter what the change is.  Thanks!
 */
#ifndef _UAPI__ASM_ARM_UNISTD_H
#define _UAPI__ASM_ARM_UNISTD_H

#define __NR_OABI_SYSCALL_BASE	0x900000

#if defined(__thumb__) || defined(__ARM_EABI__)
#define __NR_SYSCALL_BASE	0
#include <asm/unistd-eabi.h>
#else
#define __NR_SYSCALL_BASE	__NR_OABI_SYSCALL_BASE
#include <asm/unistd-oabi.h>
#endif

#include <asm/unistd-common.h>
#define __NR_clock_gettime64		(__NR_SYSCALL_BASE+394)
#define __NR_clock_settime64		(__NR_SYSCALL_BASE+395)
#define __NR_clock_adjtime64		(__NR_SYSCALL_BASE+396)
#define __NR_clock_getres64		(__NR_SYSCALL_BASE+397)
#define __NR_clock_nanosleep64		(__NR_SYSCALL_BASE+398)
#define __NR_timer_gettime64		(__NR_SYSCALL_BASE+399)
#define __NR_timer_settime64		(__NR_SYSCALL_BASE+400)
#define __NR_timerfd_gettime64		(__NR_SYSCALL_BASE+401)
#define __NR_timerfd_settime64		(__NR_SYSCALL_BASE+402)
#define __NR_pselect64			(__NR_SYSCALL_BASE+403)
#define __NR_ppoll64			(__NR_SYSCALL_BASE+404)
#define __NR_io_getevents64		(__NR_SYSCALL_BASE+405)
#define __NR_recvmmsg64			(__NR_SYSCALL_BASE+406)
#define __NR_semtimedop64		(__NR_SYSCALL_BASE+407)
#define __NR_mq_timedsend64		(__NR_SYSCALL_BASE+408)
#define __NR_mq_timedreceive64		(__NR_SYSCALL_BASE+409)
#define __NR_utimensat64		(__NR_SYSCALL_BASE+410)
#define __NR_newfstat64			(__NR_SYSCALL_BASE+411)
#define __NR_newfstatat64		(__NR_SYSCALL_BASE+412)
#define __NR_rt_sigtimedwait64		(__NR_SYSCALL_BASE+413)
#define __NR_getrusage64		(__NR_SYSCALL_BASE+414)
#define __NR_waitid64			(__NR_SYSCALL_BASE+415)

/*
 * The following SWIs are ARM private.
 */
#define __ARM_NR_BASE			(__NR_SYSCALL_BASE+0x0f0000)
#define __ARM_NR_breakpoint		(__ARM_NR_BASE+1)
#define __ARM_NR_cacheflush		(__ARM_NR_BASE+2)
#define __ARM_NR_usr26			(__ARM_NR_BASE+3)
#define __ARM_NR_usr32			(__ARM_NR_BASE+4)
#define __ARM_NR_set_tls		(__ARM_NR_BASE+5)

#endif /* _UAPI__ASM_ARM_UNISTD_H */
