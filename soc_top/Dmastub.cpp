#include "Dmastub.h"

#include <iostream>

void DmaStub::run() {
    ddr.clear();
    in_ready.write(false);
    wait(); // reset

    bool done = false;

    while (true) {

        // If encoder signals done and we haven't handled it yet,
        // append EOI (FF D9 00 00) and stop.

        if (done) {
            in_ready.write(false);
        } else {
            in_ready.write(true);
        }

        wait();
        std::cout << sc_time_stamp() << " [DMA] irq_done=" << irq_done.read() << "\n";
        if (irq_done.read() && !done) {
            std::cout << sc_time_stamp() << " [DMA] irq_done seen\n";
            ddr.push_back(0xFF);
            ddr.push_back(0xD9);
            ddr.push_back(0x00);
            ddr.push_back(0x00);
            done = true;
        }

        if (!done && in_valid.read()) {
            sc_uint<32> w = in_data.read();

            ddr.push_back((uint8_t)(w & 0xFF));
            ddr.push_back((uint8_t)((w >> 8) & 0xFF));
            ddr.push_back((uint8_t)((w >> 16) & 0xFF));
            ddr.push_back((uint8_t)((w >> 24) & 0xFF));
        }
    }

}
