/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Common values for the Poly1305 algorithm
 */

#ifndef _CRYPTO_INTERNAL_POLY1305_H
#define _CRYPTO_INTERNAL_POLY1305_H

#include <linux/types.h>
#include <crypto/poly1305.h>

struct shash_desc;

/*
 * Poly1305 core functions.  These implement the ε-almost-∆-universal hash
 * function underlying the Poly1305 MAC, i.e. they don't add an encrypted nonce
 * ("s key") at the end.  They also only support block-aligned inputs.
 */
void poly1305_core_setkey(struct poly1305_key *key, const u8 *raw_key);
static inline void poly1305_core_init(struct poly1305_state *state)
{
	*state = (struct poly1305_state){};
}

void poly1305_core_blocks(struct poly1305_state *state,
			  const struct poly1305_key *key, const void *src,
			  unsigned int nblocks, u32 hibit);
void poly1305_core_emit(const struct poly1305_state *state, void *dst);

void poly1305_init_generic(struct poly1305_desc_ctx *desc, const u8 *key);

void poly1305_update_generic(struct poly1305_desc_ctx *desc, const u8 *src,
			     unsigned int nbytes);

void poly1305_final_generic(struct poly1305_desc_ctx *desc, u8 *digest);

/* Crypto API helper functions for the Poly1305 MAC */
int crypto_poly1305_init(struct shash_desc *desc);
unsigned int crypto_poly1305_setdesckey(struct poly1305_desc_ctx *dctx,
					const u8 *src, unsigned int srclen);
int crypto_poly1305_update(struct shash_desc *desc,
			   const u8 *src, unsigned int srclen);
int crypto_poly1305_final(struct shash_desc *desc, u8 *dst);

#endif
