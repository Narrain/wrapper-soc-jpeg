// WordFifo.cpp
#include "WordFifo.h"

void WordFifo::run() {
    in_ready.write(false);
    out_valid.write(false);
    out_data.write(0);
    q.clear();

    wait();  // after reset

    while (true) {
        // WRITE SIDE
        bool can_write = (q.size() < DEPTH);
        in_ready.write(can_write);

        if (in_valid.read() && can_write) {
            q.push_back(in_data.read());
        }

        // READ SIDE
        if (!out_valid.read() && !q.empty()) {
            out_data.write(q.front());
            out_valid.write(true);
        }

        if (out_valid.read() && out_ready.read()) {
            q.pop_front();
            out_valid.write(false);
        }

        wait();
    }
}
