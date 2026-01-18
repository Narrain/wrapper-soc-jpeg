#include <systemc.h>
#include "VisionSoC.h"

int sc_main(int argc, char** argv) {
    VisionSoC soc("soc");

    // Reset pulse
    soc.rst_n.write(false);
    sc_start(50, SC_NS);
    soc.rst_n.write(true);

    // Run long enough for at least one frame
    sc_start(5, SC_MS);

    // Dump JPEG
    FILE* f = fopen("output.jpg", "wb");
    if (f && !soc.sink.buffer.empty()) {
        fwrite(soc.sink.buffer.data(), 1, soc.sink.buffer.size(), f);
        fclose(f);
        std::cout << "JPEG written to output.jpg, size = "
                  << soc.sink.buffer.size() << " bytes\n";
    } else {
        std::cout << "No data captured in sink.buffer\n";
    }

    return 0;
}
