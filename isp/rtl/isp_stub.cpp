#include "isp_stub.h"

void ISP_stub::run() {
    out_pixel.write(pixel_t());
    out_valid.write(false);
    frame_start.write(false);
    frame_end.write(false);
    wait();

    while (true) {
        // Start of frame
        frame_start.write(true);
        wait();
        frame_start.write(false);

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {

                pixel_t p;
                p.r = (x * 255) / width;
                p.g = (y * 255) / height;
                p.b = ((x + y) * 255) / (width + height);

                // Backpressure
                while (!out_ready.read())
                    wait();

                out_pixel.write(p);
                out_valid.write(true);
                wait();
            }
        }

        out_valid.write(false);

        // End of frame
        frame_end.write(true);
        wait();
        frame_end.write(false);

        // Idle a bit
        wait(20);
    }
}
