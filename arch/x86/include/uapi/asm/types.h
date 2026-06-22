#ifndef __X86_ASM_TYPES_H
#define __X86_ASM_TYPES_H

/* i386 has only aligns 64-bit types to 32-bit addresses */
#ifndef __x86_64
#define __uapi_arch_pad32
#endif

#include <asm-generic/int-ll64.h>

#endif
