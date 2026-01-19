#include "BlockMaker.h"
#include <iostream>

void BlockMaker::run() {
    in_ready.write(false);
    out_valid.write(false);
    out_type.write(BLOCK_Y);
    restart_mcu_boundary.write(false);

    for (int i = 0; i < 64; ++i)
        out_block[i].write(0);

    wait();

    sc_uint<16> width  = 0;
    sc_uint<16> height = 0;

    sc_uint<16> x = 0;
    sc_uint<16> y = 0;

    sc_int<16> blockY[64];
    sc_int<16> blockCb[64];
    sc_int<16> blockCr[64];

    int  fill      = 0;
    bool y_ready   = false;
    bool cb_ready  = false;
    bool cr_ready  = false;

    enum Phase { P_Y, P_CB, P_CR };
    Phase phase = P_Y;

    while (true) {
        width  = cfg_width.read();
        height = cfg_height.read();

        bool blocks_full = (y_ready && cb_ready && cr_ready);
        bool can_accept  = !blocks_full;
        in_ready.write(can_accept);

        // ----------------------------------------------------------
        // PIXEL INGEST → BUILD 8×8 BLOCKS FOR Y, Cb, Cr
        // ----------------------------------------------------------
        if (in_valid.read() && can_accept) {
            pixel_t p = in_pixel.read();

            // RGB → YCbCr (BT.601)
            sc_int<16> Y  = (  77*p.r + 150*p.g +  29*p.b) >> 8;
            sc_int<16> Cb = ((-43*p.r -  85*p.g + 128*p.b) >> 8) + 128;
            sc_int<16> Cr = ((128*p.r - 107*p.g -  21*p.b) >> 8) + 128;

            if (Y  < 0)   Y  = 0;   if (Y  > 255) Y  = 255;
            if (Cb < 0)   Cb = 0;   if (Cb > 255) Cb = 255;
            if (Cr < 0)   Cr = 0;   if (Cr > 255) Cr = 255;

            blockY [fill] = Y;
            blockCb[fill] = Cb - 128;
            blockCr[fill] = Cr - 128;

            fill++;

            x++;
            if (x == width) {
                x = 0;
                y++;
                if (y == height)
                    y = 0;
            }

            if (fill == 64) {
                y_ready  = true;
                cb_ready = true;
                cr_ready = true;
                fill     = 0;
            }
        }

        // ----------------------------------------------------------
        // OUTPUT BLOCKS IN MCU ORDER: Y → Cb → Cr
        // ----------------------------------------------------------
        out_valid.write(false);

        if ((y_ready || cb_ready || cr_ready) && out_ready.read()) {

            if (phase == P_Y && y_ready) {
                for (int i = 0; i < 64; ++i)
                    out_block[i].write(blockY[i]);
                out_type.write(BLOCK_Y);
                out_valid.write(true);

                std::cout << sc_time_stamp() << " [BM] out_type=Y\n";

                y_ready = false;
                phase   = P_CB;
            }
            else if (phase == P_CB && cb_ready) {
                for (int i = 0; i < 64; ++i)
                    out_block[i].write(blockCb[i]);
                out_type.write(BLOCK_CB);
                out_valid.write(true);

                std::cout << sc_time_stamp() << " [BM] out_type=Cb\n";

                cb_ready = false;
                phase    = P_CR;
            }
            else if (phase == P_CR && cr_ready) {
                for (int i = 0; i < 64; ++i)
                    out_block[i].write(blockCr[i]);
                out_type.write(BLOCK_CR);
                out_valid.write(true);

                std::cout << sc_time_stamp() << " [BM] out_type=Cr\n";

                cr_ready = false;
                phase    = P_Y;
            }
        }

        restart_mcu_boundary.write(false);
        wait();
    }
}
