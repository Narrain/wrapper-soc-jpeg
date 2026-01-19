#pragma once
#include <systemc.h>
#include "block_types.h"

struct DCTQ : sc_core::sc_module {
    sc_core::sc_in<bool> clk, rst_n;

    sc_core::sc_in<bool>      in_valid;
    sc_core::sc_out<bool>     in_ready;
    sc_core::sc_in<Block>     in_block;

    sc_core::sc_out<bool>     out_valid;
    sc_core::sc_in<bool>      out_ready;
    sc_core::sc_out<Block>    out_block;

    void proc();

    SC_CTOR(DCTQ) {
        SC_CTHREAD(proc, clk.pos());
        async_reset_signal_is(rst_n, false);
    }
};
