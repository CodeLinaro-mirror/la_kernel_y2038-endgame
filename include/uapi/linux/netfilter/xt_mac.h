/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _XT_MAC_H
#define _XT_MAC_H

#include <linux/if_ether.h>

struct xt_mac_info {
	unsigned char srcaddr[ETH_ALEN];
	__uapi_arch_pad16;
	int invert;
} __uapi_arch_align;
#endif /*_XT_MAC_H*/
