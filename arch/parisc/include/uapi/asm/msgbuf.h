/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _PARISC_MSGBUF_H
#define _PARISC_MSGBUF_H

#include <linux/types.h>
#include <asm/bitsperlong.h>
#include <asm/ipcbuf.h>

/* 
 * The msqid64_ds structure for parisc architecture, copied from sparc.
 * Note extra padding because this structure is passed back and forth
 * between kernel and user space.
 *
 * Pad space is left for:
 * - 2 miscellaneous 32-bit values
 */

struct msqid64_ds {
	struct ipc64_perm msg_perm;
#if __BITS_PER_LONG == 64
	long		 msg_stime;	/* last msgsnd time */
	long		 msg_rtime;	/* last msgrcv time */
	long		 msg_ctime;	/* last change time */
#else
	unsigned long	msg_stime_high;
	unsigned long	msg_stime;	/* last msgsnd time */
	unsigned long	msg_rtime_high;
	unsigned long	msg_rtime;	/* last msgrcv time */
	unsigned long	msg_ctime_high;
	unsigned long	msg_ctime;	/* last change time */
#endif
	unsigned long	msg_cbytes;	/* current number of bytes on queue */
	unsigned long	msg_qnum;	/* number of messages in queue */
	unsigned long	msg_qbytes;	/* max number of bytes on queue */
	__kernel_pid_t	msg_lspid;	/* pid of last msgsnd */
	__kernel_pid_t	msg_lrpid;	/* last receive pid */
	unsigned long	__unused1;
	unsigned long	__unused2;
	__uapi_arch_pad_long_to_u64;
};

#define __uapi_arch_pad_msqid_pid
#if __BITS_PER_LONG == 64
#define __uapi_arch_pad_msqid __uapi_arch_pad16; __uapi_arch_pad32
#else
#define __uapi_arch_pad_msqid __uapi_arch_pad16
#endif

#endif /* _PARISC_MSGBUF_H */
