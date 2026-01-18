#pragma once
#include <systemc>

struct pixel_t {
    sc_dt::sc_uint<8> r;
    sc_dt::sc_uint<8> g;
    sc_dt::sc_uint<8> b;
};
