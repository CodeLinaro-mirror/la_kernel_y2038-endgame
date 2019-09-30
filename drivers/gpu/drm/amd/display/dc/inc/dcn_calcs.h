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

/**
 * Bandwidth and Watermark calculations interface.
 * (Refer to "DCEx_mode_support.xlsm" from Perforce.)
 */
#ifndef __DCN_CALCS_H__
#define __DCN_CALCS_H__

#include "bw_fixed.h"
#include "amdgpu_dc_float.h"
#include "../dml/display_mode_lib.h"


struct dc;
struct dc_state;

/*******************************************************************************
 * DCN data structures.
 ******************************************************************************/

#define number_of_planes   6
#define number_of_planes_minus_one   5
#define number_of_states   4
#define number_of_states_plus_one   5

#define ddr4_dram_width   64
#define ddr4_dram_factor_single_Channel   16
enum dcn_bw_defs {
	dcn_bw_v_min0p65,
	dcn_bw_v_mid0p72,
	dcn_bw_v_nom0p8,
	dcn_bw_v_max0p9,
	dcn_bw_v_max0p91,
	dcn_bw_no_support = 5,
	dcn_bw_yes,
	dcn_bw_hor,
	dcn_bw_vert,
	dcn_bw_override,
	dcn_bw_rgb_sub_64,
	dcn_bw_rgb_sub_32,
	dcn_bw_rgb_sub_16,
	dcn_bw_no,
	dcn_bw_sw_linear,
	dcn_bw_sw_4_kb_d,
	dcn_bw_sw_4_kb_d_x,
	dcn_bw_sw_64_kb_d,
	dcn_bw_sw_64_kb_d_t,
	dcn_bw_sw_64_kb_d_x,
	dcn_bw_sw_var_d,
	dcn_bw_sw_var_d_x,
	dcn_bw_yuv420_sub_8,
	dcn_bw_sw_4_kb_s,
	dcn_bw_sw_4_kb_s_x,
	dcn_bw_sw_64_kb_s,
	dcn_bw_sw_64_kb_s_t,
	dcn_bw_sw_64_kb_s_x,
	dcn_bw_writeback,
	dcn_bw_444,
	dcn_bw_dp,
	dcn_bw_420,
	dcn_bw_hdmi,
	dcn_bw_sw_var_s,
	dcn_bw_sw_var_s_x,
	dcn_bw_yuv420_sub_10,
	dcn_bw_supported_in_v_active,
	dcn_bw_supported_in_v_blank,
	dcn_bw_not_supported,
	dcn_bw_na,
	dcn_bw_encoder_8bpc,
	dcn_bw_encoder_10bpc,
	dcn_bw_encoder_12bpc,
	dcn_bw_encoder_16bpc,
};

/*bounding box parameters*/
/*mode parameters*/
/*system configuration*/
/* display configuration*/
struct dcn_bw_internal_vars {
	amdgpu_dc_float voltage[number_of_states_plus_one + 1];
	amdgpu_dc_float max_dispclk[number_of_states_plus_one + 1];
	amdgpu_dc_float max_dppclk[number_of_states_plus_one + 1];
	amdgpu_dc_float dcfclk_per_state[number_of_states_plus_one + 1];
	amdgpu_dc_float phyclk_per_state[number_of_states_plus_one + 1];
	amdgpu_dc_float fabric_and_dram_bandwidth_per_state[number_of_states_plus_one + 1];
	amdgpu_dc_float sr_exit_time;
	amdgpu_dc_float sr_enter_plus_exit_time;
	amdgpu_dc_float dram_clock_change_latency;
	amdgpu_dc_float urgent_latency;
	amdgpu_dc_float write_back_latency;
	amdgpu_dc_float percent_of_ideal_drambw_received_after_urg_latency;
	amdgpu_dc_float dcfclkv_max0p9;
	amdgpu_dc_float dcfclkv_nom0p8;
	amdgpu_dc_float dcfclkv_mid0p72;
	amdgpu_dc_float dcfclkv_min0p65;
	amdgpu_dc_float max_dispclk_vmax0p9;
	amdgpu_dc_float max_dppclk_vmax0p9;
	amdgpu_dc_float max_dispclk_vnom0p8;
	amdgpu_dc_float max_dppclk_vnom0p8;
	amdgpu_dc_float max_dispclk_vmid0p72;
	amdgpu_dc_float max_dppclk_vmid0p72;
	amdgpu_dc_float max_dispclk_vmin0p65;
	amdgpu_dc_float max_dppclk_vmin0p65;
	amdgpu_dc_float socclk;
	amdgpu_dc_float fabric_and_dram_bandwidth_vmax0p9;
	amdgpu_dc_float fabric_and_dram_bandwidth_vnom0p8;
	amdgpu_dc_float fabric_and_dram_bandwidth_vmid0p72;
	amdgpu_dc_float fabric_and_dram_bandwidth_vmin0p65;
	amdgpu_dc_float round_trip_ping_latency_cycles;
	amdgpu_dc_float urgent_out_of_order_return_per_channel;
	amdgpu_dc_float number_of_channels;
	amdgpu_dc_float vmm_page_size;
	amdgpu_dc_float return_bus_width;
	amdgpu_dc_float rob_buffer_size_in_kbyte;
	amdgpu_dc_float det_buffer_size_in_kbyte;
	amdgpu_dc_float dpp_output_buffer_pixels;
	amdgpu_dc_float opp_output_buffer_lines;
	amdgpu_dc_float pixel_chunk_size_in_kbyte;
	amdgpu_dc_float pte_chunk_size;
	amdgpu_dc_float meta_chunk_size;
	amdgpu_dc_float writeback_chunk_size;
	enum dcn_bw_defs odm_capability;
	enum dcn_bw_defs dsc_capability;
	amdgpu_dc_float line_buffer_size;
	enum dcn_bw_defs is_line_buffer_bpp_fixed;
	amdgpu_dc_float line_buffer_fixed_bpp;
	amdgpu_dc_float max_line_buffer_lines;
	amdgpu_dc_float writeback_luma_buffer_size;
	amdgpu_dc_float writeback_chroma_buffer_size;
	amdgpu_dc_float max_num_dpp;
	amdgpu_dc_float max_num_writeback;
	amdgpu_dc_float max_dchub_topscl_throughput;
	amdgpu_dc_float max_pscl_tolb_throughput;
	amdgpu_dc_float max_lb_tovscl_throughput;
	amdgpu_dc_float max_vscl_tohscl_throughput;
	amdgpu_dc_float max_hscl_ratio;
	amdgpu_dc_float max_vscl_ratio;
	amdgpu_dc_float max_hscl_taps;
	amdgpu_dc_float max_vscl_taps;
	amdgpu_dc_float under_scan_factor;
	amdgpu_dc_float phyclkv_max0p9;
	amdgpu_dc_float phyclkv_nom0p8;
	amdgpu_dc_float phyclkv_mid0p72;
	amdgpu_dc_float phyclkv_min0p65;
	amdgpu_dc_float pte_buffer_size_in_requests;
	amdgpu_dc_float dispclk_ramping_margin;
	amdgpu_dc_float downspreading;
	amdgpu_dc_float max_inter_dcn_tile_repeaters;
	enum dcn_bw_defs can_vstartup_lines_exceed_vsync_plus_back_porch_lines_minus_one;
	enum dcn_bw_defs bug_forcing_luma_and_chroma_request_to_same_size_fixed;
	int mode;
	amdgpu_dc_float viewport_width[number_of_planes_minus_one + 1];
	amdgpu_dc_float htotal[number_of_planes_minus_one + 1];
	amdgpu_dc_float vtotal[number_of_planes_minus_one + 1];
	amdgpu_dc_float v_sync_plus_back_porch[number_of_planes_minus_one + 1];
	amdgpu_dc_float vactive[number_of_planes_minus_one + 1];
	amdgpu_dc_float pixel_clock[number_of_planes_minus_one + 1]; /*MHz*/
	amdgpu_dc_float viewport_height[number_of_planes_minus_one + 1];
	enum dcn_bw_defs dcc_enable[number_of_planes_minus_one + 1];
	amdgpu_dc_float dcc_rate[number_of_planes_minus_one + 1];
	enum dcn_bw_defs source_scan[number_of_planes_minus_one + 1];
	amdgpu_dc_float lb_bit_per_pixel[number_of_planes_minus_one + 1];
	enum dcn_bw_defs source_pixel_format[number_of_planes_minus_one + 1];
	enum dcn_bw_defs source_surface_mode[number_of_planes_minus_one + 1];
	enum dcn_bw_defs output_format[number_of_planes_minus_one + 1];
	enum dcn_bw_defs output_deep_color[number_of_planes_minus_one + 1];
	enum dcn_bw_defs output[number_of_planes_minus_one + 1];
	amdgpu_dc_float scaler_rec_out_width[number_of_planes_minus_one + 1];
	amdgpu_dc_float scaler_recout_height[number_of_planes_minus_one + 1];
	amdgpu_dc_float underscan_output[number_of_planes_minus_one + 1];
	amdgpu_dc_float interlace_output[number_of_planes_minus_one + 1];
	amdgpu_dc_float override_hta_ps[number_of_planes_minus_one + 1];
	amdgpu_dc_float override_vta_ps[number_of_planes_minus_one + 1];
	amdgpu_dc_float override_hta_pschroma[number_of_planes_minus_one + 1];
	amdgpu_dc_float override_vta_pschroma[number_of_planes_minus_one + 1];
	amdgpu_dc_float urgent_latency_support_us[number_of_planes_minus_one + 1];
	amdgpu_dc_float h_ratio[number_of_planes_minus_one + 1];
	amdgpu_dc_float v_ratio[number_of_planes_minus_one + 1];
	amdgpu_dc_float htaps[number_of_planes_minus_one + 1];
	amdgpu_dc_float vtaps[number_of_planes_minus_one + 1];
	amdgpu_dc_float hta_pschroma[number_of_planes_minus_one + 1];
	amdgpu_dc_float vta_pschroma[number_of_planes_minus_one + 1];
	enum dcn_bw_defs pte_enable;
	enum dcn_bw_defs synchronized_vblank;
	enum dcn_bw_defs ta_pscalculation;
	int voltage_override_level;
	int number_of_active_planes;
	int voltage_level;
	enum dcn_bw_defs immediate_flip_supported;
	amdgpu_dc_float dcfclk;
	amdgpu_dc_float max_phyclk;
	amdgpu_dc_float fabric_and_dram_bandwidth;
	amdgpu_dc_float dpp_per_plane_per_ratio[1 + 1][number_of_planes_minus_one + 1];
	enum dcn_bw_defs dispclk_dppclk_support_per_ratio[1 + 1];
	amdgpu_dc_float required_dispclk_per_ratio[1 + 1];
	enum dcn_bw_defs error_message[1 + 1];
	int dispclk_dppclk_ratio;
	amdgpu_dc_float dpp_per_plane[number_of_planes_minus_one + 1];
	amdgpu_dc_float det_buffer_size_y[number_of_planes_minus_one + 1];
	amdgpu_dc_float det_buffer_size_c[number_of_planes_minus_one + 1];
	amdgpu_dc_float swath_height_y[number_of_planes_minus_one + 1];
	amdgpu_dc_float swath_height_c[number_of_planes_minus_one + 1];
	enum dcn_bw_defs final_error_message;
	amdgpu_dc_float frequency;
	amdgpu_dc_float header_line;
	amdgpu_dc_float header;
	enum dcn_bw_defs voltage_override;
	enum dcn_bw_defs allow_different_hratio_vratio;
	amdgpu_dc_float acceptable_quality_hta_ps;
	amdgpu_dc_float acceptable_quality_vta_ps;
	amdgpu_dc_float no_of_dpp[number_of_states_plus_one + 1][1 + 1][number_of_planes_minus_one + 1];
	amdgpu_dc_float swath_width_yper_state[number_of_states_plus_one + 1][1 + 1][number_of_planes_minus_one + 1];
	amdgpu_dc_float swath_height_yper_state[number_of_states_plus_one + 1][1 + 1][number_of_planes_minus_one + 1];
	amdgpu_dc_float swath_height_cper_state[number_of_states_plus_one + 1][1 + 1][number_of_planes_minus_one + 1];
	amdgpu_dc_float urgent_latency_support_us_per_state[number_of_states_plus_one + 1][1 + 1][number_of_planes_minus_one + 1];
	amdgpu_dc_float v_ratio_pre_ywith_immediate_flip[number_of_states_plus_one + 1][1 + 1][number_of_planes_minus_one + 1];
	amdgpu_dc_float v_ratio_pre_cwith_immediate_flip[number_of_states_plus_one + 1][1 + 1][number_of_planes_minus_one + 1];
	amdgpu_dc_float required_prefetch_pixel_data_bw_with_immediate_flip[number_of_states_plus_one + 1][1 + 1][number_of_planes_minus_one + 1];
	amdgpu_dc_float v_ratio_pre_ywithout_immediate_flip[number_of_states_plus_one + 1][1 + 1][number_of_planes_minus_one + 1];
	amdgpu_dc_float v_ratio_pre_cwithout_immediate_flip[number_of_states_plus_one + 1][1 + 1][number_of_planes_minus_one + 1];
	amdgpu_dc_float required_prefetch_pixel_data_bw_without_immediate_flip[number_of_states_plus_one + 1][1 + 1][number_of_planes_minus_one + 1];
	enum dcn_bw_defs prefetch_supported_with_immediate_flip[number_of_states_plus_one + 1][1 + 1];
	enum dcn_bw_defs prefetch_supported_without_immediate_flip[number_of_states_plus_one + 1][1 + 1];
	enum dcn_bw_defs v_ratio_in_prefetch_supported_with_immediate_flip[number_of_states_plus_one + 1][1 + 1];
	enum dcn_bw_defs v_ratio_in_prefetch_supported_without_immediate_flip[number_of_states_plus_one + 1][1 + 1];
	amdgpu_dc_float required_dispclk[number_of_states_plus_one + 1][1 + 1];
	enum dcn_bw_defs dispclk_dppclk_support[number_of_states_plus_one + 1][1 + 1];
	enum dcn_bw_defs total_available_pipes_support[number_of_states_plus_one + 1][1 + 1];
	amdgpu_dc_float total_number_of_active_dpp[number_of_states_plus_one + 1][1 + 1];
	amdgpu_dc_float total_number_of_dcc_active_dpp[number_of_states_plus_one + 1][1 + 1];
	enum dcn_bw_defs urgent_latency_support[number_of_states_plus_one + 1][1 + 1];
	enum dcn_bw_defs mode_support_with_immediate_flip[number_of_states_plus_one + 1][1 + 1];
	enum dcn_bw_defs mode_support_without_immediate_flip[number_of_states_plus_one + 1][1 + 1];
	amdgpu_dc_float return_bw_per_state[number_of_states_plus_one + 1];
	enum dcn_bw_defs dio_support[number_of_states_plus_one + 1];
	amdgpu_dc_float urgent_round_trip_and_out_of_order_latency_per_state[number_of_states_plus_one + 1];
	enum dcn_bw_defs rob_support[number_of_states_plus_one + 1];
	enum dcn_bw_defs bandwidth_support[number_of_states_plus_one + 1];
	amdgpu_dc_float prefetch_bw[number_of_planes_minus_one + 1];
	amdgpu_dc_float meta_pte_bytes_per_frame[number_of_planes_minus_one + 1];
	amdgpu_dc_float meta_row_bytes[number_of_planes_minus_one + 1];
	amdgpu_dc_float dpte_bytes_per_row[number_of_planes_minus_one + 1];
	amdgpu_dc_float prefetch_lines_y[number_of_planes_minus_one + 1];
	amdgpu_dc_float prefetch_lines_c[number_of_planes_minus_one + 1];
	amdgpu_dc_float max_num_sw_y[number_of_planes_minus_one + 1];
	amdgpu_dc_float max_num_sw_c[number_of_planes_minus_one + 1];
	amdgpu_dc_float line_times_for_prefetch[number_of_planes_minus_one + 1];
	amdgpu_dc_float lines_for_meta_pte_with_immediate_flip[number_of_planes_minus_one + 1];
	amdgpu_dc_float lines_for_meta_pte_without_immediate_flip[number_of_planes_minus_one + 1];
	amdgpu_dc_float lines_for_meta_and_dpte_row_with_immediate_flip[number_of_planes_minus_one + 1];
	amdgpu_dc_float lines_for_meta_and_dpte_row_without_immediate_flip[number_of_planes_minus_one + 1];
	amdgpu_dc_float min_dppclk_using_single_dpp[number_of_planes_minus_one + 1];
	amdgpu_dc_float swath_width_ysingle_dpp[number_of_planes_minus_one + 1];
	amdgpu_dc_float byte_per_pixel_in_dety[number_of_planes_minus_one + 1];
	amdgpu_dc_float byte_per_pixel_in_detc[number_of_planes_minus_one + 1];
	amdgpu_dc_float number_of_dpp_required_for_det_and_lb_size[number_of_planes_minus_one + 1];
	amdgpu_dc_float required_phyclk[number_of_planes_minus_one + 1];
	amdgpu_dc_float read256_block_height_y[number_of_planes_minus_one + 1];
	amdgpu_dc_float read256_block_width_y[number_of_planes_minus_one + 1];
	amdgpu_dc_float read256_block_height_c[number_of_planes_minus_one + 1];
	amdgpu_dc_float read256_block_width_c[number_of_planes_minus_one + 1];
	amdgpu_dc_float max_swath_height_y[number_of_planes_minus_one + 1];
	amdgpu_dc_float max_swath_height_c[number_of_planes_minus_one + 1];
	amdgpu_dc_float min_swath_height_y[number_of_planes_minus_one + 1];
	amdgpu_dc_float min_swath_height_c[number_of_planes_minus_one + 1];
	amdgpu_dc_float read_bandwidth[number_of_planes_minus_one + 1];
	amdgpu_dc_float write_bandwidth[number_of_planes_minus_one + 1];
	amdgpu_dc_float pscl_factor[number_of_planes_minus_one + 1];
	amdgpu_dc_float pscl_factor_chroma[number_of_planes_minus_one + 1];
	enum dcn_bw_defs scale_ratio_support;
	enum dcn_bw_defs source_format_pixel_and_scan_support;
	amdgpu_dc_float total_read_bandwidth_consumed_gbyte_per_second;
	amdgpu_dc_float total_write_bandwidth_consumed_gbyte_per_second;
	amdgpu_dc_float total_bandwidth_consumed_gbyte_per_second;
	enum dcn_bw_defs dcc_enabled_in_any_plane;
	amdgpu_dc_float return_bw_todcn_per_state;
	amdgpu_dc_float critical_point;
	enum dcn_bw_defs writeback_latency_support;
	amdgpu_dc_float required_output_bw;
	amdgpu_dc_float total_number_of_active_writeback;
	enum dcn_bw_defs total_available_writeback_support;
	amdgpu_dc_float maximum_swath_width;
	amdgpu_dc_float number_of_dpp_required_for_det_size;
	amdgpu_dc_float number_of_dpp_required_for_lb_size;
	amdgpu_dc_float min_dispclk_using_single_dpp;
	amdgpu_dc_float min_dispclk_using_dual_dpp;
	enum dcn_bw_defs viewport_size_support;
	amdgpu_dc_float swath_width_granularity_y;
	amdgpu_dc_float rounded_up_max_swath_size_bytes_y;
	amdgpu_dc_float swath_width_granularity_c;
	amdgpu_dc_float rounded_up_max_swath_size_bytes_c;
	amdgpu_dc_float lines_in_det_luma;
	amdgpu_dc_float lines_in_det_chroma;
	amdgpu_dc_float effective_lb_latency_hiding_source_lines_luma;
	amdgpu_dc_float effective_lb_latency_hiding_source_lines_chroma;
	amdgpu_dc_float effective_detlb_lines_luma;
	amdgpu_dc_float effective_detlb_lines_chroma;
	amdgpu_dc_float projected_dcfclk_deep_sleep;
	amdgpu_dc_float meta_req_height_y;
	amdgpu_dc_float meta_req_width_y;
	amdgpu_dc_float meta_surface_width_y;
	amdgpu_dc_float meta_surface_height_y;
	amdgpu_dc_float meta_pte_bytes_per_frame_y;
	amdgpu_dc_float meta_row_bytes_y;
	amdgpu_dc_float macro_tile_block_size_bytes_y;
	amdgpu_dc_float macro_tile_block_height_y;
	amdgpu_dc_float data_pte_req_height_y;
	amdgpu_dc_float data_pte_req_width_y;
	amdgpu_dc_float dpte_bytes_per_row_y;
	amdgpu_dc_float meta_req_height_c;
	amdgpu_dc_float meta_req_width_c;
	amdgpu_dc_float meta_surface_width_c;
	amdgpu_dc_float meta_surface_height_c;
	amdgpu_dc_float meta_pte_bytes_per_frame_c;
	amdgpu_dc_float meta_row_bytes_c;
	amdgpu_dc_float macro_tile_block_size_bytes_c;
	amdgpu_dc_float macro_tile_block_height_c;
	amdgpu_dc_float macro_tile_block_width_c;
	amdgpu_dc_float data_pte_req_height_c;
	amdgpu_dc_float data_pte_req_width_c;
	amdgpu_dc_float dpte_bytes_per_row_c;
	amdgpu_dc_float v_init_y;
	amdgpu_dc_float max_partial_sw_y;
	amdgpu_dc_float v_init_c;
	amdgpu_dc_float max_partial_sw_c;
	amdgpu_dc_float dst_x_after_scaler;
	amdgpu_dc_float dst_y_after_scaler;
	amdgpu_dc_float time_calc;
	amdgpu_dc_float v_update_offset[number_of_planes_minus_one + 1][2];
	amdgpu_dc_float total_repeater_delay;
	amdgpu_dc_float v_update_width[number_of_planes_minus_one + 1][2];
	amdgpu_dc_float v_ready_offset[number_of_planes_minus_one + 1][2];
	amdgpu_dc_float time_setup;
	amdgpu_dc_float extra_latency;
	amdgpu_dc_float maximum_vstartup;
	amdgpu_dc_float bw_available_for_immediate_flip;
	amdgpu_dc_float total_immediate_flip_bytes[number_of_planes_minus_one + 1];
	amdgpu_dc_float time_for_meta_pte_with_immediate_flip;
	amdgpu_dc_float time_for_meta_pte_without_immediate_flip;
	amdgpu_dc_float time_for_meta_and_dpte_row_with_immediate_flip;
	amdgpu_dc_float time_for_meta_and_dpte_row_without_immediate_flip;
	amdgpu_dc_float line_times_to_request_prefetch_pixel_data_with_immediate_flip;
	amdgpu_dc_float line_times_to_request_prefetch_pixel_data_without_immediate_flip;
	amdgpu_dc_float maximum_read_bandwidth_with_prefetch_with_immediate_flip;
	amdgpu_dc_float maximum_read_bandwidth_with_prefetch_without_immediate_flip;
	amdgpu_dc_float voltage_level_with_immediate_flip;
	amdgpu_dc_float voltage_level_without_immediate_flip;
	amdgpu_dc_float total_number_of_active_dpp_per_ratio[1 + 1];
	amdgpu_dc_float byte_per_pix_dety;
	amdgpu_dc_float byte_per_pix_detc;
	amdgpu_dc_float read256_bytes_block_height_y;
	amdgpu_dc_float read256_bytes_block_width_y;
	amdgpu_dc_float read256_bytes_block_height_c;
	amdgpu_dc_float read256_bytes_block_width_c;
	amdgpu_dc_float maximum_swath_height_y;
	amdgpu_dc_float maximum_swath_height_c;
	amdgpu_dc_float minimum_swath_height_y;
	amdgpu_dc_float minimum_swath_height_c;
	amdgpu_dc_float swath_width;
	amdgpu_dc_float prefetch_bandwidth[number_of_planes_minus_one + 1];
	amdgpu_dc_float v_init_pre_fill_y[number_of_planes_minus_one + 1];
	amdgpu_dc_float v_init_pre_fill_c[number_of_planes_minus_one + 1];
	amdgpu_dc_float max_num_swath_y[number_of_planes_minus_one + 1];
	amdgpu_dc_float max_num_swath_c[number_of_planes_minus_one + 1];
	amdgpu_dc_float prefill_y[number_of_planes_minus_one + 1];
	amdgpu_dc_float prefill_c[number_of_planes_minus_one + 1];
	amdgpu_dc_float v_startup[number_of_planes_minus_one + 1];
	enum dcn_bw_defs allow_dram_clock_change_during_vblank[number_of_planes_minus_one + 1];
	amdgpu_dc_float allow_dram_self_refresh_during_vblank[number_of_planes_minus_one + 1];
	amdgpu_dc_float v_ratio_prefetch_y[number_of_planes_minus_one + 1];
	amdgpu_dc_float v_ratio_prefetch_c[number_of_planes_minus_one + 1];
	amdgpu_dc_float destination_lines_for_prefetch[number_of_planes_minus_one + 1];
	amdgpu_dc_float destination_lines_to_request_vm_inv_blank[number_of_planes_minus_one + 1];
	amdgpu_dc_float destination_lines_to_request_row_in_vblank[number_of_planes_minus_one + 1];
	amdgpu_dc_float min_ttuv_blank[number_of_planes_minus_one + 1];
	amdgpu_dc_float byte_per_pixel_dety[number_of_planes_minus_one + 1];
	amdgpu_dc_float byte_per_pixel_detc[number_of_planes_minus_one + 1];
	amdgpu_dc_float swath_width_y[number_of_planes_minus_one + 1];
	amdgpu_dc_float lines_in_dety[number_of_planes_minus_one + 1];
	amdgpu_dc_float lines_in_dety_rounded_down_to_swath[number_of_planes_minus_one + 1];
	amdgpu_dc_float lines_in_detc[number_of_planes_minus_one + 1];
	amdgpu_dc_float lines_in_detc_rounded_down_to_swath[number_of_planes_minus_one + 1];
	amdgpu_dc_float full_det_buffering_time_y[number_of_planes_minus_one + 1];
	amdgpu_dc_float full_det_buffering_time_c[number_of_planes_minus_one + 1];
	amdgpu_dc_float active_dram_clock_change_latency_margin[number_of_planes_minus_one + 1];
	amdgpu_dc_float v_blank_dram_clock_change_latency_margin[number_of_planes_minus_one + 1];
	amdgpu_dc_float dcfclk_deep_sleep_per_plane[number_of_planes_minus_one + 1];
	amdgpu_dc_float read_bandwidth_plane_luma[number_of_planes_minus_one + 1];
	amdgpu_dc_float read_bandwidth_plane_chroma[number_of_planes_minus_one + 1];
	amdgpu_dc_float display_pipe_line_delivery_time_luma[number_of_planes_minus_one + 1];
	amdgpu_dc_float display_pipe_line_delivery_time_chroma[number_of_planes_minus_one + 1];
	amdgpu_dc_float display_pipe_line_delivery_time_luma_prefetch[number_of_planes_minus_one + 1];
	amdgpu_dc_float display_pipe_line_delivery_time_chroma_prefetch[number_of_planes_minus_one + 1];
	amdgpu_dc_float pixel_pte_bytes_per_row[number_of_planes_minus_one + 1];
	amdgpu_dc_float meta_pte_bytes_frame[number_of_planes_minus_one + 1];
	amdgpu_dc_float meta_row_byte[number_of_planes_minus_one + 1];
	amdgpu_dc_float prefetch_source_lines_y[number_of_planes_minus_one + 1];
	amdgpu_dc_float prefetch_source_lines_c[number_of_planes_minus_one + 1];
	amdgpu_dc_float pscl_throughput[number_of_planes_minus_one + 1];
	amdgpu_dc_float pscl_throughput_chroma[number_of_planes_minus_one + 1];
	amdgpu_dc_float output_bpphdmi[number_of_planes_minus_one + 1];
	amdgpu_dc_float output_bppdp4_lane_hbr[number_of_planes_minus_one + 1];
	amdgpu_dc_float output_bppdp4_lane_hbr2[number_of_planes_minus_one + 1];
	amdgpu_dc_float output_bppdp4_lane_hbr3[number_of_planes_minus_one + 1];
	amdgpu_dc_float max_vstartup_lines[number_of_planes_minus_one + 1];
	amdgpu_dc_float dispclk_with_ramping;
	amdgpu_dc_float dispclk_without_ramping;
	amdgpu_dc_float dppclk_using_single_dpp_luma;
	amdgpu_dc_float dppclk_using_single_dpp;
	amdgpu_dc_float dppclk_using_single_dpp_chroma;
	enum dcn_bw_defs odm_capable;
	amdgpu_dc_float dispclk;
	amdgpu_dc_float dppclk;
	amdgpu_dc_float return_bandwidth_to_dcn;
	enum dcn_bw_defs dcc_enabled_any_plane;
	amdgpu_dc_float return_bw;
	amdgpu_dc_float critical_compression;
	amdgpu_dc_float total_data_read_bandwidth;
	amdgpu_dc_float total_active_dpp;
	amdgpu_dc_float total_dcc_active_dpp;
	amdgpu_dc_float urgent_round_trip_and_out_of_order_latency;
	amdgpu_dc_float last_pixel_of_line_extra_watermark;
	amdgpu_dc_float data_fabric_line_delivery_time_luma;
	amdgpu_dc_float data_fabric_line_delivery_time_chroma;
	amdgpu_dc_float urgent_extra_latency;
	amdgpu_dc_float urgent_watermark;
	amdgpu_dc_float ptemeta_urgent_watermark;
	amdgpu_dc_float dram_clock_change_watermark;
	amdgpu_dc_float total_active_writeback;
	amdgpu_dc_float writeback_dram_clock_change_watermark;
	amdgpu_dc_float min_full_det_buffering_time;
	amdgpu_dc_float frame_time_for_min_full_det_buffering_time;
	amdgpu_dc_float average_read_bandwidth_gbyte_per_second;
	amdgpu_dc_float part_of_burst_that_fits_in_rob;
	amdgpu_dc_float stutter_burst_time;
	amdgpu_dc_float stutter_efficiency_not_including_vblank;
	amdgpu_dc_float smallest_vblank;
	amdgpu_dc_float v_blank_time;
	amdgpu_dc_float stutter_efficiency;
	amdgpu_dc_float dcf_clk_deep_sleep;
	amdgpu_dc_float stutter_exit_watermark;
	amdgpu_dc_float stutter_enter_plus_exit_watermark;
	amdgpu_dc_float effective_det_plus_lb_lines_luma;
	amdgpu_dc_float urgent_latency_support_us_luma;
	amdgpu_dc_float effective_det_plus_lb_lines_chroma;
	amdgpu_dc_float urgent_latency_support_us_chroma;
	amdgpu_dc_float min_urgent_latency_support_us;
	amdgpu_dc_float non_urgent_latency_tolerance;
	amdgpu_dc_float block_height256_bytes_y;
	amdgpu_dc_float block_height256_bytes_c;
	amdgpu_dc_float meta_request_width_y;
	amdgpu_dc_float meta_surf_width_y;
	amdgpu_dc_float meta_surf_height_y;
	amdgpu_dc_float meta_pte_bytes_frame_y;
	amdgpu_dc_float meta_row_byte_y;
	amdgpu_dc_float macro_tile_size_byte_y;
	amdgpu_dc_float macro_tile_height_y;
	amdgpu_dc_float pixel_pte_req_height_y;
	amdgpu_dc_float pixel_pte_req_width_y;
	amdgpu_dc_float pixel_pte_bytes_per_row_y;
	amdgpu_dc_float meta_request_width_c;
	amdgpu_dc_float meta_surf_width_c;
	amdgpu_dc_float meta_surf_height_c;
	amdgpu_dc_float meta_pte_bytes_frame_c;
	amdgpu_dc_float meta_row_byte_c;
	amdgpu_dc_float macro_tile_size_bytes_c;
	amdgpu_dc_float macro_tile_height_c;
	amdgpu_dc_float pixel_pte_req_height_c;
	amdgpu_dc_float pixel_pte_req_width_c;
	amdgpu_dc_float pixel_pte_bytes_per_row_c;
	amdgpu_dc_float max_partial_swath_y;
	amdgpu_dc_float max_partial_swath_c;
	amdgpu_dc_float t_calc;
	amdgpu_dc_float next_prefetch_mode;
	amdgpu_dc_float v_startup_lines;
	enum dcn_bw_defs planes_with_room_to_increase_vstartup_prefetch_bw_less_than_active_bw;
	enum dcn_bw_defs planes_with_room_to_increase_vstartup_vratio_prefetch_more_than4;
	enum dcn_bw_defs planes_with_room_to_increase_vstartup_destination_line_times_for_prefetch_less_than2;
	enum dcn_bw_defs v_ratio_prefetch_more_than4;
	enum dcn_bw_defs destination_line_times_for_prefetch_less_than2;
	amdgpu_dc_float prefetch_mode;
	amdgpu_dc_float dstx_after_scaler;
	amdgpu_dc_float dsty_after_scaler;
	amdgpu_dc_float v_update_offset_pix[number_of_planes_minus_one + 1];
	amdgpu_dc_float total_repeater_delay_time;
	amdgpu_dc_float v_update_width_pix[number_of_planes_minus_one + 1];
	amdgpu_dc_float v_ready_offset_pix[number_of_planes_minus_one + 1];
	amdgpu_dc_float t_setup;
	amdgpu_dc_float t_wait;
	amdgpu_dc_float bandwidth_available_for_immediate_flip;
	amdgpu_dc_float tot_immediate_flip_bytes;
	amdgpu_dc_float max_rd_bandwidth;
	amdgpu_dc_float time_for_fetching_meta_pte;
	amdgpu_dc_float time_for_fetching_row_in_vblank;
	amdgpu_dc_float lines_to_request_prefetch_pixel_data;
	amdgpu_dc_float required_prefetch_pix_data_bw;
	enum dcn_bw_defs prefetch_mode_supported;
	amdgpu_dc_float active_dp_ps;
	amdgpu_dc_float lb_latency_hiding_source_lines_y;
	amdgpu_dc_float lb_latency_hiding_source_lines_c;
	amdgpu_dc_float effective_lb_latency_hiding_y;
	amdgpu_dc_float effective_lb_latency_hiding_c;
	amdgpu_dc_float dpp_output_buffer_lines_y;
	amdgpu_dc_float dpp_output_buffer_lines_c;
	amdgpu_dc_float dppopp_buffering_y;
	amdgpu_dc_float max_det_buffering_time_y;
	amdgpu_dc_float active_dram_clock_change_latency_margin_y;
	amdgpu_dc_float dppopp_buffering_c;
	amdgpu_dc_float max_det_buffering_time_c;
	amdgpu_dc_float active_dram_clock_change_latency_margin_c;
	amdgpu_dc_float writeback_dram_clock_change_latency_margin;
	amdgpu_dc_float min_active_dram_clock_change_margin;
	amdgpu_dc_float v_blank_of_min_active_dram_clock_change_margin;
	amdgpu_dc_float second_min_active_dram_clock_change_margin;
	amdgpu_dc_float min_vblank_dram_clock_change_margin;
	amdgpu_dc_float dram_clock_change_margin;
	amdgpu_dc_float dram_clock_change_support;
	amdgpu_dc_float wr_bandwidth;
	amdgpu_dc_float max_used_bw;
};

struct dcn_soc_bounding_box {
	amdgpu_dc_float sr_exit_time; /*us*/
	amdgpu_dc_float sr_enter_plus_exit_time; /*us*/
	amdgpu_dc_float urgent_latency; /*us*/
	amdgpu_dc_float write_back_latency; /*us*/
	amdgpu_dc_float percent_of_ideal_drambw_received_after_urg_latency; /*%*/
	int max_request_size; /*bytes*/
	amdgpu_dc_float dcfclkv_max0p9; /*MHz*/
	amdgpu_dc_float dcfclkv_nom0p8; /*MHz*/
	amdgpu_dc_float dcfclkv_mid0p72; /*MHz*/
	amdgpu_dc_float dcfclkv_min0p65; /*MHz*/
	amdgpu_dc_float max_dispclk_vmax0p9; /*MHz*/
	amdgpu_dc_float max_dispclk_vmid0p72; /*MHz*/
	amdgpu_dc_float max_dispclk_vnom0p8; /*MHz*/
	amdgpu_dc_float max_dispclk_vmin0p65; /*MHz*/
	amdgpu_dc_float max_dppclk_vmax0p9; /*MHz*/
	amdgpu_dc_float max_dppclk_vnom0p8; /*MHz*/
	amdgpu_dc_float max_dppclk_vmid0p72; /*MHz*/
	amdgpu_dc_float max_dppclk_vmin0p65; /*MHz*/
	amdgpu_dc_float socclk; /*MHz*/
	amdgpu_dc_float fabric_and_dram_bandwidth_vmax0p9; /*GB/s*/
	amdgpu_dc_float fabric_and_dram_bandwidth_vnom0p8; /*GB/s*/
	amdgpu_dc_float fabric_and_dram_bandwidth_vmid0p72; /*GB/s*/
	amdgpu_dc_float fabric_and_dram_bandwidth_vmin0p65; /*GB/s*/
	amdgpu_dc_float phyclkv_max0p9; /*MHz*/
	amdgpu_dc_float phyclkv_nom0p8; /*MHz*/
	amdgpu_dc_float phyclkv_mid0p72; /*MHz*/
	amdgpu_dc_float phyclkv_min0p65; /*MHz*/
	amdgpu_dc_float downspreading; /*%*/
	int round_trip_ping_latency_cycles; /*DCFCLK Cycles*/
	int urgent_out_of_order_return_per_channel; /*bytes*/
	int number_of_channels;
	int vmm_page_size; /*bytes*/
	amdgpu_dc_float dram_clock_change_latency; /*us*/
	int return_bus_width; /*bytes*/
	amdgpu_dc_float percent_disp_bw_limit; /*%*/
};
extern const struct dcn_soc_bounding_box dcn10_soc_defaults;

struct dcn_ip_params {
	amdgpu_dc_float rob_buffer_size_in_kbyte;
	amdgpu_dc_float det_buffer_size_in_kbyte;
	amdgpu_dc_float dpp_output_buffer_pixels;
	amdgpu_dc_float opp_output_buffer_lines;
	amdgpu_dc_float pixel_chunk_size_in_kbyte;
	enum dcn_bw_defs pte_enable;
	int pte_chunk_size; /*kbytes*/
	int meta_chunk_size; /*kbytes*/
	int writeback_chunk_size; /*kbytes*/
	enum dcn_bw_defs odm_capability;
	enum dcn_bw_defs dsc_capability;
	int line_buffer_size; /*bit*/
	int max_line_buffer_lines;
	enum dcn_bw_defs is_line_buffer_bpp_fixed;
	int line_buffer_fixed_bpp;
	int writeback_luma_buffer_size; /*kbytes*/
	int writeback_chroma_buffer_size; /*kbytes*/
	int max_num_dpp;
	int max_num_writeback;
	int max_dchub_topscl_throughput; /*pixels/dppclk*/
	int max_pscl_tolb_throughput; /*pixels/dppclk*/
	int max_lb_tovscl_throughput; /*pixels/dppclk*/
	int max_vscl_tohscl_throughput; /*pixels/dppclk*/
	amdgpu_dc_float max_hscl_ratio;
	amdgpu_dc_float max_vscl_ratio;
	int max_hscl_taps;
	int max_vscl_taps;
	int pte_buffer_size_in_requests;
	amdgpu_dc_float dispclk_ramping_margin; /*%*/
	amdgpu_dc_float under_scan_factor;
	int max_inter_dcn_tile_repeaters;
	enum dcn_bw_defs can_vstartup_lines_exceed_vsync_plus_back_porch_lines_minus_one;
	enum dcn_bw_defs bug_forcing_luma_and_chroma_request_to_same_size_fixed;
	int dcfclk_cstate_latency;
};
extern const struct dcn_ip_params dcn10_ip_defaults;

bool dcn_validate_bandwidth(
		struct dc *dc,
		struct dc_state *context,
		bool fast_validate);

unsigned int dcn_find_dcfclk_suits_all(
	const struct dc *dc,
	struct dc_clocks *clocks);

void dcn_bw_update_from_pplib(struct dc *dc);
void dcn_bw_notify_pplib_of_wm_ranges(struct dc *dc);
void dcn_bw_sync_calcs_and_dml(struct dc *dc);

enum source_macro_tile_size swizzle_mode_to_macro_tile_size(enum swizzle_mode_values sw_mode);

#endif /* __DCN_CALCS_H__ */

