#include <fstream>
#include "VisionSoC.h"

int sc_main(int argc, char** argv) {
    VisionSoC soc("soc");

    // Reset
    soc.rst_n.write(false);
    soc.cfg_wr_en.write(false);
    sc_start(20, SC_NS);
    soc.rst_n.write(true);
    sc_start(20, SC_NS);

    // Program config (same as before)
    auto write_cfg = [&](uint8_t addr, uint32_t data) {
        soc.cfg_wr_addr.write(addr);
        soc.cfg_wr_data.write(data);
        soc.cfg_wr_en.write(true);
        sc_start(10, SC_NS);
        soc.cfg_wr_en.write(false);
        sc_start(10, SC_NS);
    };

    write_cfg(0x00, 64);
    write_cfg(0x04, 64);
    write_cfg(0x08, 0);
    write_cfg(0x0C, 0);
    write_cfg(0x10, 75);
    write_cfg(0x18, 0);
    write_cfg(0x1C, 1);
    write_cfg(0x14, 1); // start

    // Run until IRQ
    while (true) {
        sc_start(1, SC_NS);
        if (soc.irq_done.read())
            break;
    }

    // Dump DMA buffer to JPEG
    std::ofstream ofs("output.jpg", std::ios::binary);
    for (auto b : soc.dma.ddr) {
        ofs.put(static_cast<char>(b));
    }
    ofs.close();

    std::cout << "IRQ DONE at " << sc_time_stamp() << "\n";
    std::cout << "JPEG written to output.jpg, bytes = " << soc.dma.ddr.size() << "\n";
    return 0;
}
