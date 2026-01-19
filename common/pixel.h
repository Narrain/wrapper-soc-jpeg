#pragma once
#include <systemc.h>

struct pixel_t {
    sc_dt::sc_uint<8> r;
    sc_dt::sc_uint<8> g;
    sc_dt::sc_uint<8> b;

    bool operator==(const pixel_t& other) const {
        return (r == other.r) && (g == other.g) && (b == other.b);
    }

    // Optional: pack RGB into 24-bit word
    sc_dt::sc_uint<24> pack() const {
        sc_dt::sc_uint<24> v = 0;
        v.range(23,16) = r;
        v.range(15, 8) = g;
        v.range( 7, 0) = b;
        return v;
    }
};

inline std::ostream& operator<<(std::ostream& os, const pixel_t& p) {
    os << "(" << p.r << "," << p.g << "," << p.b << ")";
    return os;
}

inline void sc_trace(sc_core::sc_trace_file* tf, const pixel_t& p, const std::string& name)
{
    sc_trace(tf, p.r, name + ".r");
    sc_trace(tf, p.g, name + ".g");
    sc_trace(tf, p.b, name + ".b");
}
