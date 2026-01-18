#include "encoder.h"
using namespace sc_core;

SC_CTOR(Encoder) {
    SC_CTHREAD(ingest, clk.pos());
    async_reset_signal_is(rst_n, false);

    SC_CTHREAD(transform, clk.pos());
    async_reset_signal_is(rst_n, false);

    SC_CTHREAD(entropy, clk.pos());
    async_reset_signal_is(rst_n, false);

    SC_CTHREAD(dma, clk.pos());
    async_reset_signal_is(rst_n, false);
}

void Encoder::ingest()   { while(true) wait(); }
void Encoder::transform(){ while(true) wait(); }
void Encoder::entropy()  { while(true) wait(); }
void Encoder::dma()      { while(true) wait(); }
