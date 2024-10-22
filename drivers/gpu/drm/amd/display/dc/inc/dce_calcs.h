/*
 * Copyright 2015-2017 Advanced Micro Devices, Inc.
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

/**
 * Bandwidth and Watermark calculations interface.
 * (Refer to "DCEx_mode_support.xlsm" from Perforce.)
 */
#ifndef __DCE_CALCS_H__
#define __DCE_CALCS_H__

#include "bw_fixed.h"

struct pipe_ctx;
struct dc;
struct dc_state;
struct dce_bw_output;

enum bw_calcs_version {
	BW_CALCS_VERSION_INVALID,
	BW_CALCS_VERSION_CARRIZO,
	BW_CALCS_VERSION_POLARIS10,
	BW_CALCS_VERSION_POLARIS11,
	BW_CALCS_VERSION_POLARIS12,
	BW_CALCS_VERSION_VEGAM,
	BW_CALCS_VERSION_STONEY,
	BW_CALCS_VERSION_VEGA10
};

/*******************************************************************************
 * There are three types of input into Calculations:
 * 1. per-DCE static values - these are "hardcoded" properties of the DCEIP
 * 2. board-level values - these are generally coming from VBIOS parser
 * 3. mode/configuration values - depending Mode, Scaling number of Displays etc.
 ******************************************************************************/

enum bw_defines {
	//Common
	bw_def_no = 0,
	bw_def_none = 0,
	bw_def_yes = 1,
	bw_def_ok = 1,
	bw_def_high = 2,
	bw_def_mid = 1,
	bw_def_low = 0,

	//Internal
	bw_defs_start = 255,
	bw_def_underlay422,
	bw_def_underlay420_luma,
	bw_def_underlay420_chroma,
	bw_def_underlay444,
	bw_def_graphics,
	bw_def_display_write_back420_luma,
	bw_def_display_write_back420_chroma,
	bw_def_portrait,
	bw_def_hsr_mtn_4,
	bw_def_hsr_mtn_h_taps,
	bw_def_ceiling__h_taps_div_4___meq_hsr,
	bw_def_invalid_linear_or_stereo_mode,
	bw_def_invalid_rotation_or_bpp_or_stereo,
	bw_def_vsr_mtn_v_taps,
	bw_def_vsr_mtn_4,
	bw_def_auto,
	bw_def_manual,
	bw_def_exceeded_allowed_maximum_sclk,
	bw_def_exceeded_allowed_page_close_open,
	bw_def_exceeded_allowed_outstanding_pte_req_queue_size,
	bw_def_exceeded_allowed_maximum_bw,
	bw_def_landscape,

	//Panning and bezel
	bw_def_any_lines,

	//Underlay mode
	bw_def_underlay_only,
	bw_def_blended,
	bw_def_blend,

	//Stereo mode
	bw_def_mono,
	bw_def_side_by_side,
	bw_def_top_bottom,

	//Underlay surface type
	bw_def_420,
	bw_def_422,
	bw_def_444,

	//Tiling mode
	bw_def_linear,
	bw_def_tiled,
	bw_def_array_linear_general,
	bw_def_array_linear_aligned,
	bw_def_rotated_micro_tiling,
	bw_def_display_micro_tiling,

	//Memory type
	bw_def_gddr5,
	bw_def_hbm,

	//Voltage
	bw_def_high_no_nbp_state_change,
	bw_def_0_72,
	bw_def_0_8,
	bw_def_0_9,

	bw_def_notok = -1,
	bw_def_na = -1
};

struct bw_calcs_dceip {
	enum bw_calcs_version version;
	uint32_t percent_of_ideal_port_bw_received_after_urgent_latency;
	uint32_t max_average_percent_of_ideal_port_bw_display_can_use_in_normal_system_operation;
	uint32_t max_average_percent_of_ideal_drambw_display_can_use_in_normal_system_operation;
	bool large_cursor;
	uint32_t cursor_max_outstanding_group_num;
	bool dmif_pipe_en_fbc_chunk_tracker;
	bw_fixed dmif_request_buffer_size;
	uint32_t lines_interleaved_into_lb;
	uint32_t low_power_tiling_mode;
	uint32_t chunk_width;
	uint32_t number_of_graphics_pipes;
	uint32_t number_of_underlay_pipes;
	bool display_write_back_supported;
	bool argb_compression_support;
	bw_fixed underlay_vscaler_efficiency6_bit_per_component;
	bw_fixed underlay_vscaler_efficiency8_bit_per_component;
	bw_fixed underlay_vscaler_efficiency10_bit_per_component;
	bw_fixed underlay_vscaler_efficiency12_bit_per_component;
	bw_fixed graphics_vscaler_efficiency6_bit_per_component;
	bw_fixed graphics_vscaler_efficiency8_bit_per_component;
	bw_fixed graphics_vscaler_efficiency10_bit_per_component;
	bw_fixed graphics_vscaler_efficiency12_bit_per_component;
	bw_fixed alpha_vscaler_efficiency;
	uint32_t max_dmif_buffer_allocated;
	uint32_t graphics_dmif_size;
	uint32_t underlay_luma_dmif_size;
	uint32_t underlay_chroma_dmif_size;
	bool pre_downscaler_enabled;
	bool underlay_downscale_prefetch_enabled;
	bw_fixed lb_write_pixels_per_dispclk;
	bw_fixed lb_size_per_component444;
	bool graphics_lb_nodownscaling_multi_line_prefetching;
	bw_fixed stutter_and_dram_clock_state_change_gated_before_cursor;
	bw_fixed underlay420_luma_lb_size_per_component;
	bw_fixed underlay420_chroma_lb_size_per_component;
	bw_fixed underlay422_lb_size_per_component;
	bw_fixed cursor_chunk_width;
	bw_fixed cursor_dcp_buffer_lines;
	bw_fixed underlay_maximum_width_efficient_for_tiling;
	bw_fixed underlay_maximum_height_efficient_for_tiling;
	bw_fixed peak_pte_request_to_eviction_ratio_limiting_multiple_displays_or_single_rotated_display;
	bw_fixed peak_pte_request_to_eviction_ratio_limiting_single_display_no_rotation;
	bw_fixed minimum_outstanding_pte_request_limit;
	bw_fixed maximum_total_outstanding_pte_requests_allowed_by_saw;
	bool limit_excessive_outstanding_dmif_requests;
	bw_fixed linear_mode_line_request_alternation_slice;
	uint32_t scatter_gather_lines_of_pte_prefetching_in_linear_mode;
	uint32_t display_write_back420_luma_mcifwr_buffer_size;
	uint32_t display_write_back420_chroma_mcifwr_buffer_size;
	bw_fixed request_efficiency;
	bw_fixed dispclk_per_request;
	bw_fixed dispclk_ramping_factor;
	bw_fixed display_pipe_throughput_factor;
	uint32_t scatter_gather_pte_request_rows_in_tiling_mode;
	bw_fixed mcifwr_all_surfaces_burst_time;
};

struct bw_calcs_vbios {
	enum bw_defines memory_type;
	uint32_t dram_channel_width_in_bits;
	uint32_t number_of_dram_channels;
	uint32_t number_of_dram_banks;
	bw_fixed low_yclk; /*m_hz*/
	bw_fixed mid_yclk; /*m_hz*/
	bw_fixed high_yclk; /*m_hz*/
	bw_fixed low_sclk; /*m_hz*/
	bw_fixed mid1_sclk; /*m_hz*/
	bw_fixed mid2_sclk; /*m_hz*/
	bw_fixed mid3_sclk; /*m_hz*/
	bw_fixed mid4_sclk; /*m_hz*/
	bw_fixed mid5_sclk; /*m_hz*/
	bw_fixed mid6_sclk; /*m_hz*/
	bw_fixed high_sclk; /*m_hz*/
	bw_fixed low_voltage_max_dispclk; /*m_hz*/
	bw_fixed mid_voltage_max_dispclk; /*m_hz*/
	bw_fixed high_voltage_max_dispclk; /*m_hz*/
	bw_fixed low_voltage_max_phyclk;
	bw_fixed mid_voltage_max_phyclk;
	bw_fixed high_voltage_max_phyclk;
	bw_fixed data_return_bus_width;
	bw_fixed trc;
	bw_fixed dmifmc_urgent_latency;
	bw_fixed stutter_self_refresh_exit_latency;
	bw_fixed stutter_self_refresh_entry_latency;
	bw_fixed nbp_state_change_latency;
	bw_fixed mcifwrmc_urgent_latency;
	bool scatter_gather_enable;
	bw_fixed down_spread_percentage;
	uint32_t cursor_width;
	uint32_t average_compression_rate;
	uint32_t number_of_request_slots_gmc_reserves_for_dmif_per_channel;
	bw_fixed blackout_duration;
	bw_fixed maximum_blackout_recovery_time;
};

/*******************************************************************************
 * Temporary data structure(s).
 ******************************************************************************/
#define maximum_number_of_surfaces 12
/*Units : MHz, us */

struct bw_calcs_data {
	/* data for all displays */
	bool display_synchronization_enabled;
	uint32_t number_of_displays;
	enum bw_defines underlay_surface_type;
	enum bw_defines panning_and_bezel_adjustment;
	enum bw_defines graphics_tiling_mode;
	uint32_t graphics_lb_bpc;
	uint32_t underlay_lb_bpc;
	enum bw_defines underlay_tiling_mode;
	enum bw_defines d0_underlay_mode;
	bool d1_display_write_back_dwb_enable;
	enum bw_defines d1_underlay_mode;

	bool increase_voltage_to_support_mclk_switch;
	bool cpup_state_change_enable;
	bool cpuc_state_change_enable;
	bool nbp_state_change_enable;
	bool stutter_mode_enable;
	uint32_t y_clk_level;
	uint32_t sclk_level;
	uint32_t number_of_underlay_surfaces;
	uint32_t number_of_dram_wrchannels;
	uint32_t chunk_request_delay;
	uint32_t number_of_dram_channels;
	enum bw_defines underlay_micro_tile_mode;
	enum bw_defines graphics_micro_tile_mode;
	bw_fixed max_phyclk;
	bw_fixed dram_efficiency;
	bw_fixed src_width_after_surface_type;
	bw_fixed src_height_after_surface_type;
	bw_fixed hsr_after_surface_type;
	bw_fixed vsr_after_surface_type;
	bw_fixed src_width_after_rotation;
	bw_fixed src_height_after_rotation;
	bw_fixed hsr_after_rotation;
	bw_fixed vsr_after_rotation;
	bw_fixed source_height_pixels;
	bw_fixed hsr_after_stereo;
	bw_fixed vsr_after_stereo;
	bw_fixed source_width_in_lb;
	bw_fixed lb_line_pitch;
	bw_fixed underlay_maximum_source_efficient_for_tiling;
	bw_fixed num_lines_at_frame_start;
	bw_fixed min_dmif_size_in_time;
	bw_fixed min_mcifwr_size_in_time;
	bw_fixed total_requests_for_dmif_size;
	bw_fixed peak_pte_request_to_eviction_ratio_limiting;
	bw_fixed useful_pte_per_pte_request;
	bw_fixed scatter_gather_pte_request_rows;
	bw_fixed scatter_gather_row_height;
	bw_fixed scatter_gather_pte_requests_in_vblank;
	bw_fixed inefficient_linear_pitch_in_bytes;
	bw_fixed cursor_total_data;
	bw_fixed cursor_total_request_groups;
	bw_fixed scatter_gather_total_pte_requests;
	bw_fixed scatter_gather_total_pte_request_groups;
	bw_fixed tile_width_in_pixels;
	bw_fixed dmif_total_number_of_data_request_page_close_open;
	bw_fixed mcifwr_total_number_of_data_request_page_close_open;
	bw_fixed bytes_per_page_close_open;
	bw_fixed mcifwr_total_page_close_open_time;
	bw_fixed total_requests_for_adjusted_dmif_size;
	bw_fixed total_dmifmc_urgent_trips;
	bw_fixed total_dmifmc_urgent_latency;
	bw_fixed total_display_reads_required_data;
	bw_fixed total_display_reads_required_dram_access_data;
	bw_fixed total_display_writes_required_data;
	bw_fixed total_display_writes_required_dram_access_data;
	bw_fixed display_reads_required_data;
	bw_fixed display_reads_required_dram_access_data;
	bw_fixed dmif_total_page_close_open_time;
	bw_fixed min_cursor_memory_interface_buffer_size_in_time;
	bw_fixed min_read_buffer_size_in_time;
	bw_fixed display_reads_time_for_data_transfer;
	bw_fixed display_writes_time_for_data_transfer;
	bw_fixed dmif_required_dram_bandwidth;
	bw_fixed mcifwr_required_dram_bandwidth;
	bw_fixed required_dmifmc_urgent_latency_for_page_close_open;
	bw_fixed required_mcifmcwr_urgent_latency;
	bw_fixed required_dram_bandwidth_gbyte_per_second;
	bw_fixed dram_bandwidth;
	bw_fixed dmif_required_sclk;
	bw_fixed mcifwr_required_sclk;
	bw_fixed required_sclk;
	bw_fixed downspread_factor;
	bw_fixed v_scaler_efficiency;
	bw_fixed scaler_limits_factor;
	bw_fixed display_pipe_pixel_throughput;
	bw_fixed total_dispclk_required_with_ramping;
	bw_fixed total_dispclk_required_without_ramping;
	bw_fixed total_read_request_bandwidth;
	bw_fixed total_write_request_bandwidth;
	bw_fixed dispclk_required_for_total_read_request_bandwidth;
	bw_fixed total_dispclk_required_with_ramping_with_request_bandwidth;
	bw_fixed total_dispclk_required_without_ramping_with_request_bandwidth;
	bw_fixed dispclk;
	bw_fixed blackout_recovery_time;
	bw_fixed min_pixels_per_data_fifo_entry;
	bw_fixed sclk_deep_sleep;
	bw_fixed chunk_request_time;
	bw_fixed cursor_request_time;
	bw_fixed line_source_pixels_transfer_time;
	bw_fixed dmifdram_access_efficiency;
	bw_fixed mcifwrdram_access_efficiency;
	bw_fixed total_average_bandwidth_no_compression;
	bw_fixed total_average_bandwidth;
	bw_fixed total_stutter_cycle_duration;
	bw_fixed stutter_burst_time;
	bw_fixed time_in_self_refresh;
	bw_fixed stutter_efficiency;
	bw_fixed worst_number_of_trips_to_memory;
	bw_fixed immediate_flip_time;
	bw_fixed latency_for_non_dmif_clients;
	bw_fixed latency_for_non_mcifwr_clients;
	bw_fixed dmifmc_urgent_latency_supported_in_high_sclk_and_yclk;
	bw_fixed nbp_state_dram_speed_change_margin;
	bw_fixed display_reads_time_for_data_transfer_and_urgent_latency;
	bw_fixed dram_speed_change_margin;
	bw_fixed min_vblank_dram_speed_change_margin;
	bw_fixed min_stutter_refresh_duration;
	uint32_t total_stutter_dmif_buffer_size;
	uint32_t total_bytes_requested;
	uint32_t min_stutter_dmif_buffer_size;
	uint32_t num_stutter_bursts;
	bw_fixed v_blank_nbp_state_dram_speed_change_latency_supported;
	bw_fixed nbp_state_dram_speed_change_latency_supported;
	bool fbc_en[maximum_number_of_surfaces];
	bool lpt_en[maximum_number_of_surfaces];
	bool displays_match_flag[maximum_number_of_surfaces];
	bool use_alpha[maximum_number_of_surfaces];
	bool orthogonal_rotation[maximum_number_of_surfaces];
	bool enable[maximum_number_of_surfaces];
	bool access_one_channel_only[maximum_number_of_surfaces];
	bool scatter_gather_enable_for_pipe[maximum_number_of_surfaces];
	bool interlace_mode[maximum_number_of_surfaces];
	bool display_pstate_change_enable[maximum_number_of_surfaces];
	bool line_buffer_prefetch[maximum_number_of_surfaces];
	uint32_t bytes_per_pixel[maximum_number_of_surfaces];
	uint32_t max_chunks_non_fbc_mode[maximum_number_of_surfaces];
	uint32_t lb_bpc[maximum_number_of_surfaces];
	uint32_t output_bpphdmi[maximum_number_of_surfaces];
	uint32_t output_bppdp4_lane_hbr[maximum_number_of_surfaces];
	uint32_t output_bppdp4_lane_hbr2[maximum_number_of_surfaces];
	uint32_t output_bppdp4_lane_hbr3[maximum_number_of_surfaces];
	enum bw_defines stereo_mode[maximum_number_of_surfaces];
	bw_fixed dmif_buffer_transfer_time[maximum_number_of_surfaces];
	bw_fixed displays_with_same_mode[maximum_number_of_surfaces];
	bw_fixed stutter_dmif_buffer_size[maximum_number_of_surfaces];
	bw_fixed stutter_refresh_duration[maximum_number_of_surfaces];
	bw_fixed stutter_exit_watermark[maximum_number_of_surfaces];
	bw_fixed stutter_entry_watermark[maximum_number_of_surfaces];
	bw_fixed h_total[maximum_number_of_surfaces];
	bw_fixed v_total[maximum_number_of_surfaces];
	bw_fixed pixel_rate[maximum_number_of_surfaces];
	bw_fixed src_width[maximum_number_of_surfaces];
	bw_fixed pitch_in_pixels[maximum_number_of_surfaces];
	bw_fixed pitch_in_pixels_after_surface_type[maximum_number_of_surfaces];
	bw_fixed src_height[maximum_number_of_surfaces];
	bw_fixed scale_ratio[maximum_number_of_surfaces];
	bw_fixed h_taps[maximum_number_of_surfaces];
	bw_fixed v_taps[maximum_number_of_surfaces];
	bw_fixed h_scale_ratio[maximum_number_of_surfaces];
	bw_fixed v_scale_ratio[maximum_number_of_surfaces];
	bw_fixed rotation_angle[maximum_number_of_surfaces];
	bw_fixed compression_rate[maximum_number_of_surfaces];
	bw_fixed hsr[maximum_number_of_surfaces];
	bw_fixed vsr[maximum_number_of_surfaces];
	bw_fixed source_width_rounded_up_to_chunks[maximum_number_of_surfaces];
	bw_fixed source_width_pixels[maximum_number_of_surfaces];
	bw_fixed source_height_rounded_up_to_chunks[maximum_number_of_surfaces];
	bw_fixed display_bandwidth[maximum_number_of_surfaces];
	bw_fixed request_bandwidth[maximum_number_of_surfaces];
	bw_fixed bytes_per_request[maximum_number_of_surfaces];
	bw_fixed useful_bytes_per_request[maximum_number_of_surfaces];
	bw_fixed lines_interleaved_in_mem_access[maximum_number_of_surfaces];
	bw_fixed latency_hiding_lines[maximum_number_of_surfaces];
	bw_fixed lb_partitions[maximum_number_of_surfaces];
	bw_fixed lb_partitions_max[maximum_number_of_surfaces];
	bw_fixed dispclk_required_with_ramping[maximum_number_of_surfaces];
	bw_fixed dispclk_required_without_ramping[maximum_number_of_surfaces];
	bw_fixed data_buffer_size[maximum_number_of_surfaces];
	bw_fixed outstanding_chunk_request_limit[maximum_number_of_surfaces];
	bw_fixed urgent_watermark[maximum_number_of_surfaces];
	bw_fixed nbp_state_change_watermark[maximum_number_of_surfaces];
	bw_fixed v_filter_init[maximum_number_of_surfaces];
	bw_fixed stutter_cycle_duration[maximum_number_of_surfaces];
	bw_fixed average_bandwidth[maximum_number_of_surfaces];
	bw_fixed average_bandwidth_no_compression[maximum_number_of_surfaces];
	bw_fixed scatter_gather_pte_request_limit[maximum_number_of_surfaces];
	bw_fixed lb_size_per_component[maximum_number_of_surfaces];
	bw_fixed memory_chunk_size_in_bytes[maximum_number_of_surfaces];
	bw_fixed pipe_chunk_size_in_bytes[maximum_number_of_surfaces];
	bw_fixed number_of_trips_to_memory_for_getting_apte_row[maximum_number_of_surfaces];
	bw_fixed adjusted_data_buffer_size[maximum_number_of_surfaces];
	bw_fixed adjusted_data_buffer_size_in_memory[maximum_number_of_surfaces];
	bw_fixed pixels_per_data_fifo_entry[maximum_number_of_surfaces];
	bw_fixed scatter_gather_pte_requests_in_row[maximum_number_of_surfaces];
	bw_fixed pte_request_per_chunk[maximum_number_of_surfaces];
	bw_fixed scatter_gather_page_width[maximum_number_of_surfaces];
	bw_fixed scatter_gather_page_height[maximum_number_of_surfaces];
	bw_fixed lb_lines_in_per_line_out_in_beginning_of_frame[maximum_number_of_surfaces];
	bw_fixed lb_lines_in_per_line_out_in_middle_of_frame[maximum_number_of_surfaces];
	bw_fixed cursor_width_pixels[maximum_number_of_surfaces];
	bw_fixed minimum_latency_hiding[maximum_number_of_surfaces];
	bw_fixed maximum_latency_hiding[maximum_number_of_surfaces];
	bw_fixed minimum_latency_hiding_with_cursor[maximum_number_of_surfaces];
	bw_fixed maximum_latency_hiding_with_cursor[maximum_number_of_surfaces];
	bw_fixed src_pixels_for_first_output_pixel[maximum_number_of_surfaces];
	bw_fixed src_pixels_for_last_output_pixel[maximum_number_of_surfaces];
	bw_fixed src_data_for_first_output_pixel[maximum_number_of_surfaces];
	bw_fixed src_data_for_last_output_pixel[maximum_number_of_surfaces];
	bw_fixed active_time[maximum_number_of_surfaces];
	bw_fixed horizontal_blank_and_chunk_granularity_factor[maximum_number_of_surfaces];
	bw_fixed cursor_latency_hiding[maximum_number_of_surfaces];
	bw_fixed v_blank_dram_speed_change_margin[maximum_number_of_surfaces];
	uint32_t num_displays_with_margin[3][8];
	bw_fixed dmif_burst_time[3][8];
	bw_fixed mcifwr_burst_time[3][8];
	bw_fixed line_source_transfer_time[maximum_number_of_surfaces][3][8];
	bw_fixed dram_speed_change_line_source_transfer_time[maximum_number_of_surfaces][3][8];
	bw_fixed min_dram_speed_change_margin[3][8];
	bw_fixed dispclk_required_for_dram_speed_change[3][8];
	bw_fixed dispclk_required_for_dram_speed_change_pipe[3][8];
	bw_fixed blackout_duration_margin[3][8];
	bw_fixed dispclk_required_for_blackout_duration[3][8];
	bw_fixed dispclk_required_for_blackout_recovery[3][8];
	bw_fixed dmif_required_sclk_for_urgent_latency[6];
};

/**
 * Initialize structures with data which will NOT change at runtime.
 */
void bw_calcs_init(
	struct bw_calcs_dceip *bw_dceip,
	struct bw_calcs_vbios *bw_vbios,
	struct hw_asic_id asic_id);

/**
 * Return:
 *	true -	Display(s) configuration supported.
 *		In this case 'calcs_output' contains data for HW programming
 *	false - Display(s) configuration not supported (not enough bandwidth).
 */
bool bw_calcs(
	struct dc_context *ctx,
	const struct bw_calcs_dceip *dceip,
	const struct bw_calcs_vbios *vbios,
	const struct pipe_ctx *pipe,
	int pipe_count,
	struct dce_bw_output *calcs_output);

#endif /* __BANDWIDTH_CALCS_H__ */

