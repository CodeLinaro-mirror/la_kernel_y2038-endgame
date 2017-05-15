#ifndef _ASM_GENERIC_BITOPS_FIND_H_
#define _ASM_GENERIC_BITOPS_FIND_H_

#ifndef find_next_bit
/**
 * find_next_bit - find the next set bit in a memory region
 * @addr: The address to base the search on
 * @offset: The bitnumber to start searching at
 * @size: The bitmap size in bits
 *
 * Returns the bit number for the next set bit
 * If no bits are set, returns @size.
 */
extern unsigned long find_next_bit(const unsigned long *addr, unsigned long
		size, unsigned long offset);
#endif

#ifndef find_next_zero_bit
/**
 * find_next_zero_bit - find the next cleared bit in a memory region
 * @addr: The address to base the search on
 * @offset: The bitnumber to start searching at
 * @size: The bitmap size in bits
 *
 * Returns the bit number of the next zero bit
 * If no bits are zero, returns @size.
 */
extern unsigned long find_next_zero_bit(const unsigned long *addr, unsigned
		long size, unsigned long offset);
#endif

#ifdef CONFIG_GENERIC_FIND_FIRST_BIT
/**
 * find_first_bit - find the first set bit in a memory region
 * @addr: The address to start the search at
 * @size: The maximum number of bits to search
 *
 * Returns the bit number of the first set bit.
 * If no bits are set, returns @size.
 */
extern unsigned long __find_first_bit(const unsigned long *addr,
				    unsigned long size);

static inline unsigned long find_first_bit(const unsigned long *addr,
				    unsigned long size)
{
	unsigned long idx;

	if (!__builtin_constant_p(size))
		return __find_first_bit(addr, size);

	idx = 0;
	switch (size) {
	case BITS_PER_LONG * 4:
		if (addr[0])
			return __ffs(addr[0]) + idx;
		addr++;
		idx += BITS_PER_LONG;
	case BITS_PER_LONG * 3:
		if (addr[0])
			return __ffs(addr[0]) + idx;
		addr++;
		idx += BITS_PER_LONG;
	case BITS_PER_LONG * 2:
		if (addr[0])
			return __ffs(addr[0]) + idx;
		addr++;
		idx += BITS_PER_LONG;
	case BITS_PER_LONG * 1:
		if (addr[0])
			return __ffs(addr[0]) + idx;
		addr++;
		idx += BITS_PER_LONG;
		return idx;
	}

	return __find_first_bit(addr, size);
}

/**
 * find_first_zero_bit - find the first cleared bit in a memory region
 * @addr: The address to start the search at
 * @size: The maximum number of bits to search
 *
 * Returns the bit number of the first cleared bit.
 * If no bits are zero, returns @size.
 */
extern unsigned long find_first_zero_bit(const unsigned long *addr,
					 unsigned long size);
#else /* CONFIG_GENERIC_FIND_FIRST_BIT */

#define find_first_bit(addr, size) find_next_bit((addr), (size), 0)
#define find_first_zero_bit(addr, size) find_next_zero_bit((addr), (size), 0)

#endif /* CONFIG_GENERIC_FIND_FIRST_BIT */

#endif /*_ASM_GENERIC_BITOPS_FIND_H_ */
