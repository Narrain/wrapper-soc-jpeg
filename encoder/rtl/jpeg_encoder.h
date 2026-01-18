#pragma once
#include <systemc.h>
#include <vector>
#include "pixel.h"
#include <jpeglib.h>

using namespace sc_core;
using namespace sc_dt;

SC_MODULE(JPEG_Encoder) {
    sc_in<bool> clk;
    sc_in<bool> rst_n;

    sc_in<pixel_t> in_pixel;
    sc_in<bool>    in_valid;
    sc_out<bool>   in_ready;

    sc_out<sc_dt::sc_uint<32>> bs_data;
    sc_out<bool>               bs_valid;
    sc_in<bool>                bs_ready;

    sc_in<bool> frame_start;
    sc_in<bool> frame_end;

    sc_out<bool> irq_done;

    int width;
    int height;

    std::vector<unsigned char> jpeg_buffer;
    unsigned long jpeg_size;

    std::vector<pixel_t> frame_pixels;

    void ingest_frame();
    void encode_and_stream();
    void run();

    SC_CTOR(JPEG_Encoder);

};
