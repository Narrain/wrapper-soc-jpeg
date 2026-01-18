#pragma once
#include <systemc.h>
#include <vector>

using namespace sc_core;
using namespace sc_dt;

SC_MODULE(BitstreamSink) {
    sc_in<bool> clk;
    sc_in<bool> rst_n;

    sc_in<sc_uint<32>> data;
    sc_in<bool>        valid;
    sc_out<bool>       ready;

    std::vector<uint8_t> buffer;

    void run();

    SC_CTOR(BitstreamSink) {
        SC_CTHREAD(run, clk.pos());
        async_reset_signal_is(rst_n, false);
    }
};

// #pragma once
// #include <systemc.h>
// #include <vector>

// using namespace sc_core;
// using namespace sc_dt;

// SC_MODULE(BitstreamSink) {
//     sc_in<bool> clk;
//     sc_in<bool> rst_n;

//     sc_in<sc_uint<32>> data;
//     sc_in<bool>        valid;
//     sc_out<bool>       ready;

//     std::vector<uint8_t> buffer;

//     void run() {
//         ready.write(true);
//         buffer.clear();
//         wait();

//         while (true) {
//             if (valid.read()) {
//                 uint32_t w = data.read();
//                 buffer.push_back((w >> 0) & 0xFF);
//                 buffer.push_back((w >> 8) & 0xFF);
//                 buffer.push_back((w >> 16) & 0xFF);
//                 buffer.push_back((w >> 24) & 0xFF);
//             }
//             wait();
//         }
//     }

//     SC_CTOR(BitstreamSink) {
//         SC_CTHREAD(run, clk.pos());
//         async_reset_signal_is(rst_n, false);
//     }
// };
