#include "encoder.h"
using namespace sc_core;
using namespace sc_dt;

Encoder::Encoder(sc_module_name name) : sc_module(name)
{
    SC_CTHREAD(ingest_fifo, clk.pos());
    async_reset_signal_is(rst_n, false);

    SC_CTHREAD(block_maker, clk.pos());
    async_reset_signal_is(rst_n, false);

    SC_CTHREAD(transform_quant, clk.pos());
    async_reset_signal_is(rst_n, false);

    SC_CTHREAD(entropy_coder, clk.pos());
    async_reset_signal_is(rst_n, false);

    SC_CTHREAD(bitstream_fifo, clk.pos());
    async_reset_signal_is(rst_n, false);
}

// Ingest FIFO: ready/valid pass-through for now
void Encoder::ingest_fifo()
{
    in_ready.write(false);
    s_ing2blk_valid.write(false);
    wait();

    while (true) {
        bool v = in_valid.read();
        bool r = s_ing2blk_ready.read();

        if (v && r) {
            s_ing2blk_pix.write(in_pixel.read());
            s_ing2blk_valid.write(true);
            in_ready.write(true);
        } else {
            s_ing2blk_valid.write(false);
            in_ready.write(r);
        }

        wait();
    }
}

// Block maker: placeholder (1 pixel → 1 "block")
void Encoder::block_maker()
{
    s_blk2dct_valid.write(false);
    s_ing2blk_ready.write(false);
    wait();

    while (true) {
        bool v = s_ing2blk_valid.read();
        bool r = s_blk2dct_ready.read();

        if (v && r) {
            pixel_t p = s_ing2blk_pix.read();
            s_blk2dct_data.write(p);
            s_blk2dct_valid.write(true);
            s_ing2blk_ready.write(true);
        } else {
            s_blk2dct_valid.write(false);
            s_ing2blk_ready.write(r);
        }

        wait();
    }
}

// Transform / Quant: placeholder (pixel → symbol)
void Encoder::transform_quant()
{
    s_dct2ent_valid.write(false);
    s_blk2dct_ready.write(false);
    wait();

    while (true) {
        bool v = s_blk2dct_valid.read();
        bool r = s_dct2ent_ready.read();

        if (v && r) {
            pixel_t p = s_blk2dct_data.read();
            sc_uint<16> sym = (p.r, p.g.range(7,0)); // toy symbol
            s_dct2ent_sym.write(sym);
            s_dct2ent_valid.write(true);
            s_blk2dct_ready.write(true);
        } else {
            s_dct2ent_valid.write(false);
            s_blk2dct_ready.write(r);
        }

        wait();
    }
}

// Entropy coder: placeholder (symbol → 32-bit word)
void Encoder::entropy_coder()
{
    s_ent2bs_valid.write(false);
    s_dct2ent_ready.write(false);
    wait();

    while (true) {
        bool v = s_dct2ent_valid.read();
        bool r = s_ent2bs_ready.read();

        if (v && r) {
            sc_uint<16> sym = s_dct2ent_sym.read();
            sc_uint<32> word = 0;
            word.range(15,0) = sym;
            s_ent2bs_word.write(word);
            s_ent2bs_valid.write(true);
            s_dct2ent_ready.write(true);
        } else {
            s_ent2bs_valid.write(false);
            s_dct2ent_ready.write(r);
        }

        wait();
    }
}

// Bitstream FIFO: placeholder, direct mapping to bs_* ports
void Encoder::bitstream_fifo()
{
    bs_valid.write(false);
    s_ent2bs_ready.write(false);
    irq_out.write(false);
    wait();

    while (true) {
        bool v = s_ent2bs_valid.read();
        bool r = bs_ready.read();

        if (v && r) {
            bs_data.write(s_ent2bs_word.read());
            bs_valid.write(true);
            s_ent2bs_ready.write(true);
        } else {
            bs_valid.write(false);
            s_ent2bs_ready.write(r);
        }

        // TODO: frame_done / buffer_full / error → irq_out
        irq_out.write(false);

        wait();
    }
}
