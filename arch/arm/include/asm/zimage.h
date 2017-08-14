/*
 * Copyright (C) 2017 Linaro Ltd;  <ard.biesheuvel@linaro.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __ASM_ZIMAGE_H
#define __ASM_ZIMAGE_H

#define ZIMAGE_HEADER_MAGIC		0x016f2818
#define ZIMAGE_OPTIONAL_HEADER_MAGIC	0xe7fedef0

#if defined(__ASSEMBLY__) && !defined(LINKER_SCRIPT)

	.macro	le_short, x
	.byte	(\x) & 0xff
	.byte	((\x) >> 8) & 0xff
	.endm

	.macro	le_long, x
	.byte	(\x) & 0xff
	.byte	((\x) >> 8) & 0xff
	.byte	((\x) >> 16) & 0xff
	.byte	((\x) >> 24) & 0xff
	.endm

	.macro		__ZIMAGE_HEADER
	le_long		ZIMAGE_HEADER_MAGIC
	.word		_magic_start		@ absolute load/run zImage address
	.word		_magic_end		@ zImage end address
	.word		0x04030201		@ endianness flag

	/* optional headers */
	le_long		ZIMAGE_OPTIONAL_HEADER_MAGIC
ENTRY(__zimage_opt_header_ref)
	.word		__zimage_opt_header_offset_le

	.pushsection	".rodata", "a", %progbits
	.align		2
ENTRY(__zimage_opt_header)
	/*
	 * Each header starts with a u16[2] containing id and size of the
	 * entire header, including the u16[] itself.
	 */
	.long		0xffffffff	@ end of optional headers
	.popsection
	.endm

#endif
#endif
