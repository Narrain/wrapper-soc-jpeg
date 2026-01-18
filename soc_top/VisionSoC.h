#pragma once
#include <systemc.h>
#include "isp_stub.h"
#include "jpeg_encoder.h"
#include "BitstreamSink.h"

using namespace sc_core;
using namespace sc_dt;

SC_MODULE(VisionSoC) {
    // Clock + reset
    sc_clock clk;
    sc_signal<bool> rst_n;

    // ISP ↔ Encoder
    sc_signal<pixel_t> isp_pixel;
    sc_signal<bool>    isp_valid;
    sc_signal<bool>    isp_ready;
    sc_signal<bool>    frame_start;
    sc_signal<bool>    frame_end;

    // Encoder ↔ Sink
    sc_signal<sc_uint<32>> bs_data;
    sc_signal<bool>        bs_valid;
    sc_signal<bool>        bs_ready;
    sc_signal<bool>        irq_done;


    // Blocks
    ISP_stub     isp;
    JPEG_Encoder enc;
    BitstreamSink sink;

    SC_CTOR(VisionSoC)
    : clk("clk", 10, SC_NS)
    , isp("isp")
    , enc("encoder")
    , sink("sink")
    {
        // ISP wiring
        isp.clk(clk);
        isp.rst_n(rst_n);
        isp.out_pixel(isp_pixel);
        isp.out_valid(isp_valid);
        isp.out_ready(isp_ready);
        isp.frame_start(frame_start);
        isp.frame_end(frame_end);

        // Encoder wiring
        enc.clk(clk);
        enc.rst_n(rst_n);
        enc.in_pixel(isp_pixel);
        enc.in_valid(isp_valid);
        enc.in_ready(isp_ready);
        enc.frame_start(frame_start);
        enc.frame_end(frame_end);
        enc.bs_data(bs_data);
        enc.bs_valid(bs_valid);
        enc.bs_ready(bs_ready);
        enc.irq_done(irq_done);

        // Sink wiring
        sink.clk(clk);
        sink.rst_n(rst_n);
        sink.data(bs_data);
        sink.valid(bs_valid);
        sink.ready(bs_ready);
    }
};
