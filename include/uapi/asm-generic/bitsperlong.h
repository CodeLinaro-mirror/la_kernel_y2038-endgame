#ifndef _UAPI__ASM_GENERIC_BITS_PER_LONG
#define _UAPI__ASM_GENERIC_BITS_PER_LONG

/*
 * There seems to be no way of detecting this automatically from user
 * space, so 64 bit architectures should override this in their
 * bitsperlong.h. In particular, an architecture that supports
 * both 32 and 64 bit user space must not rely on CONFIG_64BIT
 * to decide it, but rather check a compiler provided macro.
 */
#ifndef __BITS_PER_LONG
#define __BITS_PER_LONG 32
#endif

/*
 * Traditionally we define defines 'time_t' as 'long', but we need to
 * migrate to a 64-bit type until 2038. This one is designed to be
 * overridden by user space if it's prepared to handle 64-bit time_t.
 */
#ifndef __KERNEL_TIME_BITS
#define __KERNEL_TIME_BITS __BITS_PER_LONG
#endif

#endif /* _UAPI__ASM_GENERIC_BITS_PER_LONG */
