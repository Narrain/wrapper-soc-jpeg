#pragma once
#include <systemc>

SC_MODULE(I2C) {
    sc_core::sc_in<bool> clk;
    sc_core::sc_in<bool> rst_n;

    void run();

    SC_CTOR(I2C);
};
