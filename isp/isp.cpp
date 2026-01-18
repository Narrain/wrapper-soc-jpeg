#include "isp.h"

using namespace sc_core;

SC_CTOR(ISP) {
    SC_CTHREAD(input_formatter, clk.pos());
    async_reset_signal_is(rst_n, false);

    SC_CTHREAD(demosaic_stage, clk.pos());
    async_reset_signal_is(rst_n, false);

    SC_CTHREAD(color_stage, clk.pos());
    async_reset_signal_is(rst_n, false);

    SC_CTHREAD(noise_stage, clk.pos());
    async_reset_signal_is(rst_n, false);

    SC_CTHREAD(output_formatter, clk.pos());
    async_reset_signal_is(rst_n, false);
}

void ISP::input_formatter() { while(true) wait(); }
void ISP::demosaic_stage()  { while(true) wait(); }
void ISP::color_stage()     { while(true) wait(); }
void ISP::noise_stage()     { while(true) wait(); }
void ISP::output_formatter(){ while(true) wait(); }
