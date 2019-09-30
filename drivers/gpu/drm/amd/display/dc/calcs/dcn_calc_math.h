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

#ifndef _DCN_CALC_MATH_H_
#define _DCN_CALC_MATH_H_

#include <inc/amdgpu_dc_float.h>

amdgpu_dc_float dcn_bw_mod(const amdgpu_dc_float arg1, const amdgpu_dc_float arg2);
amdgpu_dc_float dcn_bw_min2(const amdgpu_dc_float arg1, const amdgpu_dc_float arg2);
unsigned int dcn_bw_max(const unsigned int arg1, const unsigned int arg2);
amdgpu_dc_float dcn_bw_max2(const amdgpu_dc_float arg1, const amdgpu_dc_float arg2);
amdgpu_dc_float dcn_bw_floor2(const amdgpu_dc_float arg, const amdgpu_dc_float significance);
amdgpu_dc_float dcn_bw_floor(const amdgpu_dc_float arg);
amdgpu_dc_float dcn_bw_ceil2(const amdgpu_dc_float arg, const amdgpu_dc_float significance);
amdgpu_dc_float dcn_bw_ceil(const amdgpu_dc_float arg);
amdgpu_dc_float dcn_bw_max3(amdgpu_dc_float v1, amdgpu_dc_float v2, amdgpu_dc_float v3);
amdgpu_dc_float dcn_bw_max5(amdgpu_dc_float v1, amdgpu_dc_float v2, amdgpu_dc_float v3, amdgpu_dc_float v4, amdgpu_dc_float v5);
amdgpu_dc_float dcn_bw_pow(amdgpu_dc_float a, amdgpu_dc_float exp);
amdgpu_dc_float dcn_bw_log(amdgpu_dc_float a, amdgpu_dc_float b);
amdgpu_dc_double dcn_bw_fabs(amdgpu_dc_double a);

#endif /* _DCN_CALC_MATH_H_ */
