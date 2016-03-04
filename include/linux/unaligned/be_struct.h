#ifndef _LINUX_UNALIGNED_BE_STRUCT_H
#define _LINUX_UNALIGNED_BE_STRUCT_H

#include <linux/unaligned/packed_struct.h>
#include <asm/byteorder.h>

static inline u16 get_unaligned_be16(const void *p)
{
	return be16_to_cpu((__le16 __force)__get_unaligned_cpu16((const u8 *)p));
}

static inline u32 get_unaligned_be32(const void *p)
{
	return be32_to_cpu((__le32 __force)__get_unaligned_cpu32((const u8 *)p));
}

static inline u64 get_unaligned_be64(const void *p)
{
	return be64_to_cpu((__le64 __force)__get_unaligned_cpu64((const u8 *)p));
}

static inline void put_unaligned_be16(u16 val, void *p)
{
	__put_unaligned_cpu16((u16 __force)cpu_to_be16(val), p);
}

static inline void put_unaligned_be32(u32 val, void *p)
{
	__put_unaligned_cpu32((u32 __force)cpu_to_be32(val), p);
}

static inline void put_unaligned_be64(u64 val, void *p)
{
	__put_unaligned_cpu64((u64 __force)cpu_to_be64(val), p);
}

#endif /* _LINUX_UNALIGNED_BE_STRUCT_H */
