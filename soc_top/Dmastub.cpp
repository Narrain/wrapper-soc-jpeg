#include "Dmastub.h"
#include <iostream>
#include <fstream>

void DmaStub::run() {
    ddr.clear();
    active = false;

    in_ready.write(false);
    wait(); // reset

    bool done = false;

    while (true) {

        // Ready only while we're actively capturing and not done
        if (!done)
            in_ready.write(true);
        else
            in_ready.write(false);

        // Capture data when valid and ready
        if (!done && in_valid.read() && in_ready.read()) {
            sc_uint<32> w = in_data.read();

            ddr.push_back(static_cast<uint8_t>( w        & 0xFF));
            ddr.push_back(static_cast<uint8_t>((w >> 8)  & 0xFF));
            ddr.push_back(static_cast<uint8_t>((w >> 16) & 0xFF));
            ddr.push_back(static_cast<uint8_t>((w >> 24) & 0xFF));

            active = true;
        }

        // If encoder signals done and we haven't handled it yet,
        // append EOI (FF D9 00 00) and stop.
        if (irq_done.read() && !done) {
            std::cout << sc_time_stamp() << " [DMA] irq_done seen\n";
            ddr.push_back(0xFF);
            ddr.push_back(0xD9);
            ddr.push_back(0x00);
            ddr.push_back(0x00);
            done = true;
        }

        wait();
    }
}

DmaStub::~DmaStub() {
    if (!ddr.empty()) {
        std::ofstream ofs(out_filename.c_str(), std::ios::binary);
        if (ofs) {
            ofs.write(reinterpret_cast<const char*>(ddr.data()),
                      static_cast<std::streamsize>(ddr.size()));
            std::cout << "[DMA] Wrote " << ddr.size()
                      << " bytes to " << out_filename << "\n";
        } else {
            std::cout << "[DMA] ERROR: could not open " << out_filename
                      << " for write\n";
        }
    } else {
        std::cout << "[DMA] No data captured, nothing written\n";
    }
}
