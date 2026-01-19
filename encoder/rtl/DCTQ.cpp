#include "DCTQ.h"
#include <cmath>

using namespace sc_core;
using namespace sc_dt;

// Example JPEG-like quant tables (can be later driven by cfg.quality)
static const int QTABLE_LUMA[64] = {
     16,11,10,16,24,40,51,61,
     12,12,14,19,26,58,60,55,
     14,13,16,24,40,57,69,56,
     14,17,22,29,51,87,80,62,
     18,22,37,56,68,109,103,77,
     24,35,55,64,81,104,113,92,
     49,64,78,87,103,121,120,101,
     72,92,95,98,112,100,103,99
};

static const int QTABLE_CHROMA[64] = {
    17,18,24,47,99,99,99,99,
    18,21,26,66,99,99,99,99,
    24,26,56,99,99,99,99,99,
    47,66,99,99,99,99,99,99,
    99,99,99,99,99,99,99,99,
    99,99,99,99,99,99,99,99,
    99,99,99,99,99,99,99,99,
    99,99,99,99,99,99,99,99
};

// 2D DCT using floating-point math, then rounded back to int16
static void fdct8x8(const sc_int<16> in[64], sc_int<16> out[64]) {
    const double PI = 3.14159265358979323846;
    double tmp[64];

    // Level shift (if needed you can subtract 128 here for 8-bit input)
    for (int i = 0; i < 64; ++i)
        tmp[i] = (double)in[i].to_int();

    // Row-wise 1D DCT
    for (int y = 0; y < 8; ++y) {
        for (int u = 0; u < 8; ++u) {
            double sum = 0.0;
            for (int x = 0; x < 8; ++x) {
                double s = tmp[y*8 + x];
                sum += s * std::cos((PI / 8.0) * (x + 0.5) * u);
            }
            double cu = (u == 0) ? std::sqrt(0.5) : 1.0;
            tmp[y*8 + u] = 0.5 * cu * sum;
        }
    }

    // Column-wise 1D DCT
    double col[8];
    for (int x = 0; x < 8; ++x) {
        for (int v = 0; v < 8; ++v) {
            double sum = 0.0;
            for (int y = 0; y < 8; ++y) {
                double s = tmp[y*8 + x];
                sum += s * std::cos((PI / 8.0) * (y + 0.5) * v);
            }
            double cv = (v == 0) ? std::sqrt(0.5) : 1.0;
            col[v] = 0.5 * cv * sum;
        }
        for (int v = 0; v < 8; ++v)
            tmp[v*8 + x] = col[v];
    }

    // Back to int16
    for (int i = 0; i < 64; ++i) {
        int v = (int)std::round(tmp[i]);
        if (v < -32768) v = -32768;
        if (v >  32767) v =  32767;
        out[i] = v;
    }
}

static void quantize_block(const sc_int<16> in[64],
                           sc_int<16> out[64],
                           BlockType type)
{
    const int* q = (type == BLOCK_Y) ? QTABLE_LUMA : QTABLE_CHROMA;
    for (int i = 0; i < 64; ++i) {
        int v  = in[i].to_int();
        int qv = q[i];
        int qd = (qv == 0) ? v : (v / qv);
        if (qd < -32768) qd = -32768;
        if (qd >  32767) qd =  32767;
        out[i] = qd;
    }
}

void DCTQ::proc() {
    in_ready.write(true);
    out_valid.write(false);
    wait();

    while (true) {
        if (!rst_n.read()) {
            in_ready.write(true);
            out_valid.write(false);
        } else {
            bool can_accept = (!out_valid.read() || out_ready.read());
            in_ready.write(can_accept);

            if (in_valid.read() && can_accept) {
                Block inb = in_block.read();
                Block outb;
                outb.type = inb.type;

                sc_int<16> dct_out[64];
                fdct8x8(inb.data, dct_out);
                quantize_block(dct_out, outb.data, inb.type);

                out_block.write(outb);
                out_valid.write(true);
            } else if (out_ready.read()) {
                out_valid.write(false);
            }
        }
        wait();
    }
}
