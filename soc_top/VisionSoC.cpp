#include <systemc.h>
#include "VisionSoC.h"

int sc_main(int argc, char** argv) {
    VisionSoC soc("soc");

    // Reset pulse
    soc.rst_n.write(false);
    sc_start(50, SC_NS);
    soc.rst_n.write(true);

    // Run long enough for at least one frame
    sc_start(20, SC_MS);

    // Dump JPEG from DMA buffer
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
