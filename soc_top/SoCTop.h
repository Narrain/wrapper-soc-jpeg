#pragma once
#include <systemc.h>
#include "pixel.h"
#include "ISPTop.h"
#include "EncoderTop.h"

SC_MODULE(SoCTop) {
    sc_clock clk;
    sc_signal<bool> rst_n;

    // Pixel path
    sc_signal<pixel_t> sig_pix;
    sc_signal<bool>    sig_pix_valid;
    sc_signal<bool>    sig_pix_ready;
    sc_signal<bool>    sig_frame_start;
    sc_signal<bool>    sig_frame_end;

    // Reg bus (stubbed for now)
    sc_signal<bool>        sig_reg_write;
    sc_signal<bool>        sig_reg_read;
    sc_signal<sc_uint<8>>  sig_reg_addr;
    sc_signal<sc_uint<32>> sig_reg_wdata;
    sc_signal<sc_uint<32>> sig_reg_rdata;
    sc_signal<bool>        sig_reg_ready;

    // DMA + IRQ (not yet driven by real memory/MCU)
    sc_signal<sc_uint<32>> sig_dma_addr;
    sc_signal<sc_uint<32>> sig_dma_wdata;
    sc_signal<bool>        sig_dma_wvalid;
    sc_signal<bool>        sig_dma_wready;
    sc_signal<bool>        sig_dma_start;
    sc_signal<bool>        sig_dma_done;
    sc_signal<bool>        sig_irq;

    ISPTop*     isp;
    EncoderTop* enc;

    void reset_gen();

    SC_CTOR(SoCTop)
    : clk("clk", 10, SC_NS)
    {
        isp = new ISPTop("ISPTop");
        enc = new EncoderTop("EncoderTop");

        // Reset generator
        SC_THREAD(reset_gen);

        // ISP wiring
        isp->clk(clk);
        isp->rst_n(rst_n);
        isp->out_pixel(sig_pix);
        isp->out_valid(sig_pix_valid);
        isp->in_ready(sig_pix_ready);
        isp->out_frame_start(sig_frame_start);
        isp->out_frame_end(sig_frame_end);

        // Encoder wiring
        enc->clk(clk);
        enc->rst_n(rst_n);

        enc->in_pixel(sig_pix);
        enc->in_valid(sig_pix_valid);
        enc->in_ready(sig_pix_ready);
        enc->frame_start(sig_frame_start);
        enc->frame_end(sig_frame_end);

        // Reg bus: for now, left idle or driven by a testbench
        enc->reg_write(sig_reg_write);
        enc->reg_read(sig_reg_read);
        enc->reg_addr(sig_reg_addr);
        enc->reg_wdata(sig_reg_wdata);
        enc->reg_rdata(sig_reg_rdata);
        enc->reg_ready(sig_reg_ready);

        // DMA + IRQ
        enc->dma_addr(sig_dma_addr);
        enc->dma_wdata(sig_dma_wdata);
        enc->dma_wvalid(sig_dma_wvalid);
        enc->dma_wready(sig_dma_wready);
        enc->dma_start(sig_dma_start);
        enc->dma_done(sig_dma_done);
        enc->irq_out(sig_irq);

        // For now, tie-offs for DMA/memory model
        sig_dma_wready.write(true); // always ready sink
        sig_dma_done.write(false);  // you can pulse this in a TB later
    }

    ~SoCTop() {
        delete isp;
        delete enc;
    }
};
