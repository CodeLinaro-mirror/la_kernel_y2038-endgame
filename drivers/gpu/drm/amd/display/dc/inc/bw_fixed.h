/*
 * Copyright 2015 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: AMD
 *
 */

#ifndef BW_FIXED_H_
#define BW_FIXED_H_

#define BW_FIXED_BITS_PER_FRACTIONAL_PART 24

#define BW_FIXED_GET_INTEGER_PART(x) ((x) >> BW_FIXED_BITS_PER_FRACTIONAL_PART)
typedef int64_t bw_fixed;

#define BW_FIXED_MIN_I32 \
	(int64_t)(-(1LL << (63 - BW_FIXED_BITS_PER_FRACTIONAL_PART)))

#define BW_FIXED_MAX_I32 \
	(int64_t)((1ULL << (63 - BW_FIXED_BITS_PER_FRACTIONAL_PART)) - 1)

static inline bw_fixed bw_min2(const bw_fixed arg1,
				      const bw_fixed arg2)
{
	return (arg1 <= arg2) ? arg1 : arg2;
}

static inline bw_fixed bw_max2(const bw_fixed arg1,
				      const bw_fixed arg2)
{
	return (arg2 <= arg1) ? arg1 : arg2;
}

static inline bw_fixed bw_min3(bw_fixed v1,
				      bw_fixed v2,
				      bw_fixed v3)
{
	return bw_min2(bw_min2(v1, v2), v3);
}

static inline bw_fixed bw_max3(bw_fixed v1,
				      bw_fixed v2,
				      bw_fixed v3)
{
	return bw_max2(bw_max2(v1, v2), v3);
}

bw_fixed bw_int_to_fixed_nonconst(int64_t value);
static inline bw_fixed bw_int_to_fixed(int64_t value)
{
	if (__builtin_constant_p(value)) {
		bw_fixed res;
		BUILD_BUG_ON(value > BW_FIXED_MAX_I32 || value < BW_FIXED_MIN_I32);
		res = value << BW_FIXED_BITS_PER_FRACTIONAL_PART;
		return res;
	} else
		return bw_int_to_fixed_nonconst(value);
}

static inline int32_t bw_fixed_to_int(bw_fixed value)
{
	return BW_FIXED_GET_INTEGER_PART(value);
}

bw_fixed bw_frc_to_fixed(int64_t num, int64_t denum);

static inline bw_fixed fixed31_32_to_bw_fixed(int64_t raw)
{
	bw_fixed result = { 0 };

	if (raw < 0) {
		raw = -raw;
		result = -(raw >> (32 - BW_FIXED_BITS_PER_FRACTIONAL_PART));
	} else {
		result = raw >> (32 - BW_FIXED_BITS_PER_FRACTIONAL_PART);
	}

	return result;
}

static inline bw_fixed bw_add(const bw_fixed arg1,
				     const bw_fixed arg2)
{
	bw_fixed res;

	res = arg1 + arg2;

	return res;
}

static inline bw_fixed bw_sub(const bw_fixed arg1, const bw_fixed arg2)
{
	bw_fixed res;

	res = arg1 - arg2;

	return res;
}

bw_fixed bw_mul(const bw_fixed arg1, const bw_fixed arg2);
static inline bw_fixed bw_div(const bw_fixed arg1, const bw_fixed arg2)
{
	return bw_frc_to_fixed(arg1, arg2);
}

static inline bw_fixed bw_mod(const bw_fixed arg1, const bw_fixed arg2)
{
	bw_fixed res;
	div64_u64_rem(arg1, arg2, (uint64_t *)&res);
	return res;
}

bw_fixed bw_floor2(const bw_fixed arg, const bw_fixed significance);
bw_fixed bw_ceil2(const bw_fixed arg, const bw_fixed significance);

static inline bool bw_equ(const bw_fixed arg1, const bw_fixed arg2)
{
	return arg1 == arg2;
}

static inline bool bw_neq(const bw_fixed arg1, const bw_fixed arg2)
{
	return arg1 != arg2;
}

static inline bool bw_leq(const bw_fixed arg1, const bw_fixed arg2)
{
	return arg1 <= arg2;
}

static inline bool bw_meq(const bw_fixed arg1, const bw_fixed arg2)
{
	return arg1 >= arg2;
}

static inline bool bw_ltn(const bw_fixed arg1, const bw_fixed arg2)
{
	return arg1 < arg2;
}

static inline bool bw_mtn(const bw_fixed arg1, const bw_fixed arg2)
{
	return arg1 > arg2;
}

#endif //BW_FIXED_H_
