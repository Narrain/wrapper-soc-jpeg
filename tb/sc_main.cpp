#include <systemc.h>
#include "VisionSoC.h"

int sc_main(int argc, char** argv) {
    VisionSoC soc("soc");

    // Apply reset
    soc.rst_n.write(false);
    sc_start(20, SC_NS);
    soc.rst_n.write(true);

    // After reset in your testbench:
    soc.dummy_valid.write(true);
    soc.dummy_data.write(0x11223344);
    // wait(10, SC_NS);
    soc.dummy_valid.write(false);

    // Tie dummy_ready high
    soc.dummy_ready.write(true);

    // Run long enough for ISP → Encoder → FIFO → DMA
    sc_start(5, SC_MS);

    // Dump DMA output
    if (!soc.dma.ddr.empty()) {
        FILE* f = fopen("output.jpg", "wb");
        fwrite(soc.dma.ddr.data(), 1, soc.dma.ddr.size(), f);
        fclose(f);

        std::cout << "JPEG written to output.jpg, size = "
                  << soc.dma.ddr.size() << " bytes\n";
    } else {
        std::cout << "No data captured in DMA buffer\n";
    }

    return 0;
}
