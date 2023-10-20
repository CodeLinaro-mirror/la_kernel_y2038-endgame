/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2023 Hannes Reinecke, SUSE Labs
 */

#ifndef _NVME_KEYRING_H
#define _NVME_KEYRING_H

/* internal helpers only, don't call directly */
key_serial_t nvme_tls_psk_default(struct key *keyring,
		const char *hostnqn, const char *subnqn);

key_serial_t nvme_keyring_id(void);
int nvme_keyring_init(void);
void nvme_keyring_exit(void);

static inline key_serial_t nvme_host_tls_psk_default(struct key *keyring,
		const char *hostnqn, const char *subnqn)
{
	if (IS_ENABLED(CONFIG_NVME_TCP_TLS))
		return nvme_tls_psk_default(keyring, hostnqn, subnqn);

	return 0;
}
static inline key_serial_t nvme_host_keyring_id(void)
{
	if (IS_ENABLED(CONFIG_NVME_TCP_TLS))
		return nvme_keyring_id();

	return 0;
}
static inline int nvme_host_keyring_init(void)
{
	if (IS_ENABLED(CONFIG_NVME_TCP_TLS))
		return nvme_keyring_init();

	return 0;
}
static inline void nvme_host_keyring_exit(void)
{
	if (IS_ENABLED(CONFIG_NVME_TCP_TLS))
		nvme_keyring_exit();
}

static inline key_serial_t nvme_target_tls_psk_default(struct key *keyring,
		const char *hostnqn, const char *subnqn)
{
	if (IS_ENABLED(CONFIG_NVME_TARGET_TCP_TLS))
		return nvme_host_tls_psk_default(keyring, hostnqn, subnqn);

	return 0;
}

static inline key_serial_t nvme_target_keyring_id(void)
{
	if (IS_ENABLED(CONFIG_NVME_TARGET_TCP_TLS))
		return nvme_keyring_id();

	return 0;
}

static inline int nvme_target_keyring_init(void)
{
	if (IS_ENABLED(CONFIG_NVME_TCP_TLS))
		return nvme_keyring_init();

	return 0;
}

static inline void nvme_target_keyring_exit(void)
{
	if (IS_ENABLED(CONFIG_NVME_TARGET_TCP_TLS))
		nvme_keyring_exit();
}

#endif /* _NVME_KEYRING_H */
