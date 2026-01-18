#include <systemc>
#include "jpeg_encoder.h"

SC_MODULE(TB) {
    sc_signal<bool> clk, rst_n;
    sc_signal<pixel_t> in_pixel;
    sc_signal<bool> in_valid, in_ready;
    sc_signal<sc_uint<32>> bs_data;
    sc_signal<bool> bs_valid, bs_ready;
    sc_signal<bool> frame_start, frame_end;
    sc_signal<bool> irq_done;

    JPEG_Encoder* enc;

    void clock_gen() {
        while (true) {
            clk.write(false); wait(5, sc_core::SC_NS);
            clk.write(true);  wait(5, sc_core::SC_NS);
        }
    }

    void stimulus() {
        rst_n.write(false);
        bs_ready.write(true);
        frame_start.write(false);
        frame_end.write(false);
        in_valid.write(false);
        wait(20, sc_core::SC_NS);
        rst_n.write(true);

        frame_start.write(true);
        wait(clk.posedge_event());
        frame_start.write(false);

        // send a small 4x4 test frame
        for (int i = 0; i < 16; i++) {
            pixel_t p;
            p.r = i;
            p.g = 255 - i;
            p.b = i * 10;
            in_pixel.write(p);
            in_valid.write(true);
            wait(clk.posedge_event());
        }

        in_valid.write(false);
        frame_end.write(true);
        wait(clk.posedge_event());
        frame_end.write(false);

        wait(500, sc_core::SC_NS);
        sc_stop();
    }

    SC_CTOR(TB) {
        enc = new JPEG_Encoder("enc");
        enc->clk(clk);
        enc->rst_n(rst_n);
        enc->in_pixel(in_pixel);
        enc->in_valid(in_valid);
        enc->in_ready(in_ready);
        enc->bs_data(bs_data);
        enc->bs_valid(bs_valid);
        enc->bs_ready(bs_ready);
        enc->frame_start(frame_start);
        enc->frame_end(frame_end);
        enc->irq_done(irq_done);

        enc->width = 4;
        enc->height = 4;

        SC_THREAD(clock_gen);
        SC_THREAD(stimulus);
    }
};

// #include <systemc>
// #include <fstream>
// #include "encoder.h"

// SC_MODULE(TB_Encoder) {
//     sc_core::sc_signal<bool> clk{"clk"};
//     sc_core::sc_signal<bool> rst_n{"rst_n"};

//     sc_core::sc_signal<pixel_t> in_pixel{"in_pixel"};
//     sc_core::sc_signal<bool>    in_valid{"in_valid"};
//     sc_core::sc_signal<bool>    in_ready{"in_ready"};

//     sc_core::sc_signal<sc_dt::sc_uint<32>> bs_data{"bs_data"};
//     sc_core::sc_signal<bool>               bs_valid{"bs_valid"};
//     sc_core::sc_signal<bool>               bs_ready{"bs_ready"};

//     sc_core::sc_signal<bool> irq_out{"irq_out"};

//     Encoder* enc;
//     std::ofstream fout;

//     void clock_gen() {
//         while (true) {
//             clk.write(false);
//             wait(5, sc_core::SC_NS);
//             clk.write(true);
//             wait(5, sc_core::SC_NS);
//         }
//     }

//     void stimulus() {
//         rst_n.write(false);
//         in_valid.write(false);
//         bs_ready.write(true);
//         wait(20, sc_core::SC_NS);
//         rst_n.write(true);

//         for (int i = 0; i < 5; i++) {
//             pixel_t p;
//             p.r = i;
//             p.g = i + 1;
//             p.b = i + 2;

//             in_pixel.write(p);
//             in_valid.write(true);

//             do { wait(clk.posedge_event()); }
//             while (!in_ready.read());

//             in_valid.write(false);
//             wait(clk.posedge_event());
//         }

//         wait(200, sc_core::SC_NS);
//         sc_core::sc_stop();
//     }

//     void monitor() {
//         while (true) {
//             wait(clk.posedge_event());

//             if (bs_valid.read() && bs_ready.read()) {
//                 sc_dt::sc_uint<32> w = bs_data.read();
//                 std::cout << sc_core::sc_time_stamp()
//                           << "  BS_WORD: 0x"
//                           << std::hex << w.to_uint()
//                           << std::dec << std::endl;

//                 fout << sc_core::sc_time_stamp()
//                      << "  0x" << std::hex << w.to_uint()
//                      << std::dec << "\n";
//             }
//         }
//     }

//     SC_CTOR(TB_Encoder) : fout("encoder_out.txt") {
//         enc = new Encoder("encoder");
//         enc->clk(clk);
//         enc->rst_n(rst_n);
//         enc->in_pixel(in_pixel);
//         enc->in_valid(in_valid);
//         enc->in_ready(in_ready);
//         enc->bs_data(bs_data);
//         enc->bs_valid(bs_valid);
//         enc->bs_ready(bs_ready);
//         enc->irq_out(irq_out);

//         SC_THREAD(clock_gen);
//         SC_THREAD(stimulus);
//         SC_THREAD(monitor);

//         sc_core::sc_trace_file* tf =
//             sc_core::sc_create_vcd_trace_file("encoder_wave");

//         sc_trace(tf, clk, "clk");
//         sc_trace(tf, rst_n, "rst_n");
//         sc_trace(tf, in_valid, "in_valid");
//         sc_trace(tf, in_ready, "in_ready");
//         sc_trace(tf, in_pixel, "in_pixel");
//         sc_trace(tf, bs_valid, "bs_valid");
//         sc_trace(tf, bs_ready, "bs_ready");
//         sc_trace(tf, bs_data, "bs_data");
//         sc_trace(tf, irq_out, "irq_out");
//     }
// };
