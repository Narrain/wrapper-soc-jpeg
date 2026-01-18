#include <systemc.h>
#include "VisionSoC.h"

int sc_main(int argc, char** argv) {
    VisionSoC soc("soc");

    // Apply reset
    soc.rst_n.write(false);
    sc_start(50, SC_NS);   // hold reset for 50ns
    soc.rst_n.write(true);

    // Run long enough for ISP → Encoder → Sink
    sc_start(5, SC_MS);

    // Write JPEG output
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
