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

/*
 * Convert common ioctl arguments based on their command number
 *
 * Please do not add any code in here. Instead, implement
 * a compat_ioctl operation in the place that handleѕ the
 * ioctl for the native case.
 */
static long do_ioctl_trans(unsigned int cmd,
		 unsigned long arg, struct file *file)
{
	switch (cmd) {
	}

	return -ENOIOCTLCMD;
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

	switch (cmd) {
	/* these are never seen by ->ioctl(), no argument or int argument */
	case FIOCLEX:
	case FIONCLEX:
	case FIFREEZE:
	case FITHAW:
	case FICLONE:
		goto do_ioctl;
	/* these are never seen by ->ioctl(), pointer argument */
	case FIONBIO:
	case FIOASYNC:
	case FIOQSIZE:
	case FS_IOC_FIEMAP:
	case FIGETBSZ:
	case FICLONERANGE:
	case FIDEDUPERANGE:
		goto found_handler;
	/*
	 * The next group is the stuff handled inside file_ioctl().
	 * For regular files these never reach ->ioctl(); for
	 * devices, sockets, etc. they do and one (FIONREAD) is
	 * even accepted in some cases.  In all those cases
	 * argument has the same type, so we can handle these
	 * here, shunting them towards do_vfs_ioctl().
	 * ->compat_ioctl() will never see any of those.
	 */
	/* pointer argument, never actually handled by ->ioctl() */
	case FIBMAP:
		goto found_handler;
	/* handled by some ->ioctl(); always a pointer to int */
	case FIONREAD:
		goto found_handler;
	/* these two get messy on amd64 due to alignment differences */
#if defined(CONFIG_X86_64)
	case FS_IOC_RESVSP_32:
	case FS_IOC_RESVSP64_32:
		error = compat_ioctl_preallocate(f.file, compat_ptr(arg));
		goto out_fput;
#else
	case FS_IOC_RESVSP:
	case FS_IOC_RESVSP64:
		goto found_handler;
#endif

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

	error = do_ioctl_trans(cmd, arg, f.file);
	if (error == -ENOIOCTLCMD)
		error = -ENOTTY;

	goto out_fput;

 found_handler:
	arg = (unsigned long)compat_ptr(arg);
 do_ioctl:
	error = do_vfs_ioctl(f.file, fd, cmd, arg);
 out_fput:
	fdput(f);
 out:
	return error;
}
