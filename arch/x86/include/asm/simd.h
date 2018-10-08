/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2015-2018 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#include <linux/simd.h>
#ifndef _ASM_SIMD_H
#define _ASM_SIMD_H

#include <asm/fpu/api.h>

/*
 * may_use_simd - whether it is allowable at this time to issue SIMD
 *                instructions or access the SIMD register file
 */
static __must_check inline bool may_use_simd(void)
{
	return irq_fpu_usable();
}

static inline void simd_get(simd_context_t *ctx)
{
#if !defined(CONFIG_UML)
	*ctx = may_use_simd() ? HAVE_FULL_SIMD : HAVE_NO_SIMD;
#else
	*ctx = HAVE_NO_SIMD;
#endif
}

static inline void simd_put(simd_context_t *ctx)
{
#if !defined(CONFIG_UML)
	if (*ctx & HAVE_SIMD_IN_USE)
		kernel_fpu_end();
#endif
	*ctx = HAVE_NO_SIMD;
}

static __must_check inline bool simd_use(simd_context_t *ctx)
{
#if !defined(CONFIG_UML)
	if (!(*ctx & HAVE_FULL_SIMD))
		return false;
	if (*ctx & HAVE_SIMD_IN_USE)
		return true;
	kernel_fpu_begin();
	*ctx |= HAVE_SIMD_IN_USE;
	return true;
#else
	return false;
#endif
}

#endif /* _ASM_SIMD_H */
