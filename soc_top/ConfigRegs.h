#pragma once
#include <systemc.h>

SC_MODULE(ConfigRegs) {
    sc_in<bool> clk;
    sc_in<bool> rst_n;

    // Simple write-only control bus
    sc_in<bool>        wr_en;
    sc_in<sc_uint<8>>  wr_addr;
    sc_in<sc_uint<32>> wr_data;

    // Outputs to blocks
    sc_out<sc_uint<16>> width;
    sc_out<sc_uint<16>> height;
    sc_out<sc_uint<2>>  subsampling;
    sc_out<sc_uint<16>> restart_interval;
    sc_out<sc_uint<8>>  quality;
    sc_out<bool>        start;
    sc_out<sc_uint<32>> dma_base;
    sc_out<sc_uint<8>>  irq_mask;

    // Internal registers
    sc_uint<16> width_reg;
    sc_uint<16> height_reg;
    sc_uint<2>  subsampling_reg;
    sc_uint<16> restart_interval_reg;
    sc_uint<8>  quality_reg;
    bool        start_reg;
    sc_uint<32> dma_base_reg;
    sc_uint<8>  irq_mask_reg;

    void run();

    SC_CTOR(ConfigRegs) {
        SC_CTHREAD(run, clk.pos());
        async_reset_signal_is(rst_n, false);
    }
};
