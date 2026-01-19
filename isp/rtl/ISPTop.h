#pragma once
#include <systemc.h>
#include "pixel.h"

using namespace sc_core;
using namespace sc_dt;

SC_MODULE(ISPTop) {
    sc_in<bool> clk;
    sc_in<bool> rst_n;

    // Matches VisionSoC wiring exactly:
    // isp.out_pixel(isp_pixel);
    // isp.out_valid(isp_valid);
    // isp.out_ready(isp_ready);
    // isp.frame_start(frame_start);
    // isp.frame_end(frame_end);
    sc_out<pixel_t> out_pixel;
    sc_out<bool>    out_valid;
    sc_in<bool>     out_ready;     // NOTE: input, name kept to match VisionSoC
    sc_out<bool>    frame_start;
    sc_out<bool>    frame_end;

    void run();

    SC_CTOR(ISPTop) {
        SC_CTHREAD(run, clk.pos());
        async_reset_signal_is(rst_n, false);
    }
};
