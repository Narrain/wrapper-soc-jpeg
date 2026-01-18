#include <systemc>
#include <fstream>
#include <deque>
#include "isp.h"

SC_MODULE(TB_ISP) {
    sc_core::sc_signal<bool> clk{"clk"};
    sc_core::sc_signal<bool> rst_n{"rst_n"};

    sc_core::sc_signal<pixel_t> in_pixel{"in_pixel"};
    sc_core::sc_signal<bool>    in_valid{"in_valid"};
    sc_core::sc_signal<bool>    in_ready{"in_ready"};

    sc_core::sc_signal<pixel_t> out_pixel{"out_pixel"};
    sc_core::sc_signal<bool>    out_valid{"out_valid"};
    sc_core::sc_signal<bool>    out_ready{"out_ready"};

    ISP* isp;

    std::ofstream fout;

    // Scoreboard queue
    std::deque<pixel_t> expected_fifo;

    // Latency timestamps
    std::deque<sc_core::sc_time> timestamp_fifo;

    // Skip first output (warm-up)
    bool first_output_seen = false;

    // -------------------------
    // Clock generator
    // -------------------------
    void clock_gen() {
        while (true) {
            clk.write(false);
            wait(5, sc_core::SC_NS);
            clk.write(true);
            wait(5, sc_core::SC_NS);
        }
    }

    // -------------------------
    // Stimulus driver
    // -------------------------
    void stimulus() {
        rst_n.write(false);
        in_valid.write(false);
        out_ready.write(true);
        wait(20, sc_core::SC_NS);
        rst_n.write(true);

        // Send 5 pixels
        for (int i = 0; i < 5; i++) {
            pixel_t p;
            p.r = i;
            p.g = i + 1;
            p.b = i + 2;

            in_pixel.write(p);
            in_valid.write(true);

            // Wait for handshake
            do { wait(clk.posedge_event()); }
            while (!in_ready.read());

            // Scoreboard push
            expected_fifo.push_back(p);

            // Timestamp push
            timestamp_fifo.push_back(sc_core::sc_time_stamp());

            in_valid.write(false);
            wait(clk.posedge_event());
        }

        wait(200, sc_core::SC_NS);
        sc_core::sc_stop();
    }

    // -------------------------
    // Monitor + Scoreboard + Latency
    // -------------------------
    void monitor() {
        while (true) {
            wait(clk.posedge_event());

            if (out_valid.read() && out_ready.read()) {
                pixel_t p = out_pixel.read();

                // Print with timestamp
                std::cout << sc_core::sc_time_stamp() << "  OUT: "
                          << (int)p.r << " "
                          << (int)p.g << " "
                          << (int)p.b << std::endl;

                // Log to file
                fout << sc_core::sc_time_stamp() << " "
                     << (int)p.r << " "
                     << (int)p.g << " "
                     << (int)p.b << "\n";

                // -------------------------
                // Skip first output (warm-up)
                // -------------------------
                if (!first_output_seen) {
                    first_output_seen = true;

                    // Also pop one timestamp to keep queues aligned
                    if (!timestamp_fifo.empty())
                        timestamp_fifo.pop_front();

                    continue;
                }

                // -------------------------
                // Scoreboard check
                // -------------------------
                if (!expected_fifo.empty()) {
                    pixel_t exp = expected_fifo.front();
                    expected_fifo.pop_front();

                    if (!(exp == p)) {
                        std::cout << "SCOREBOARD ERROR at "
                                  << sc_core::sc_time_stamp()
                                  << " expected=("
                                  << (int)exp.r << ","
                                  << (int)exp.g << ","
                                  << (int)exp.b << ") got=("
                                  << (int)p.r << ","
                                  << (int)p.g << ","
                                  << (int)p.b << ")"
                                  << std::endl;
                    }
                }

                // -------------------------
                // Latency measurement
                // -------------------------
                if (!timestamp_fifo.empty()) {
                    sc_core::sc_time t_in = timestamp_fifo.front();
                    timestamp_fifo.pop_front();

                    sc_core::sc_time latency =
                        sc_core::sc_time_stamp() - t_in;

                    std::cout << "LATENCY: " << latency << std::endl;
                }
            }
        }
    }

    // -------------------------
    // Constructor
    // -------------------------
    SC_CTOR(TB_ISP) : fout("output.txt") {
        isp = new ISP("isp");
        isp->clk(clk);
        isp->rst_n(rst_n);
        isp->in_pixel(in_pixel);
        isp->in_valid(in_valid);
        isp->in_ready(in_ready);
        isp->out_pixel(out_pixel);
        isp->out_valid(out_valid);
        isp->out_ready(out_ready);

        SC_THREAD(clock_gen);
        SC_THREAD(stimulus);
        SC_THREAD(monitor);

        // -------------------------
        // VCD tracing
        // -------------------------
        sc_core::sc_trace_file* tf =
            sc_core::sc_create_vcd_trace_file("isp_wave");

        sc_trace(tf, clk, "clk");
        sc_trace(tf, rst_n, "rst_n");

        sc_trace(tf, in_valid, "in_valid");
        sc_trace(tf, in_ready, "in_ready");
        sc_trace(tf, in_pixel, "in_pixel");

        sc_trace(tf, out_valid, "out_valid");
        sc_trace(tf, out_ready, "out_ready");
        sc_trace(tf, out_pixel, "out_pixel");
    }
};
