#include "BitstreamFifo.h"

void WordFifo::run() {
    in_ready.write(false);
    out_valid.write(false);
    out_data.write(0);

    wr_ptr = 0;
    rd_ptr = 0;
    count  = 0;

    wait();

    while (true) {
        // WRITE SIDE
        bool can_write = (count < DEPTH);
        in_ready.write(can_write);

        if (in_valid.read() && can_write) {
            mem[wr_ptr] = in_data.read();
            wr_ptr = (wr_ptr + 1) % DEPTH;
            count++;
        }

        // READ SIDE (correct ready/valid behavior)
        if (!out_valid.read() && count > 0) {
            out_data.write(mem[rd_ptr]);
            out_valid.write(true);
        }

        if (out_valid.read() && out_ready.read()) {
            rd_ptr = (rd_ptr + 1) % DEPTH;
            count--;
            out_valid.write(false);
        }

        wait();
    }
}
