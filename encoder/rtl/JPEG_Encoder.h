// jpeg_encoder.h
#pragma once
#include <systemc.h>
#include "pixel.h"
#include "block_types.h"
#include "HeaderGen.h"
#include "DCTQ.h"
#include "Entropy.h"

using namespace sc_core;
using namespace sc_dt;

struct jpeg_cfg_t {
    sc_uint<16> width;
    sc_uint<16> height;
    sc_uint<8>  quality;
    sc_uint<1>  irq_en;
};

enum {
    IRQ_FRAME_DONE  = 1 << 0,
    IRQ_FIFO_OVF    = 1 << 1,
    IRQ_DMA_ERR     = 1 << 2,
};

SC_MODULE(JPEG_Encoder) {
    sc_in<bool> clk;
    sc_in<bool> rst_n;

    sc_in<sc_int<16>> in_block[64];
    sc_in<bool>       in_valid;
    sc_out<bool>      in_ready;
    sc_in<sc_uint<2>> in_type;

    sc_in<bool> frame_start;
    sc_in<bool> frame_end;

    sc_in<bool> restart_mcu_boundary;

    sc_out<sc_uint<32>> bs_data;
    sc_out<bool>        bs_valid;
    sc_in<bool>         bs_ready;

    sc_out<bool>        irq_done;
    sc_signal<sc_uint<32>> irq_status;

    sc_in<sc_uint<16>> cfg_width;
    sc_in<sc_uint<16>> cfg_height;
    sc_in<sc_uint<8>>  cfg_quality;

    // Internal: header → entropy mux
    sc_signal<sc_uint<32>> s_hdr_data;
    sc_signal<bool>        s_hdr_valid;
    sc_signal<bool>        s_hdr_ready;

    sc_signal<bool>        dct_in_valid,  dct_in_ready;
    sc_signal<Block>       dct_in_block;
    sc_signal<bool>        dct_out_valid, dct_out_ready;
    sc_signal<Block>       dct_out_block;

    sc_signal<bool>        ent_in_valid,  ent_in_ready;
    sc_signal<Block>       ent_in_block;
    sc_signal<bool>        ent_out_valid;
    sc_signal<sc_uint<32>> ent_out_data;

    HeaderGen* header;
    DCTQ*      dctq;
    Entropy*   entropy;

    enum State {
        S_IDLE,
        S_HEADER,
        S_ENTROPY,
        S_SCAN,
        S_EOI,
        S_DONE
    };
    sc_signal<State> state;

    void run();

    SC_CTOR(JPEG_Encoder) {
        header  = new HeaderGen("header_gen");
        dctq    = new DCTQ("dctq");
        entropy = new Entropy("entropy");

        header->clk(clk);
        header->rst_n(rst_n);
        header->start(frame_start);
        header->out_data(s_hdr_data);
        header->out_valid(s_hdr_valid);
        header->out_ready(s_hdr_ready);
        header->width(cfg_width);
        header->height(cfg_height);
        header->quality(cfg_quality);

        dctq->clk(clk);
        dctq->rst_n(rst_n);
        dctq->in_valid(dct_in_valid);
        dctq->in_ready(dct_in_ready);
        dctq->in_block(dct_in_block);
        dctq->out_valid(dct_out_valid);
        dctq->out_ready(dct_out_ready);
        dctq->out_block(dct_out_block);

        entropy->clk(clk);
        entropy->rst_n(rst_n);
        entropy->in_valid(ent_in_valid);
        entropy->in_ready(ent_in_ready);
        entropy->in_block(ent_in_block);
        entropy->out_valid(ent_out_valid);
        entropy->out_ready(bs_ready);
        entropy->out_data(ent_out_data);
        entropy->restart_mcu_boundary(restart_mcu_boundary);

        SC_CTHREAD(run, clk.pos());
        async_reset_signal_is(rst_n, false);
    }
};
