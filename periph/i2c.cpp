#include "i2c.h"
using namespace sc_core;

SC_CTOR(I2C) {
    SC_CTHREAD(run, clk.pos());
    async_reset_signal_is(rst_n, false);
}

void I2C::run() {
    while(true) wait();
}
