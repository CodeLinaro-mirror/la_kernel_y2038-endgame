/*
 * Copyright 2017 Advanced Micro Devices, Inc.
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

#ifndef __DML_INLINE_DEFS_H__
#define __DML_INLINE_DEFS_H__

#include "dml_common_defs.h"
#include "../calcs/dcn_calc_math.h"
#include "dml_logger.h"

static inline amdgpu_dc_double dml_min(amdgpu_dc_double a, amdgpu_dc_double b)
{
	return (amdgpu_dc_double) dcn_bw_min2(a, b);
}

static inline amdgpu_dc_double dml_min3(amdgpu_dc_double a, amdgpu_dc_double b, amdgpu_dc_double c)
{
	return dml_min(dml_min(a, b), c);
}

static inline amdgpu_dc_double dml_min4(amdgpu_dc_double a, amdgpu_dc_double b, amdgpu_dc_double c, amdgpu_dc_double d)
{
	return dml_min(dml_min(a, b), dml_min(c, d));
}

static inline amdgpu_dc_double dml_max(amdgpu_dc_double a, amdgpu_dc_double b)
{
	return (amdgpu_dc_double) dcn_bw_max2(a, b);
}

static inline amdgpu_dc_double dml_max3(amdgpu_dc_double a, amdgpu_dc_double b, amdgpu_dc_double c)
{
	return dml_max(dml_max(a, b), c);
}

static inline amdgpu_dc_double dml_max4(amdgpu_dc_double a, amdgpu_dc_double b, amdgpu_dc_double c, amdgpu_dc_double d)
{
	return dml_max(dml_max(a, b), dml_max(c, d));
}

static inline amdgpu_dc_double dml_max5(amdgpu_dc_double a, amdgpu_dc_double b, amdgpu_dc_double c, amdgpu_dc_double d, amdgpu_dc_double e)
{
	return dml_max(dml_max4(a, b, c, d), e);
}

static inline amdgpu_dc_double dml_ceil(amdgpu_dc_double a, amdgpu_dc_double granularity)
{
	return (amdgpu_dc_double) dcn_bw_ceil2(a, granularity);
}

static inline amdgpu_dc_double dml_floor(amdgpu_dc_double a, amdgpu_dc_double granularity)
{
	return (amdgpu_dc_double) dcn_bw_floor2(a, granularity);
}

static inline int dml_log2(amdgpu_dc_double x)
{
	return dml_round((amdgpu_dc_double)dcn_bw_log(x, 2));
}

static inline amdgpu_dc_double dml_pow(amdgpu_dc_double a, int exp)
{
	return (amdgpu_dc_double) dcn_bw_pow(a, exp);
}

static inline amdgpu_dc_double dml_fmod(amdgpu_dc_double f, int val)
{
	return (amdgpu_dc_double) dcn_bw_mod(f, val);
}

static inline amdgpu_dc_double dml_ceil_2(amdgpu_dc_double f)
{
	return (amdgpu_dc_double) dcn_bw_ceil2(f, 2);
}

static inline amdgpu_dc_double dml_ceil_ex(amdgpu_dc_double x, amdgpu_dc_double granularity)
{
	return (amdgpu_dc_double) dcn_bw_ceil2(x, granularity);
}

static inline amdgpu_dc_double dml_floor_ex(amdgpu_dc_double x, amdgpu_dc_double granularity)
{
	return (amdgpu_dc_double) dcn_bw_floor2(x, granularity);
}

static inline amdgpu_dc_double dml_log(amdgpu_dc_double x, amdgpu_dc_double base)
{
	return (amdgpu_dc_double) dcn_bw_log(x, base);
}

static inline unsigned int dml_round_to_multiple(unsigned int num,
						 unsigned int multiple,
						 bool up)
{
	unsigned int remainder;

	if (multiple == 0)
		return num;

	remainder = num % multiple;

	if (remainder == 0)
		return num;

	if (up)
		return (num + multiple - remainder);
	else
		return (num - remainder);
}
static inline amdgpu_dc_double dml_abs(amdgpu_dc_double a)
{
	if (a > 0)
		return a;
	else
		return (a*(-1));
}

#endif
