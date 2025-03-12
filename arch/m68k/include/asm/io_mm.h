/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linux/include/asm-m68k/io.h
 *
 * 4/1/00 RZ: - rewritten to avoid clashes between ISA/PCI and other
 *              IO access
 *            - added Q40 support
 *            - added skeleton for GG-II and Amiga PCMCIA
 * 2/3/01 RZ: - moved a few more defs into raw_io.h
 *
 * inX/outX should not be used by any driver unless it does
 * ISA access. Other drivers should use function defined in raw_io.h
 * or define its own macros on top of these.
 *
 *    inX(),outX()              are for ISA I/O
 *    isa_readX(),isa_writeX()  are for ISA memory
 */

#ifndef _M68K_IO_MM_H
#define _M68K_IO_MM_H

#ifdef __KERNEL__

#include <linux/compiler.h>
#include <asm/raw_io.h>
#include <asm/virtconvert.h>
#include <asm/kmap.h>

#ifdef CONFIG_ATARI
#define atari_readb   raw_inb
#define atari_writeb  raw_outb

#define atari_inb_p   raw_inb
#define atari_outb_p  raw_outb
#endif

#ifdef CONFIG_ISA

#define q40_isa_io_base  0xff400000
#define q40_isa_mem_base 0xff800000

#define Q40_ISA_IO_B(ioaddr) (q40_isa_io_base+1+4*((unsigned long)(ioaddr)))
#define Q40_ISA_IO_W(ioaddr) (q40_isa_io_base+  4*((unsigned long)(ioaddr)))
#define Q40_ISA_MEM_B(madr)  (q40_isa_mem_base+1+4*((unsigned long)(madr)))
#define Q40_ISA_MEM_W(madr)  (q40_isa_mem_base+  4*((unsigned long)(madr)))

/*
 * define inline addr translation functions. Normally only one variant will
 * be compiled in so the case statement will be optimised away
 */

static inline u8 __iomem *isa_itb(unsigned long addr)
{
	return (u8 __iomem *)Q40_ISA_IO_B(addr);
}
static inline u16 __iomem *isa_itw(unsigned long addr)
{
	return (u16 __iomem *)Q40_ISA_IO_W(addr);
}
static inline u32 __iomem *isa_itl(unsigned long addr)
{
	return 0; /* avoid warnings, just in case */
}
static inline u8 __iomem *isa_mtb(unsigned long addr)
{
	return (u8 __iomem *)Q40_ISA_MEM_B(addr);
}
static inline u16 __iomem *isa_mtw(unsigned long addr)
{
	return (u16 __iomem *)Q40_ISA_MEM_W(addr);
}

#define isa_inb(port)      in_8(isa_itb(port))
#define isa_inw(port)      in_le16(isa_itw(port))
#define isa_inl(port)      in_le32(isa_itl(port))
#define isa_outb(val,port) out_8(isa_itb(port),(val))
#define isa_outw(val,port) out_le16(isa_itw(port),(val))
#define isa_outl(val,port) out_le32(isa_itl(port),(val))

#define isa_readb(p)       in_8(isa_mtb((unsigned long)(p)))
#define isa_readw(p)       in_le16(isa_mtw((unsigned long)(p)))
#define isa_writeb(val,p)  out_8(isa_mtb((unsigned long)(p)),(val))
#define isa_writew(val,p)  out_le16(isa_mtw((unsigned long)(p)),(val))

static inline void isa_delay(void)
{
	isa_outb(0,0x80);
}

#define isa_inb_p(p)      ({u8 v=isa_inb(p);isa_delay();v;})
#define isa_outb_p(v,p)   ({isa_outb((v),(p));isa_delay();})
#define isa_inw_p(p)      ({u16 v=isa_inw(p);isa_delay();v;})
#define isa_outw_p(v,p)   ({isa_outw((v),(p));isa_delay();})
#define isa_inl_p(p)      ({u32 v=isa_inl(p);isa_delay();v;})
#define isa_outl_p(v,p)   ({isa_outl((v),(p));isa_delay();})

#define isa_insb(port, buf, nr) raw_insb(isa_itb(port), (u8 *)(buf), (nr))
#define isa_outsb(port, buf, nr) raw_outsb(isa_itb(port), (u8 *)(buf), (nr))

#define isa_insw(port, buf, nr)     \
		   raw_insw_swapw(isa_itw(port), (u16 *)(buf), (nr))

#define isa_outsw(port, buf, nr)    \
		   raw_outsw_swapw(isa_itw(port), (u16 *)(buf), (nr))

#define isa_insl(port, buf, nr)     \
		   raw_insw_swapw(isa_itw(port), (u16 *)(buf), (nr)<<1)

#define isa_outsl(port, buf, nr)    \
		   raw_outsw_swapw(isa_itw(port), (u16 *)(buf), (nr)<<1)

#define inb     isa_inb
#define inb_p   isa_inb_p
#define outb    isa_outb
#define outb_p  isa_outb_p
#define inw     isa_inw
#define inw_p   isa_inw_p
#define outw    isa_outw
#define outw_p  isa_outw_p
#define inl     isa_inl
#define inl_p   isa_inl_p
#define outl    isa_outl
#define outl_p  isa_outl_p
#define insb    isa_insb
#define insw    isa_insw
#define insl    isa_insl
#define outsb   isa_outsb
#define outsw   isa_outsw
#define outsl   isa_outsl
#endif  /* CONFIG_ISA */

#define readsb(port, buf, nr)     raw_insb((port), (u8 *)(buf), (nr))
#define readsw(port, buf, nr)     raw_insw((port), (u16 *)(buf), (nr))
#define readsl(port, buf, nr)     raw_insl((port), (u32 *)(buf), (nr))
#define writesb(port, buf, nr)    raw_outsb((port), (u8 *)(buf), (nr))
#define writesw(port, buf, nr)    raw_outsw((port), (u16 *)(buf), (nr))
#define writesl(port, buf, nr)    raw_outsl((port), (u32 *)(buf), (nr))

#ifndef CONFIG_SUN3
#define IO_SPACE_LIMIT 0xffff
#else
#define IO_SPACE_LIMIT 0x0fffffff
#endif

#endif /* __KERNEL__ */

#define __ARCH_HAS_NO_PAGE_ZERO_MAPPED		1

#endif /* _M68K_IO_MM_H */
