#include "jpeg_encoder.h"
#include <systemc.h>
#include <cstring>   // for memcpy

using namespace sc_core;
using namespace sc_dt;

// ------------------------------------------------------------
// Constructor
// ------------------------------------------------------------
#if 0
JPEG_Encoder::JPEG_Encoder(sc_core::sc_module_name name)
    : sc_core::sc_module(name)
{
    SC_CTHREAD(ingest_frame, clk.pos());
    async_reset_signal_is(rst_n, false);

    SC_CTHREAD(encode_and_stream, clk.pos());
    async_reset_signal_is(rst_n, false);
}
#endif 
JPEG_Encoder::JPEG_Encoder(sc_core::sc_module_name name)
    : sc_core::sc_module(name)
{
    width = 128; 
    height = 128;
    SC_CTHREAD(run, clk.pos()); 
    async_reset_signal_is(rst_n, false); 
}

// ------------------------------------------------------------
// Frame ingestion thread
// ------------------------------------------------------------
#if 0
void JPEG_Encoder::ingest_frame() {
    in_ready.write(false);
    frame_pixels.clear();
    wait();

    while (true) {
        if (frame_start.read()) {
            frame_pixels.clear();
        }

        if (in_valid.read()) {
            frame_pixels.push_back(in_pixel.read());
            in_ready.write(true);
        } else {
            in_ready.write(false);
        }

        wait();
    }
}
#endif
#if 0
void JPEG_Encoder::ingest_frame() {
    frame_pixels.clear();

    while (true) {
        // Capture pixels
        if (in_valid.read()) {
            frame_pixels.push_back(in_pixel.read());
            in_ready.write(true);
        } else {
            in_ready.write(false);
        }

        // Detect frame_end rising edge
        if (frame_end.read())
            break;

        wait();
    }

    in_ready.write(false);
    wait(); // consume the frame_end cycle
}
#endif 
void JPEG_Encoder::ingest_frame() {
    frame_pixels.clear();

    while (true) {
        // We are ready to accept a pixel every cycle
        in_ready.write(true);

        if (in_valid.read()) {
            frame_pixels.push_back(in_pixel.read());
        }

        // Detect end of frame
        if (frame_end.read())
            break;

        wait();
    }

    in_ready.write(false);
    wait(); // consume the frame_end cycle
}


// ------------------------------------------------------------
// JPEG encoding + bitstream streaming thread
// ------------------------------------------------------------
#if 0
void JPEG_Encoder::encode_and_stream() {
    bs_valid.write(false);
    irq_done.write(false);
    wait();

    while (true) {
        if (frame_end.read()) {

            // ------------------------------------------------------------
            // REAL JPEG ENCODER (libjpeg-turbo)
            // ------------------------------------------------------------
            struct jpeg_compress_struct cinfo;
            struct jpeg_error_mgr jerr;

            cinfo.err = jpeg_std_error(&jerr);
            jpeg_create_compress(&cinfo);

            unsigned char* outbuf = nullptr;
            unsigned long outsize = 0;
            jpeg_mem_dest(&cinfo, &outbuf, &outsize);


            cinfo.image_width = width;
            cinfo.image_height = height;
            cinfo.input_components = 3;
            cinfo.in_color_space = JCS_RGB;

            jpeg_set_defaults(&cinfo);
            jpeg_start_compress(&cinfo, TRUE);

            JSAMPROW row[1];
            std::vector<unsigned char> rowbuf(width * 3);

            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    pixel_t p = frame_pixels[y * width + x];
                    rowbuf[x * 3 + 0] = p.r.to_uint();
                    rowbuf[x * 3 + 1] = p.g.to_uint();
                    rowbuf[x * 3 + 2] = p.b.to_uint();
                }
                row[0] = &rowbuf[0];
                jpeg_write_scanlines(&cinfo, row, 1);
            }

            jpeg_finish_compress(&cinfo);
            jpeg_buffer.assign(outbuf, outbuf + outsize);
            jpeg_size = outsize;
            free(outbuf);
            jpeg_destroy_compress(&cinfo);

            // ------------------------------------------------------------
            // STREAM OUT BITSTREAM (32-bit words)
            // ------------------------------------------------------------
            int idx = 0;
            std::cout << sc_time_stamp()
                    << " [ENC] jpeg_size = " << jpeg_size
                    << " (w=" << width << ", h=" << height << ")\n";

            while (idx < (int)jpeg_size) {
                sc_dt::sc_uint<32> word = 0;

                for (int b = 0; b < 4 && idx < (int)jpeg_size; b++, idx++) {
                    word.range(b * 8 + 7, b * 8) = jpeg_buffer[idx];
                }

                while (!bs_ready.read())
                    wait();

                bs_data.write(word);
                bs_valid.write(true);
                wait();

                bs_valid.write(false);
                wait();
            }

            // ------------------------------------------------------------
            // IRQ: encoding done
            // ------------------------------------------------------------
            irq_done.write(true);
            wait();
            irq_done.write(false);
        }

        wait();
    }
}
#endif

void JPEG_Encoder::encode_and_stream() {
    // JPEG encode
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    unsigned char* outbuf = nullptr;
    unsigned long outsize = 0;
    jpeg_mem_dest(&cinfo, &outbuf, &outsize);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_start_compress(&cinfo, TRUE);

    JSAMPROW row[1];
    std::vector<unsigned char> rowbuf(width * 3);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            pixel_t p = frame_pixels[y * width + x];
            rowbuf[x * 3 + 0] = p.r.to_uint();
            rowbuf[x * 3 + 1] = p.g.to_uint();
            rowbuf[x * 3 + 2] = p.b.to_uint();
        }
        row[0] = &rowbuf[0];
        jpeg_write_scanlines(&cinfo, row, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_buffer.assign(outbuf, outbuf + outsize);
    jpeg_size = outsize;
    free(outbuf);
    jpeg_destroy_compress(&cinfo);

    std::cout << sc_time_stamp() << " [ENC] jpeg_size = " << jpeg_size << "\n";

    // Stream out
    int idx = 0;
    while (idx < (int)jpeg_size) {
        sc_dt::sc_uint<32> word = 0;

        for (int b = 0; b < 4 && idx < (int)jpeg_size; b++, idx++) {
            word.range(b * 8 + 7, b * 8) = jpeg_buffer[idx];
        }

        while (!bs_ready.read())
            wait();

        bs_data.write(word);
        bs_valid.write(true);
        wait();

        bs_valid.write(false);
        wait();
    }
}

#if 0
void JPEG_Encoder::run() {
    in_ready.write(false);
    bs_valid.write(false);
    irq_done.write(false);
    wait();

    while (true) {
        // Wait for frame_start
        while (!frame_start.read())
            wait();

        // Ingest pixels
        ingest_frame();

        // Encode + stream
        encode_and_stream();

        // IRQ
        irq_done.write(true);
        wait();
        irq_done.write(false);

        // Wait for frame_end
        while (!frame_end.read())
            wait();

        wait(); // idle
    }
}
#endif
void JPEG_Encoder::run() {
    in_ready.write(false);
    bs_valid.write(false);
    irq_done.write(false);
    wait();

    while (true) {
        std::cout << sc_time_stamp() << " [ENC] waiting for frame_start\n";
        while (!frame_start.read())
            wait();

        std::cout << sc_time_stamp() << " [ENC] frame_start seen, ingesting\n";
        ingest_frame();
        std::cout << sc_time_stamp() << " [ENC] ingest_frame done, pixels = "
                  << frame_pixels.size() << "\n";

        std::cout << sc_time_stamp() << " [ENC] starting encode_and_stream\n";
        encode_and_stream();
        std::cout << sc_time_stamp() << " [ENC] encode_and_stream done\n";

        irq_done.write(true);
        wait();
        irq_done.write(false);

        std::cout << sc_time_stamp() << " [ENC] waiting for frame_end low\n";
        while (frame_end.read())
            wait();

        wait(); // idle
    }
}
