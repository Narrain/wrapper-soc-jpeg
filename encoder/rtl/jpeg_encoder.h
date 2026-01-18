#pragma once
#include <systemc.h>
#include <vector>
#include "pixel.h"
#include <jpeglib.h>

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
    // Clock / reset
    sc_in<bool> clk;
    sc_in<bool> rst_n;

    // Pixel input
    sc_in<pixel_t> in_pixel;
    sc_in<bool>    in_valid;
    sc_out<bool>   in_ready;

    // Bitstream output
    sc_out<sc_uint<32>> bs_data;
    sc_out<bool>        bs_valid;
    sc_in<bool>         bs_ready;

    // Frame control
    sc_in<bool> frame_start;
    sc_in<bool> frame_end;

    // IRQ
    sc_out<bool>        irq_done;
    sc_signal<sc_uint<32>> irq_status;

    // Config (now per‑instance, not global)
    jpeg_cfg_t cfg;

    // Software JPEG buffers
    std::vector<unsigned char> jpeg_buffer;
    unsigned long jpeg_size;

    std::vector<pixel_t> frame_pixels;

    // Processes
    void ingest_frame();
    void encode_and_stream();
    void run();

    SC_CTOR(JPEG_Encoder);
};
