#include "Dmastub.h"

#include <iostream>

void DmaStub::run() {
    ddr.clear();
    in_ready.write(false);
    wait(); // reset

    while (true) {
        // Always ready to accept data
        in_ready.write(true);
        wait();

        if (in_valid.read()) {
            sc_uint<32> w = in_data.read();
            static int dbg_count = 0;
            if (dbg_count < 8) {
                std::cout << sc_time_stamp()
                        << " [DMA] w = 0x" << std::hex << w.to_uint()
                        << std::dec << "\n";
                dbg_count++;
            }

            ddr.push_back(static_cast<uint8_t>( w        & 0xFF));
            ddr.push_back(static_cast<uint8_t>((w >> 8)  & 0xFF));
            ddr.push_back(static_cast<uint8_t>((w >> 16) & 0xFF));
            ddr.push_back(static_cast<uint8_t>((w >> 24) & 0xFF));

            std::cout << sc_time_stamp() << " [DMA] word received, ddr size = "
                    << ddr.size() << "\n";
        }

    }
}
