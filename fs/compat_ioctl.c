// SPDX-License-Identifier: GPL-2.0
/*
 * ioctl32.c: Conversion between 32bit and 64bit native ioctls.
 *
 * Copyright (C) 1997-2000  Jakub Jelinek  (jakub@redhat.com)
 * Copyright (C) 1998  Eddie C. Dost  (ecd@skynet.be)
 * Copyright (C) 2001,2002  Andi Kleen, SuSE Labs 
 * Copyright (C) 2003       Pavel Machek (pavel@ucw.cz)
 *
 * These routines maintain argument size conversion between 32bit and 64bit
 * ioctls.
 */

#include <linux/types.h>
#include <linux/compat.h>
#include <linux/kernel.h>
#include <linux/capability.h>
#include <linux/compiler.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/ioctl.h>
#include <linux/if.h>
#include <linux/raid/md_u.h>
#include <linux/falloc.h>
#include <linux/file.h>
#include <linux/ppp-ioctl.h>
#include <linux/if_pppox.h>
#include <linux/tty.h>
#include <linux/vt_kern.h>
#include <linux/blkdev.h>
#include <linux/serial.h>
#include <linux/ctype.h>
#include <linux/syscalls.h>
#include <linux/gfp.h>
#include <linux/cec.h>

#include "internal.h"

#include <linux/uaccess.h>
#include <linux/watchdog.h>

#include <linux/hiddev.h>


#include <linux/sort.h>

/* helper function to avoid trivial compat_ioctl() implementations */
long generic_compat_ioctl_ptrarg(struct file *file, unsigned int cmd, unsigned long arg)
{
	if (!file->f_op->unlocked_ioctl)
		return -ENOIOCTLCMD;

	return file->f_op->unlocked_ioctl(file, cmd, (unsigned long)compat_ptr(arg));
}
EXPORT_SYMBOL_GPL(generic_compat_ioctl_ptrarg);

static inline int do_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int err;

	err = security_file_ioctl(file, cmd, arg);
	if (err)
		return err;

	return vfs_ioctl(file, cmd, arg);
}

/* on ia32 l_start is on a 32-bit boundary */
#if defined(CONFIG_IA64) || defined(CONFIG_X86_64)
struct space_resv_32 {
	__s16		l_type;
	__s16		l_whence;
	__s64		l_start	__attribute__((packed));
			/* len == 0 means until end of file */
	__s64		l_len __attribute__((packed));
	__s32		l_sysid;
	__u32		l_pid;
	__s32		l_pad[4];	/* reserve area */
};

#define FS_IOC_RESVSP_32		_IOW ('X', 40, struct space_resv_32)
#define FS_IOC_RESVSP64_32	_IOW ('X', 42, struct space_resv_32)

/* just account for different alignment */
static int compat_ioctl_preallocate(struct file *file,
			struct space_resv_32    __user *p32)
{
	struct space_resv	__user *p = compat_alloc_user_space(sizeof(*p));

	if (copy_in_user(&p->l_type,	&p32->l_type,	sizeof(s16)) ||
	    copy_in_user(&p->l_whence,	&p32->l_whence, sizeof(s16)) ||
	    copy_in_user(&p->l_start,	&p32->l_start,	sizeof(s64)) ||
	    copy_in_user(&p->l_len,	&p32->l_len,	sizeof(s64)) ||
	    copy_in_user(&p->l_sysid,	&p32->l_sysid,	sizeof(s32)) ||
	    copy_in_user(&p->l_pid,	&p32->l_pid,	sizeof(u32)) ||
	    copy_in_user(&p->l_pad,	&p32->l_pad,	4*sizeof(u32)))
		return -EFAULT;

	return ioctl_preallocate(file, p);
}
#endif

/*
 * simple reversible transform to make our table more evenly
 * distributed after sorting.
 */
#define XFORM(i) (((i) ^ ((i) << 27) ^ ((i) << 17)) & 0xffffffff)

#define COMPATIBLE_IOCTL(cmd) XFORM((u32)cmd),
static unsigned int ioctl_pointer[] = {
/* Little t */
COMPATIBLE_IOCTL(TIOCOUTQ)
/* Little f */
COMPATIBLE_IOCTL(FIOCLEX)
COMPATIBLE_IOCTL(FIONCLEX)
COMPATIBLE_IOCTL(FIOASYNC)
COMPATIBLE_IOCTL(FIONBIO)
COMPATIBLE_IOCTL(FIONREAD)  /* This is also TIOCINQ */
COMPATIBLE_IOCTL(FS_IOC_FIEMAP)
/* 0x00 */
COMPATIBLE_IOCTL(FIBMAP)
COMPATIBLE_IOCTL(FIGETBSZ)
/* 'X' - originally XFS but some now in the VFS */
COMPATIBLE_IOCTL(FIFREEZE)
COMPATIBLE_IOCTL(FITHAW)
COMPATIBLE_IOCTL(FITRIM)
/* Socket level stuff */
COMPATIBLE_IOCTL(FIOQSIZE)
/* PPP stuff */
COMPATIBLE_IOCTL(PPPIOCGUNIT)
COMPATIBLE_IOCTL(PPPIOCGCHAN)
/* Watchdog */
COMPATIBLE_IOCTL(WDIOC_GETSUPPORT)
COMPATIBLE_IOCTL(WDIOC_GETSTATUS)
COMPATIBLE_IOCTL(WDIOC_GETBOOTSTATUS)
COMPATIBLE_IOCTL(WDIOC_GETTEMP)
COMPATIBLE_IOCTL(WDIOC_SETOPTIONS)
COMPATIBLE_IOCTL(WDIOC_KEEPALIVE)
COMPATIBLE_IOCTL(WDIOC_SETTIMEOUT)
COMPATIBLE_IOCTL(WDIOC_GETTIMEOUT)
COMPATIBLE_IOCTL(WDIOC_SETPRETIMEOUT)
COMPATIBLE_IOCTL(WDIOC_GETPRETIMEOUT)
};

static int compat_ioctl_check_table(unsigned int xcmd)
{
	int i;
	const int max = ARRAY_SIZE(ioctl_pointer) - 1;

	BUILD_BUG_ON(max >= (1 << 16));

	/* guess initial offset into table, assuming a
	   normalized distribution */
	i = ((xcmd >> 16) * max) >> 16;

	/* do linear search up first, until greater or equal */
	while (ioctl_pointer[i] < xcmd && i < max)
		i++;

	/* then do linear search down */
	while (ioctl_pointer[i] > xcmd && i > 0)
		i--;

	return ioctl_pointer[i] == xcmd;
}

COMPAT_SYSCALL_DEFINE3(ioctl, unsigned int, fd, unsigned int, cmd,
		       compat_ulong_t, arg32)
{
	unsigned long arg = arg32;
	struct fd f = fdget(fd);
	int error = -EBADF;
	if (!f.file)
		goto out;

	/* RED-PEN how should LSM module know it's handling 32bit? */
	error = security_file_ioctl(f.file, cmd, arg);
	if (error)
		goto out_fput;

	/*
	 * To allow the compat_ioctl handlers to be self contained
	 * we need to check the common ioctls here first.
	 * Just handle them with the standard handlers below.
	 */
	switch (cmd) {
	case FIOCLEX:
	case FIONCLEX:
	case FIONBIO:
	case FIOASYNC:
	case FIOQSIZE:
		break;

#if defined(CONFIG_IA64) || defined(CONFIG_X86_64)
	case FS_IOC_RESVSP_32:
	case FS_IOC_RESVSP64_32:
		error = compat_ioctl_preallocate(f.file, compat_ptr(arg));
		goto out_fput;
#else
	case FS_IOC_RESVSP:
	case FS_IOC_RESVSP64:
		error = ioctl_preallocate(f.file, compat_ptr(arg));
		goto out_fput;
#endif

	case FICLONE:
	case FICLONERANGE:
	case FIDEDUPERANGE:
	case FS_IOC_FIEMAP:
		goto do_ioctl;

	case FIBMAP:
	case FIGETBSZ:
	case FIONREAD:
		if (S_ISREG(file_inode(f.file)->i_mode))
			break;
		/*FALL THROUGH*/

	default:
		if (f.file->f_op->compat_ioctl) {
			error = f.file->f_op->compat_ioctl(f.file, cmd, arg);
			if (error != -ENOIOCTLCMD)
				goto out_fput;
		}

		if (!f.file->f_op->unlocked_ioctl)
			goto do_ioctl;
		break;
	}

	if (!compat_ioctl_check_table(XFORM(cmd)))
		goto out_fput;

	arg = (unsigned long)compat_ptr(arg);
 do_ioctl:
	error = do_vfs_ioctl(f.file, fd, cmd, arg);
 out_fput:
	fdput(f);
 out:
	return error;
}

static int __init init_sys32_ioctl_cmp(const void *p, const void *q)
{
	unsigned int a, b;
	a = *(unsigned int *)p;
	b = *(unsigned int *)q;
	if (a > b)
		return 1;
	if (a < b)
		return -1;
	return 0;
}

static int __init init_sys32_ioctl(void)
{
	sort(ioctl_pointer, ARRAY_SIZE(ioctl_pointer), sizeof(*ioctl_pointer),
		init_sys32_ioctl_cmp, NULL);
	return 0;
}
__initcall(init_sys32_ioctl);
