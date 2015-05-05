#ifndef __ASM_GENERIC_SHMBUF_H
#define __ASM_GENERIC_SHMBUF_H

#include <asm/bitsperlong.h>

/*
 * The shmid64_ds structure for x86 architecture.
 * Note extra padding because this structure is passed back and forth
 * between kernel and user space.
 *
 * shmid64_ds was originally meant to be architecture specific, but
 * everyone just ended up making identical copies without specific
 * optimizations, so we may just as well all use the same one.
 *
 * i386 uses the padding for 64-bit time_t as it was originally designed,
 * but uses two 'unsigned long' members to keep existing user space happy.
 * x32 and x86_64 have a 64-bit __kernel_time_t.
 *
 * Pad space is left for:
 * - 2 miscellaneous 32-bit values
 */

struct shmid64_ds {
	struct ipc64_perm	shm_perm;	/* operation perms */
	size_t			shm_segsz;	/* size of segment (bytes) */
#ifdef __i386__
	unsigned long		shm_atime;	/* last attach time */
	unsigned long		shm_atime_high;
	unsigned long		shm_dtime;	/* last detach time */
	unsigned long		shm_dtime_high;
	unsigned long		shm_ctime;	/* last change time */
	unsigned long		shm_ctime_high;
#else
	__kernel_time_t		shm_atime;	/* last attach time */
	__kernel_time_t		shm_dtime;	/* last detach time */
	__kernel_time_t		shm_ctime;	/* last change time */
#endif
	__kernel_pid_t		shm_cpid;	/* pid of creator */
	__kernel_pid_t		shm_lpid;	/* pid of last operator */
	__kernel_ulong_t	shm_nattch;	/* no. of current attaches */
	__kernel_ulong_t	__unused4;
	__kernel_ulong_t	__unused5;
};

struct shminfo64 {
	__kernel_ulong_t	shmmax;
	__kernel_ulong_t	shmmin;
	__kernel_ulong_t	shmmni;
	__kernel_ulong_t	shmseg;
	__kernel_ulong_t	shmall;
	__kernel_ulong_t	__unused1;
	__kernel_ulong_t	__unused2;
	__kernel_ulong_t	__unused3;
	__kernel_ulong_t	__unused4;
};

#endif /* __ASM_GENERIC_SHMBUF_H */
