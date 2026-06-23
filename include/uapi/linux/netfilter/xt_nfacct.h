/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _XT_NFACCT_MATCH_H
#define _XT_NFACCT_MATCH_H

#include <linux/netfilter/nfnetlink_acct.h>
#include <linux/types.h>

struct nf_acct;

struct xt_nfacct_match_info {
	char		name[NFACCT_NAME_MAX];
	struct nf_acct	*nfacct;
};

struct xt_nfacct_match_info_v1 {
	char		name[NFACCT_NAME_MAX];

	/* Used internally by the kernel */
	struct nf_acct	*nfacct __attribute__((aligned(8)));
	__uapi_arch_pad_long_to_aligned_u64;
};

#endif /* _XT_NFACCT_MATCH_H */
