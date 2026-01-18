#pragma once
#include <systemc.h>
#include "isp_stub.h"
#include "PixelFifo.h"
#include "jpeg_encoder.h"
// #include "WordFifo.h"
#include "BitstreamFifo.h"
#include "Dmastub.h"

using namespace sc_core;
using namespace sc_dt;

SC_MODULE(VisionSoC) {
    // Clock + reset
    sc_clock clk;
    sc_signal<bool> rst_n;

    // ISP → FIFO
    sc_signal<pixel_t> isp_pixel;
    sc_signal<bool>    isp_valid;
    sc_signal<bool>    isp_ready;
    sc_signal<bool>    frame_start;
    sc_signal<bool>    frame_end;

    // FIFO → Encoder
    sc_signal<pixel_t> s_pix_fifo_out;
    sc_signal<bool>    s_pix_fifo_valid;
    sc_signal<bool>    s_pix_fifo_ready;

    // Encoder → Bitstream FIFO
    sc_signal<sc_uint<32>> enc_bs_data;
    sc_signal<bool>        enc_bs_valid;
    sc_signal<bool>        enc_bs_ready;

    // Bitstream FIFO → DMA
    sc_signal<sc_uint<32>> s_bs_fifo_out;
    sc_signal<bool>        s_bs_fifo_valid;
    sc_signal<bool>        s_bs_fifo_ready;

    sc_signal<sc_uint<32>> dummy_data;
    sc_signal<bool> dummy_valid, dummy_ready;

    // IRQ
    sc_signal<bool> irq_done;

    // Blocks
    ISP_stub     isp;
    PixelFifo    pix_fifo;
    JPEG_Encoder enc;
    WordFifo     bs_fifo;
    DmaStub      dma;

    SC_CTOR(VisionSoC)
    : clk("clk", 10, SC_NS)
    , isp("isp")
    , pix_fifo("pix_fifo")
    , enc("encoder")
    , bs_fifo("bs_fifo")
    , dma("dma")
    {
        // ISP → FIFO
        isp.clk(clk);
        isp.rst_n(rst_n);
        isp.out_pixel(isp_pixel);
        isp.out_valid(isp_valid);
        isp.out_ready(isp_ready);
        isp.frame_start(frame_start);
        isp.frame_end(frame_end);

        // FIFO → Encoder
        pix_fifo.clk(clk);
        pix_fifo.rst_n(rst_n);
        pix_fifo.in_data(isp_pixel);
        pix_fifo.in_valid(isp_valid);
        pix_fifo.in_ready(isp_ready);
        pix_fifo.out_data(s_pix_fifo_out);
        pix_fifo.out_valid(s_pix_fifo_valid);
        pix_fifo.out_ready(s_pix_fifo_ready);

        // Encoder
        enc.clk(clk);
        enc.rst_n(rst_n);
        enc.in_pixel(s_pix_fifo_out);
        enc.in_valid(s_pix_fifo_valid);
        enc.in_ready(s_pix_fifo_ready);
        enc.frame_start(frame_start);
        enc.frame_end(frame_end);
        enc.bs_data(enc_bs_data);
        enc.bs_valid(enc_bs_valid);
        enc.bs_ready(enc_bs_ready);
        enc.irq_done(irq_done);



        bs_fifo.clk(clk);
        bs_fifo.rst_n(rst_n);
        bs_fifo.in_data(enc_bs_data);
        bs_fifo.in_valid(enc_bs_valid);
        bs_fifo.in_ready(enc_bs_ready);

        bs_fifo.out_data(s_bs_fifo_out);
        bs_fifo.out_valid(s_bs_fifo_valid);
        bs_fifo.out_ready(s_bs_fifo_ready);

        dma.clk(clk);
        dma.rst_n(rst_n);
        dma.in_data(s_bs_fifo_out);
        dma.in_valid(s_bs_fifo_valid);
        dma.in_ready(s_bs_fifo_ready);

    }

};
