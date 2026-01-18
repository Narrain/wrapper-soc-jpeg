#include "isp.h"
using namespace sc_core;
using namespace sc_dt;

ISP::ISP(sc_module_name name) : sc_module(name)
{
    SC_CTHREAD(input_formatter, clk.pos());
    async_reset_signal_is(rst_n, false);

    SC_CTHREAD(demosaic_stage, clk.pos());
    async_reset_signal_is(rst_n, false);

    SC_CTHREAD(color_stage, clk.pos());
    async_reset_signal_is(rst_n, false);

    SC_CTHREAD(noise_stage, clk.pos());
    async_reset_signal_is(rst_n, false);

    SC_CTHREAD(output_formatter, clk.pos());
    async_reset_signal_is(rst_n, false);
}

// Input Formatter
void ISP::input_formatter()
{
    in_ready.write(false);
    s_ifm2dem_valid.write(false);
    wait();

    while (true) {
        bool in_v  = in_valid.read();
        bool nxt_r = s_ifm2dem_ready.read();

        if (in_v && nxt_r) {
            s_ifm2dem_pix.write(in_pixel.read());
            s_ifm2dem_valid.write(true);
            in_ready.write(true);
        } else {
            s_ifm2dem_valid.write(false);
            in_ready.write(nxt_r);
        }

        wait();
    }
}

// Demosaic stage (placeholder: pass-through)
void ISP::demosaic_stage()
{
    s_dem2col_valid.write(false);
    s_ifm2dem_ready.write(false);
    wait();

    while (true) {
        bool in_v  = s_ifm2dem_valid.read();
        bool out_r = s_dem2col_ready.read();

        if (in_v && out_r) {
            pixel_t in_p = s_ifm2dem_pix.read();
            pixel_t out_p = in_p;

            s_dem2col_pix.write(out_p);
            s_dem2col_valid.write(true);
            s_ifm2dem_ready.write(true);
        } else {
            s_dem2col_valid.write(false);
            s_ifm2dem_ready.write(out_r);
        }

        wait();
    }
}

// Color / Gamma stage (placeholder)
void ISP::color_stage()
{
    s_col2nr_valid.write(false);
    s_dem2col_ready.write(false);
    wait();

    while (true) {
        bool in_v  = s_dem2col_valid.read();
        bool out_r = s_col2nr_ready.read();

        if (in_v && out_r) {
            pixel_t in_p = s_dem2col_pix.read();
            pixel_t out_p;

            out_p.r = in_p.r;
            out_p.g = in_p.g;
            out_p.b = in_p.b;

            s_col2nr_pix.write(out_p);
            s_col2nr_valid.write(true);
            s_dem2col_ready.write(true);
        } else {
            s_col2nr_valid.write(false);
            s_dem2col_ready.write(out_r);
        }

        wait();
    }
}

// Noise / WDR stage (placeholder)
void ISP::noise_stage()
{
    s_nr2ofm_valid.write(false);
    s_col2nr_ready.write(false);
    wait();

    while (true) {
        bool in_v  = s_col2nr_valid.read();
        bool out_r = s_nr2ofm_ready.read();

        if (in_v && out_r) {
            pixel_t in_p = s_col2nr_pix.read();
            pixel_t out_p = in_p;

            s_nr2ofm_pix.write(out_p);
            s_nr2ofm_valid.write(true);
            s_col2nr_ready.write(true);
        } else {
            s_nr2ofm_valid.write(false);
            s_col2nr_ready.write(out_r);
        }

        wait();
    }
}

// Output Formatter
void ISP::output_formatter()
{
    out_valid.write(false);
    s_nr2ofm_ready.write(false);
    wait();

    while (true) {
        bool in_v  = s_nr2ofm_valid.read();
        bool out_r = out_ready.read();

        if (in_v && out_r) {
            out_pixel.write(s_nr2ofm_pix.read());
            out_valid.write(true);
            s_nr2ofm_ready.write(true);
        } else {
            out_valid.write(false);
            s_nr2ofm_ready.write(out_r);
        }

        wait();
    }
}
