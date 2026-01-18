#include "mcu.h"
using namespace sc_core;

SC_CTOR(MCU) {
    SC_CTHREAD(run_firmware, clk.pos());
    async_reset_signal_is(rst_n, false);
}

void MCU::run_firmware() {
    while(true) wait();
}
