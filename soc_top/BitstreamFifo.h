#pragma once
#include <systemc.h>
#include <vector>

using namespace sc_core;
using namespace sc_dt;

SC_MODULE(BitstreamFifo) {
    sc_in<bool> clk;
    sc_in<bool> rst_n;

    sc_in<sc_uint<32>> in_data;
    sc_in<bool>        in_valid;
    sc_out<bool>       in_ready;

    sc_out<sc_uint<32>> out_data;
    sc_out<bool>        out_valid;
    sc_in<bool>         out_ready;

    static const int DEPTH = 16;

    sc_uint<32> mem[DEPTH];
    sc_uint<5>  rd_ptr;
    sc_uint<5>  wr_ptr;
    sc_uint<6>  count;

    void run();

    SC_CTOR(BitstreamFifo) {
        SC_CTHREAD(run, clk.pos());
        async_reset_signal_is(rst_n, false);
    }
};
