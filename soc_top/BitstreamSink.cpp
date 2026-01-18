#include "BitstreamSink.h"

#if 0
void BitstreamSink::run() {
    ready.write(true);
    buffer.clear();
    wait();

    while (true) {
        if (valid.read()) {
            uint32_t w = data.read();
            buffer.push_back((w >> 0) & 0xFF);
            buffer.push_back((w >> 8) & 0xFF);
            buffer.push_back((w >> 16) & 0xFF);
            buffer.push_back((w >> 24) & 0xFF);
        }
        wait();
    }
}
#endif 
void BitstreamSink::run() {
    ready.write(true);
    buffer.clear();
    wait();

    while (true) {
        if (valid.read()) {
            uint32_t w = data.read();
            buffer.push_back((w >> 0) & 0xFF);
            buffer.push_back((w >> 8) & 0xFF);
            buffer.push_back((w >> 16) & 0xFF);
            buffer.push_back((w >> 24) & 0xFF);
            std::cout << sc_time_stamp() << " [SINK] captured word, total bytes = "
                      << buffer.size() << "\n";
        }
        wait();
    }
}
