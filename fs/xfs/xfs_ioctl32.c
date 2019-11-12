// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2004-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */
#include <linux/mount.h>
#include <linux/fsmap.h>
#include "xfs.h"
#include "xfs_fs.h"
#include "xfs_shared.h"
#include "xfs_format.h"
#include "xfs_log_format.h"
#include "xfs_trans_resv.h"
#include "xfs_mount.h"
#include "xfs_inode.h"
#include "xfs_iwalk.h"
#include "xfs_itable.h"
#include "xfs_fsops.h"
#include "xfs_rtalloc.h"
#include "xfs_attr.h"
#include "xfs_ioctl.h"
#include "xfs_ioctl32.h"
#include "xfs_trace.h"
#include "xfs_sb.h"

#define  _NATIVE_IOC(cmd, type) \
	  _IOC(_IOC_DIR(cmd), _IOC_TYPE(cmd), _IOC_NR(cmd), sizeof(type))

#ifdef BROKEN_X86_ALIGNMENT
STATIC int
xfs_compat_flock64_copyin(
	xfs_flock64_t		*bf,
	compat_xfs_flock64_t	__user *arg32)
{
	if (get_user(bf->l_type,	&arg32->l_type) ||
	    get_user(bf->l_whence,	&arg32->l_whence) ||
	    get_user(bf->l_start,	&arg32->l_start) ||
	    get_user(bf->l_len,		&arg32->l_len) ||
	    get_user(bf->l_sysid,	&arg32->l_sysid) ||
	    get_user(bf->l_pid,		&arg32->l_pid) ||
	    copy_from_user(bf->l_pad,	&arg32->l_pad,	4*sizeof(u32)))
		return -EFAULT;
	return 0;
}

STATIC int
xfs_compat_ioc_fsgeometry_v1(
	struct xfs_mount	  *mp,
	compat_xfs_fsop_geom_v1_t __user *arg32)
{
	struct xfs_fsop_geom	  fsgeo;

	xfs_fs_geometry(&mp->m_sb, &fsgeo, 3);
	/* The 32-bit variant simply has some padding at the end */
	if (copy_to_user(arg32, &fsgeo, sizeof(struct compat_xfs_fsop_geom_v1)))
		return -EFAULT;
	return 0;
}

STATIC int
xfs_compat_growfs_data_copyin(
	struct xfs_growfs_data	 *in,
	compat_xfs_growfs_data_t __user *arg32)
{
	if (get_user(in->newblocks, &arg32->newblocks) ||
	    get_user(in->imaxpct,   &arg32->imaxpct))
		return -EFAULT;
	return 0;
}

STATIC int
xfs_compat_growfs_rt_copyin(
	struct xfs_growfs_rt	 *in,
	compat_xfs_growfs_rt_t	__user *arg32)
{
	if (get_user(in->newblocks, &arg32->newblocks) ||
	    get_user(in->extsize,   &arg32->extsize))
		return -EFAULT;
	return 0;
}

STATIC int
xfs_fsinumbers_fmt_compat(
	struct xfs_ibulk		*breq,
	const struct xfs_inumbers	*ig)
{
	struct compat_xfs_inogrp __user	*p32 = breq->ubuffer;
	struct xfs_inogrp		ig1;
	struct xfs_inogrp		*igrp = &ig1;

	xfs_inumbers_to_inogrp(&ig1, ig);

	if (put_user(igrp->xi_startino,   &p32->xi_startino) ||
	    put_user(igrp->xi_alloccount, &p32->xi_alloccount) ||
	    put_user(igrp->xi_allocmask,  &p32->xi_allocmask))
		return -EFAULT;

	return xfs_ibulk_advance(breq, sizeof(struct compat_xfs_inogrp));
}

#else
#define xfs_fsinumbers_fmt_compat xfs_fsinumbers_fmt
#endif	/* BROKEN_X86_ALIGNMENT */

/* copied from xfs_ioctl.c */
STATIC int
xfs_compat_ioc_fsbulkstat(
	xfs_mount_t		  *mp,
	unsigned int		  cmd,
	struct compat_xfs_fsop_bulkreq __user *p32)
{
	u32			addr;
	struct xfs_fsop_bulkreq	bulkreq;
	struct xfs_ibulk	breq = {
		.mp		= mp,
		.ocount		= 0,
	};
	xfs_ino_t		lastino;
	int			error;

	/*
	 * Output structure handling functions.  Depending on the command,
	 * either the xfs_bstat and xfs_inogrp structures are written out
	 * to userpace memory via bulkreq.ubuffer.  Normally the compat
	 * functions and structure size are the correct ones to use ...
	 */
	inumbers_fmt_pf		inumbers_func = xfs_fsinumbers_fmt_compat;
	bulkstat_one_fmt_pf	bs_one_func_old = xfs_fsbulkstat_time32_one_fmt;

#ifdef CONFIG_X86_X32
	if (in_x32_syscall()) {
		/*
		 * ... but on x32 the input xfs_fsop_bulkreq has pointers
		 * which must be handled in the "compat" (32-bit) way, while
		 * the xfs_bstat and xfs_inogrp structures follow native 64-
		 * bit layout convention.  So adjust accordingly, otherwise
		 * the data written out in compat layout will not match what
		 * x32 userspace expects.
		 */
		inumbers_func = xfs_fsinumbers_fmt;
		bs_one_func_old = xfs_fsbulkstat_one_fmt;
	}
#endif

	/* done = 1 if there are more stats to get and if bulkstat */
	/* should be called again (unused here, but used in dmapi) */

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;

	if (XFS_FORCED_SHUTDOWN(mp))
		return -EIO;

	if (get_user(addr, &p32->lastip))
		return -EFAULT;
	bulkreq.lastip = compat_ptr(addr);
	if (get_user(bulkreq.icount, &p32->icount) ||
	    get_user(addr, &p32->ubuffer))
		return -EFAULT;
	bulkreq.ubuffer = compat_ptr(addr);
	if (get_user(addr, &p32->ocount))
		return -EFAULT;
	bulkreq.ocount = compat_ptr(addr);

	if (copy_from_user(&lastino, bulkreq.lastip, sizeof(__s64)))
		return -EFAULT;

	if (bulkreq.icount <= 0)
		return -EINVAL;

	if (bulkreq.ubuffer == NULL)
		return -EINVAL;

	breq.ubuffer = bulkreq.ubuffer;
	breq.icount = bulkreq.icount;

	/*
	 * FSBULKSTAT_SINGLE expects that *lastip contains the inode number
	 * that we want to stat.  However, FSINUMBERS and FSBULKSTAT expect
	 * that *lastip contains either zero or the number of the last inode to
	 * be examined by the previous call and return results starting with
	 * the next inode after that.  The new bulk request back end functions
	 * take the inode to start with, so we have to compute the startino
	 * parameter from lastino to maintain correct function.  lastino == 0
	 * is a special case because it has traditionally meant "first inode
	 * in filesystem".
	 */
	switch (cmd) {
	case XFS_IOC_FSINUMBERS_32:
		breq.startino = lastino ? lastino + 1 : 0;
		error = xfs_inumbers(&breq, inumbers_func);
		lastino = breq.startino - 1;
		break;
	case XFS_IOC_FSBULKSTAT_SINGLE_OLD32:
		breq.startino = lastino;
		breq.icount = 1;
		error = xfs_bulkstat_one(&breq, bs_one_func_old);
		lastino = breq.startino;
		break;
	case XFS_IOC_FSBULKSTAT_OLD32:
		breq.startino = lastino ? lastino + 1 : 0;
		error = xfs_bulkstat(&breq, bs_one_func_old);
		lastino = breq.startino - 1;
		break;
	case XFS_IOC_FSBULKSTAT_SINGLE_NEW32:
		breq.startino = lastino;
		breq.icount = 1;
		error = xfs_bulkstat_one(&breq, xfs_fsbulkstat_one_fmt);
		lastino = breq.startino;
		break;
	case XFS_IOC_FSBULKSTAT_NEW32:
		breq.startino = lastino ? lastino + 1 : 0;
		error = xfs_bulkstat(&breq, xfs_fsbulkstat_one_fmt);
		lastino = breq.startino - 1;
		break;
	default:
		error = -EINVAL;
		break;
	}
	if (error)
		return error;

	if (bulkreq.lastip != NULL &&
	    copy_to_user(bulkreq.lastip, &lastino, sizeof(xfs_ino_t)))
		return -EFAULT;

	if (bulkreq.ocount != NULL &&
	    copy_to_user(bulkreq.ocount, &breq.ocount, sizeof(__s32)))
		return -EFAULT;

	return 0;
}

STATIC int
xfs_compat_handlereq_copyin(
	xfs_fsop_handlereq_t		*hreq,
	compat_xfs_fsop_handlereq_t	__user *arg32)
{
	compat_xfs_fsop_handlereq_t	hreq32;

	if (copy_from_user(&hreq32, arg32, sizeof(compat_xfs_fsop_handlereq_t)))
		return -EFAULT;

	hreq->fd = hreq32.fd;
	hreq->path = compat_ptr(hreq32.path);
	hreq->oflags = hreq32.oflags;
	hreq->ihandle = compat_ptr(hreq32.ihandle);
	hreq->ihandlen = hreq32.ihandlen;
	hreq->ohandle = compat_ptr(hreq32.ohandle);
	hreq->ohandlen = compat_ptr(hreq32.ohandlen);

	return 0;
}

STATIC struct dentry *
xfs_compat_handlereq_to_dentry(
	struct file		*parfilp,
	compat_xfs_fsop_handlereq_t *hreq)
{
	return xfs_handle_to_dentry(parfilp,
			compat_ptr(hreq->ihandle), hreq->ihandlen);
}

STATIC int
xfs_compat_attrlist_by_handle(
	struct file		*parfilp,
	void			__user *arg)
{
	int			error;
	attrlist_cursor_kern_t	*cursor;
	compat_xfs_fsop_attrlist_handlereq_t __user *p = arg;
	compat_xfs_fsop_attrlist_handlereq_t al_hreq;
	struct dentry		*dentry;
	char			*kbuf;

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;
	if (copy_from_user(&al_hreq, arg,
			   sizeof(compat_xfs_fsop_attrlist_handlereq_t)))
		return -EFAULT;
	if (al_hreq.buflen < sizeof(struct attrlist) ||
	    al_hreq.buflen > XFS_XATTR_LIST_MAX)
		return -EINVAL;

	/*
	 * Reject flags, only allow namespaces.
	 */
	if (al_hreq.flags & ~(ATTR_ROOT | ATTR_SECURE))
		return -EINVAL;

	dentry = xfs_compat_handlereq_to_dentry(parfilp, &al_hreq.hreq);
	if (IS_ERR(dentry))
		return PTR_ERR(dentry);

	error = -ENOMEM;
	kbuf = kmem_zalloc_large(al_hreq.buflen, 0);
	if (!kbuf)
		goto out_dput;

	cursor = (attrlist_cursor_kern_t *)&al_hreq.pos;
	error = xfs_attr_list(XFS_I(d_inode(dentry)), kbuf, al_hreq.buflen,
					al_hreq.flags, cursor);
	if (error)
		goto out_kfree;

	if (copy_to_user(&p->pos, cursor, sizeof(attrlist_cursor_kern_t))) {
		error = -EFAULT;
		goto out_kfree;
	}

	if (copy_to_user(compat_ptr(al_hreq.buffer), kbuf, al_hreq.buflen))
		error = -EFAULT;

out_kfree:
	kmem_free(kbuf);
out_dput:
	dput(dentry);
	return error;
}

STATIC int
xfs_compat_attrmulti_by_handle(
	struct file				*parfilp,
	void					__user *arg)
{
	int					error;
	compat_xfs_attr_multiop_t		*ops;
	compat_xfs_fsop_attrmulti_handlereq_t	am_hreq;
	struct dentry				*dentry;
	unsigned int				i, size;
	unsigned char				*attr_name;

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;
	if (copy_from_user(&am_hreq, arg,
			   sizeof(compat_xfs_fsop_attrmulti_handlereq_t)))
		return -EFAULT;

	/* overflow check */
	if (am_hreq.opcount >= INT_MAX / sizeof(compat_xfs_attr_multiop_t))
		return -E2BIG;

	dentry = xfs_compat_handlereq_to_dentry(parfilp, &am_hreq.hreq);
	if (IS_ERR(dentry))
		return PTR_ERR(dentry);

	error = -E2BIG;
	size = am_hreq.opcount * sizeof(compat_xfs_attr_multiop_t);
	if (!size || size > 16 * PAGE_SIZE)
		goto out_dput;

	ops = memdup_user(compat_ptr(am_hreq.ops), size);
	if (IS_ERR(ops)) {
		error = PTR_ERR(ops);
		goto out_dput;
	}

	error = -ENOMEM;
	attr_name = kmalloc(MAXNAMELEN, GFP_KERNEL);
	if (!attr_name)
		goto out_kfree_ops;

	error = 0;
	for (i = 0; i < am_hreq.opcount; i++) {
		ops[i].am_error = strncpy_from_user((char *)attr_name,
				compat_ptr(ops[i].am_attrname),
				MAXNAMELEN);
		if (ops[i].am_error == 0 || ops[i].am_error == MAXNAMELEN)
			error = -ERANGE;
		if (ops[i].am_error < 0)
			break;

		switch (ops[i].am_opcode) {
		case ATTR_OP_GET:
			ops[i].am_error = xfs_attrmulti_attr_get(
					d_inode(dentry), attr_name,
					compat_ptr(ops[i].am_attrvalue),
					&ops[i].am_length, ops[i].am_flags);
			break;
		case ATTR_OP_SET:
			ops[i].am_error = mnt_want_write_file(parfilp);
			if (ops[i].am_error)
				break;
			ops[i].am_error = xfs_attrmulti_attr_set(
					d_inode(dentry), attr_name,
					compat_ptr(ops[i].am_attrvalue),
					ops[i].am_length, ops[i].am_flags);
			mnt_drop_write_file(parfilp);
			break;
		case ATTR_OP_REMOVE:
			ops[i].am_error = mnt_want_write_file(parfilp);
			if (ops[i].am_error)
				break;
			ops[i].am_error = xfs_attrmulti_attr_remove(
					d_inode(dentry), attr_name,
					ops[i].am_flags);
			mnt_drop_write_file(parfilp);
			break;
		default:
			ops[i].am_error = -EINVAL;
		}
	}

	if (copy_to_user(compat_ptr(am_hreq.ops), ops, size))
		error = -EFAULT;

	kfree(attr_name);
 out_kfree_ops:
	kfree(ops);
 out_dput:
	dput(dentry);
	return error;
}

STATIC int
xfs_compat_fssetdm_by_handle(
	struct file		*parfilp,
	void			__user *arg)
{
	int			error;
	struct fsdmidata	fsd;
	compat_xfs_fsop_setdm_handlereq_t dmhreq;
	struct dentry		*dentry;

	if (!capable(CAP_MKNOD))
		return -EPERM;
	if (copy_from_user(&dmhreq, arg,
			   sizeof(compat_xfs_fsop_setdm_handlereq_t)))
		return -EFAULT;

	dentry = xfs_compat_handlereq_to_dentry(parfilp, &dmhreq.hreq);
	if (IS_ERR(dentry))
		return PTR_ERR(dentry);

	if (IS_IMMUTABLE(d_inode(dentry)) || IS_APPEND(d_inode(dentry))) {
		error = -EPERM;
		goto out;
	}

	if (copy_from_user(&fsd, compat_ptr(dmhreq.data), sizeof(fsd))) {
		error = -EFAULT;
		goto out;
	}

	error = xfs_set_dmattrs(XFS_I(d_inode(dentry)), fsd.fsd_dmevmask,
				 fsd.fsd_dmstate);

out:
	dput(dentry);
	return error;
}

long
xfs_file_compat_ioctl(
	struct file		*filp,
	unsigned		cmd,
	unsigned long		p)
{
	struct inode		*inode = file_inode(filp);
	struct xfs_inode	*ip = XFS_I(inode);
	struct xfs_mount	*mp = ip->i_mount;
	void			__user *arg = compat_ptr(p);
#if defined(BROKEN_X86_ALIGNMENT)
	int			error;
#endif

	trace_xfs_file_compat_ioctl(ip);

	switch (cmd) {
#if defined(BROKEN_X86_ALIGNMENT)
	case XFS_IOC_ALLOCSP_32:
	case XFS_IOC_FREESP_32:
	case XFS_IOC_ALLOCSP64_32:
	case XFS_IOC_FREESP64_32:
	case XFS_IOC_RESVSP_32:
	case XFS_IOC_UNRESVSP_32:
	case XFS_IOC_RESVSP64_32:
	case XFS_IOC_UNRESVSP64_32:
	case XFS_IOC_ZERO_RANGE_32: {
		struct xfs_flock64	bf;

		if (xfs_compat_flock64_copyin(&bf, arg))
			return -EFAULT;
		cmd = _NATIVE_IOC(cmd, struct xfs_flock64);
		return xfs_ioc_space(filp, cmd, &bf);
	}
	case XFS_IOC_FSGEOMETRY_V1_32:
		return xfs_compat_ioc_fsgeometry_v1(mp, arg);
	case XFS_IOC_FSGROWFSDATA_32: {
		struct xfs_growfs_data	in;

		if (xfs_compat_growfs_data_copyin(&in, arg))
			return -EFAULT;
		error = mnt_want_write_file(filp);
		if (error)
			return error;
		error = xfs_growfs_data(mp, &in);
		mnt_drop_write_file(filp);
		return error;
	}
	case XFS_IOC_FSGROWFSRT_32: {
		struct xfs_growfs_rt	in;

		if (xfs_compat_growfs_rt_copyin(&in, arg))
			return -EFAULT;
		error = mnt_want_write_file(filp);
		if (error)
			return error;
		error = xfs_growfs_rt(mp, &in);
		mnt_drop_write_file(filp);
		return error;
	}
#endif
	/* long changes size, but xfs only copiese out 32 bits */
	case XFS_IOC_GETXFLAGS_32:
	case XFS_IOC_SETXFLAGS_32:
	case XFS_IOC_GETVERSION_32:
		cmd = _NATIVE_IOC(cmd, long);
		return xfs_file_ioctl(filp, cmd, p);
	case XFS_IOC_FSBULKSTAT_OLD32:
	case XFS_IOC_FSBULKSTAT_SINGLE_OLD32:
	case XFS_IOC_FSBULKSTAT_NEW32:
	case XFS_IOC_FSBULKSTAT_SINGLE_NEW32:
	case XFS_IOC_FSINUMBERS_32:
		return xfs_compat_ioc_fsbulkstat(mp, cmd, arg);
	case XFS_IOC_FD_TO_HANDLE_32:
	case XFS_IOC_PATH_TO_HANDLE_32:
	case XFS_IOC_PATH_TO_FSHANDLE_32: {
		struct xfs_fsop_handlereq	hreq;

		if (xfs_compat_handlereq_copyin(&hreq, arg))
			return -EFAULT;
		cmd = _NATIVE_IOC(cmd, struct xfs_fsop_handlereq);
		return xfs_find_handle(cmd, &hreq);
	}
	case XFS_IOC_OPEN_BY_HANDLE_32: {
		struct xfs_fsop_handlereq	hreq;

		if (xfs_compat_handlereq_copyin(&hreq, arg))
			return -EFAULT;
		return xfs_open_by_handle(filp, &hreq);
	}
	case XFS_IOC_READLINK_BY_HANDLE_32: {
		struct xfs_fsop_handlereq	hreq;

		if (xfs_compat_handlereq_copyin(&hreq, arg))
			return -EFAULT;
		return xfs_readlink_by_handle(filp, &hreq);
	}
	case XFS_IOC_ATTRLIST_BY_HANDLE_32:
		return xfs_compat_attrlist_by_handle(filp, arg);
	case XFS_IOC_ATTRMULTI_BY_HANDLE_32:
		return xfs_compat_attrmulti_by_handle(filp, arg);
	case XFS_IOC_FSSETDM_BY_HANDLE_32:
		return xfs_compat_fssetdm_by_handle(filp, arg);
	default:
		/* try the native version */
		return xfs_file_ioctl(filp, cmd, (unsigned long)arg);
	}
}
