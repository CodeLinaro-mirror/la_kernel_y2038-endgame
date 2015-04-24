#ifndef _LINUX_RESOURCE_H
#define _LINUX_RESOURCE_H

#ifndef CONFIG_COMPAT_TIME
#define __kernel_rusage rusage
#endif

#include <uapi/linux/resource.h>

struct task_struct;

int getrusage(struct task_struct *p, int who,
	      struct __kernel_rusage __user *ru);
int do_prlimit(struct task_struct *tsk, unsigned int resource,
		struct rlimit *new_rlim, struct rlimit *old_rlim);

#endif
