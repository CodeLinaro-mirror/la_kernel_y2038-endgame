/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _ASMPARISC_SIGCONTEXT_H
#define _ASMPARISC_SIGCONTEXT_H

#include <linux/types.h>

#define PARISC_SC_FLAG_ONSTACK 1<<0
#define PARISC_SC_FLAG_IN_SYSCALL 1<<1

/* We will add more stuff here as it becomes necessary, until we know
   it works. */
struct sigcontext {
	unsigned long sc_flags;

	unsigned long sc_gr[32]; /* PSW in sc_gr[0] */
	__uapi_arch_pad_long_to_u64;
	unsigned long long sc_fr[32]; /* FIXME, do we need other state info? */
	unsigned long sc_iasq[2];
	unsigned long sc_iaoq[2];
	unsigned long sc_sar; /* cr11 */
	__uapi_arch_pad_long_to_u64;
};


#endif
