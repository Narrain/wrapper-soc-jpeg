#include "jpeg_encoder.h"
#include "pixel.h"
#include <iostream>
#include <cstring>

// Convert frame_pixels (pixel_t) into libjpeg RGB buffer
static void fill_jpeg_input_buffer(const std::vector<pixel_t>& frame,
                                   JSAMPLE* buffer,
                                   int width,
                                   int height)
{
    // pixel_t: sc_uint<8> r,g,b
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx  = y * width + x;
            const pixel_t& p = frame[idx];
            int base = idx * 3;

            buffer[base + 0] = static_cast<uint8_t>(p.r.to_uint());
            buffer[base + 1] = static_cast<uint8_t>(p.g.to_uint());
            buffer[base + 2] = static_cast<uint8_t>(p.b.to_uint());
        }
    }
}

void JPEG_Encoder::ingest_frame() {
    frame_pixels.clear();
    in_ready.write(true);

    while (true) {
        wait();

        // Capture pixel if valid
        if (in_valid.read()) {
            frame_pixels.push_back(in_pixel.read());
        }

        // Exit AFTER capturing the last pixel
        if (frame_end.read()) {
            break;
        }
    }

    in_ready.write(false);
}

void JPEG_Encoder::encode_and_stream() {

    std::cout << sc_time_stamp()
              << " [ENC] encode_and_stream entered, frame_pixels = "
              << frame_pixels.size() << "\n";

    int width  = cfg.width.to_int();
    int height = cfg.height.to_int();

    // Fallback: infer from frame size (assume square)
    if (width == 0 || height == 0) {
        int pixels = static_cast<int>(frame_pixels.size());
        int side = static_cast<int>(std::sqrt(pixels));
        if (side * side == pixels && side > 0) {
            width  = side;
            height = side;
            std::cout << "[ENC] inferred width/height = "
                      << width << "x" << height << "\n";
        } else {
            std::cerr << "[ENC] ERROR: cfg width/height unset and cannot infer from "
                      << pixels << " pixels\n";
            return; // bail out safely
        }
    }

    if (width <= 0 || height <= 0) {
        std::cerr << "[ENC] ERROR: non-positive width/height: "
                  << width << "x" << height << "\n";
        return;
    }

    int expected = width * height;
    if ((int)frame_pixels.size() < expected) {
        std::cerr << "[ENC] ERROR: frame_pixels < expected ("
                  << frame_pixels.size() << " vs " << expected << ")\n";
        return; // do NOT call libjpeg or fill buffer
    }

    jpeg_buffer.clear();
    jpeg_size = 0;

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    unsigned char* outbuffer = nullptr;
    unsigned long  outsize   = 0;
    jpeg_mem_dest(&cinfo, &outbuffer, &outsize);

    cinfo.image_width      = width;
    cinfo.image_height     = height;
    cinfo.input_components = 3;
    cinfo.in_color_space   = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, cfg.quality.to_int(), TRUE);

    jpeg_start_compress(&cinfo, TRUE);

    std::vector<JSAMPLE> rgb_buf(width * height * 3);
    fill_jpeg_input_buffer(frame_pixels, rgb_buf.data(), width, height);

    JSAMPROW row_pointer[1];
    int row_stride = width * 3;

    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = &rgb_buf[cinfo.next_scanline * row_stride];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    jpeg_buffer.assign(outbuffer, outbuffer + outsize);
    jpeg_size = outsize;
    std::cout << "[ENC] jpeg_size = " << jpeg_size << " bytes\n";

    std::cout << "[ENC] first bytes of jpeg_buffer: ";
    for (int i = 0; i < 16 && i < (int)jpeg_buffer.size(); ++i) {
        printf("%02x ", jpeg_buffer[i]);
    }
    printf("\n");

    free(outbuffer);


    // ---------------- STREAM TO DMA ----------------
    // Assumes: bs_data (sc_out<sc_uint<32>>),
    //          bs_valid (sc_out<bool>),
    //          bs_ready (sc_in<bool>)
    //
    // We are inside SC_CTHREAD (run()), so wait() is legal here.

    std::cout << "[ENC] streaming " << jpeg_size << " bytes to DMA\n";

    bs_valid.write(false);
    wait();  // align to clock

    std::size_t idx = 0;
    while (idx < jpeg_buffer.size()) {
        // Pack up to 4 bytes into one 32-bit word (little-endian)
        sc_uint<32> w = 0;
        for (int b = 0; b < 4; ++b) {
            uint8_t val = 0;
            if (idx < jpeg_buffer.size()) {
                val = jpeg_buffer[idx++];
            }
            w |= (sc_uint<32>)val << (8 * b);
        }

        // Handshake: wait until downstream is ready, then send one beat
        bool sent = false;
        while (!sent) {
            wait();  // next clock
            if (bs_ready.read()) {
                bs_data.write(w);
                bs_valid.write(true);
                wait();          // one cycle with valid high
                bs_valid.write(false);
                sent = true;
            }
        }
    }

    std::cout << "[ENC] streaming done\n";

}

void JPEG_Encoder::run() {
    in_ready.write(false);
    bs_valid.write(false);
    irq_done.write(false);
    irq_status.write(0);

    // Default config for now
    cfg.width   = 128;
    cfg.height  = 128;
    cfg.quality = 80;
    cfg.irq_en  = true;

    wait();

    while (true) {
        std::cout << sc_time_stamp() << " [ENC] waiting for frame_start\n";
        while (!frame_start.read())
            wait();

        irq_status.write(0);

        std::cout << sc_time_stamp() << " [ENC] frame_start seen, ingesting\n";
        ingest_frame();
        std::cout << sc_time_stamp() << " [ENC] ingest_frame done, pixels = "
                  << frame_pixels.size() << "\n";

        std::cout << sc_time_stamp() << " [ENC] starting encode_and_stream\n";
        encode_and_stream();
        std::cout << sc_time_stamp() << " [ENC] encode_and_stream done\n";

        sc_uint<32> st = irq_status.read();
        st |= IRQ_FRAME_DONE;
        irq_status.write(st);

        irq_done.write(cfg.irq_en && (irq_status.read() != 0));
        wait();
        irq_done.write(false);

        std::cout << sc_time_stamp() << " [ENC] waiting for frame_end low\n";
        while (frame_end.read())
            wait();

        wait();
    }
}

JPEG_Encoder::JPEG_Encoder(sc_module_name n)
: sc_module(n)
{
    cfg.width   = 128;
    cfg.height  = 128;
    cfg.quality = 75;
    cfg.irq_en  = 1;

    SC_CTHREAD(run, clk.pos());
    async_reset_signal_is(rst_n, false);
}
