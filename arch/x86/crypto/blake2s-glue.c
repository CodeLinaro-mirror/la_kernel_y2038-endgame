// SPDX-License-Identifier: GPL-2.0 OR MIT
/*
 * Copyright (C) 2015-2019 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#include <crypto/blake2s.h>
#include <crypto/internal/simd.h>

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <asm/cpufeature.h>
#include <asm/fpu/api.h>
#include <asm/processor.h>
#include <asm/simd.h>

asmlinkage void blake2s_compress_avx(struct blake2s_state *state,
				     const u8 *block, const size_t nblocks,
				     const u32 inc);
asmlinkage void blake2s_compress_avx512(struct blake2s_state *state,
					const u8 *block, const size_t nblocks,
					const u32 inc);

static bool blake2s_use_avx __ro_after_init;
static bool blake2s_use_avx512 __ro_after_init;

bool blake2s_compress_arch(struct blake2s_state *state,
			   const u8 *block, size_t nblocks,
			   const u32 inc)
{
	/* SIMD disables preemption, so relax after processing each page. */
	BUILD_BUG_ON(PAGE_SIZE / BLAKE2S_BLOCK_SIZE < 8);

	if (!blake2s_use_avx || !crypto_simd_usable())
		return false;

	for (;;) {
		const size_t blocks = min_t(size_t, nblocks,
					    PAGE_SIZE / BLAKE2S_BLOCK_SIZE);

		kernel_fpu_begin();
		if (IS_ENABLED(CONFIG_AS_AVX512) && blake2s_use_avx512)
			blake2s_compress_avx512(state, block, blocks, inc);
		else
			blake2s_compress_avx(state, block, blocks, inc);
		kernel_fpu_end();

		nblocks -= blocks;
		if (!nblocks)
			break;
		block += blocks * BLAKE2S_BLOCK_SIZE;
	}
	return true;
}
EXPORT_SYMBOL(blake2s_compress_arch);

static int __init blake2s_mod_init(void)
{
	blake2s_use_avx =
		boot_cpu_has(X86_FEATURE_AVX) &&
		cpu_has_xfeatures(XFEATURE_MASK_SSE | XFEATURE_MASK_YMM, NULL);
	blake2s_use_avx512 =
		boot_cpu_has(X86_FEATURE_AVX) &&
		boot_cpu_has(X86_FEATURE_AVX2) &&
		boot_cpu_has(X86_FEATURE_AVX512F) &&
		boot_cpu_has(X86_FEATURE_AVX512VL) &&
		cpu_has_xfeatures(XFEATURE_MASK_SSE | XFEATURE_MASK_YMM |
				  XFEATURE_MASK_AVX512, NULL);

	return 0;
}

module_init(blake2s_mod_init);

MODULE_LICENSE("GPL v2");
