/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _CRYPTO_INTERNAL_CHACHA_H
#define _CRYPTO_INTERNAL_CHACHA_H

#include <crypto/chacha.h>
#include <crypto/skcipher.h>
#include <linux/crypto.h>

struct chacha_ctx {
	u32 key[8];
	int nrounds;
};

void crypto_chacha_init(u32 *state, const struct chacha_ctx *ctx, const u8 *iv);

int crypto_chacha20_setkey(struct crypto_skcipher *tfm, const u8 *key,
			   unsigned int keysize);
int crypto_chacha12_setkey(struct crypto_skcipher *tfm, const u8 *key,
			   unsigned int keysize);

int crypto_chacha_crypt(struct skcipher_request *req);
int crypto_xchacha_crypt(struct skcipher_request *req);

#endif /* _CRYPTO_CHACHA_H */
