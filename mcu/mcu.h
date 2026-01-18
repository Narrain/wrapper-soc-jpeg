#pragma once
#include <systemc>

SC_MODULE(MCU) {
    sc_core::sc_in<bool> clk;
    sc_core::sc_in<bool> rst_n;

    sc_core::sc_in<bool> irq_isp;
    sc_core::sc_in<bool> irq_enc;

    void run_firmware();

    SC_CTOR(MCU);
};
