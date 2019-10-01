// SPDX-License-Identifier: GPL-2.0

#ifndef __AMDGPU_DC_FLOAT_H__
#define __AMDGPU_DC_FLOAT_H__

#if 1
typedef union {
	float f;
	unsigned int x;
} amdgpu_dc_float;

typedef union {
	double d;
	unsigned long long x;
} amdgpu_dc_double;

#define amdgpu_dc_float_constant(x) ((amdgpu_dc_float){ .f = (x) })
#define amdgpu_dc_double_constant(x) ((amdgpu_dc_double){ .d = (x) })

static inline _Bool amdgpu_dc_float_is_NaN(amdgpu_dc_float x)
{
	return (x.f != x.f);
}

#else
typedef float amdgpu_dc_float;
typedef double amdgpu_dc_double;

#define amdgpu_dc_float_constant(x) ((amdgpu_dc_float)(x))
#define amdgpu_dc_double_constant(x) ((amdgpu_dc_double)(x))

#define amdgpu_dc_float_is_NaN(x) ((x) != (x))

#endif

#endif
