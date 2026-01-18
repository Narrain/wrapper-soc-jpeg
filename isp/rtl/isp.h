#pragma once
#include <systemc>
#include "pixel.h"

SC_MODULE(ISP) {
    sc_core::sc_in<bool> clk;
    sc_core::sc_in<bool> rst_n;

    sc_core::sc_in<pixel_t> in_pixel;
    sc_core::sc_in<bool>    in_valid;
    sc_core::sc_out<bool>   in_ready;

    sc_core::sc_out<pixel_t> out_pixel;
    sc_core::sc_out<bool>    out_valid;
    sc_core::sc_in<bool>     out_ready;

    // internal signals
    sc_core::sc_signal<pixel_t> s_ifm2dem_pix;
    sc_core::sc_signal<bool>    s_ifm2dem_valid;
    sc_core::sc_signal<bool>    s_ifm2dem_ready;

    sc_core::sc_signal<pixel_t> s_dem2col_pix;
    sc_core::sc_signal<bool>    s_dem2col_valid;
    sc_core::sc_signal<bool>    s_dem2col_ready;

    sc_core::sc_signal<pixel_t> s_col2nr_pix;
    sc_core::sc_signal<bool>    s_col2nr_valid;
    sc_core::sc_signal<bool>    s_col2nr_ready;

    sc_core::sc_signal<pixel_t> s_nr2ofm_pix;
    sc_core::sc_signal<bool>    s_nr2ofm_valid;
    sc_core::sc_signal<bool>    s_nr2ofm_ready;

    void input_formatter();
    void demosaic_stage();
    void color_stage();
    void noise_stage();
    void output_formatter();

    SC_CTOR(ISP);
};
