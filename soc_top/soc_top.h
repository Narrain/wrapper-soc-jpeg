#pragma once
#include <systemc>
#include "isp.h"
#include "encoder.h"
#include "mcu.h"
#include "mem_ctrl.h"
#include "i2c.h"

SC_MODULE(SocTop) {
    sc_core::sc_in<bool> clk;
    sc_core::sc_in<bool> rst_n;

    ISP* isp;
    Encoder* enc;
    MCU* mcu;
    MemCtrl* mem;
    I2C* i2c;

    SC_CTOR(SocTop);
};
