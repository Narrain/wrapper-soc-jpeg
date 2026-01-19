#pragma once
#include <systemc.h>
#include "block_types.h"

using namespace sc_core;
using namespace sc_dt;

SC_MODULE(Entropy) {
    sc_in<bool> clk;
    sc_in<bool> rst_n;

    sc_in<bool>   in_valid;
    sc_out<bool>  in_ready;
    sc_in<Block>  in_block;

    sc_out<bool>        out_valid;
    sc_in<bool>         out_ready;
    sc_out<sc_uint<32>> out_data;

    sc_in<bool> restart_mcu_boundary;

    void proc();

    SC_CTOR(Entropy) {
        SC_CTHREAD(proc, clk.pos());
        async_reset_signal_is(rst_n, false);
    }
};
