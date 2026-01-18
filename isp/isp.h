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

    void input_formatter();
    void demosaic_stage();
    void color_stage();
    void noise_stage();
    void output_formatter();

    SC_CTOR(ISP);
};
