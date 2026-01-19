#include "ISPTop.h"

void ISPTop::run() {
    pixel_t zero_pix;
    zero_pix.r = 0;
    zero_pix.g = 0;
    zero_pix.b = 0;

    out_pixel.write(zero_pix);
    out_valid.write(false);
    frame_start.write(false);
    frame_end.write(false);

    wait();

    const int width  = 64;   // can later tie to cfg_width
    const int height = 64;

    while (true) {
        // One frame
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {

                // Backpressure: wait until FIFO/encoder is ready
                do { wait(); } while (!out_ready.read());

                pixel_t p;
                p.r = x & 0xFF;
                p.g = y & 0xFF;
                p.b = (x + y) & 0xFF;

                bool first = (x == 0 && y == 0);
                bool last  = (x == width-1 && y == height-1);

                out_pixel.write(p);
                out_valid.write(true);
                frame_start.write(first);
                frame_end.write(last);

                wait();

                out_valid.write(false);
                frame_start.write(false);
                frame_end.write(false);
            }
        }

        // idle between frames
        for (int i = 0; i < 100; ++i)
            wait();
    }
}
