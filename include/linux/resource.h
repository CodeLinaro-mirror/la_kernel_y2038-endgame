/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_RESOURCE_H
#define _LINUX_RESOURCE_H

#include <uapi/linux/resource.h>


struct task_struct;

void getrusage(struct task_struct *p, int who, struct __kernel_rusage *ru);
int do_prlimit(struct task_struct *tsk, unsigned int resource,
		struct rlimit *new_rlim, struct rlimit *old_rlim);

int put_rusage(const struct __kernel_rusage *rk, struct rusage __user *ru);

#endif
