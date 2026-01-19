#pragma once
#include <systemc.h>
#include "isp_stub.h"
#include "PixelFifo.h"
#include "BlockMaker.h"
#include "JPEG_Encoder.h"
#include "BitstreamFifo.h"
#include "Dmastub.h"
#include "ConfigRegs.h"

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

    // FIFO → BlockMaker
    sc_signal<pixel_t> s_pix_fifo_out;
    sc_signal<bool>    s_pix_fifo_valid;
    sc_signal<bool>    s_pix_fifo_ready;

    // BlockMaker → Encoder (blocks)
    sc_signal<sc_int<16>> bm_block[64];
    sc_signal<bool>       bm_valid;
    sc_signal<bool>       bm_ready;
    sc_signal<sc_uint<2>> bm_type;
    sc_signal<bool>       bm_restart_mcu;

    // Encoder → Bitstream FIFO
    sc_signal<sc_uint<32>> enc_bs_data;
    sc_signal<bool>        enc_bs_valid;
    sc_signal<bool>        enc_bs_ready;

    // Bitstream FIFO → DMA
    sc_signal<sc_uint<32>> s_bs_fifo_out;
    sc_signal<bool>        s_bs_fifo_valid;
    sc_signal<bool>        s_bs_fifo_ready;

    // IRQ
    sc_signal<bool> irq_done;

    // Simple control bus (for now driven from testbench/sc_main)
    sc_signal<bool>        cfg_wr_en;
    sc_signal<sc_uint<8>>  cfg_wr_addr;
    sc_signal<sc_uint<32>> cfg_wr_data;

    // Config outputs
    sc_signal<sc_uint<16>> cfg_width;
    sc_signal<sc_uint<16>> cfg_height;
    sc_signal<sc_uint<2>>  cfg_subsampling;
    sc_signal<sc_uint<16>> cfg_restart_interval;
    sc_signal<sc_uint<8>>  cfg_quality;
    sc_signal<bool>        cfg_start;
    sc_signal<sc_uint<32>> cfg_dma_base;
    sc_signal<sc_uint<8>>  cfg_irq_mask;

    // Blocks
    ISP_stub      isp;
    PixelFifo     pix_fifo;
    BlockMaker    block_maker;
    JPEG_Encoder  enc;
    BitstreamFifo bs_fifo;
    DmaStub       dma;
    ConfigRegs    config_regs;

    SC_CTOR(VisionSoC)
    : clk("clk", 10, SC_NS)
    , isp("isp")
    , pix_fifo("pix_fifo")
    , block_maker("block_maker")
    , enc("encoder")
    , bs_fifo("bs_fifo")
    , dma("dma")
    , config_regs("config_regs")
    {
        // ISP → FIFO
        isp.clk(clk);
        isp.rst_n(rst_n);
        isp.out_pixel(isp_pixel);
        isp.out_valid(isp_valid);
        isp.out_ready(isp_ready);
        isp.frame_start(frame_start);
        isp.frame_end(frame_end);

        // Pixel FIFO
        pix_fifo.clk(clk);
        pix_fifo.rst_n(rst_n);
        pix_fifo.in_data(isp_pixel);
        pix_fifo.in_valid(isp_valid);
        pix_fifo.in_ready(isp_ready);
        pix_fifo.out_data(s_pix_fifo_out);
        pix_fifo.out_valid(s_pix_fifo_valid);
        pix_fifo.out_ready(s_pix_fifo_ready);

        // ConfigRegs
        config_regs.clk(clk);
        config_regs.rst_n(rst_n);
        config_regs.wr_en(cfg_wr_en);
        config_regs.wr_addr(cfg_wr_addr);
        config_regs.wr_data(cfg_wr_data);

        config_regs.width(cfg_width);
        config_regs.height(cfg_height);
        config_regs.subsampling(cfg_subsampling);
        config_regs.restart_interval(cfg_restart_interval);
        config_regs.quality(cfg_quality);
        config_regs.start(cfg_start);
        config_regs.dma_base(cfg_dma_base);
        config_regs.irq_mask(cfg_irq_mask);

        // BlockMaker config + streaming
        block_maker.clk(clk);
        block_maker.rst_n(rst_n);
        block_maker.cfg_width(cfg_width);
        block_maker.cfg_height(cfg_height);
        block_maker.cfg_subsampling(cfg_subsampling);
        block_maker.cfg_restart_interval(cfg_restart_interval);

        block_maker.in_pixel(s_pix_fifo_out);
        block_maker.in_valid(s_pix_fifo_valid);
        block_maker.in_ready(s_pix_fifo_ready);

        for (int i = 0; i < 64; ++i) {
            block_maker.out_block[i](bm_block[i]);
        }
        block_maker.out_valid(bm_valid);
        block_maker.out_ready(bm_ready);
        block_maker.out_type(bm_type);
        block_maker.restart_mcu_boundary(bm_restart_mcu);

        // JPEG_Encoder
        enc.clk(clk);
        enc.rst_n(rst_n);
        for (int i = 0; i < 64; ++i) {
            enc.in_block[i](bm_block[i]);
        }
        enc.in_valid(bm_valid);
        enc.in_ready(bm_ready);
        enc.in_type(bm_type);
        enc.frame_start(frame_start);
        enc.frame_end(frame_end);

        enc.restart_mcu_boundary(bm_restart_mcu);
        enc.cfg_width(cfg_width);
        enc.cfg_height(cfg_height);
        enc.cfg_quality(cfg_quality);

        enc.bs_data(enc_bs_data);
        enc.bs_valid(enc_bs_valid);
        enc.bs_ready(enc_bs_ready);
        enc.irq_done(irq_done);

        // Bitstream FIFO
        bs_fifo.clk(clk);
        bs_fifo.rst_n(rst_n);
        bs_fifo.in_data(enc_bs_data);
        bs_fifo.in_valid(enc_bs_valid);
        bs_fifo.in_ready(enc_bs_ready);
        bs_fifo.out_data(s_bs_fifo_out);
        bs_fifo.out_valid(s_bs_fifo_valid);
        bs_fifo.out_ready(s_bs_fifo_ready);

        // DMA stub
        dma.clk(clk);
        dma.rst_n(rst_n);
        dma.in_data(s_bs_fifo_out);
        dma.in_valid(s_bs_fifo_valid);
        dma.in_ready(s_bs_fifo_ready);
        dma.irq_done(irq_done);

        SC_THREAD(bs_monitor);
        sensitive << enc_bs_valid.posedge_event();
        dont_initialize();

        SC_THREAD(bm_monitor);
        sensitive << bm_valid.posedge_event();
        dont_initialize();


    }

    void bs_monitor() {
        while (true) {
            wait(); // on enc_bs_valid.pos()
            auto w = enc_bs_data.read();
            std::cout << sc_time_stamp()
                    << " [ENC] bs_data=0x" << std::hex << w.to_uint()
                    << std::dec << "\n";
        }
    }
    void bm_monitor() {
        while (true) {
            wait(); // on bm_valid.pos()
            std::cout << sc_time_stamp()
                    << " [BM] out_valid=1, type=" << bm_type.read()
                    << " restart=" << bm_restart_mcu.read() << "\n";

            // Dump first few coefficients of the block
            std::cout << "       [BM] block[0..7]: ";
            for (int i = 0; i < 8; ++i)
                std::cout << bm_block[i].read() << " ";
            std::cout << "\n";
        }
    }

};
