#include "BitstreamFifo.h"

void BitstreamFifo::run() {
    rd_ptr = 0;
    wr_ptr = 0;
    count  = 0;

    out_valid.write(false);
    in_ready.write(false);
    out_data.write(0);

    wait();

    while (true) {
        bool can_write = (count < DEPTH);
        bool can_read  = (count > 0);

        in_ready.write(can_write);
        out_valid.write(can_read);

        // WRITE
        if (in_valid.read() && can_write) {
            mem[wr_ptr] = in_data.read();
            wr_ptr = (wr_ptr + 1) % DEPTH;
            count++;
        }

        // READ
        if (out_ready.read() && can_read) {
            out_data.write(mem[rd_ptr]);
            rd_ptr = (rd_ptr + 1) % DEPTH;
            count--;
        }

        wait();
    }
}
