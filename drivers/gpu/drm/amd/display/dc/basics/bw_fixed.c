// SPDX-License-Identifier: MIT
/*
 * Copyright 2023 Advanced Micro Devices, Inc.
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
#include "dm_services.h"
#include "bw_fixed.h"

#define MAX_I64 \
	((int64_t)((1ULL << 63) - 1))

#define MIN_I64 \
	(-MAX_I64 - 1)

#define FRACTIONAL_PART_MASK \
	((1ULL << BW_FIXED_BITS_PER_FRACTIONAL_PART) - 1)

#define GET_FRACTIONAL_PART(x) \
	(FRACTIONAL_PART_MASK & (x))

static uint64_t abs_i64(int64_t arg)
{
	if (arg >= 0)
		return (uint64_t)(arg);
	else
		return (uint64_t)(-arg);
}

bw_fixed bw_int_to_fixed_nonconst(int64_t value)
{
	bw_fixed res;

	ASSERT(value < BW_FIXED_MAX_I32 && value > BW_FIXED_MIN_I32);
	res = value << BW_FIXED_BITS_PER_FRACTIONAL_PART;
	return res;
}

bw_fixed bw_frc_to_fixed(int64_t numerator, int64_t denominator)
{
	bw_fixed res;
	bool arg1_negative = numerator < 0;
	bool arg2_negative = denominator < 0;
	uint64_t arg1_value;
	uint64_t arg2_value;
	uint64_t remainder;

	/* determine integer part */
	uint64_t res_value;

	ASSERT(denominator != 0);

	arg1_value = abs_i64(numerator);
	arg2_value = abs_i64(denominator);
	res_value = div64_u64_rem(arg1_value, arg2_value, &remainder);

	ASSERT(res_value <= BW_FIXED_MAX_I32);

	/* determine fractional part */
	{
		uint32_t i = BW_FIXED_BITS_PER_FRACTIONAL_PART;

		do {
			remainder <<= 1;

			res_value <<= 1;

			if (remainder >= arg2_value) {
				res_value |= 1;
				remainder -= arg2_value;
			}
		} while (--i != 0);
	}

	/* round up LSB */
	{
		uint64_t summand = (remainder << 1) >= arg2_value;

		ASSERT(res_value <= MAX_I64 - summand);

		res_value += summand;
	}

	res = (int64_t)(res_value);

	if (arg1_negative ^ arg2_negative)
		res = -res;
	return res;
}

bw_fixed bw_floor2(const bw_fixed arg,
			  const bw_fixed significance)
{
	bw_fixed result;
	int64_t multiplicand;

	multiplicand = div64_s64(arg, abs_i64(significance));
	result = abs_i64(significance) * multiplicand;
	ASSERT(abs_i64(result) <= abs_i64(arg));
	return result;
}

bw_fixed bw_ceil2(const bw_fixed arg,
			 const bw_fixed significance)
{
	bw_fixed result;
	int64_t multiplicand;

	multiplicand = div64_s64(arg, abs_i64(significance));
	result = abs_i64(significance) * multiplicand;
	if (abs_i64(result) < abs_i64(arg)) {
		if (arg < 0)
			result -= abs_i64(significance);
		else
			result += abs_i64(significance);
	}
	return result;
}

bw_fixed bw_mul(const bw_fixed arg1, const bw_fixed arg2)
{
	bw_fixed res;

	bool arg1_negative = arg1 < 0;
	bool arg2_negative = arg2 < 0;

	uint64_t arg1_value = abs_i64(arg1);
	uint64_t arg2_value = abs_i64(arg2);

	uint64_t arg1_int = BW_FIXED_GET_INTEGER_PART(arg1_value);
	uint64_t arg2_int = BW_FIXED_GET_INTEGER_PART(arg2_value);

	uint64_t arg1_fra = GET_FRACTIONAL_PART(arg1_value);
	uint64_t arg2_fra = GET_FRACTIONAL_PART(arg2_value);

	uint64_t tmp;

	res = arg1_int * arg2_int;

	ASSERT(res <= BW_FIXED_MAX_I32);

	res <<= BW_FIXED_BITS_PER_FRACTIONAL_PART;

	tmp = arg1_int * arg2_fra;

	ASSERT(tmp <= (uint64_t)(MAX_I64 - res));

	res += tmp;

	tmp = arg2_int * arg1_fra;

	ASSERT(tmp <= (uint64_t)(MAX_I64 - res));

	res += tmp;

	tmp = arg1_fra * arg2_fra;

	tmp = (tmp >> BW_FIXED_BITS_PER_FRACTIONAL_PART) +
		(tmp >= (uint64_t)(bw_frc_to_fixed(1, 2)));

	ASSERT(tmp <= (uint64_t)(MAX_I64 - res));

	res += tmp;

	if (arg1_negative ^ arg2_negative)
		res = -res;
	return res;
}

