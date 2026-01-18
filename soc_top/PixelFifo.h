#pragma once
#include <systemc.h>
#include "pixel.h"

using namespace sc_core;
using namespace sc_dt;

SC_MODULE(PixelFifo) {
    sc_in<bool> clk, rst_n;
    sc_in<pixel_t>  in_data;
    sc_in<bool>     in_valid;
    sc_out<bool>    in_ready;

    sc_out<pixel_t> out_data;
    sc_out<bool>    out_valid;
    sc_in<bool>     out_ready;

    static const int DEPTH = 1024;
    pixel_t mem[DEPTH];
    int wr_ptr, rd_ptr, count;

    void run();

    SC_CTOR(PixelFifo) {
        SC_CTHREAD(run, clk.pos());
        async_reset_signal_is(rst_n, false);
    }
};
