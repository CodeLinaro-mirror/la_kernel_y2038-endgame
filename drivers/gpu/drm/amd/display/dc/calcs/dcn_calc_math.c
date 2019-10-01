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

#include "dcn_calc_math.h"

#define isNaN(number) amdgpu_dc_float_is_NaN(number)

/*
 * NOTE:
 *   This file is gcc-parseable HW gospel, coming straight from HW engineers.
 *
 * It doesn't adhere to Linux kernel style and sometimes will do things in odd
 * ways. Unless there is something clearly wrong with it the code should
 * remain as-is as it provides us with a guarantee from HW that it is correct.
 */

amdgpu_dc_float dcn_bw_mod(const amdgpu_dc_float arg1, const amdgpu_dc_float arg2)
{
	if (isNaN(arg1))
		return arg2;
	if (isNaN(arg2))
		return arg1;
	return arg1 - arg1 * ((int) (arg1 / arg2));
}

amdgpu_dc_float dcn_bw_min2(const amdgpu_dc_float arg1, const amdgpu_dc_float arg2)
{
	if (isNaN(arg1))
		return arg2;
	if (isNaN(arg2))
		return arg1;
	return arg1 < arg2 ? arg1 : arg2;
}

unsigned int dcn_bw_max(const unsigned int arg1, const unsigned int arg2)
{
	return arg1 > arg2 ? arg1 : arg2;
}
amdgpu_dc_float dcn_bw_max2(const amdgpu_dc_float arg1, const amdgpu_dc_float arg2)
{
	if (isNaN(arg1))
		return arg2;
	if (isNaN(arg2))
		return arg1;
	return arg1 > arg2 ? arg1 : arg2;
}

amdgpu_dc_float dcn_bw_floor2(const amdgpu_dc_float arg, const amdgpu_dc_float significance)
{
	if (significance == 0)
		return amdgpu_dc_float_constant(0);
	return ((int) (arg / significance)) * significance;
}
amdgpu_dc_float dcn_bw_floor(const amdgpu_dc_float arg)
{
	return ((int) (arg));
}

amdgpu_dc_float dcn_bw_ceil(const amdgpu_dc_float arg)
{
	amdgpu_dc_float flr = dcn_bw_floor2(arg, 1);

	return flr + 0.00001 >= arg ? arg : flr + 1;
}

amdgpu_dc_float dcn_bw_ceil2(const amdgpu_dc_float arg, const amdgpu_dc_float significance)
{
	amdgpu_dc_float flr = dcn_bw_floor2(arg, significance);
	if (significance == 0)
		return amdgpu_dc_float_constant(0);
	return flr + 0.00001 >= arg ? arg : flr + significance;
}

amdgpu_dc_float dcn_bw_max3(amdgpu_dc_float v1, amdgpu_dc_float v2, amdgpu_dc_float v3)
{
	return v3 > dcn_bw_max2(v1, v2) ? v3 : dcn_bw_max2(v1, v2);
}

amdgpu_dc_float dcn_bw_max5(amdgpu_dc_float v1, amdgpu_dc_float v2, amdgpu_dc_float v3, amdgpu_dc_float v4, amdgpu_dc_float v5)
{
	return dcn_bw_max3(v1, v2, v3) > dcn_bw_max2(v4, v5) ? dcn_bw_max3(v1, v2, v3) : dcn_bw_max2(v4, v5);
}

amdgpu_dc_float dcn_bw_pow(amdgpu_dc_float a, amdgpu_dc_float exp)
{
	amdgpu_dc_float temp;
	/*ASSERT(exp == (int)exp);*/
	if ((int)exp == 0)
		return amdgpu_dc_float_constant(1);
	temp = dcn_bw_pow(a, (int)(exp / 2));
	if (((int)exp % 2) == 0) {
		return temp * temp;
	} else {
		if ((int)exp > 0)
			return a * temp * temp;
		else
			return (temp * temp) / a;
	}
}

amdgpu_dc_double dcn_bw_fabs(amdgpu_dc_double a)
{
	if (a > 0)
		return (a);
	else
		return (-a);
}


amdgpu_dc_float dcn_bw_log(amdgpu_dc_float a, amdgpu_dc_float b)
{
	int * const exp_ptr = (int *)(&a);
	int x = *exp_ptr;
	const int log_2 = ((x >> 23) & 255) - 128;
	x &= ~(255 << 23);
	x += 127 << 23;
	*exp_ptr = x;

	a = ((-1.0f / 3) * a + 2) * a - 2.0f / 3;

	if (b > 2.00001 || b < 1.99999)
		return (a + log_2) / dcn_bw_log(b, 2);
	else
		return (a + log_2);
}
