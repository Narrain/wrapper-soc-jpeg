#include "HeaderGen.h"

using namespace sc_core;
using namespace sc_dt;

// Helper: write a byte into a 32-bit packer
static inline void put_byte(uint8_t b,
                            sc_uint<32> &buf,
                            int &count,
                            bool &have_word,
                            sc_uint<32> &word)
{
    buf = (buf << 8) | b;
    count += 8;

    have_word = false;
    if (count >= 32) {
        word = buf >> (count - 32);
        count -= 32;
        have_word = true;
    }
}


#if 0
void HeaderGen::run() {
    out_valid.write(false);
    out_data.write(0);

    sc_uint<32> buf = 0;
    int count = 0;

    wait();

    while (true) {
        if (!rst_n.read()) {
            out_valid.write(false);
            out_data.write(0);
            buf = 0;
            count = 0;
        } else {
            if (start.read()) {
                bool have = false;
                sc_uint<32> w = 0;

                auto emit = [&](uint8_t b) {
                    put_byte(b, buf, count, have, w);
                    if (have) {
                        while (!out_ready.read()) wait();
                        out_data.write(w);
                        out_valid.write(true);
                        wait();
                        out_valid.write(false);
                    }
                };

                // ------------------------------------------------------------
                // SOI
                // ------------------------------------------------------------
                emit(0xFF); emit(0xD8);

                // ------------------------------------------------------------
                // APP0 (JFIF)
                // ------------------------------------------------------------
                emit(0xFF); emit(0xE0);
                emit(0x00); emit(0x10);     // length
                emit(0x4A); emit(0x46); emit(0x49); emit(0x46); emit(0x00);
                emit(0x01); emit(0x02);     // version
                emit(0x00);                 // density units
                emit(0x00); emit(0x01);     // X density
                emit(0x00); emit(0x01);     // Y density
                emit(0x00); emit(0x00);     // thumbnail

                // ------------------------------------------------------------
                // DQT (luma + chroma)
                // ------------------------------------------------------------
                emit(0xFF); emit(0xDB);
                emit(0x00); emit(0x43);
                emit(0x00);                 // table 0 (luma)
                static const uint8_t QL[64] = {
                    16,11,10,16,24,40,51,61,
                    12,12,14,19,26,58,60,55,
                    14,13,16,24,40,57,69,56,
                    14,17,22,29,51,87,80,62,
                    18,22,37,56,68,109,103,77,
                    24,35,55,64,81,104,113,92,
                    49,64,78,87,103,121,120,101,
                    72,92,95,98,112,100,103,99
                };
                for (int i = 0; i < 64; ++i) emit(QL[i]);

                emit(0xFF); emit(0xDB);
                emit(0x00); emit(0x43);
                emit(0x01);                 // table 1 (chroma)
                static const uint8_t QC[64] = {
                    17,18,24,47,99,99,99,99,
                    18,21,26,66,99,99,99,99,
                    24,26,56,99,99,99,99,99,
                    47,66,99,99,99,99,99,99,
                    99,99,99,99,99,99,99,99,
                    99,99,99,99,99,99,99,99,
                    99,99,99,99,99,99,99,99,
                    99,99,99,99,99,99,99,99
                };
                for (int i = 0; i < 64; ++i) emit(QC[i]);

                // ------------------------------------------------------------
                // SOF0 (baseline)
                // ------------------------------------------------------------
                emit(0xFF); emit(0xC0);
                emit(0x00); emit(0x11);     // length
                emit(0x08);                 // precision
                emit((uint8_t)(height.read() >> 8));
                emit((uint8_t)(height.read() & 0xFF));
                emit((uint8_t)(width.read() >> 8));
                emit((uint8_t)(width.read() & 0xFF));
                emit(0x03);                 // 3 components

                // Y
                emit(0x01); emit(0x22); emit(0x00);
                // Cb
                emit(0x02); emit(0x11); emit(0x01);
                // Cr
                emit(0x03); emit(0x11); emit(0x01);

                // ------------------------------------------------------------
                // DHT (baseline tables)
                // ------------------------------------------------------------
                // Luma DC
                emit(0xFF); emit(0xC4);
                emit(0x00); emit(0x1F);
                emit(0x00);
                static const uint8_t DC_L_bits[16] =
                    {0,1,5,1,1,1,1,1,1,0,0,0,0,0,0,0};
                static const uint8_t DC_L_val[12] =
                    {0,1,2,3,4,5,6,7,8,9,10,11};
                for (int i = 0; i < 16; ++i) emit(DC_L_bits[i]);
                for (int i = 0; i < 12; ++i) emit(DC_L_val[i]);

                // Luma AC
                emit(0xFF); emit(0xC4);
                emit(0x00); emit(0xB5);
                emit(0x10);
                static const uint8_t AC_L_bits[16] =
                    {0,2,1,3,3,2,4,3,5,5,4,4,0,0,1,0x7d};
                static const uint8_t AC_L_val[162] = {
                    // Full standard AC table (omitted here for brevity)
                    // You will paste the full 162-byte table here.
                };
                for (int i = 0; i < 16; ++i) emit(AC_L_bits[i]);
                for (int i = 0; i < 162; ++i) emit(AC_L_val[i]);

                // Chroma DC
                emit(0xFF); emit(0xC4);
                emit(0x00); emit(0x1F);
                emit(0x01);
                static const uint8_t DC_C_bits[16] =
                    {0,3,1,1,1,1,1,1,1,1,0,0,0,0,0,0};
                static const uint8_t DC_C_val[12] =
                    {0,1,2,3,4,5,6,7,8,9,10,11};
                for (int i = 0; i < 16; ++i) emit(DC_C_bits[i]);
                for (int i = 0; i < 12; ++i) emit(DC_C_val[i]);

                // Chroma AC
                emit(0xFF); emit(0xC4);
                emit(0x00); emit(0xB5);
                emit(0x11);
                static const uint8_t AC_C_bits[16] =
                    {0,2,1,2,4,4,3,4,7,5,4,4,0,1,2,0x77};
                static const uint8_t AC_C_val[162] = {
                    // Full standard AC chroma table (paste here)
                };
                for (int i = 0; i < 16; ++i) emit(AC_C_bits[i]);
                for (int i = 0; i < 162; ++i) emit(AC_C_val[i]);

                // ------------------------------------------------------------
                // SOS
                // ------------------------------------------------------------
                emit(0xFF); emit(0xDA);
                emit(0x00); emit(0x0C);
                emit(0x03);
                emit(0x01); emit(0x00);
                emit(0x02); emit(0x11);
                emit(0x03); emit(0x11);
                emit(0x00); emit(0x3F); emit(0x00);

                // ------------------------------------------------------------
                // Flush remaining header bits
                // ------------------------------------------------------------
                {
                    bool have = false;
                    sc_uint<32> w = 0;
                    if (count > 0) {
                        w = buf << (32 - count);
                        count = 0;
                        have = true;
                    }
                    if (have) {
                        while (!out_ready.read()) wait();
                        out_data.write(w);
                        out_valid.write(true);
                        wait();
                        out_valid.write(false);
                    }
                }

            }
        }
        wait();
    }
}
#endif
#if 0
void HeaderGen::run() {
    out_valid.write(false);
    wait();

    while (true) {
        if (!rst_n.read()) {
            out_valid.write(false);
        } else if (start.read()) {
            unsigned idx = 0;
            while (idx < sizeof(header_bytes)) {
                if (out_ready.read()) {
                    sc_uint<32> w = 0;
                    uint8_t b0 = (idx + 0 < sizeof(header_bytes)) ? header_bytes[idx + 0] : 0;
                    uint8_t b1 = (idx + 1 < sizeof(header_bytes)) ? header_bytes[idx + 1] : 0;
                    uint8_t b2 = (idx + 2 < sizeof(header_bytes)) ? header_bytes[idx + 2] : 0;
                    uint8_t b3 = (idx + 3 < sizeof(header_bytes)) ? header_bytes[idx + 3] : 0;

                    w |= (sc_uint<32>)b0;
                    w |= (sc_uint<32>)b1 << 8;
                    w |= (sc_uint<32>)b2 << 16;
                    w |= (sc_uint<32>)b3 << 24;

                    out_data.write(w);
                    out_valid.write(true);

                    idx += 4;
                }
                wait();
            }
            out_valid.write(false);
        }
        wait();
    }
}
#endif

void HeaderGen::run() {
    out_valid.write(false);
    wait();

    while (true) {
        if (!rst_n.read()) {
            out_valid.write(false);
        } else if (start.read()) {

            // ------------------------------------------------------
            // PATCH WIDTH + HEIGHT INTO SOF0
            // ------------------------------------------------------
            uint16_t h = height.read();
            uint16_t w = width.read();

            // Your SOF0 begins at index 0x8A
            // Layout:
            // 0x8A FF
            // 0x8B C0
            // 0x8C 00
            // 0x8D 11
            // 0x8E 08
            // 0x8F height high
            // 0x90 height low
            // 0x91 width high
            // 0x92 width low

            header_bytes[0x8F] = (h >> 8) & 0xFF;
            header_bytes[0x90] = (h >> 0) & 0xFF;

            header_bytes[0x91] = (w >> 8) & 0xFF;
            header_bytes[0x92] = (w >> 0) & 0xFF;

            // ------------------------------------------------------
            // STREAM HEADER (32‑bit words)
            // ------------------------------------------------------
            unsigned idx = 0;
            while (idx < sizeof(header_bytes)) {
                if (out_ready.read()) {
                    sc_uint<32> w32 = 0;

                    uint8_t b0 = (idx + 0 < sizeof(header_bytes)) ? header_bytes[idx + 0] : 0;
                    uint8_t b1 = (idx + 1 < sizeof(header_bytes)) ? header_bytes[idx + 1] : 0;
                    uint8_t b2 = (idx + 2 < sizeof(header_bytes)) ? header_bytes[idx + 2] : 0;
                    uint8_t b3 = (idx + 3 < sizeof(header_bytes)) ? header_bytes[idx + 3] : 0;

                    w32 |= (sc_uint<32>)b0;
                    w32 |= (sc_uint<32>)b1 << 8;
                    w32 |= (sc_uint<32>)b2 << 16;
                    w32 |= (sc_uint<32>)b3 << 24;

                    out_data.write(w32);
                    out_valid.write(true);

                    idx += 4;
                }
                wait();
            }

            out_valid.write(false);
        }
        wait();
    }
}
