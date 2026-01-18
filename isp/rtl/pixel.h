#pragma once
#include <systemc>

struct pixel_t {
    sc_dt::sc_uint<8> r;
    sc_dt::sc_uint<8> g;
    sc_dt::sc_uint<8> b;

    // Required for sc_signal<T>
    bool operator==(const pixel_t& other) const {
        return (r == other.r) && (g == other.g) && (b == other.b);
    }
};

// Required for printing (SystemC debug)
inline std::ostream& operator<<(std::ostream& os, const pixel_t& p) {
    os << "(" << p.r << "," << p.g << "," << p.b << ")";
    return os;
}

// Required for VCD tracing
inline void sc_trace(sc_core::sc_trace_file* tf, const pixel_t& p, const std::string& name)
{
    sc_trace(tf, p.r, name + ".r");
    sc_trace(tf, p.g, name + ".g");
    sc_trace(tf, p.b, name + ".b");
}
