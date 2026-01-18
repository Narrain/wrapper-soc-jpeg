#pragma once
#include <systemc.h>
#include "pixel.h"

using namespace sc_core;
using namespace sc_dt;

SC_MODULE(ISP_stub) {
    sc_in<bool> clk;
    sc_in<bool> rst_n;

    sc_out<pixel_t> out_pixel;
    sc_out<bool>    out_valid;
    sc_in<bool>     out_ready;

    sc_out<bool> frame_start;
    sc_out<bool> frame_end;

    int width;
    int height;

    void run();

    SC_CTOR(ISP_stub) {
        width  = 128;
        height = 128;

        SC_CTHREAD(run, clk.pos());
        async_reset_signal_is(rst_n, false);
    }
};
