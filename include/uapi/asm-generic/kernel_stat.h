#ifndef __ASM_GENERIC_KERNEL_STAT_H
#define __ASM_GENERIC_KERNEL_STAT_H

/*
 * The new structure that works on both 32-bit and 64-bit and survives y2038
 * The layout matches 'struct stat' from asm-generic/stat.h on 64-bit
 * architecture, but is identical on 32-bit architectures and uses 64-bit
 * st_?time members so we don't wrap around in 2038.
 */

#ifndef __kernel_stat
struct __kernel_stat {
	unsigned long long st_dev;	/* Device.  */
	unsigned long long st_ino;	/* File serial number.  */
	unsigned int	   st_mode;	/* File mode.  */
	unsigned int	   st_nlink;	/* Link count.  */
	unsigned int	   st_uid;	/* User ID of the file's owner.  */
	unsigned int	   st_gid;	/* Group ID of the file's group. */
	unsigned long long st_rdev;	/* Device number, if device.  */
	unsigned long long __pad1;
	long long	   st_size;	/* Size of file, in bytes.  */
	int		   st_blksize;	/* Optimal block size for I/O.  */
	int		   __pad2;
	long long	   st_blocks;	/* Number 512-byte blocks allocated. */
	long long	   st_atime;	/* Time of last access.  */
	unsigned long long st_atime_nsec;
	long long	   st_mtime;	/* Time of last modification.  */
	unsigned long long st_mtime_nsec;
	long long	   st_ctime;	/* Time of last status change.  */
	unsigned long long st_ctime_nsec;
	unsigned int	   __unused4;
	unsigned int	   __unused5;
};
#endif

#endif /* __ASM_GENERIC_KERNEL_STAT_H */
