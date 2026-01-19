// JPEG_Encoder.cpp
#include "JPEG_Encoder.h"

using namespace sc_core;
using namespace sc_dt;

#if 0
void JPEG_Encoder::run() {
    in_ready.write(false);
    dct_in_valid.write(false);
    dct_out_ready.write(false);
    ent_in_valid.write(false);
    bs_valid.write(false);
    irq_done.write(false);
    irq_status.write(0);
    s_hdr_ready.write(false);
    state.write(S_IDLE);

    bool in_frame = false;
    bool eoi_sent = false;

    wait();

    while (true) {
        if (!rst_n.read()) {
            in_ready.write(false);
            dct_in_valid.write(false);
            dct_out_ready.write(false);
            ent_in_valid.write(false);
            bs_valid.write(false);
            irq_done.write(false);
            irq_status.write(0);
            s_hdr_ready.write(false);
            state.write(S_IDLE);
            in_frame = false;
            eoi_sent = false;
        } else {
            if (frame_start.read())
                in_frame = true;
            if (frame_end.read())
                in_frame = false;

            State st = state.read();

            // Default
            bs_valid.write(false);
            s_hdr_ready.write(false);
            dct_out_ready.write(ent_in_ready.read());

            switch (st) {
            case S_IDLE:
                if (frame_start.read()) {
                    state.write(S_HEADER);
                }
                break;

            case S_HEADER:
                if (s_hdr_valid.read() && bs_ready.read()) {
                    bs_data.write(s_hdr_data.read());
                    bs_valid.write(true);
                    s_hdr_ready.write(true);
                }
                if (!s_hdr_valid.read()) {
                    state.write(S_SCAN);
                }
                break;

            case S_SCAN: {
                bool can_take_block = in_frame && dct_in_ready.read();
                in_ready.write(can_take_block);

                if (in_valid.read() && can_take_block) {
                    Block b;
                    for (int i = 0; i < 64; ++i)
                        b.data[i] = in_block[i].read();
                    b.type = static_cast<BlockType>(in_type.read().to_uint());
                    dct_in_block.write(b);
                    dct_in_valid.write(true);
                } else {
                    dct_in_valid.write(false);
                }

                if (dct_out_valid.read() && ent_in_ready.read()) {
                    ent_in_block.write(dct_out_block.read());
                    ent_in_valid.write(true);
                } else {
                    ent_in_valid.write(false);
                }

                if (ent_out_valid.read() && bs_ready.read()) {
                    bs_data.write(ent_out_data.read());
                    bs_valid.write(true);
                }

                if (!in_frame && !dct_out_valid.read() && !ent_out_valid.read()) {
                    state.write(S_EOI);
                }
                break;
            }

            case S_EOI:
                if (!eoi_sent && bs_ready.read()) {
                    sc_uint<32> w = 0;
                    w = (0xFFu << 24) | (0xD9u << 16); // EOI in top bytes
                    bs_data.write(w);
                    bs_valid.write(true);
                    eoi_sent = true;
                } else if (eoi_sent) {
                    state.write(S_DONE);
                }
                break;

            case S_DONE:
                irq_done.write(true);
                irq_status.write(IRQ_FRAME_DONE);
                break;
            }
        }
        wait();
    }
}
#endif

void JPEG_Encoder::run() {
    in_ready.write(false);
    dct_in_valid.write(false);
    dct_out_ready.write(false);
    ent_in_valid.write(false);
    bs_valid.write(false);
    irq_done.write(false);
    irq_status.write(0);
    s_hdr_ready.write(false);
    state.write(S_IDLE);

    bool frame_done = false;
    bool eoi_sent = false;
    bool header_started = false;

    wait();

    static bool dbg_printed = false;
    while (true) {
        if (!rst_n.read()) {
            in_ready.write(false);
            dct_in_valid.write(false);
            dct_out_ready.write(false);
            ent_in_valid.write(false);
            bs_valid.write(false);
            irq_done.write(false);
            irq_status.write(0);
            s_hdr_ready.write(false);
            state.write(S_IDLE);
            frame_done = false;
            eoi_sent = false;
            header_started = false;
        } else {
            // Track frame boundaries
            if (frame_start.read())
                frame_done = false;
            if (frame_end.read())
                frame_done = true;

            State st = state.read();

            // defaults
            bs_valid.write(false);
            s_hdr_ready.write(false);
            dct_out_ready.write(ent_in_ready.read());

            switch (st) {

            case S_IDLE:
                if (frame_start.read()) {
                    header_started = false;
                    state.write(S_HEADER);
                }
                break;

            case S_HEADER:
                s_hdr_ready.write(true);

                if (s_hdr_valid.read()) {
                    bs_data.write(s_hdr_data.read());
                    bs_valid.write(true);
                    header_started = true;
                }

                if (header_started && !s_hdr_valid.read()) {
                    s_hdr_ready.write(false);
                    state.write(S_SCAN);
                }
                break;

            case S_SCAN: {
                // FIX #1 — do NOT gate on in_frame
                bool can_take_block = dct_in_ready.read();
                in_ready.write(can_take_block);

                if (in_valid.read() && can_take_block) {
                    Block b;
                    for (int i = 0; i < 64; ++i)
                        b.data[i] = in_block[i].read();
                    b.type = static_cast<BlockType>(in_type.read().to_uint());
                    dct_in_block.write(b);
                    dct_in_valid.write(true);
                } else {
                    dct_in_valid.write(false);
                }

                if (dct_out_valid.read() && ent_in_ready.read()) {
                    ent_in_block.write(dct_out_block.read());
                    ent_in_valid.write(true);
                } else {
                    ent_in_valid.write(false);
                }

                if (ent_out_valid.read() && bs_ready.read()) {
                    bs_data.write(ent_out_data.read());
                    bs_valid.write(true);
                }

                // FIX #2 — correct EOI condition
                if (frame_done &&
                    !in_valid.read() &&
                    !dct_out_valid.read() &&
                    !ent_out_valid.read()) {
                    state.write(S_EOI);
                }
                break;
            }

            case S_EOI:
                if (!dbg_printed) {
                    std::cout << sc_time_stamp() << " [ENC] Enter S_EOI\n";
                    dbg_printed = true;
                }
                if (!eoi_sent && bs_ready.read()) {
                    sc_uint<32> w = 0;
                    w |= 0xFF;
                    w |= 0xD9 << 8;
                    bs_data.write(w);
                    bs_valid.write(true);
                    eoi_sent = true;
                } else if (eoi_sent) {
                    state.write(S_DONE);
                }
                break;

            case S_DONE:
                irq_done.write(true);
                irq_status.write(IRQ_FRAME_DONE);
                break;
            }
        }
        wait();
    }
}
