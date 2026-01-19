#pragma once
#include <systemc.h>
#include "pixel.h"
#include "block_types.h"

using namespace sc_core;
using namespace sc_dt;

SC_MODULE(BlockMaker) {
    sc_in<bool> clk;
    sc_in<bool> rst_n;

    sc_in<sc_uint<16>> cfg_width;
    sc_in<sc_uint<16>> cfg_height;
    sc_in<sc_uint<2>>  cfg_subsampling;
    sc_in<sc_uint<16>> cfg_restart_interval;

    sc_in<pixel_t> in_pixel;
    sc_in<bool>    in_valid;
    sc_out<bool>   in_ready;

    sc_out<sc_int<16>> out_block[64];
    sc_out<bool>       out_valid;
    sc_in<bool>        out_ready;
    sc_out<sc_uint<2>> out_type;  // BLOCK_Y / BLOCK_CB / BLOCK_CR

    sc_out<bool>       restart_mcu_boundary;

    void run();

    SC_CTOR(BlockMaker) {
        SC_CTHREAD(run, clk.pos());
        async_reset_signal_is(rst_n, false);
    }
};
