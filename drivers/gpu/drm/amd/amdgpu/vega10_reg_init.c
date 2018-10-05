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
 */
#include "amdgpu.h"
#include "soc15.h"

#include "soc15_common.h"
#include "soc15_hw_ip.h"
#include "vega10_ip_offset.h"

#define MAX_INSTANCE                                       5
#define MAX_SEGMENT                                        5

struct IP_BASE_INSTANCE
{
    unsigned int segment[MAX_SEGMENT];
};

struct IP_BASE
{
    struct IP_BASE_INSTANCE instance[MAX_INSTANCE];
};


static const struct IP_BASE NBIF_BASE			= { { { { 0x00000000, 0x00000014, 0x00000D20, 0x00010400, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } };
static const struct IP_BASE NBIO_BASE			= { { { { 0x00000000, 0x00000014, 0x00000D20, 0x00010400, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } };
static const struct IP_BASE DCE_BASE			= { { { { 0x00000012, 0x000000C0, 0x000034C0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } };
static const struct IP_BASE DCN_BASE			= { { { { 0x00000012, 0x000000C0, 0x000034C0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } };
static const struct IP_BASE MP0_BASE			= { { { { 0x00016000, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } };
static const struct IP_BASE MP1_BASE			= { { { { 0x00016000, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } };
static const struct IP_BASE MP2_BASE			= { { { { 0x00016000, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } };
static const struct IP_BASE DF_BASE			= { { { { 0x00007000, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } };
static const struct IP_BASE UVD_BASE			= { { { { 0x00007800, 0x00007E00, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } };  //note: GLN does not use the first segment
static const struct IP_BASE VCN_BASE			= { { { { 0x00007800, 0x00007E00, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } };  //note: GLN does not use the first segment
static const struct IP_BASE DBGU_BASE			= { { { { 0x00000180, 0x000001A0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } }; // not exist
static const struct IP_BASE DBGU_NBIO_BASE		= { { { { 0x000001C0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } }; // not exist
static const struct IP_BASE DBGU_IO_BASE		= { { { { 0x000001E0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } }; // not exist
static const struct IP_BASE DFX_DAP_BASE		= { { { { 0x000005A0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } }; // not exist
static const struct IP_BASE DFX_BASE			= { { { { 0x00000580, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } }; // this file does not contain registers
static const struct IP_BASE ISP_BASE			= { { { { 0x00018000, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } }; // not exist
static const struct IP_BASE SYSTEMHUB_BASE		= { { { { 0x00000EA0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } }; // not exist
static const struct IP_BASE L2IMU_BASE			= { { { { 0x00007DC0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } };
static const struct IP_BASE IOHC_BASE			= { { { { 0x00010000, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } };
static const struct IP_BASE ATHUB_BASE			= { { { { 0x00000C20, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } };
static const struct IP_BASE VCE_BASE			= { { { { 0x00007E00, 0x00048800, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } };
static const struct IP_BASE GC_BASE			= { { { { 0x00002000, 0x0000A000, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } };
static const struct IP_BASE MMHUB_BASE			= { { { { 0x0001A000, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } };
static const struct IP_BASE RSMU_BASE			= { { { { 0x00012000, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } };
static const struct IP_BASE HDP_BASE			= { { { { 0x00000F20, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } };
static const struct IP_BASE OSSSYS_BASE		= { { { { 0x000010A0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } };
static const struct IP_BASE SDMA0_BASE			= { { { { 0x00001260, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } };
static const struct IP_BASE SDMA1_BASE			= { { { { 0x00001460, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } };
static const struct IP_BASE XDMA_BASE			= { { { { 0x00003400, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } };
static const struct IP_BASE UMC_BASE			= { { { { 0x00014000, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } };
static const struct IP_BASE THM_BASE			= { { { { 0x00016600, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } };
static const struct IP_BASE SMUIO_BASE			= { { { { 0x00016800, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } };
static const struct IP_BASE PWR_BASE			= { { { { 0x00016A00, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } };
static const struct IP_BASE CLK_BASE			= { { { { 0x00016C00, 0, 0, 0, 0 } },
									    { { 0x00016E00, 0, 0, 0, 0 } },
										{ { 0x00017000, 0, 0, 0, 0 } },
	                                    { { 0x00017200, 0, 0, 0, 0 } },
						                { { 0x00017E00, 0, 0, 0, 0 } } } };
static const struct IP_BASE FUSE_BASE			= { { { { 0x00017400, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } },
										{ { 0, 0, 0, 0, 0 } } } };

int vega10_reg_base_init(struct amdgpu_device *adev)
{
	/* HW has more IP blocks,  only initialized the blocke beend by our driver  */
	uint32_t i;
	for (i = 0 ; i < MAX_INSTANCE ; ++i) {
		adev->reg_offset[GC_HWIP][i] = (uint32_t *)(&(GC_BASE.instance[i]));
		adev->reg_offset[HDP_HWIP][i] = (uint32_t *)(&(HDP_BASE.instance[i]));
		adev->reg_offset[MMHUB_HWIP][i] = (uint32_t *)(&(MMHUB_BASE.instance[i]));
		adev->reg_offset[ATHUB_HWIP][i] = (uint32_t *)(&(ATHUB_BASE.instance[i]));
		adev->reg_offset[NBIO_HWIP][i] = (uint32_t *)(&(NBIO_BASE.instance[i]));
		adev->reg_offset[MP0_HWIP][i] = (uint32_t *)(&(MP0_BASE.instance[i]));
		adev->reg_offset[MP1_HWIP][i] = (uint32_t *)(&(MP1_BASE.instance[i]));
		adev->reg_offset[UVD_HWIP][i] = (uint32_t *)(&(UVD_BASE.instance[i]));
		adev->reg_offset[VCE_HWIP][i] = (uint32_t *)(&(VCE_BASE.instance[i]));
		adev->reg_offset[VCN_HWIP][i] = (uint32_t *)(&(VCN_BASE.instance[i]));
		adev->reg_offset[DF_HWIP][i] = (uint32_t *)(&(DF_BASE.instance[i]));
		adev->reg_offset[DCE_HWIP][i] = (uint32_t *)(&(DCE_BASE.instance[i]));
		adev->reg_offset[OSSSYS_HWIP][i] = (uint32_t *)(&(OSSSYS_BASE.instance[i]));
		adev->reg_offset[SDMA0_HWIP][i] = (uint32_t *)(&(SDMA0_BASE.instance[i]));
		adev->reg_offset[SDMA1_HWIP][i] = (uint32_t *)(&(SDMA1_BASE.instance[i]));
		adev->reg_offset[SMUIO_HWIP][i] = (uint32_t *)(&(SMUIO_BASE.instance[i]));
		adev->reg_offset[PWR_HWIP][i] = (uint32_t *)(&(PWR_BASE.instance[i]));
		adev->reg_offset[NBIF_HWIP][i] = (uint32_t *)(&(NBIF_BASE.instance[i]));
		adev->reg_offset[THM_HWIP][i] = (uint32_t *)(&(THM_BASE.instance[i]));
		adev->reg_offset[CLK_HWIP][i] = (uint32_t *)(&(CLK_BASE.instance[i]));
	}
	return 0;
}


