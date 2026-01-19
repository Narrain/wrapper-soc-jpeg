#include "Entropy.h"

using namespace sc_core;
using namespace sc_dt;

struct HuffCode { uint16_t code; uint8_t size; };

static const int ZIGZAG[64] = {
     0,  1,  5,  6, 14, 15, 27, 28,
     2,  4,  7, 13, 16, 26, 29, 42,
     3,  8, 12, 17, 25, 30, 41, 43,
     9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};

// DC Luma (table 0) – from DHT (0x00)
static const HuffCode DC_LUMA[12] = {
    {0x0000,2}, // 0
    {0x0002,3}, // 1
    {0x0003,3}, // 2
    {0x0004,3}, // 3
    {0x0005,3}, // 4
    {0x0006,3}, // 5
    {0x000E,4}, // 6
    {0x001E,5}, // 7
    {0x003E,6}, // 8
    {0x007E,7}, // 9
    {0x00FE,8}, // 10
    {0x01FE,9}  // 11
};

// DC Chroma (table 1) – from DHT (0x01)
static const HuffCode DC_CHROMA[12] = {
    {0x0000,2}, // 0
    {0x0001,2}, // 1
    {0x0002,2}, // 2
    {0x0006,3}, // 3
    {0x000E,4}, // 4
    {0x001E,5}, // 5
    {0x003E,6}, // 6
    {0x007E,7}, // 7
    {0x00FE,8}, // 8
    {0x01FE,9}, // 9
    {0x03FE,10},// 10
    {0x07FE,11} // 11
};

static inline const HuffCode* dc_table(BlockType t) {
    return (t == BLOCK_Y) ? DC_LUMA : DC_CHROMA;
}

// AC Luma (table 0x10) – standard JPEG, canonical
static const HuffCode AC_LUMA[] = {
    // run=0
    {0x000A,4},{0x0000,0},{0x0001,2},{0x0004,3},{0x000B,4},{0x001A,5},{0x0078,7},{0x00F8,8},
    {0x03F6,10},{0xFF82,16},{0xFF83,16},{0xFF84,16},{0xFF85,16},{0xFF86,16},{0xFF87,16},{0xFF88,16},
    // run=1
    {0x0000,0},{0x000C,4},{0x001B,5},{0x0079,7},{0x01F6,9},{0x07F6,11},{0xFF89,16},{0xFF8A,16},
    {0xFF8B,16},{0xFF8C,16},{0xFF8D,16},{0xFF8E,16},{0xFF8F,16},{0xFF90,16},{0xFF91,16},{0xFF92,16},
    // run=2
    {0x0000,0},{0x000D,4},{0x003A,6},{0x00F9,8},{0x03F7,10},{0xFF93,16},{0xFF94,16},{0xFF95,16},
    {0xFF96,16},{0xFF97,16},{0xFF98,16},{0xFF99,16},{0xFF9A,16},{0xFF9B,16},{0xFF9C,16},{0xFF9D,16},
    // run=3
    {0x0000,0},{0x001C,5},{0x00FA,8},{0x01F7,9},{0x0FF4,12},{0xFF9E,16},{0xFF9F,16},{0xFFA0,16},
    {0xFFA1,16},{0xFFA2,16},{0xFFA3,16},{0xFFA4,16},{0xFFA5,16},{0xFFA6,16},{0xFFA7,16},{0xFFA8,16},
    // run=4
    {0x0000,0},{0x001D,5},{0x01F8,9},{0x07F7,11},{0xFFA9,16},{0xFFAA,16},{0xFFAB,16},{0xFFAC,16},
    {0xFFAD,16},{0xFFAE,16},{0xFFAF,16},{0xFFB0,16},{0xFFB1,16},{0xFFB2,16},{0xFFB3,16},{0xFFB4,16},
    // run=5
    {0x0000,0},{0x003B,6},{0x03F8,10},{0x0FF5,12},{0xFFB5,16},{0xFFB6,16},{0xFFB7,16},{0xFFB8,16},
    {0xFFB9,16},{0xFFBA,16},{0xFFBB,16},{0xFFBC,16},{0xFFBD,16},{0xFFBE,16},{0xFFBF,16},{0xFFC0,16},
    // run=6
    {0x0000,0},{0x007A,7},{0x07F8,11},{0xFFC1,16},{0xFFC2,16},{0xFFC3,16},{0xFFC4,16},{0xFFC5,16},
    {0xFFC6,16},{0xFFC7,16},{0xFFC8,16},{0xFFC9,16},{0xFFCA,16},{0xFFCB,16},{0xFFCC,16},{0xFFCD,16},
    // run=7
    {0x0000,0},{0x00FB,8},{0x0FF6,12},{0xFFCE,16},{0xFFCF,16},{0xFFD0,16},{0xFFD1,16},{0xFFD2,16},
    {0xFFD3,16},{0xFFD4,16},{0xFFD5,16},{0xFFD6,16},{0xFFD7,16},{0xFFD8,16},{0xFFD9,16},{0xFFDA,16},
    // run=8
    {0x0000,0},{0x01F9,9},{0xFFDB,16},{0xFFDC,16},{0xFFDD,16},{0xFFDE,16},{0xFFDF,16},{0xFFE0,16},
    {0xFFE1,16},{0xFFE2,16},{0xFFE3,16},{0xFFE4,16},{0xFFE5,16},{0xFFE6,16},{0xFFE7,16},{0xFFE8,16},
    // run=9
    {0x0000,0},{0x03F9,10},{0xFFE9,16},{0xFFEA,16},{0xFFEB,16},{0xFFEC,16},{0xFFED,16},{0xFFEE,16},
    {0xFFEF,16},{0xFFF0,16},{0xFFF1,16},{0xFFF2,16},{0xFFF3,16},{0xFFF4,16},{0xFFF5,16},{0xFFF6,16},
    // run=10
    {0x0000,0},{0x07F9,11},{0xFFF7,16},{0xFFF8,16},{0xFFF9,16},{0xFFFA,16},{0xFFFB,16},{0xFFFC,16},
    {0xFFFD,16},{0xFFFE,16},{0xFFFF,16},{0x0000,0},{0x0000,0},{0x0000,0},{0x0000,0},{0x0000,0},
    // run=11
    {0x0000,0},{0x0FF7,12},{0x0000,0},{0x0000,0},{0x0000,0},{0x0000,0},{0x0000,0},{0x0000,0},
    {0x0000,0},{0x0000,0},{0x0000,0},{0x0000,0},{0x0000,0},{0x0000,0},{0x0000,0},{0x0000,0}
};

// AC Chroma (table 0x11) – standard JPEG, canonical
static const HuffCode AC_CHROMA[] = {
    // run=0
    {0x0000,2},{0x0000,0},{0x0001,2},{0x0004,3},{0x000A,4},{0x0018,5},{0x0038,6},{0x00F8,8},
    {0x01F6,9},{0x07F6,11},{0xFF88,16},{0xFF89,16},{0xFF8A,16},{0xFF8B,16},{0xFF8C,16},{0xFF8D,16},
    // run=1
    {0x0000,0},{0x000B,4},{0x0039,6},{0x00F9,8},{0x03F6,10},{0x0FF4,12},{0xFF8E,16},{0xFF8F,16},
    {0xFF90,16},{0xFF91,16},{0xFF92,16},{0xFF93,16},{0xFF94,16},{0xFF95,16},{0xFF96,16},{0xFF97,16},
    // run=2
    {0x0000,0},{0x0019,5},{0x00FA,8},{0x01F7,9},{0x07F7,11},{0xFF98,16},{0xFF99,16},{0xFF9A,16},
    {0xFF9B,16},{0xFF9C,16},{0xFF9D,16},{0xFF9E,16},{0xFF9F,16},{0xFFA0,16},{0xFFA1,16},{0xFFA2,16},
    // run=3
    {0x0000,0},{0x001A,5},{0x01F8,9},{0x0FF5,12},{0xFFA3,16},{0xFFA4,16},{0xFFA5,16},{0xFFA6,16},
    {0xFFA7,16},{0xFFA8,16},{0xFFA9,16},{0xFFAA,16},{0xFFAB,16},{0xFFAC,16},{0xFFAD,16},{0xFFAE,16},
    // run=4
    {0x0000,0},{0x003A,6},{0x03F7,10},{0xFFAF,16},{0xFFB0,16},{0xFFB1,16},{0xFFB2,16},{0xFFB3,16},
    {0xFFB4,16},{0xFFB5,16},{0xFFB6,16},{0xFFB7,16},{0xFFB8,16},{0xFFB9,16},{0xFFBA,16},{0xFFBB,16},
    // run=5
    {0x0000,0},{0x003B,6},{0x07F8,11},{0xFFBC,16},{0xFFBD,16},{0xFFBE,16},{0xFFBF,16},{0xFFC0,16},
    {0xFFC1,16},{0xFFC2,16},{0xFFC3,16},{0xFFC4,16},{0xFFC5,16},{0xFFC6,16},{0xFFC7,16},{0xFFC8,16},
    // run=6
    {0x0000,0},{0x0078,7},{0x0FF6,12},{0xFFC9,16},{0xFFCA,16},{0xFFCB,16},{0xFFCC,16},{0xFFCD,16},
    {0xFFCE,16},{0xFFCF,16},{0xFFD0,16},{0xFFD1,16},{0xFFD2,16},{0xFFD3,16},{0xFFD4,16},{0xFFD5,16},
    // run=7
    {0x0000,0},{0x00F8,8},{0xFFD6,16},{0xFFD7,16},{0xFFD8,16},{0xFFD9,16},{0xFFDA,16},{0xFFDB,16},
    {0xFFDC,16},{0xFFDD,16},{0xFFDE,16},{0xFFDF,16},{0xFFE0,16},{0xFFE1,16},{0xFFE2,16},{0xFFE3,16},
    // run=8
    {0x0000,0},{0x01F6,9},{0xFFE4,16},{0xFFE5,16},{0xFFE6,16},{0xFFE7,16},{0xFFE8,16},{0xFFE9,16},
    {0xFFEA,16},{0xFFEB,16},{0xFFEC,16},{0xFFED,16},{0xFFEE,16},{0xFFEF,16},{0xFFF0,16},{0xFFF1,16},
    // run=9
    {0x0000,0},{0x03F6,10},{0xFFF2,16},{0xFFF3,16},{0xFFF4,16},{0xFFF5,16},{0xFFF6,16},{0xFFF7,16},
    {0xFFF8,16},{0xFFF9,16},{0xFFFA,16},{0xFFFB,16},{0xFFFC,16},{0xFFFD,16},{0xFFFE,16},{0xFFFF,16},
    // run=10
    {0x0000,0},{0x07F6,11},{0x0000,0},{0x0000,0},{0x0000,0},{0x0000,0},{0x0000,0},{0x0000,0},
    {0x0000,0},{0x0000,0},{0x0000,0},{0x0000,0},{0x0000,0},{0x0000,0},{0x0000,0},{0x0000,0},
    // run=11
    {0x0000,0},{0x0FF7,12},{0x0000,0},{0x0000,0},{0x0000,0},{0x0000,0},{0x0000,0},{0x0000,0},
    {0x0000,0},{0x0000,0},{0x0000,0},{0x0000,0},{0x0000,0},{0x0000,0},{0x0000,0},{0x0000,0}
};

static inline const HuffCode* ac_table(BlockType t) {
    return (t == BLOCK_Y) ? AC_LUMA : AC_CHROMA;
}

// Minimal AC support: EOB + ZRL per component
static const HuffCode AC_LUMA_EOB   = {0x000A,4};
static const HuffCode AC_LUMA_ZRL   = {0x00F0,8};
static const HuffCode AC_CHROMA_EOB = {0x0000,2};
static const HuffCode AC_CHROMA_ZRL = {0x00F0,8};

static inline HuffCode ac_eob(BlockType t) {
    return (t == BLOCK_Y) ? AC_LUMA_EOB : AC_CHROMA_EOB;
}

static inline HuffCode ac_zrl(BlockType t) {
    return (t == BLOCK_Y) ? AC_LUMA_ZRL : AC_CHROMA_ZRL;
}

static int category(int v) {
    if (v == 0) return 0;
    int a = (v < 0) ? -v : v;
    int c = 0;
    while (a) { a >>= 1; ++c; }
    return c;
}

static uint16_t amplitude_bits(int v, int cat) {
    if (cat == 0) return 0;
    if (v >= 0) return (uint16_t)v;
    int mask = (1 << cat) - 1;
    return (uint16_t)((v - 1) & mask);
}

#if 0
void Entropy::proc() {
    in_ready.write(true);
    out_valid.write(false);
    out_data.write(0);

    int prev_dc_y  = 0;
    int prev_dc_cb = 0;
    int prev_dc_cr = 0;

    sc_uint<32> bitbuf = 0;
    int bitcnt = 0;

    auto put_bits = [&](uint16_t code, int size) {
        bitbuf = (bitbuf << size) | code;
        bitcnt += size;
    };

    auto flush_word = [&](bool force, bool &have, sc_uint<32> &w) {
        have = false;
        if (bitcnt >= 32) {
            w = bitbuf >> (bitcnt - 32);
            bitcnt -= 32;
            have = true;
        } else if (force && bitcnt > 0) {
            w = bitbuf << (32 - bitcnt);
            bitcnt = 0;
            have = true;
        }
    };

    wait();

    while (true) {
        if (!rst_n.read()) {
            in_ready.write(true);
            out_valid.write(false);
            out_data.write(0);
            prev_dc_y = prev_dc_cb = prev_dc_cr = 0;
            bitbuf = 0;
            bitcnt = 0;
        } else {
            bool can_accept = (!out_valid.read() || out_ready.read());
            in_ready.write(can_accept);

            if (out_valid.read() && out_ready.read())
                out_valid.write(false);

            if (in_valid.read() && can_accept) {
                Block b = in_block.read();
                BlockType comp = b.type;

                int zz[64];
                for (int i = 0; i < 64; ++i)
                    zz[i] = b.data[ZIGZAG[i]].to_int();

                int prev_dc =
                    (comp == BLOCK_Y)  ? prev_dc_y :
                    (comp == BLOCK_CB) ? prev_dc_cb : prev_dc_cr;

                int dc = zz[0];
                int diff = dc - prev_dc;
                int cat = category(diff);
                uint16_t amp = amplitude_bits(diff, cat);

                const HuffCode* dct = dc_table(comp);
                HuffCode hdc = dct[cat];
                put_bits(hdc.code, hdc.size);
                if (cat) put_bits(amp, cat);

                if (comp == BLOCK_Y) prev_dc_y = dc;
                else if (comp == BLOCK_CB) prev_dc_cb = dc;
                else prev_dc_cr = dc;

                int run = 0;

                for (int i = 1; i < 64; ++i) {
                    int v = zz[i];
                    if (v == 0) {
                        run++;
                    } else {
                        while (run >= 16) {
                            HuffCode zrl = ac_zrl(comp);
                            put_bits(zrl.code, zrl.size);
                            run -= 16;
                        }
                        int ac_cat = category(v);
                        uint16_t ac_amp = amplitude_bits(v, ac_cat);
                        // For now, just emit EOB after nonzero (no full AC table)
                        put_bits(ac_amp, ac_cat);
                        run = 0;
                    }
                }

                if (run > 0) {
                    HuffCode eob = ac_eob(comp);
                    put_bits(eob.code, eob.size);
                }

                if (restart_mcu_boundary.read()) {
                    prev_dc_y = prev_dc_cb = prev_dc_cr = 0;
                }

                bool have = false;
                sc_uint<32> w = 0;
                flush_word(false, have, w);
                if (have) {
                    out_data.write(w);
                    out_valid.write(true);
                }
            } else {
                if (!out_valid.read()) {
                    bool have = false;
                    sc_uint<32> w = 0;
                    flush_word(false, have, w);
                    if (have) {
                        out_data.write(w);
                        out_valid.write(true);
                    }
                }
            }
        }
        wait();
    }
}
#endif

void Entropy::proc() {
    in_ready.write(true);
    out_valid.write(false);
    out_data.write(0);

    int prev_dc_y  = 0;
    int prev_dc_cb = 0;
    int prev_dc_cr = 0;

    // Bit accumulator (MSB-first)
    sc_uint<64> bitbuf = 0;
    int bitcnt = 0;

    // Byte → 32-bit word packer
    sc_uint<32> wordbuf = 0;
    int word_bytes = 0;

    auto put_byte = [&](uint8_t byte, bool &have, sc_uint<32> &w) {
        have = false;

        // JPEG byte stuffing: 0xFF → 0xFF 00
        auto push_raw = [&](uint8_t b, bool &have2, sc_uint<32> &w2) {
            have2 = false;
            wordbuf = (wordbuf << 8) | b;
            word_bytes++;
            if (word_bytes == 4) {
                w2 = wordbuf;
                wordbuf = 0;
                word_bytes = 0;
                have2 = true;
            }
        };

        if (byte == 0xFF) {
            bool h = false;
            sc_uint<32> tmp = 0;
            push_raw(0xFF, h, tmp);
            if (h) { have = true; w = tmp; return; }
            push_raw(0x00, h, tmp);
            if (h) { have = true; w = tmp; return; }
        } else {
            bool h = false;
            sc_uint<32> tmp = 0;
            push_raw(byte, h, tmp);
            if (h) { have = true; w = tmp; return; }
        }
    };

    auto put_bits = [&](uint16_t code, int size, bool &have, sc_uint<32> &w) {
        have = false;
        if (size == 0) return;
        bitbuf = (bitbuf << size) | code;
        bitcnt += size;

        while (bitcnt >= 8) {
            uint8_t byte = (bitbuf >> (bitcnt - 8)) & 0xFF;
            bitcnt -= 8;
            bool h = false;
            sc_uint<32> tmp = 0;
            put_byte(byte, h, tmp);
            if (h && !have) {
                have = true;
                w = tmp;
                // if more bytes exist, they’ll be flushed on later calls
                return;
            }
        }
    };

    auto flush_word = [&](bool force, bool &have, sc_uint<32> &w) {
        have = false;

        // First, flush any full bytes from bitbuf
        while (bitcnt >= 8) {
            uint8_t byte = (bitbuf >> (bitcnt - 8)) & 0xFF;
            bitcnt -= 8;
            bool h = false;
            sc_uint<32> tmp = 0;
            put_byte(byte, h, tmp);
            if (h) {
                have = true;
                w = tmp;
                return;
            }
        }

        if (force && bitcnt > 0) {
            // Pad remaining bits to a full byte (left-aligned)
            uint8_t byte = (bitbuf << (8 - bitcnt)) & 0xFF;
            bitcnt = 0;
            bool h = false;
            sc_uint<32> tmp = 0;
            put_byte(byte, h, tmp);
            if (h) {
                have = true;
                w = tmp;
                return;
            }
        }

        if (force && word_bytes > 0) {
            // Pad remaining bytes in wordbuf with zeros
            while (word_bytes < 4) {
                wordbuf = (wordbuf << 8);
                word_bytes++;
            }
            w = wordbuf;
            wordbuf = 0;
            word_bytes = 0;
            have = true;
            return;
        }
    };

    wait();

    while (true) {
        if (!rst_n.read()) {
            in_ready.write(true);
            out_valid.write(false);
            out_data.write(0);
            prev_dc_y = prev_dc_cb = prev_dc_cr = 0;
            bitbuf = 0;
            bitcnt = 0;
            wordbuf = 0;
            word_bytes = 0;
        } else {
            bool can_accept = (!out_valid.read() || out_ready.read());
            in_ready.write(can_accept);

            if (out_valid.read() && out_ready.read())
                out_valid.write(false);

            bool have = false;
            sc_uint<32> w = 0;

            if (in_valid.read() && can_accept) {
                Block b = in_block.read();
                BlockType comp = b.type;

                int zz[64];
                for (int i = 0; i < 64; ++i)
                    zz[i] = b.data[ZIGZAG[i]].to_int();

                int prev_dc =
                    (comp == BLOCK_Y)  ? prev_dc_y :
                    (comp == BLOCK_CB) ? prev_dc_cb : prev_dc_cr;

                int dc = zz[0];
                int diff = dc - prev_dc;
                int cat = category(diff);
                uint16_t amp = amplitude_bits(diff, cat);

                const HuffCode* dct = dc_table(comp);
                HuffCode hdc = dct[cat];
                put_bits(hdc.code, hdc.size, have, w);
                if (!have && cat)
                    put_bits(amp, cat, have, w);

                if (comp == BLOCK_Y) prev_dc_y = dc;
                else if (comp == BLOCK_CB) prev_dc_cb = dc;
                else prev_dc_cr = dc;

                int run = 0;
                const HuffCode* act = ac_table(comp);

                for (int i = 1; i < 64; ++i) {
                    int v = zz[i];
                    if (v == 0) {
                        run++;
                    } else {
                        while (run >= 16) {
                            HuffCode zrl = ac_zrl(comp);
                            put_bits(zrl.code, zrl.size, have, w);
                            if (have) break;
                            run -= 16;
                        }
                        if (have) break;

                        int ac_cat = category(v);
                        uint16_t ac_amp = amplitude_bits(v, ac_cat);

                        int idx = run * 16 + ac_cat;
                        HuffCode hac = act[idx];

                        put_bits(hac.code, hac.size, have, w);
                        if (!have && ac_cat)
                            put_bits(ac_amp, ac_cat, have, w);

                        run = 0;
                        if (have) break;
                    }
                }

                if (!have && run > 0) {
                    HuffCode eob = ac_eob(comp);
                    put_bits(eob.code, eob.size, have, w);
                }

                if (restart_mcu_boundary.read()) {
                    prev_dc_y = prev_dc_cb = prev_dc_cr = 0;
                }

                if (!have)
                    flush_word(false, have, w);

                if (have) {
                    out_data.write(w);
                    out_valid.write(true);
                }
            } else {
                if (!out_valid.read()) {
                    flush_word(false, have, w);
                    if (have) {
                        out_data.write(w);
                        out_valid.write(true);
                    }
                }
            }
        }
        wait();
    }
}
