/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _ASM_POWERPC_MSGBUF_H
#define _ASM_POWERPC_MSGBUF_H

#include <asm/ipcbuf.h>

/*
 * The msqid64_ds structure for the PowerPC architecture.
 * Note extra padding because this structure is passed back and forth
 * between kernel and user space.
 */

struct msqid64_ds {
	struct ipc64_perm msg_perm;
#ifdef __powerpc64__
	long		 msg_stime;	/* last msgsnd time */
	long		 msg_rtime;	/* last msgrcv time */
	long		 msg_ctime;	/* last change time */
#else
	unsigned long  msg_stime_high;
	unsigned long  msg_stime;	/* last msgsnd time */
	unsigned long  msg_rtime_high;
	unsigned long  msg_rtime;	/* last msgrcv time */
	unsigned long  msg_ctime_high;
	unsigned long  msg_ctime;	/* last change time */
#endif
	unsigned long  msg_cbytes;	/* current number of bytes on queue */
	unsigned long  msg_qnum;	/* number of messages in queue */
	unsigned long  msg_qbytes;	/* max number of bytes on queue */
	__kernel_pid_t msg_lspid;	/* pid of last msgsnd */
	__kernel_pid_t msg_lrpid;	/* last receive pid */
	unsigned long  __unused4;
	unsigned long  __unused5;
	__uapi_arch_pad_long_to_u64;
};

#ifndef __powerpc64__
#define __uapi_arch_pad_msqid_pid
#define __uapi_arch_pad_msqid __uapi_arch_pad16
#else
#define __uapi_arch_pad_msqid_pid __uapi_arch_pad16
#define __uapi_arch_pad_msqid
#endif

#endif	/* _ASM_POWERPC_MSGBUF_H */
