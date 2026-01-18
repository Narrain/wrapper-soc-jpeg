// WordFifo.h
#pragma once
#include <systemc.h>
#include <deque>

using namespace sc_core;
using namespace sc_dt;

SC_MODULE(WordFifo) {
    sc_in<bool>        clk, rst_n;
    sc_in<sc_uint<32>> in_data;
    sc_in<bool>        in_valid;
    sc_out<bool>       in_ready;

    sc_out<sc_uint<32>> out_data;
    sc_out<bool>        out_valid;
    sc_in<bool>         out_ready;

    static const int DEPTH = 1024;
    std::deque<sc_uint<32>> q;

    void run();

    SC_CTOR(WordFifo) {
        SC_CTHREAD(run, clk.pos());
        async_reset_signal_is(rst_n, false);
    }
};
