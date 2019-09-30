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

#ifdef CONFIG_DRM_AMD_DC_DCN2_0

#ifndef __DML2_DISPLAY_MODE_VBA_H__
#define __DML2_DISPLAY_MODE_VBA_H__

#include "dml_common_defs.h"

struct display_mode_lib;

void ModeSupportAndSystemConfiguration(struct display_mode_lib *mode_lib);

#define dml_get_attr_decl(attr) amdgpu_dc_double get_##attr(struct display_mode_lib *mode_lib, const display_e2e_pipe_params_st *pipes, unsigned int num_pipes)

dml_get_attr_decl(clk_dcf_deepsleep);
dml_get_attr_decl(wm_urgent);
dml_get_attr_decl(wm_memory_trip);
dml_get_attr_decl(wm_writeback_urgent);
dml_get_attr_decl(wm_stutter_exit);
dml_get_attr_decl(wm_stutter_enter_exit);
dml_get_attr_decl(wm_dram_clock_change);
dml_get_attr_decl(wm_writeback_dram_clock_change);
dml_get_attr_decl(wm_xfc_underflow);
dml_get_attr_decl(stutter_efficiency_no_vblank);
dml_get_attr_decl(stutter_efficiency);
dml_get_attr_decl(urgent_latency);
dml_get_attr_decl(urgent_extra_latency);
dml_get_attr_decl(nonurgent_latency);
dml_get_attr_decl(dram_clock_change_latency);
dml_get_attr_decl(dispclk_calculated);
dml_get_attr_decl(total_data_read_bw);
dml_get_attr_decl(return_bw);
dml_get_attr_decl(tcalc);
dml_get_attr_decl(fraction_of_urgent_bandwidth);
dml_get_attr_decl(fraction_of_urgent_bandwidth_imm_flip);

#define dml_get_pipe_attr_decl(attr) amdgpu_dc_double get_##attr(struct display_mode_lib *mode_lib, const display_e2e_pipe_params_st *pipes, unsigned int num_pipes, unsigned int which_pipe)

dml_get_pipe_attr_decl(dsc_delay);
dml_get_pipe_attr_decl(dppclk_calculated);
dml_get_pipe_attr_decl(dscclk_calculated);
dml_get_pipe_attr_decl(min_ttu_vblank);
dml_get_pipe_attr_decl(vratio_prefetch_l);
dml_get_pipe_attr_decl(vratio_prefetch_c);
dml_get_pipe_attr_decl(dst_x_after_scaler);
dml_get_pipe_attr_decl(dst_y_after_scaler);
dml_get_pipe_attr_decl(dst_y_per_vm_vblank);
dml_get_pipe_attr_decl(dst_y_per_row_vblank);
dml_get_pipe_attr_decl(dst_y_prefetch);
dml_get_pipe_attr_decl(dst_y_per_vm_flip);
dml_get_pipe_attr_decl(dst_y_per_row_flip);
dml_get_pipe_attr_decl(xfc_transfer_delay);
dml_get_pipe_attr_decl(xfc_precharge_delay);
dml_get_pipe_attr_decl(xfc_remote_surface_flip_latency);
dml_get_pipe_attr_decl(xfc_prefetch_margin);
dml_get_pipe_attr_decl(refcyc_per_vm_group_vblank);
dml_get_pipe_attr_decl(refcyc_per_vm_group_flip);
dml_get_pipe_attr_decl(refcyc_per_vm_req_vblank);
dml_get_pipe_attr_decl(refcyc_per_vm_req_flip);

unsigned int get_vstartup_calculated(
		struct display_mode_lib *mode_lib,
		const display_e2e_pipe_params_st *pipes,
		unsigned int num_pipes,
		unsigned int which_pipe);

amdgpu_dc_double get_total_immediate_flip_bytes(
		struct display_mode_lib *mode_lib,
		const display_e2e_pipe_params_st *pipes,
		unsigned int num_pipes);
amdgpu_dc_double get_total_immediate_flip_bw(
		struct display_mode_lib *mode_lib,
		const display_e2e_pipe_params_st *pipes,
		unsigned int num_pipes);
amdgpu_dc_double get_total_prefetch_bw(
		struct display_mode_lib *mode_lib,
		const display_e2e_pipe_params_st *pipes,
		unsigned int num_pipes);
unsigned int dml_get_voltage_level(
		struct display_mode_lib *mode_lib,
		const display_e2e_pipe_params_st *pipes,
		unsigned int num_pipes);

void PixelClockAdjustmentForProgressiveToInterlaceUnit(struct display_mode_lib *mode_lib);

bool Calculate256BBlockSizes(
		enum source_format_class SourcePixelFormat,
		enum dm_swizzle_mode SurfaceTiling,
		unsigned int BytePerPixelY,
		unsigned int BytePerPixelC,
		unsigned int *BlockHeight256BytesY,
		unsigned int *BlockHeight256BytesC,
		unsigned int *BlockWidth256BytesY,
		unsigned int *BlockWidth256BytesC);

struct vba_vars_st {
	ip_params_st ip;
	soc_bounding_box_st soc;

	int maxMpcComb;
	bool UseMaximumVStartup;

	amdgpu_dc_double WritebackDISPCLK;
	amdgpu_dc_double DPPCLKUsingSingleDPPLuma;
	amdgpu_dc_double DPPCLKUsingSingleDPPChroma;
	amdgpu_dc_double DISPCLKWithRamping;
	amdgpu_dc_double DISPCLKWithoutRamping;
	amdgpu_dc_double GlobalDPPCLK;
	amdgpu_dc_double DISPCLKWithRampingRoundedToDFSGranularity;
	amdgpu_dc_double DISPCLKWithoutRampingRoundedToDFSGranularity;
	amdgpu_dc_double MaxDispclkRoundedToDFSGranularity;
	bool DCCEnabledAnyPlane;
	amdgpu_dc_double ReturnBandwidthToDCN;
	unsigned int TotalActiveDPP;
	unsigned int TotalDCCActiveDPP;
	amdgpu_dc_double UrgentRoundTripAndOutOfOrderLatency;
	amdgpu_dc_double StutterPeriod;
	amdgpu_dc_double FrameTimeForMinFullDETBufferingTime;
	amdgpu_dc_double AverageReadBandwidth;
	amdgpu_dc_double TotalRowReadBandwidth;
	amdgpu_dc_double PartOfBurstThatFitsInROB;
	amdgpu_dc_double StutterBurstTime;
	unsigned int NextPrefetchMode;
	amdgpu_dc_double NextMaxVStartup;
	amdgpu_dc_double VBlankTime;
	amdgpu_dc_double SmallestVBlank;
	amdgpu_dc_double DCFCLKDeepSleepPerPlane[DC__NUM_DPP__MAX];
	amdgpu_dc_double EffectiveDETPlusLBLinesLuma;
	amdgpu_dc_double EffectiveDETPlusLBLinesChroma;
	amdgpu_dc_double UrgentLatencySupportUsLuma;
	amdgpu_dc_double UrgentLatencySupportUsChroma;
	unsigned int DSCFormatFactor;

	bool PrefetchModeSupported;
	enum self_refresh_affinity AllowDRAMSelfRefreshOrDRAMClockChangeInVblank; // Mode Support only
	amdgpu_dc_double XFCRemoteSurfaceFlipDelay;
	amdgpu_dc_double TInitXFill;
	amdgpu_dc_double TslvChk;
	amdgpu_dc_double SrcActiveDrainRate;
	bool ImmediateFlipSupported;
	enum mpc_combine_affinity WhenToDoMPCCombine; // Mode Support only

	bool PrefetchERROR;

	unsigned int VStartupLines;
	unsigned int ActiveDPPs;
	unsigned int LBLatencyHidingSourceLinesY;
	unsigned int LBLatencyHidingSourceLinesC;
	amdgpu_dc_double ActiveDRAMClockChangeLatencyMargin[DC__NUM_DPP__MAX];
	amdgpu_dc_double MinActiveDRAMClockChangeMargin;
	amdgpu_dc_double InitFillLevel;
	amdgpu_dc_double FinalFillMargin;
	amdgpu_dc_double FinalFillLevel;
	amdgpu_dc_double RemainingFillLevel;
	amdgpu_dc_double TFinalxFill;

	//
	// SOC Bounding Box Parameters
	//
	amdgpu_dc_double SRExitTime;
	amdgpu_dc_double SREnterPlusExitTime;
	amdgpu_dc_double UrgentLatencyPixelDataOnly;
	amdgpu_dc_double UrgentLatencyPixelMixedWithVMData;
	amdgpu_dc_double UrgentLatencyVMDataOnly;
	amdgpu_dc_double UrgentLatency; // max of the above three
	amdgpu_dc_double WritebackLatency;
	amdgpu_dc_double PercentOfIdealDRAMFabricAndSDPPortBWReceivedAfterUrgLatencyPixelDataOnly; // Mode Support
	amdgpu_dc_double PercentOfIdealDRAMFabricAndSDPPortBWReceivedAfterUrgLatencyPixelMixedWithVMData; // Mode Support
	amdgpu_dc_double PercentOfIdealDRAMFabricAndSDPPortBWReceivedAfterUrgLatencyVMDataOnly; // Mode Support
	amdgpu_dc_double MaxAveragePercentOfIdealSDPPortBWDisplayCanUseInNormalSystemOperation; // Mode Support
	amdgpu_dc_double MaxAveragePercentOfIdealDRAMBWDisplayCanUseInNormalSystemOperation; // Mode Support
	amdgpu_dc_double NumberOfChannels;
	amdgpu_dc_double DRAMChannelWidth;
	amdgpu_dc_double FabricDatapathToDCNDataReturn;
	amdgpu_dc_double ReturnBusWidth;
	amdgpu_dc_double Downspreading;
	amdgpu_dc_double DISPCLKDPPCLKDSCCLKDownSpreading;
	amdgpu_dc_double DISPCLKDPPCLKVCOSpeed;
	amdgpu_dc_double RoundTripPingLatencyCycles;
	amdgpu_dc_double UrgentOutOfOrderReturnPerChannel;
	amdgpu_dc_double UrgentOutOfOrderReturnPerChannelPixelDataOnly;
	amdgpu_dc_double UrgentOutOfOrderReturnPerChannelPixelMixedWithVMData;
	amdgpu_dc_double UrgentOutOfOrderReturnPerChannelVMDataOnly;
	unsigned int VMMPageSize;
	amdgpu_dc_double DRAMClockChangeLatency;
	amdgpu_dc_double XFCBusTransportTime;
	bool UseUrgentBurstBandwidth;
	amdgpu_dc_double XFCXBUFLatencyTolerance;

	//
	// IP Parameters
	//
	unsigned int ROBBufferSizeInKByte;
	amdgpu_dc_double DETBufferSizeInKByte;
	amdgpu_dc_double DETBufferSizeInTime;
	unsigned int DPPOutputBufferPixels;
	unsigned int OPPOutputBufferLines;
	unsigned int PixelChunkSizeInKByte;
	amdgpu_dc_double ReturnBW;
	bool GPUVMEnable;
	bool HostVMEnable;
	unsigned int GPUVMMaxPageTableLevels;
	unsigned int HostVMMaxPageTableLevels;
	unsigned int HostVMCachedPageTableLevels;
	unsigned int OverrideGPUVMPageTableLevels;
	unsigned int OverrideHostVMPageTableLevels;
	unsigned int MetaChunkSize;
	amdgpu_dc_double MinPixelChunkSizeBytes;
	amdgpu_dc_double MinMetaChunkSizeBytes;
	unsigned int WritebackChunkSize;
	bool ODMCapability;
	unsigned int NumberOfDSC;
	unsigned int LineBufferSize;
	unsigned int MaxLineBufferLines;
	unsigned int WritebackInterfaceLumaBufferSize;
	unsigned int WritebackInterfaceChromaBufferSize;
	unsigned int WritebackChromaLineBufferWidth;
	enum writeback_config WritebackConfiguration;
	amdgpu_dc_double MaxDCHUBToPSCLThroughput;
	amdgpu_dc_double MaxPSCLToLBThroughput;
	unsigned int PTEBufferSizeInRequestsLuma;
	unsigned int PTEBufferSizeInRequestsChroma;
	amdgpu_dc_double DISPCLKRampingMargin;
	unsigned int MaxInterDCNTileRepeaters;
	bool XFCSupported;
	amdgpu_dc_double XFCSlvChunkSize;
	amdgpu_dc_double XFCFillBWOverhead;
	amdgpu_dc_double XFCFillConstant;
	amdgpu_dc_double XFCTSlvVupdateOffset;
	amdgpu_dc_double XFCTSlvVupdateWidth;
	amdgpu_dc_double XFCTSlvVreadyOffset;
	amdgpu_dc_double DPPCLKDelaySubtotal;
	amdgpu_dc_double DPPCLKDelaySCL;
	amdgpu_dc_double DPPCLKDelaySCLLBOnly;
	amdgpu_dc_double DPPCLKDelayCNVCFormater;
	amdgpu_dc_double DPPCLKDelayCNVCCursor;
	amdgpu_dc_double DISPCLKDelaySubtotal;
	bool ProgressiveToInterlaceUnitInOPP;
	// Pipe/Plane Parameters
	int VoltageLevel;
	amdgpu_dc_double FabricClock;
	amdgpu_dc_double DRAMSpeed;
	amdgpu_dc_double DISPCLK;
	amdgpu_dc_double SOCCLK;
	amdgpu_dc_double DCFCLK;

	unsigned int NumberOfActivePlanes;
	unsigned int NumberOfDSCSlices[DC__NUM_DPP__MAX];
	unsigned int ViewportWidth[DC__NUM_DPP__MAX];
	unsigned int ViewportHeight[DC__NUM_DPP__MAX];
	unsigned int ViewportYStartY[DC__NUM_DPP__MAX];
	unsigned int ViewportYStartC[DC__NUM_DPP__MAX];
	unsigned int PitchY[DC__NUM_DPP__MAX];
	unsigned int PitchC[DC__NUM_DPP__MAX];
	amdgpu_dc_double HRatio[DC__NUM_DPP__MAX];
	amdgpu_dc_double VRatio[DC__NUM_DPP__MAX];
	unsigned int htaps[DC__NUM_DPP__MAX];
	unsigned int vtaps[DC__NUM_DPP__MAX];
	unsigned int HTAPsChroma[DC__NUM_DPP__MAX];
	unsigned int VTAPsChroma[DC__NUM_DPP__MAX];
	unsigned int HTotal[DC__NUM_DPP__MAX];
	unsigned int VTotal[DC__NUM_DPP__MAX];
	unsigned int VTotal_Max[DC__NUM_DPP__MAX];
	unsigned int VTotal_Min[DC__NUM_DPP__MAX];
	int DPPPerPlane[DC__NUM_DPP__MAX];
	amdgpu_dc_double PixelClock[DC__NUM_DPP__MAX];
	amdgpu_dc_double PixelClockBackEnd[DC__NUM_DPP__MAX];
	bool DCCEnable[DC__NUM_DPP__MAX];
	bool FECEnable[DC__NUM_DPP__MAX];
	unsigned int DCCMetaPitchY[DC__NUM_DPP__MAX];
	unsigned int DCCMetaPitchC[DC__NUM_DPP__MAX];
	enum scan_direction_class SourceScan[DC__NUM_DPP__MAX];
	enum source_format_class SourcePixelFormat[DC__NUM_DPP__MAX];
	bool WritebackEnable[DC__NUM_DPP__MAX];
	unsigned int ActiveWritebacksPerPlane[DC__NUM_DPP__MAX];
	amdgpu_dc_double WritebackDestinationWidth[DC__NUM_DPP__MAX];
	amdgpu_dc_double WritebackDestinationHeight[DC__NUM_DPP__MAX];
	amdgpu_dc_double WritebackSourceHeight[DC__NUM_DPP__MAX];
	enum source_format_class WritebackPixelFormat[DC__NUM_DPP__MAX];
	unsigned int WritebackLumaHTaps[DC__NUM_DPP__MAX];
	unsigned int WritebackLumaVTaps[DC__NUM_DPP__MAX];
	unsigned int WritebackChromaHTaps[DC__NUM_DPP__MAX];
	unsigned int WritebackChromaVTaps[DC__NUM_DPP__MAX];
	amdgpu_dc_double WritebackHRatio[DC__NUM_DPP__MAX];
	amdgpu_dc_double WritebackVRatio[DC__NUM_DPP__MAX];
	unsigned int HActive[DC__NUM_DPP__MAX];
	unsigned int VActive[DC__NUM_DPP__MAX];
	bool Interlace[DC__NUM_DPP__MAX];
	enum dm_swizzle_mode SurfaceTiling[DC__NUM_DPP__MAX];
	unsigned int ScalerRecoutWidth[DC__NUM_DPP__MAX];
	bool DynamicMetadataEnable[DC__NUM_DPP__MAX];
	int DynamicMetadataLinesBeforeActiveRequired[DC__NUM_DPP__MAX];
	unsigned int DynamicMetadataTransmittedBytes[DC__NUM_DPP__MAX];
	amdgpu_dc_double DCCRate[DC__NUM_DPP__MAX];
	amdgpu_dc_double AverageDCCCompressionRate;
	bool ODMCombineEnabled[DC__NUM_DPP__MAX];
	enum odm_combine_mode ODMCombineTypeEnabled[DC__NUM_DPP__MAX];
	amdgpu_dc_double OutputBpp[DC__NUM_DPP__MAX];
	bool DSCEnabled[DC__NUM_DPP__MAX];
	unsigned int DSCInputBitPerComponent[DC__NUM_DPP__MAX];
	enum output_format_class OutputFormat[DC__NUM_DPP__MAX];
	enum output_encoder_class Output[DC__NUM_DPP__MAX];
	unsigned int BlendingAndTiming[DC__NUM_DPP__MAX];
	bool SynchronizedVBlank;
	unsigned int NumberOfCursors[DC__NUM_DPP__MAX];
	unsigned int CursorWidth[DC__NUM_DPP__MAX][DC__NUM_CURSOR__MAX];
	unsigned int CursorBPP[DC__NUM_DPP__MAX][DC__NUM_CURSOR__MAX];
	bool XFCEnabled[DC__NUM_DPP__MAX];
	bool ScalerEnabled[DC__NUM_DPP__MAX];

	// Intermediates/Informational
	bool ImmediateFlipSupport;
	amdgpu_dc_double DETBufferSizeY[DC__NUM_DPP__MAX];
	amdgpu_dc_double DETBufferSizeC[DC__NUM_DPP__MAX];
	unsigned int SwathHeightY[DC__NUM_DPP__MAX];
	unsigned int SwathHeightC[DC__NUM_DPP__MAX];
	unsigned int LBBitPerPixel[DC__NUM_DPP__MAX];
	amdgpu_dc_double LastPixelOfLineExtraWatermark;
	amdgpu_dc_double TotalDataReadBandwidth;
	unsigned int TotalActiveWriteback;
	unsigned int EffectiveLBLatencyHidingSourceLinesLuma;
	unsigned int EffectiveLBLatencyHidingSourceLinesChroma;
	amdgpu_dc_double BandwidthAvailableForImmediateFlip;
	unsigned int PrefetchMode[DC__VOLTAGE_STATES + 1][2];
	unsigned int MinPrefetchMode;
	unsigned int MaxPrefetchMode;
	bool AnyLinesForVMOrRowTooLarge;
	amdgpu_dc_double MaxVStartup;
	bool IgnoreViewportPositioning;
	bool ErrorResult[DC__NUM_DPP__MAX];
	//
	// Calculated dml_ml->vba.Outputs
	//
	amdgpu_dc_double DCFCLKDeepSleep;
	amdgpu_dc_double UrgentWatermark;
	amdgpu_dc_double UrgentExtraLatency;
	amdgpu_dc_double WritebackUrgentWatermark;
	amdgpu_dc_double StutterExitWatermark;
	amdgpu_dc_double StutterEnterPlusExitWatermark;
	amdgpu_dc_double DRAMClockChangeWatermark;
	amdgpu_dc_double WritebackDRAMClockChangeWatermark;
	amdgpu_dc_double StutterEfficiency;
	amdgpu_dc_double StutterEfficiencyNotIncludingVBlank;
	amdgpu_dc_double NonUrgentLatencyTolerance;
	amdgpu_dc_double MinActiveDRAMClockChangeLatencySupported;

	// These are the clocks calcuated by the library but they are not actually
	// used explicitly. They are fetched by tests and then possibly used. The
	// ultimate values to use are the ones specified by the parameters to DML
	amdgpu_dc_double DISPCLK_calculated;
	amdgpu_dc_double DPPCLK_calculated[DC__NUM_DPP__MAX];

	unsigned int VUpdateOffsetPix[DC__NUM_DPP__MAX];
	amdgpu_dc_double VUpdateWidthPix[DC__NUM_DPP__MAX];
	amdgpu_dc_double VReadyOffsetPix[DC__NUM_DPP__MAX];

	unsigned int TotImmediateFlipBytes;
	amdgpu_dc_double TCalc;

	display_e2e_pipe_params_st cache_pipes[DC__NUM_DPP__MAX];
	unsigned int cache_num_pipes;
	unsigned int pipe_plane[DC__NUM_DPP__MAX];

	/* vba mode support */
	/*inputs*/
	bool SupportGFX7CompatibleTilingIn32bppAnd64bpp;
	amdgpu_dc_double MaxHSCLRatio;
	amdgpu_dc_double MaxVSCLRatio;
	unsigned int MaxNumWriteback;
	bool WritebackLumaAndChromaScalingSupported;
	bool Cursor64BppSupport;
	amdgpu_dc_double DCFCLKPerState[DC__VOLTAGE_STATES + 1];
	amdgpu_dc_double FabricClockPerState[DC__VOLTAGE_STATES + 1];
	amdgpu_dc_double SOCCLKPerState[DC__VOLTAGE_STATES + 1];
	amdgpu_dc_double PHYCLKPerState[DC__VOLTAGE_STATES + 1];
	amdgpu_dc_double DTBCLKPerState[DC__VOLTAGE_STATES + 1];
	amdgpu_dc_double MaxDppclk[DC__VOLTAGE_STATES + 1];
	amdgpu_dc_double MaxDSCCLK[DC__VOLTAGE_STATES + 1];
	amdgpu_dc_double DRAMSpeedPerState[DC__VOLTAGE_STATES + 1];
	amdgpu_dc_double MaxDispclk[DC__VOLTAGE_STATES + 1];
	int VoltageOverrideLevel;

	/*outputs*/
	bool ScaleRatioAndTapsSupport;
	bool SourceFormatPixelAndScanSupport;
	amdgpu_dc_double TotalBandwidthConsumedGBytePerSecond;
	bool DCCEnabledInAnyPlane;
	bool WritebackLatencySupport;
	bool WritebackModeSupport;
	bool Writeback10bpc420Supported;
	bool BandwidthSupport[DC__VOLTAGE_STATES + 1];
	unsigned int TotalNumberOfActiveWriteback;
	amdgpu_dc_double CriticalPoint;
	amdgpu_dc_double ReturnBWToDCNPerState;
	bool IsErrorResult[DC__VOLTAGE_STATES + 1][2][DC__NUM_DPP__MAX];
	bool prefetch_vm_bw_valid;
	bool prefetch_row_bw_valid;
	bool NumberOfOTGSupport;
	bool NonsupportedDSCInputBPC;
	bool WritebackScaleRatioAndTapsSupport;
	bool CursorSupport;
	bool PitchSupport;
	enum dm_validation_status ValidationStatus[DC__VOLTAGE_STATES + 1];

	amdgpu_dc_double WritebackLineBufferLumaBufferSize;
	amdgpu_dc_double WritebackLineBufferChromaBufferSize;
	amdgpu_dc_double WritebackMinHSCLRatio;
	amdgpu_dc_double WritebackMinVSCLRatio;
	amdgpu_dc_double WritebackMaxHSCLRatio;
	amdgpu_dc_double WritebackMaxVSCLRatio;
	amdgpu_dc_double WritebackMaxHSCLTaps;
	amdgpu_dc_double WritebackMaxVSCLTaps;
	unsigned int MaxNumDPP;
	unsigned int MaxNumOTG;
	amdgpu_dc_double CursorBufferSize;
	amdgpu_dc_double CursorChunkSize;
	unsigned int Mode;
	amdgpu_dc_double OutputLinkDPLanes[DC__NUM_DPP__MAX];
	amdgpu_dc_double ForcedOutputLinkBPP[DC__NUM_DPP__MAX]; // Mode Support only
	amdgpu_dc_double ImmediateFlipBW[DC__NUM_DPP__MAX];
	amdgpu_dc_double MaxMaxVStartup;

	amdgpu_dc_double WritebackLumaVExtra;
	amdgpu_dc_double WritebackChromaVExtra;
	amdgpu_dc_double WritebackRequiredDISPCLK;
	amdgpu_dc_double MaximumSwathWidthSupport;
	amdgpu_dc_double MaximumSwathWidthInDETBuffer;
	amdgpu_dc_double MaximumSwathWidthInLineBuffer;
	amdgpu_dc_double MaxDispclkRoundedDownToDFSGranularity;
	amdgpu_dc_double MaxDppclkRoundedDownToDFSGranularity;
	amdgpu_dc_double PlaneRequiredDISPCLKWithoutODMCombine;
	amdgpu_dc_double PlaneRequiredDISPCLKWithODMCombine;
	amdgpu_dc_double PlaneRequiredDISPCLK;
	amdgpu_dc_double TotalNumberOfActiveOTG;
	amdgpu_dc_double FECOverhead;
	amdgpu_dc_double EffectiveFECOverhead;
	amdgpu_dc_double Outbpp;
	unsigned int OutbppDSC;
	amdgpu_dc_double TotalDSCUnitsRequired;
	amdgpu_dc_double bpp;
	unsigned int slices;
	amdgpu_dc_double SwathWidthGranularityY;
	amdgpu_dc_double RoundedUpMaxSwathSizeBytesY;
	amdgpu_dc_double SwathWidthGranularityC;
	amdgpu_dc_double RoundedUpMaxSwathSizeBytesC;
	amdgpu_dc_double EffectiveDETLBLinesLuma;
	amdgpu_dc_double EffectiveDETLBLinesChroma;
	amdgpu_dc_double ProjectedDCFCLKDeepSleep;
	amdgpu_dc_double PDEAndMetaPTEBytesPerFrameY;
	amdgpu_dc_double PDEAndMetaPTEBytesPerFrameC;
	unsigned int MetaRowBytesY;
	unsigned int MetaRowBytesC;
	unsigned int DPTEBytesPerRowC;
	unsigned int DPTEBytesPerRowY;
	amdgpu_dc_double ExtraLatency;
	amdgpu_dc_double TimeCalc;
	amdgpu_dc_double TWait;
	amdgpu_dc_double MaximumReadBandwidthWithPrefetch;
	amdgpu_dc_double MaximumReadBandwidthWithoutPrefetch;
	amdgpu_dc_double total_dcn_read_bw_with_flip;
	amdgpu_dc_double total_dcn_read_bw_with_flip_no_urgent_burst;
	amdgpu_dc_double FractionOfUrgentBandwidth;
	amdgpu_dc_double FractionOfUrgentBandwidthImmediateFlip; // Mode Support debugging output

	/* ms locals */
	amdgpu_dc_double IdealSDPPortBandwidthPerState[DC__VOLTAGE_STATES + 1];
	unsigned int NoOfDPP[DC__VOLTAGE_STATES + 1][2][DC__NUM_DPP__MAX];
	int NoOfDPPThisState[DC__NUM_DPP__MAX];
	bool ODMCombineEnablePerState[DC__VOLTAGE_STATES + 1][DC__NUM_DPP__MAX];
	enum odm_combine_mode ODMCombineTypeEnablePerState[DC__VOLTAGE_STATES + 1][DC__NUM_DPP__MAX];
	unsigned int SwathWidthYThisState[DC__NUM_DPP__MAX];
	unsigned int SwathHeightCPerState[DC__VOLTAGE_STATES + 1][2][DC__NUM_DPP__MAX];
	unsigned int SwathHeightYThisState[DC__NUM_DPP__MAX];
	unsigned int SwathHeightCThisState[DC__NUM_DPP__MAX];
	amdgpu_dc_double VRatioPreY[DC__VOLTAGE_STATES + 1][2][DC__NUM_DPP__MAX];
	amdgpu_dc_double VRatioPreC[DC__VOLTAGE_STATES + 1][2][DC__NUM_DPP__MAX];
	amdgpu_dc_double RequiredPrefetchPixelDataBWLuma[DC__VOLTAGE_STATES + 1][2][DC__NUM_DPP__MAX];
	amdgpu_dc_double RequiredPrefetchPixelDataBWChroma[DC__VOLTAGE_STATES + 1][2][DC__NUM_DPP__MAX];
	amdgpu_dc_double RequiredDPPCLK[DC__VOLTAGE_STATES + 1][2][DC__NUM_DPP__MAX];
	amdgpu_dc_double RequiredDPPCLKThisState[DC__NUM_DPP__MAX];
	bool PTEBufferSizeNotExceededY[DC__VOLTAGE_STATES + 1][2][DC__NUM_DPP__MAX];
	bool PTEBufferSizeNotExceededC[DC__VOLTAGE_STATES + 1][2][DC__NUM_DPP__MAX];
	bool BandwidthWithoutPrefetchSupported[DC__VOLTAGE_STATES + 1];
	bool PrefetchSupported[DC__VOLTAGE_STATES + 1][2];
	bool VRatioInPrefetchSupported[DC__VOLTAGE_STATES + 1][2];
	amdgpu_dc_double RequiredDISPCLK[DC__VOLTAGE_STATES + 1][2];
	bool DISPCLK_DPPCLK_Support[DC__VOLTAGE_STATES + 1][2];
	bool TotalAvailablePipesSupport[DC__VOLTAGE_STATES + 1][2];
	unsigned int TotalNumberOfActiveDPP[DC__VOLTAGE_STATES + 1][2];
	unsigned int TotalNumberOfDCCActiveDPP[DC__VOLTAGE_STATES + 1][2];
	bool ModeSupport[DC__VOLTAGE_STATES + 1][2];
	amdgpu_dc_double ReturnBWPerState[DC__VOLTAGE_STATES + 1];
	bool DIOSupport[DC__VOLTAGE_STATES + 1];
	bool NotEnoughDSCUnits[DC__VOLTAGE_STATES + 1];
	bool DSCCLKRequiredMoreThanSupported[DC__VOLTAGE_STATES + 1];
	bool DTBCLKRequiredMoreThanSupported[DC__VOLTAGE_STATES + 1];
	amdgpu_dc_double UrgentRoundTripAndOutOfOrderLatencyPerState[DC__VOLTAGE_STATES + 1];
	bool ROBSupport[DC__VOLTAGE_STATES + 1];
	bool PTEBufferSizeNotExceeded[DC__VOLTAGE_STATES + 1][2];
	bool TotalVerticalActiveBandwidthSupport[DC__VOLTAGE_STATES + 1];
	amdgpu_dc_double MaxTotalVerticalActiveAvailableBandwidth[DC__VOLTAGE_STATES + 1];
	amdgpu_dc_double PrefetchBW[DC__NUM_DPP__MAX];
	amdgpu_dc_double PDEAndMetaPTEBytesPerFrame[DC__NUM_DPP__MAX];
	amdgpu_dc_double MetaRowBytes[DC__NUM_DPP__MAX];
	amdgpu_dc_double DPTEBytesPerRow[DC__NUM_DPP__MAX];
	amdgpu_dc_double PrefetchLinesY[DC__NUM_DPP__MAX];
	amdgpu_dc_double PrefetchLinesC[DC__NUM_DPP__MAX];
	unsigned int MaxNumSwY[DC__NUM_DPP__MAX];
	unsigned int MaxNumSwC[DC__NUM_DPP__MAX];
	amdgpu_dc_double PrefillY[DC__NUM_DPP__MAX];
	amdgpu_dc_double PrefillC[DC__NUM_DPP__MAX];
	amdgpu_dc_double LineTimesForPrefetch[DC__NUM_DPP__MAX];
	amdgpu_dc_double LinesForMetaPTE[DC__NUM_DPP__MAX];
	amdgpu_dc_double LinesForMetaAndDPTERow[DC__NUM_DPP__MAX];
	amdgpu_dc_double MinDPPCLKUsingSingleDPP[DC__NUM_DPP__MAX];
	unsigned int SwathWidthYSingleDPP[DC__NUM_DPP__MAX];
	amdgpu_dc_double BytePerPixelInDETY[DC__NUM_DPP__MAX];
	amdgpu_dc_double BytePerPixelInDETC[DC__NUM_DPP__MAX];
	bool RequiresDSC[DC__VOLTAGE_STATES + 1][DC__NUM_DPP__MAX];
	unsigned int NumberOfDSCSlice[DC__VOLTAGE_STATES + 1][DC__NUM_DPP__MAX];
	amdgpu_dc_double RequiresFEC[DC__VOLTAGE_STATES + 1][DC__NUM_DPP__MAX];
	amdgpu_dc_double OutputBppPerState[DC__VOLTAGE_STATES + 1][DC__NUM_DPP__MAX];
	amdgpu_dc_double DSCDelayPerState[DC__VOLTAGE_STATES + 1][DC__NUM_DPP__MAX];
	bool ViewportSizeSupport[DC__VOLTAGE_STATES + 1];
	unsigned int Read256BlockHeightY[DC__NUM_DPP__MAX];
	unsigned int Read256BlockWidthY[DC__NUM_DPP__MAX];
	unsigned int Read256BlockHeightC[DC__NUM_DPP__MAX];
	unsigned int Read256BlockWidthC[DC__NUM_DPP__MAX];
	amdgpu_dc_double MaxSwathHeightY[DC__NUM_DPP__MAX];
	amdgpu_dc_double MaxSwathHeightC[DC__NUM_DPP__MAX];
	amdgpu_dc_double MinSwathHeightY[DC__NUM_DPP__MAX];
	amdgpu_dc_double MinSwathHeightC[DC__NUM_DPP__MAX];
	amdgpu_dc_double ReadBandwidthLuma[DC__NUM_DPP__MAX];
	amdgpu_dc_double ReadBandwidthChroma[DC__NUM_DPP__MAX];
	amdgpu_dc_double ReadBandwidth[DC__NUM_DPP__MAX];
	amdgpu_dc_double WriteBandwidth[DC__NUM_DPP__MAX];
	amdgpu_dc_double PSCL_FACTOR[DC__NUM_DPP__MAX];
	amdgpu_dc_double PSCL_FACTOR_CHROMA[DC__NUM_DPP__MAX];
	amdgpu_dc_double MaximumVStartup[DC__NUM_DPP__MAX];
	unsigned int MacroTileWidthY[DC__NUM_DPP__MAX];
	unsigned int MacroTileWidthC[DC__NUM_DPP__MAX];
	amdgpu_dc_double AlignedDCCMetaPitch[DC__NUM_DPP__MAX];
	amdgpu_dc_double AlignedYPitch[DC__NUM_DPP__MAX];
	amdgpu_dc_double AlignedCPitch[DC__NUM_DPP__MAX];
	amdgpu_dc_double MaximumSwathWidth[DC__NUM_DPP__MAX];
	amdgpu_dc_double cursor_bw[DC__NUM_DPP__MAX];
	amdgpu_dc_double cursor_bw_pre[DC__NUM_DPP__MAX];
	amdgpu_dc_double Tno_bw[DC__NUM_DPP__MAX];
	amdgpu_dc_double prefetch_vmrow_bw[DC__NUM_DPP__MAX];
	amdgpu_dc_double DestinationLinesToRequestVMInImmediateFlip[DC__NUM_DPP__MAX];
	amdgpu_dc_double DestinationLinesToRequestRowInImmediateFlip[DC__NUM_DPP__MAX];
	amdgpu_dc_double final_flip_bw[DC__NUM_DPP__MAX];
	bool ImmediateFlipSupportedForState[DC__VOLTAGE_STATES + 1][2];
	amdgpu_dc_double WritebackDelay[DC__VOLTAGE_STATES + 1][DC__NUM_DPP__MAX];
	unsigned int vm_group_bytes[DC__NUM_DPP__MAX];
	long dpte_group_bytes[DC__NUM_DPP__MAX];
	unsigned int dpte_row_height[DC__NUM_DPP__MAX];
	unsigned int meta_req_height[DC__NUM_DPP__MAX];
	unsigned int meta_req_width[DC__NUM_DPP__MAX];
	unsigned int meta_row_height[DC__NUM_DPP__MAX];
	unsigned int meta_row_width[DC__NUM_DPP__MAX];
	unsigned int dpte_row_height_chroma[DC__NUM_DPP__MAX];
	unsigned int meta_req_height_chroma[DC__NUM_DPP__MAX];
	unsigned int meta_req_width_chroma[DC__NUM_DPP__MAX];
	unsigned int meta_row_height_chroma[DC__NUM_DPP__MAX];
	unsigned int meta_row_width_chroma[DC__NUM_DPP__MAX];
	bool ImmediateFlipSupportedForPipe[DC__NUM_DPP__MAX];
	amdgpu_dc_double meta_row_bw[DC__NUM_DPP__MAX];
	amdgpu_dc_double dpte_row_bw[DC__NUM_DPP__MAX];
	amdgpu_dc_double DisplayPipeLineDeliveryTimeLuma[DC__NUM_DPP__MAX];                     // WM
	amdgpu_dc_double DisplayPipeLineDeliveryTimeChroma[DC__NUM_DPP__MAX];                     // WM
	amdgpu_dc_double DisplayPipeRequestDeliveryTimeLuma[DC__NUM_DPP__MAX];
	amdgpu_dc_double DisplayPipeRequestDeliveryTimeChroma[DC__NUM_DPP__MAX];
	enum clock_change_support DRAMClockChangeSupport[DC__VOLTAGE_STATES + 1][2];
	amdgpu_dc_double UrgentBurstFactorCursor[DC__NUM_DPP__MAX];
	amdgpu_dc_double UrgentBurstFactorCursorPre[DC__NUM_DPP__MAX];
	amdgpu_dc_double UrgentBurstFactorLuma[DC__NUM_DPP__MAX];
	amdgpu_dc_double UrgentBurstFactorLumaPre[DC__NUM_DPP__MAX];
	amdgpu_dc_double UrgentBurstFactorChroma[DC__NUM_DPP__MAX];
	amdgpu_dc_double UrgentBurstFactorChromaPre[DC__NUM_DPP__MAX];

	bool           MPCCombine[DC__VOLTAGE_STATES + 1][2][DC__NUM_DPP__MAX];
	amdgpu_dc_double         SwathWidthCSingleDPP[DC__NUM_DPP__MAX];
	amdgpu_dc_double         MaximumSwathWidthInLineBufferLuma;
	amdgpu_dc_double         MaximumSwathWidthInLineBufferChroma;
	amdgpu_dc_double         MaximumSwathWidthLuma[DC__NUM_DPP__MAX];
	amdgpu_dc_double         MaximumSwathWidthChroma[DC__NUM_DPP__MAX];
	bool odm_combine_dummy[DC__NUM_DPP__MAX];
	enum odm_combine_mode odm_combine_mode_dummy[DC__NUM_DPP__MAX];
	amdgpu_dc_double         dummy1[DC__NUM_DPP__MAX];
	amdgpu_dc_double         dummy2[DC__NUM_DPP__MAX];
	amdgpu_dc_double         dummy3[DC__NUM_DPP__MAX];
	amdgpu_dc_double         dummy4[DC__NUM_DPP__MAX];
	amdgpu_dc_double         dummy5;
	amdgpu_dc_double         dummy6;
	amdgpu_dc_double         dummy7[DC__NUM_DPP__MAX];
	amdgpu_dc_double         dummy8[DC__NUM_DPP__MAX];
	unsigned int        dummyinteger1ms[DC__NUM_DPP__MAX];
	unsigned int        dummyinteger2ms[DC__NUM_DPP__MAX];
	unsigned int        dummyinteger3[DC__NUM_DPP__MAX];
	unsigned int        dummyinteger4;
	unsigned int        dummyinteger5;
	unsigned int        dummyinteger6;
	unsigned int        dummyinteger7;
	unsigned int        dummyinteger8;
	unsigned int        dummyinteger9;
	unsigned int        dummyinteger10;
	unsigned int        dummyinteger11;
	unsigned int        dummyinteger12;
	unsigned int        dummyintegerarr1[DC__NUM_DPP__MAX];
	unsigned int        dummyintegerarr2[DC__NUM_DPP__MAX];
	unsigned int        dummyintegerarr3[DC__NUM_DPP__MAX];
	unsigned int        dummyintegerarr4[DC__NUM_DPP__MAX];
	long                dummylongarr1[DC__NUM_DPP__MAX];
	bool           dummysinglestring;
	bool           SingleDPPViewportSizeSupportPerPlane[DC__NUM_DPP__MAX];
	amdgpu_dc_double         PlaneRequiredDISPCLKWithODMCombine2To1;
	amdgpu_dc_double         PlaneRequiredDISPCLKWithODMCombine4To1;
	unsigned int   TotalNumberOfSingleDPPPlanes[DC__VOLTAGE_STATES + 1][2];
	bool           LinkDSCEnable;
	bool           ODMCombine4To1SupportCheckOK[DC__VOLTAGE_STATES + 1];
	bool ODMCombineEnableThisState[DC__NUM_DPP__MAX];
	enum odm_combine_mode ODMCombineEnableTypeThisState[DC__NUM_DPP__MAX];
	unsigned int   SwathWidthCThisState[DC__NUM_DPP__MAX];
	bool           ViewportSizeSupportPerPlane[DC__NUM_DPP__MAX];
	amdgpu_dc_double         AlignedDCCMetaPitchY[DC__NUM_DPP__MAX];
	amdgpu_dc_double         AlignedDCCMetaPitchC[DC__NUM_DPP__MAX];

	unsigned int NotEnoughUrgentLatencyHiding;
	unsigned int NotEnoughUrgentLatencyHidingPre;
	long PTEBufferSizeInRequestsForLuma;
	long PTEBufferSizeInRequestsForChroma;

	// Missing from VBA
	long dpte_group_bytes_chroma;
	unsigned int vm_group_bytes_chroma;
	amdgpu_dc_double dst_x_after_scaler;
	amdgpu_dc_double dst_y_after_scaler;
	unsigned int VStartupRequiredWhenNotEnoughTimeForDynamicMetadata;

	/* perf locals*/
	amdgpu_dc_double PrefetchBandwidth[DC__NUM_DPP__MAX];
	amdgpu_dc_double VInitPreFillY[DC__NUM_DPP__MAX];
	amdgpu_dc_double VInitPreFillC[DC__NUM_DPP__MAX];
	unsigned int MaxNumSwathY[DC__NUM_DPP__MAX];
	unsigned int MaxNumSwathC[DC__NUM_DPP__MAX];
	unsigned int VStartup[DC__NUM_DPP__MAX];
	amdgpu_dc_double DSTYAfterScaler[DC__NUM_DPP__MAX];
	amdgpu_dc_double DSTXAfterScaler[DC__NUM_DPP__MAX];
	bool AllowDRAMClockChangeDuringVBlank[DC__NUM_DPP__MAX];
	bool AllowDRAMSelfRefreshDuringVBlank[DC__NUM_DPP__MAX];
	amdgpu_dc_double VRatioPrefetchY[DC__NUM_DPP__MAX];
	amdgpu_dc_double VRatioPrefetchC[DC__NUM_DPP__MAX];
	amdgpu_dc_double DestinationLinesForPrefetch[DC__NUM_DPP__MAX];
	amdgpu_dc_double DestinationLinesToRequestVMInVBlank[DC__NUM_DPP__MAX];
	amdgpu_dc_double DestinationLinesToRequestRowInVBlank[DC__NUM_DPP__MAX];
	amdgpu_dc_double MinTTUVBlank[DC__NUM_DPP__MAX];
	amdgpu_dc_double BytePerPixelDETY[DC__NUM_DPP__MAX];
	amdgpu_dc_double BytePerPixelDETC[DC__NUM_DPP__MAX];
	unsigned int SwathWidthY[DC__NUM_DPP__MAX];
	unsigned int SwathWidthSingleDPPY[DC__NUM_DPP__MAX];
	amdgpu_dc_double CursorRequestDeliveryTime[DC__NUM_DPP__MAX];
	amdgpu_dc_double CursorRequestDeliveryTimePrefetch[DC__NUM_DPP__MAX];
	amdgpu_dc_double ReadBandwidthPlaneLuma[DC__NUM_DPP__MAX];
	amdgpu_dc_double ReadBandwidthPlaneChroma[DC__NUM_DPP__MAX];
	amdgpu_dc_double DisplayPipeLineDeliveryTimeLumaPrefetch[DC__NUM_DPP__MAX];
	amdgpu_dc_double DisplayPipeLineDeliveryTimeChromaPrefetch[DC__NUM_DPP__MAX];
	amdgpu_dc_double DisplayPipeRequestDeliveryTimeLumaPrefetch[DC__NUM_DPP__MAX];
	amdgpu_dc_double DisplayPipeRequestDeliveryTimeChromaPrefetch[DC__NUM_DPP__MAX];
	amdgpu_dc_double PixelPTEBytesPerRow[DC__NUM_DPP__MAX];
	amdgpu_dc_double PDEAndMetaPTEBytesFrame[DC__NUM_DPP__MAX];
	amdgpu_dc_double MetaRowByte[DC__NUM_DPP__MAX];
	amdgpu_dc_double PrefetchSourceLinesY[DC__NUM_DPP__MAX];
	amdgpu_dc_double RequiredPrefetchPixDataBWLuma[DC__NUM_DPP__MAX];
	amdgpu_dc_double RequiredPrefetchPixDataBWChroma[DC__NUM_DPP__MAX];
	amdgpu_dc_double PrefetchSourceLinesC[DC__NUM_DPP__MAX];
	amdgpu_dc_double PSCL_THROUGHPUT_LUMA[DC__NUM_DPP__MAX];
	amdgpu_dc_double PSCL_THROUGHPUT_CHROMA[DC__NUM_DPP__MAX];
	amdgpu_dc_double DSCCLK_calculated[DC__NUM_DPP__MAX];
	unsigned int DSCDelay[DC__NUM_DPP__MAX];
	unsigned int MaxVStartupLines[DC__NUM_DPP__MAX];
	amdgpu_dc_double DPPCLKUsingSingleDPP[DC__NUM_DPP__MAX];
	amdgpu_dc_double DPPCLK[DC__NUM_DPP__MAX];
	unsigned int DCCYMaxUncompressedBlock[DC__NUM_DPP__MAX];
	unsigned int DCCYMaxCompressedBlock[DC__NUM_DPP__MAX];
	unsigned int DCCYIndependent64ByteBlock[DC__NUM_DPP__MAX];
	amdgpu_dc_double MaximumDCCCompressionYSurface[DC__NUM_DPP__MAX];
	unsigned int BlockHeight256BytesY[DC__NUM_DPP__MAX];
	unsigned int BlockHeight256BytesC[DC__NUM_DPP__MAX];
	unsigned int BlockWidth256BytesY[DC__NUM_DPP__MAX];
	unsigned int BlockWidth256BytesC[DC__NUM_DPP__MAX];
	amdgpu_dc_double XFCSlaveVUpdateOffset[DC__NUM_DPP__MAX];
	amdgpu_dc_double XFCSlaveVupdateWidth[DC__NUM_DPP__MAX];
	amdgpu_dc_double XFCSlaveVReadyOffset[DC__NUM_DPP__MAX];
	amdgpu_dc_double XFCTransferDelay[DC__NUM_DPP__MAX];
	amdgpu_dc_double XFCPrechargeDelay[DC__NUM_DPP__MAX];
	amdgpu_dc_double XFCRemoteSurfaceFlipLatency[DC__NUM_DPP__MAX];
	amdgpu_dc_double XFCPrefetchMargin[DC__NUM_DPP__MAX];
	unsigned int dpte_row_width_luma_ub[DC__NUM_DPP__MAX];
	unsigned int dpte_row_width_chroma_ub[DC__NUM_DPP__MAX];
	amdgpu_dc_double FullDETBufferingTimeY[DC__NUM_DPP__MAX];                     // WM
	amdgpu_dc_double FullDETBufferingTimeC[DC__NUM_DPP__MAX];                     // WM
	amdgpu_dc_double DST_Y_PER_PTE_ROW_NOM_L[DC__NUM_DPP__MAX];
	amdgpu_dc_double DST_Y_PER_PTE_ROW_NOM_C[DC__NUM_DPP__MAX];
	amdgpu_dc_double DST_Y_PER_META_ROW_NOM_L[DC__NUM_DPP__MAX];
	amdgpu_dc_double TimePerMetaChunkNominal[DC__NUM_DPP__MAX];
	amdgpu_dc_double TimePerMetaChunkVBlank[DC__NUM_DPP__MAX];
	amdgpu_dc_double TimePerMetaChunkFlip[DC__NUM_DPP__MAX];
	unsigned int swath_width_luma_ub[DC__NUM_DPP__MAX];
	unsigned int swath_width_chroma_ub[DC__NUM_DPP__MAX];
	unsigned int PixelPTEReqWidthY[DC__NUM_DPP__MAX];
	unsigned int PixelPTEReqHeightY[DC__NUM_DPP__MAX];
	unsigned int PTERequestSizeY[DC__NUM_DPP__MAX];
	unsigned int PixelPTEReqWidthC[DC__NUM_DPP__MAX];
	unsigned int PixelPTEReqHeightC[DC__NUM_DPP__MAX];
	unsigned int PTERequestSizeC[DC__NUM_DPP__MAX];
	amdgpu_dc_double time_per_pte_group_nom_luma[DC__NUM_DPP__MAX];
	amdgpu_dc_double time_per_pte_group_nom_chroma[DC__NUM_DPP__MAX];
	amdgpu_dc_double time_per_pte_group_vblank_luma[DC__NUM_DPP__MAX];
	amdgpu_dc_double time_per_pte_group_vblank_chroma[DC__NUM_DPP__MAX];
	amdgpu_dc_double time_per_pte_group_flip_luma[DC__NUM_DPP__MAX];
	amdgpu_dc_double time_per_pte_group_flip_chroma[DC__NUM_DPP__MAX];
	amdgpu_dc_double TimePerVMGroupVBlank[DC__NUM_DPP__MAX];
	amdgpu_dc_double TimePerVMGroupFlip[DC__NUM_DPP__MAX];
	amdgpu_dc_double TimePerVMRequestVBlank[DC__NUM_DPP__MAX];
	amdgpu_dc_double TimePerVMRequestFlip[DC__NUM_DPP__MAX];
	unsigned int dpde0_bytes_per_frame_ub_l[DC__NUM_DPP__MAX];
	unsigned int meta_pte_bytes_per_frame_ub_l[DC__NUM_DPP__MAX];
	unsigned int dpde0_bytes_per_frame_ub_c[DC__NUM_DPP__MAX];
	unsigned int meta_pte_bytes_per_frame_ub_c[DC__NUM_DPP__MAX];
	amdgpu_dc_double LinesToFinishSwathTransferStutterCriticalPlane;
	unsigned int BytePerPixelYCriticalPlane;
	amdgpu_dc_double SwathWidthYCriticalPlane;
	amdgpu_dc_double LinesInDETY[DC__NUM_DPP__MAX];
	amdgpu_dc_double LinesInDETYRoundedDownToSwath[DC__NUM_DPP__MAX];

	unsigned int SwathWidthSingleDPPC[DC__NUM_DPP__MAX];
	unsigned int SwathWidthC[DC__NUM_DPP__MAX];
	unsigned int BytePerPixelY[DC__NUM_DPP__MAX];
	unsigned int BytePerPixelC[DC__NUM_DPP__MAX];
	long dummyinteger1;
	long dummyinteger2;
	amdgpu_dc_double FinalDRAMClockChangeLatency;
	amdgpu_dc_double Tdmdl_vm[DC__NUM_DPP__MAX];
	amdgpu_dc_double Tdmdl[DC__NUM_DPP__MAX];
	unsigned int ThisVStartup;
	bool WritebackAllowDRAMClockChangeEndPosition[DC__NUM_DPP__MAX];
	amdgpu_dc_double DST_Y_PER_META_ROW_NOM_C[DC__NUM_DPP__MAX];
	amdgpu_dc_double TimePerChromaMetaChunkNominal[DC__NUM_DPP__MAX];
	amdgpu_dc_double TimePerChromaMetaChunkVBlank[DC__NUM_DPP__MAX];
	amdgpu_dc_double TimePerChromaMetaChunkFlip[DC__NUM_DPP__MAX];
	unsigned int DCCCMaxUncompressedBlock[DC__NUM_DPP__MAX];
	unsigned int DCCCMaxCompressedBlock[DC__NUM_DPP__MAX];
	unsigned int DCCCIndependent64ByteBlock[DC__NUM_DPP__MAX];
	amdgpu_dc_double VStartupMargin;

	/* Missing from VBA */
	unsigned int MaximumMaxVStartupLines;
	amdgpu_dc_double FabricAndDRAMBandwidth;
	amdgpu_dc_double LinesInDETLuma;
	amdgpu_dc_double LinesInDETChroma;
	unsigned int ImmediateFlipBytes[DC__NUM_DPP__MAX];
	unsigned int LinesInDETC[DC__NUM_DPP__MAX];
	unsigned int LinesInDETCRoundedDownToSwath[DC__NUM_DPP__MAX];
	amdgpu_dc_double UrgentLatencySupportUsPerState[DC__VOLTAGE_STATES + 1][2][DC__NUM_DPP__MAX];
	amdgpu_dc_double UrgentLatencySupportUs[DC__NUM_DPP__MAX];
	amdgpu_dc_double FabricAndDRAMBandwidthPerState[DC__VOLTAGE_STATES + 1];
	bool UrgentLatencySupport[DC__VOLTAGE_STATES + 1][2];
	unsigned int SwathWidthYPerState[DC__VOLTAGE_STATES + 1][2][DC__NUM_DPP__MAX];
	unsigned int SwathHeightYPerState[DC__VOLTAGE_STATES + 1][2][DC__NUM_DPP__MAX];
	amdgpu_dc_double qual_row_bw[DC__NUM_DPP__MAX];
	amdgpu_dc_double prefetch_row_bw[DC__NUM_DPP__MAX];
	amdgpu_dc_double prefetch_vm_bw[DC__NUM_DPP__MAX];

	amdgpu_dc_double PTEGroupSize;
	unsigned int PDEProcessingBufIn64KBReqs;

	amdgpu_dc_double MaxTotalVActiveRDBandwidth;
	bool DoUrgentLatencyAdjustment;
	amdgpu_dc_double UrgentLatencyAdjustmentFabricClockComponent;
	amdgpu_dc_double UrgentLatencyAdjustmentFabricClockReference;
	amdgpu_dc_double MinUrgentLatencySupportUs;
	amdgpu_dc_double MinFullDETBufferingTime;
	amdgpu_dc_double AverageReadBandwidthGBytePerSecond;
	bool   FirstMainPlane;

	unsigned int ViewportWidthChroma[DC__NUM_DPP__MAX];
	unsigned int ViewportHeightChroma[DC__NUM_DPP__MAX];
	amdgpu_dc_double HRatioChroma[DC__NUM_DPP__MAX];
	amdgpu_dc_double VRatioChroma[DC__NUM_DPP__MAX];
	long WritebackSourceWidth[DC__NUM_DPP__MAX];

	bool ModeIsSupported;
	bool ODMCombine4To1Supported;

	unsigned int SurfaceWidthY[DC__NUM_DPP__MAX];
	unsigned int SurfaceWidthC[DC__NUM_DPP__MAX];
	unsigned int SurfaceHeightY[DC__NUM_DPP__MAX];
	unsigned int SurfaceHeightC[DC__NUM_DPP__MAX];
	unsigned int WritebackHTaps[DC__NUM_DPP__MAX];
	unsigned int WritebackVTaps[DC__NUM_DPP__MAX];
	bool DSCEnable[DC__NUM_DPP__MAX];

	amdgpu_dc_double DRAMClockChangeLatencyOverride;

	amdgpu_dc_double GPUVMMinPageSize;
	amdgpu_dc_double HostVMMinPageSize;

	bool   MPCCombineEnable[DC__NUM_DPP__MAX];
	unsigned int HostVMMaxNonCachedPageTableLevels;
	bool   DynamicMetadataVMEnabled;
	amdgpu_dc_double       WritebackInterfaceBufferSize;
	amdgpu_dc_double       WritebackLineBufferSize;

	amdgpu_dc_double DCCRateLuma[DC__NUM_DPP__MAX];
	amdgpu_dc_double DCCRateChroma[DC__NUM_DPP__MAX];

	amdgpu_dc_double PHYCLKD18PerState[DC__VOLTAGE_STATES + 1];
	int MinVoltageLevel;
	int MaxVoltageLevel;

	bool WritebackSupportInterleaveAndUsingWholeBufferForASingleStream;
	bool NumberOfHDMIFRLSupport;
	unsigned int MaxNumHDMIFRLOutputs;
	int    AudioSampleRate[DC__NUM_DPP__MAX];
	int    AudioSampleLayout[DC__NUM_DPP__MAX];
};

bool CalculateMinAndMaxPrefetchMode(
		enum self_refresh_affinity AllowDRAMSelfRefreshOrDRAMClockChangeInVblank,
		unsigned int *MinPrefetchMode,
		unsigned int *MaxPrefetchMode);

amdgpu_dc_double CalculateWriteBackDISPCLK(
		enum source_format_class WritebackPixelFormat,
		amdgpu_dc_double PixelClock,
		amdgpu_dc_double WritebackHRatio,
		amdgpu_dc_double WritebackVRatio,
		unsigned int WritebackLumaHTaps,
		unsigned int WritebackLumaVTaps,
		unsigned int WritebackChromaHTaps,
		unsigned int WritebackChromaVTaps,
		amdgpu_dc_double WritebackDestinationWidth,
		unsigned int HTotal,
		unsigned int WritebackChromaLineBufferWidth);

#endif /* _DML2_DISPLAY_MODE_VBA_H_ */
#endif
