/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _XT_HELPER_H
#define _XT_HELPER_H

#include <linux/types.h>

struct xt_helper_info {
	int invert;
	char name[30];
	__uapi_arch_pad16;
} __uapi_arch_align;
#endif /* _XT_HELPER_H */
