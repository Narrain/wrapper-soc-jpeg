#include "mem_ctrl.h"
using namespace sc_core;

SC_CTOR(MemCtrl) {
    SC_CTHREAD(run, clk.pos());
    async_reset_signal_is(rst_n, false);
}

void MemCtrl::run() {
    while(true) wait();
}
