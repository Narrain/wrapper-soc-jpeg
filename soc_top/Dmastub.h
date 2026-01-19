#pragma once
#include <systemc.h>
#include <vector>

using namespace sc_core;
using namespace sc_dt;

SC_MODULE(DmaStub) {
    sc_in<bool> clk, rst_n;

    sc_in<sc_uint<32>> in_data;
    sc_in<bool>        in_valid;
    sc_out<bool>       in_ready;
    sc_in<bool>       irq_done;

    std::vector<uint8_t> ddr;
    bool active;

    void run();

    SC_CTOR(DmaStub) {
        SC_CTHREAD(run, clk.pos());
        async_reset_signal_is(rst_n, false);
    }
};
