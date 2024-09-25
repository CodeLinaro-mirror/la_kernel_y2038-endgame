/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_MMAN_H__
#define __ASM_MMAN_H__

#include <asm/system_info.h>
#include <asm-generic/mman.h>

static inline bool arch_memory_deny_write_exec_supported(void)
{
	return cpu_architecture() >= CPU_ARCH_ARMv6;
}
#define arch_memory_deny_write_exec_supported arch_memory_deny_write_exec_supported

#define arch_mmap_check(addr, len, flags) \
	(((flags) & MAP_FIXED && (addr) < FIRST_USER_ADDRESS) ? -EINVAL : 0)

#endif /* __ASM_MMAN_H__ */
