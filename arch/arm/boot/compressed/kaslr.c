/*
 * Copyright (C) 2017 Linaro Ltd;  <ard.biesheuvel@linaro.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <libfdt.h>
#include <linux/types.h>

#include <asm/pgtable.h>

struct regions {
	u32 pa_start;
	u32 pa_end;
	u32 image_size;
	u32 zimage_start;
	u32 zimage_size;
	u32 dtb_start;
	u32 dtb_size;
	u32 initrd_start;
	u32 initrd_size;
	int reserved_mem;
	int reserved_mem_addr_cells;
	int reserved_mem_size_cells;
};

/** CRC table for the CRC-16. The poly is 0x8005 (x^16 + x^15 + x^2 + 1) */
static u16 const crc16_table[256] __cacheline_aligned = {
	0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
	0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
	0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
	0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
	0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
	0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
	0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
	0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
	0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
	0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
	0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
	0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
	0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
	0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
	0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
	0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
	0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
	0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
	0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
	0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
	0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
	0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
	0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
	0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
	0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
	0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
	0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
	0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
	0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
	0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
	0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
	0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

static u16 crc16(u16 crc, u8 const *buffer, size_t len)
{
	while (len--)
		crc = (crc >> 8) ^ crc16_table[(crc ^ *buffer++) & 0xff];
	return crc;
}

static u32 __memparse(const char *val, const char **retptr)
{
	int base = 10;
	u32 ret = 0;

	if (*val == '0') {
		val++;
		if (*val == 'x' || *val == 'X') {
			val++;
			base = 16;
		} else {
			base = 8;
		}
	}

	while (*val != ',' && *val != ' ' && *val != '\0') {
		char c = *val++;

		switch (c) {
		case '0' ... '9':
			ret = ret * base + (c - '0');
			continue;
		case 'a' ... 'f':
			ret = ret * base + (c - 'a' + 10);
			continue;
		case 'A' ... 'F':
			ret = ret * base + (c - 'A' + 10);
			continue;
		case 'g':
		case 'G':
			ret <<= 10;
		case 'm':
		case 'M':
			ret <<= 10;
		case 'k':
		case 'K':
			ret <<= 10;
			break;
		default:
			if (retptr)
				*retptr = NULL;
			return 0;
		}
	}
	if (retptr)
		*retptr = val;
	return ret;
}

static bool regions_intersect(u32 s1, u32 e1, u32 s2, u32 e2)
{
	return e1 >= s2 && e2 >= s1;
}

static bool intersects_reserved_region(const void *fdt, u32 start,
				       u32 end, struct regions *regions)
{
	int subnode, len, i;
	u64 base, size;

	/* check for overlap with /memreserve/ entries */
	for (i = 0; i < fdt_num_mem_rsv(fdt); i++) {
		if (fdt_get_mem_rsv(fdt, i, &base, &size) < 0)
			continue;
		if (regions_intersect(start, end, base, base + size))
			return true;
	}

	if (regions->reserved_mem < 0)
		return false;

	/* check for overlap with static reservations in /reserved-memory */
	for (subnode = fdt_first_subnode(fdt, regions->reserved_mem);
	     subnode >= 0;
	     subnode = fdt_next_subnode(fdt, subnode)) {
		const fdt32_t *reg;

		len = 0;
		reg = fdt_getprop(fdt, subnode, "reg", &len);
		while (len >= (regions->reserved_mem_addr_cells +
			       regions->reserved_mem_size_cells)) {

			base = fdt32_to_cpu(reg[0]);
			if (regions->reserved_mem_addr_cells == 2)
				base = (base << 32) | fdt32_to_cpu(reg[1]);

			reg += regions->reserved_mem_addr_cells;
			len -= 4 * regions->reserved_mem_addr_cells;

			size = fdt32_to_cpu(reg[0]);
			if (regions->reserved_mem_size_cells == 2)
				size = (size << 32) | fdt32_to_cpu(reg[1]);

			reg += regions->reserved_mem_size_cells;
			len -= 4 * regions->reserved_mem_size_cells;

			if (base >= regions->pa_end)
				continue;

			if (regions_intersect(start, end, base,
					      min(base + size, (u64)U32_MAX)))
				return true;
		}
	}
	return false;
}

static bool intersects_occupied_region(const void *fdt, u32 start,
				       u32 end, struct regions *regions)
{
	if (regions_intersect(start, end, regions->zimage_start,
			      regions->zimage_start + regions->zimage_size))
		return true;

	if (regions_intersect(start, end, regions->initrd_start,
			      regions->initrd_start + regions->initrd_size))
		return true;

	if (regions_intersect(start, end, regions->dtb_start,
			      regions->dtb_start + regions->dtb_size))
		return true;

	return intersects_reserved_region(fdt, start, end, regions);
}

static u32 count_suitable_regions(const void *fdt, struct regions *regions)
{
	u32 pa, ret = 0;

	for (pa = regions->pa_start; pa < regions->pa_end; pa += SZ_2M) {
		if (!intersects_occupied_region(fdt, pa,
						pa + regions->image_size,
						regions))
			ret++;
	}
	return ret;
}

static u32 get_numbered_region(const void *fdt,
					 struct regions *regions,
					 int num)
{
	u32 pa;

	for (pa = regions->pa_start; pa < regions->pa_end; pa += SZ_2M) {
		if (!intersects_occupied_region(fdt, pa,
						pa + regions->image_size,
						regions))
			if (num-- == 0)
				return pa;
	}
	return regions->pa_start; /* should not happen */
}

static void get_cell_sizes(const void *fdt, int node, int *addr_cells,
			   int *size_cells)
{
	const int *prop;
	int len;

	/*
	 * Retrieve the #address-cells and #size-cells properties
	 * from the 'node', or use the default if not provided.
	 */
	*addr_cells = *size_cells = 1;

	prop = fdt_getprop(fdt, node, "#address-cells", &len);
	if (len == 4)
		*addr_cells = fdt32_to_cpu(*prop);
	prop = fdt_getprop(fdt, node, "#size-cells", &len);
	if (len == 4)
		*size_cells = fdt32_to_cpu(*prop);
}

static u32 get_memory_end(const void *fdt)
{
	int mem_node, address_cells, size_cells, len;
	const fdt32_t *reg;
	u64 memory_end = 0;

	/* Look for a node called "memory" at the lowest level of the tree */
	mem_node = fdt_path_offset(fdt, "/memory");
	if (mem_node <= 0)
		return 0;

	get_cell_sizes(fdt, 0, &address_cells, &size_cells);

	/*
	 * Now find the 'reg' property of the /memory node, and iterate over
	 * the base/size pairs.
	 */
	len = 0;
	reg = fdt_getprop (fdt, mem_node, "reg", &len);
	while (len >= 4 * (address_cells + size_cells)) {
		u64 base, size;

		base = fdt32_to_cpu(reg[0]);
		if (address_cells == 2)
			base = (base << 32) | fdt32_to_cpu(reg[1]);

		reg += address_cells;
		len -= 4 * address_cells;

		size = fdt32_to_cpu(reg[0]);
		if (size_cells == 2)
			size = (size << 32) | fdt32_to_cpu(reg[1]);

		reg += size_cells;
		len -= 4 * size_cells;

		memory_end = max(memory_end, base + size);
	}
	return min(memory_end, (u64)U32_MAX);
}

static char *__strstr(const char *s1, const char *s2, int l2)
{
	int l1;

	l1 = strlen(s1);
	while (l1 >= l2) {
		l1--;
		if (!memcmp(s1, s2, l2))
			return (char *)s1;
		s1++;
	}
	return NULL;
}

static const char *get_cmdline_param(const char *cmdline, const char *param,
				     int param_size)
{
	static const char default_cmdline[] = CONFIG_CMDLINE;
	const char *p;

	if (!IS_ENABLED(CONFIG_CMDLINE_FORCE) && cmdline != NULL) {
		p = __strstr(cmdline, param, param_size);
		if (p == cmdline ||
		    (p > cmdline && *(p - 1) == ' '))
			return p;
	}

	if (IS_ENABLED(CONFIG_CMDLINE_FORCE)  ||
	    IS_ENABLED(CONFIG_CMDLINE_EXTEND)) {
		p = __strstr(default_cmdline, param, param_size);
		if (p == default_cmdline ||
		    (p > default_cmdline && *(p - 1) == ' '))
			return p;
	}
	return NULL;
}

u32 kaslr_early_init(u32 *kaslr_offset, u32 image_base, u32 image_size,
		     u16 seed, u32 zimage_start, const void *fdt,
		     u32 zimage_end)
{
	struct regions regions;
	const char *command_line;
	const char *p;
	int chosen, len;
	u32 lowmem_top, num;

	if (fdt_check_header(fdt))
		return 0;

	chosen = fdt_path_offset(fdt, "/chosen");
	if (chosen < 0)
		return 0;

	command_line = fdt_getprop(fdt, chosen, "bootargs", &len);

	/* check the command line for the presence of 'nokaslr' */
	p = get_cmdline_param(command_line, "nokaslr", sizeof("nokaslr") - 1);
	if (p != NULL)
		return 0;

	/* check the command line for the presence of 'vmalloc=' */
	p = get_cmdline_param(command_line, "vmalloc=", sizeof("vmalloc=") - 1);
	if (p != NULL)
		lowmem_top = VMALLOC_END - __memparse(p + 8, NULL) -
			     VMALLOC_OFFSET;
	else
		lowmem_top = VMALLOC_DEFAULT_BASE;

	regions.pa_start = round_down(image_base, SZ_128M);
	regions.pa_end = lowmem_top - PAGE_OFFSET + regions.pa_start;
	regions.image_size = round_up(image_size, SZ_2M);
	regions.zimage_start = zimage_start;
	regions.zimage_size = zimage_end - zimage_start;
	regions.dtb_start = (u32)fdt;
	regions.dtb_size = fdt_totalsize(fdt);

	/*
	 * Stir up the seed a bit by taking the CRC of the DTB:
	 * hopefully there's a /chosen/kaslr-seed in there.
	 */
	seed = crc16(seed, fdt, regions.dtb_size);

	/* check for initrd on the command line */
	regions.initrd_start = regions.initrd_size = 0;
	p = get_cmdline_param(command_line, "initrd=", sizeof("initrd=") - 1);
	if (p != NULL) {
		regions.initrd_start = __memparse(p + 7, &p);
		if (*p++ == ',')
			regions.initrd_size = __memparse(p, NULL);
		if (regions.initrd_size == 0)
			regions.initrd_start = 0;
	}

	/* ... or in /chosen */
	if (regions.initrd_size == 0) {
		const fdt32_t *prop;
		u64 start = 0, end = 0;

		prop = fdt_getprop(fdt, chosen, "linux,initrd-start", &len);
		if (prop) {
			start = fdt32_to_cpu(prop[0]);
			if (len == 8)
				start = (start << 32) | fdt32_to_cpu(prop[1]);
		}

		prop = fdt_getprop(fdt, chosen, "linux,initrd-end", &len);
		if (prop) {
			end = fdt32_to_cpu(prop[0]);
			if (len == 8)
				end = (end << 32) | fdt32_to_cpu(prop[1]);
		}
		if (start != 0 && end != 0 && start < U32_MAX) {
			regions.initrd_start = start;
			regions.initrd_size = max(end, (u64)U32_MAX) - start;
		}
	}

	/* check the memory nodes for the size of the lowmem region */
	regions.pa_end = min(regions.pa_end, get_memory_end(fdt));

	/* check for a reserved-memory node and record its cell sizes */
	regions.reserved_mem = fdt_path_offset(fdt, "/reserved-memory");
	if (regions.reserved_mem >= 0)
		get_cell_sizes(fdt, regions.reserved_mem,
			       &regions.reserved_mem_addr_cells,
			       &regions.reserved_mem_size_cells);

	/*
	 * Iterate over the physical memory range covered by the lowmem region
	 * in 2 MB increments, and count each offset at which we don't overlap
	 * with any of the reserved regions for the zImage itself, the DTB,
	 * the initrd and any regions described as reserved in the device tree.
	 * This produces a count, which we will scale by multiplying by a 16-bit
	 * random value and shifting right by 16 places.
	 * Using this random value, we iterate over the physical memory range
	 * again until we counted enough iterations, and return the offset we
	 * ended up at.
	 */
	num = ((u32)seed * count_suitable_regions(fdt, &regions)) >> 16;

	*kaslr_offset = get_numbered_region(fdt, &regions, num) -
		        regions.pa_start;

	return *kaslr_offset;
}
