/*
 * x86 msqid64_ds structure.
 *
 * Note extra padding because this structure is passed back and forth
 * between kernel and user space.
 *
 * msqid64_ds was originally meant to be architecture specific, but
 * everyone just ended up making identical copies without specific
 * optimizations, so we may just as well all use the same one.
 *
 * i386 uses the padding words to extend the time_t values, so user
 * space must combine the two words. x32 and x86_64 have a 64-bit
 * __kernel_time_t, so the data is already in the right place.
 *
 * Pad space is left for:
 * - 2 miscellaneous 32-bit values
 */

struct msqid64_ds {
	struct ipc64_perm msg_perm;
#ifdef __i386__
	unsigned long	msg_stime;	/* last msgsnd time */
	unsigned long	msg_stime_high;
	unsigned long	msg_rtime;	/* last msgrcv time */
	unsigned long	msg_rtime_high;
	unsigned long	msg_ctime;	/* last change time */
	unsigned long	msg_ctime_high;
#else
	__kernel_time_t msg_stime;	/* last msgsnd time */
	__kernel_time_t msg_rtime;	/* last msgrcv time */
	__kernel_time_t msg_ctime;	/* last change time */
#endif
	__kernel_ulong_t msg_cbytes;	/* current number of bytes on queue */
	__kernel_ulong_t msg_qnum;	/* number of messages in queue */
	__kernel_ulong_t msg_qbytes;	/* max number of bytes on queue */
	__kernel_pid_t msg_lspid;	/* pid of last msgsnd */
	__kernel_pid_t msg_lrpid;	/* last receive pid */
	__kernel_ulong_t __unused4;
	__kernel_ulong_t __unused5;
};
