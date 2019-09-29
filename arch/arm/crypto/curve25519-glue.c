// SPDX-License-Identifier: GPL-2.0 OR MIT
/*
 * Copyright (C) 2015-2019 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 *
 * Based on public domain code from Daniel J. Bernstein and Peter Schwabe. This
 * began from SUPERCOP's curve25519/neon2/scalarmult.s, but has subsequently been
 * manually reworked for use in kernel space.
 */

#include <asm/hwcap.h>
#include <asm/neon.h>
#include <asm/simd.h>
#include <crypto/internal/simd.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/init.h>
#include <crypto/curve25519.h>

asmlinkage void curve25519_neon(u8 mypublic[CURVE25519_KEY_SIZE],
				const u8 secret[CURVE25519_KEY_SIZE],
				const u8 basepoint[CURVE25519_KEY_SIZE]);

static bool have_neon __ro_after_init;

bool curve25519_arch(u8 out[CURVE25519_KEY_SIZE],
		     const u8 scalar[CURVE25519_KEY_SIZE],
		     const u8 point[CURVE25519_KEY_SIZE])
{
	if (!have_neon || !crypto_simd_usable())
		return false;
	kernel_neon_begin();
	curve25519_neon(out, scalar, point);
	kernel_neon_end();
	return true;
}
EXPORT_SYMBOL(curve25519_arch);

static int __init mod_init(void)
{
	have_neon = (elf_hwcap & HWCAP_NEON);
	return 0;
}

module_init(mod_init);
MODULE_LICENSE("GPL v2");
