#ifndef _ASMARM_STAT_H
#define _ASMARM_STAT_H

struct __old_kernel_stat {
	unsigned short st_dev;
	unsigned short st_ino;
	unsigned short st_mode;
	unsigned short st_nlink;
	unsigned short st_uid;
	unsigned short st_gid;
	unsigned short st_rdev;
	unsigned long  st_size;
	unsigned long  st_atime;
	unsigned long  st_mtime;
	unsigned long  st_ctime;
};

#define STAT_HAVE_NSEC 

struct stat {
#if defined(__ARMEB__)
	unsigned short st_dev;
	unsigned short __pad1;
#else
	unsigned long  st_dev;
#endif
	unsigned long  st_ino;
	unsigned short st_mode;
	unsigned short st_nlink;
	unsigned short st_uid;
	unsigned short st_gid;
#if defined(__ARMEB__)
	unsigned short st_rdev;
	unsigned short __pad2;
#else
	unsigned long  st_rdev;
#endif
	unsigned long  st_size;
	unsigned long  st_blksize;
	unsigned long  st_blocks;
	unsigned long  st_atime;
	unsigned long  st_atime_nsec;
	unsigned long  st_mtime;
	unsigned long  st_mtime_nsec;
	unsigned long  st_ctime;
	unsigned long  st_ctime_nsec;
	unsigned long  __unused4;
	unsigned long  __unused5;
};

/* this matches the arm64 'struct stat' to allow a simpler compat ABI */
#define __ARCH_HAS_NEWSTAT64
struct newstat64 {
	unsigned long long	st_dev;		/* Device.  */
	unsigned long long	st_ino;		/* File serial number.  */
	unsigned int		st_mode;	/* File mode.  */
	unsigned int		st_nlink;	/* Link count.  */
	unsigned int		st_uid;		/* User ID of the file's owner.  */
	unsigned int		st_gid;		/* Group ID of the file's group. */
	unsigned long long	st_rdev;	/* Device number, if device.  */
	unsigned long long	__pad1;
	long long		st_size;	/* Size of file, in bytes.  */
	int			st_blksize;	/* Optimal block size for I/O.  */
	int			__pad2;
	long long		st_blocks;	/* Number 512-byte blocks allocated. */
	long long		st_atime;	/* Time of last access.  */
	unsigned long long	st_atime_nsec;
	long long		st_mtime;	/* Time of last modification.  */
	unsigned long long	st_mtime_nsec;
	long long		st_ctime;	/* Time of last status change.  */
	unsigned long long	st_ctime_nsec;
	unsigned int		__unused4;
	unsigned int		__unused5;
};

/* This matches struct stat64 in glibc2.1, hence the absolutely
 * insane amounts of padding around dev_t's.
 * Note: The kernel zero's the padded region because glibc might read them
 * in the hope that the kernel has stretched to using larger sizes.
 */
struct stat64 {
	unsigned long long	st_dev;
	unsigned char   __pad0[4];

#define STAT64_HAS_BROKEN_ST_INO	1
	unsigned long	__st_ino;
	unsigned int	st_mode;
	unsigned int	st_nlink;

	unsigned long	st_uid;
	unsigned long	st_gid;

	unsigned long long	st_rdev;
	unsigned char   __pad3[4];

	long long	st_size;
	unsigned long	st_blksize;
	unsigned long long st_blocks;	/* Number 512-byte blocks allocated. */

	unsigned long	st_atime;
	unsigned long	st_atime_nsec;

	unsigned long	st_mtime;
	unsigned long	st_mtime_nsec;

	unsigned long	st_ctime;
	unsigned long	st_ctime_nsec;

	unsigned long long	st_ino;
};

#endif
