/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef __LINUX_BRIDGE_EBT_ARPREPLY_H
#define __LINUX_BRIDGE_EBT_ARPREPLY_H

#include <linux/if_ether.h>

struct ebt_arpreply_info {
	unsigned char mac[ETH_ALEN];
	__uapi_arch_pad16;
	int target;
} __uapi_arch_align;
#define EBT_ARPREPLY_TARGET "arpreply"

#endif
