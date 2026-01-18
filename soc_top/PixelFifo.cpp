#include "PixelFifo.h"

// PixelFifo::PixelFifo(sc_core::sc_module_name name)
//     : sc_core::sc_module(name) {
//     SC_CTHREAD(run, clk.pos()); 
//     async_reset_signal_is(rst_n, false);
// }

void PixelFifo::run() {
    // Async reset
    in_ready.write(false);
    out_valid.write(false);
    // Optional: drive a known pixel value
    pixel_t zero_pix;
    zero_pix.r = 0;
    zero_pix.g = 0;
    zero_pix.b = 0;
    out_data.write(zero_pix);

    wr_ptr = 0;
    rd_ptr = 0;
    count  = 0;

    wait();  // first clock after reset

    while (true) {
        // WRITE SIDE
        bool can_write = (count < DEPTH);
        in_ready.write(can_write);

        if (in_valid.read() && can_write) {
            mem[wr_ptr] = in_data.read();
            wr_ptr = (wr_ptr + 1) % DEPTH;
            count++;
        }

        // READ SIDE
        bool can_read = (count > 0);
        if (can_read && out_ready.read()) {
            out_data.write(mem[rd_ptr]);
            out_valid.write(true);
            rd_ptr = (rd_ptr + 1) % DEPTH;
            count--;
        } else {
            out_valid.write(false);
        }

        wait();
    }
}
