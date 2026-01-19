#pragma once
#include <systemc.h>

enum BlockType : uint8_t {
    BLOCK_Y  = 0,
    BLOCK_CB = 1,
    BLOCK_CR = 2
};

struct Block {
    sc_int<16> data[64];
    BlockType  type;
};


inline void sc_trace(sc_core::sc_trace_file* tf, const Block& b, const std::string& name)
{
    for (int i = 0; i < 64; ++i) {
        sc_trace(tf, b.data[i], name + ".d" + std::to_string(i));
    }
    sc_trace(tf, (int)b.type, name + ".type");
}

inline std::ostream& operator<<(std::ostream& os, const Block& b)
{
    os << "Block(type=" << (int)b.type << ", data=[";
    for (int i = 0; i < 64; ++i) {
        os << b.data[i];
        if (i != 63) os << ",";
    }
    os << "])";
    return os;
}

inline bool operator==(const Block& a, const Block& b)
{
    if (a.type != b.type) return false;
    for (int i = 0; i < 64; ++i)
        if (a.data[i] != b.data[i]) return false;
    return true;
}
