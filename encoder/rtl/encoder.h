#pragma once
#include <systemc>
#include "pixel.h"   // reuse from ISP, or define block_t later

SC_MODULE(Encoder) {
    // Clock / reset
    sc_core::sc_in<bool> clk;
    sc_core::sc_in<bool> rst_n;

    // Input from ISP (pixel stream)
    sc_core::sc_in<pixel_t> in_pixel;
    sc_core::sc_in<bool>    in_valid;
    sc_core::sc_out<bool>   in_ready;

    // Simplified bitstream output (for now: word stream)
    sc_core::sc_out<sc_dt::sc_uint<32>> bs_data;
    sc_core::sc_out<bool>               bs_valid;
    sc_core::sc_in<bool>                bs_ready;

    // IRQ
    sc_core::sc_out<bool> irq_out;

    // Internal signals between stages
    // Ingest FIFO → Block Maker
    sc_core::sc_signal<pixel_t> s_ing2blk_pix;
    sc_core::sc_signal<bool>    s_ing2blk_valid;
    sc_core::sc_signal<bool>    s_ing2blk_ready;

    // Block Maker → Transform/Quant
    // (later: block_t instead of pixel_t)
    sc_core::sc_signal<pixel_t> s_blk2dct_data;
    sc_core::sc_signal<bool>    s_blk2dct_valid;
    sc_core::sc_signal<bool>    s_blk2dct_ready;

    // Transform/Quant → Entropy
    sc_core::sc_signal<sc_dt::sc_uint<16>> s_dct2ent_sym;
    sc_core::sc_signal<bool>               s_dct2ent_valid;
    sc_core::sc_signal<bool>               s_dct2ent_ready;

    // Entropy → Bitstream FIFO
    sc_core::sc_signal<sc_dt::sc_uint<32>> s_ent2bs_word;
    sc_core::sc_signal<bool>               s_ent2bs_valid;
    sc_core::sc_signal<bool>               s_ent2bs_ready;

    // Bitstream FIFO → DMA (for now, we expose bs_* directly)

    // Processes
    void ingest_fifo();
    void block_maker();
    void transform_quant();
    void entropy_coder();
    void bitstream_fifo();

    SC_CTOR(Encoder);
};