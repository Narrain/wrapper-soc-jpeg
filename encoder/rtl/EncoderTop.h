#pragma once
#include <systemc.h>
#include "pixel.h"

// Forward declarations of your existing modules
#include "HeaderGen.h"
#include "BlockMaker.h"
#include "DCTQ.h"
#include "Entropy.h"
#include "BitstreamFifo.h"
#include "DmaEngine.h"

SC_MODULE(EncoderTop) {

    // ------------------------------------------------------------
    // Clock / Reset
    // ------------------------------------------------------------
    sc_in<bool> clk;
    sc_in<bool> rst_n;

    // ------------------------------------------------------------
    // Register Bus (simple APB-like)
    // ------------------------------------------------------------
    sc_in<bool>        reg_write;
    sc_in<bool>        reg_read;
    sc_in<sc_uint<8>>  reg_addr;
    sc_in<sc_uint<32>> reg_wdata;
    sc_out<sc_uint<32>> reg_rdata;
    sc_out<bool>       reg_ready;

    // ------------------------------------------------------------
    // Pixel Input (from ISP)
    // ------------------------------------------------------------
    sc_in<pixel_t> in_pixel;
    sc_in<bool>    in_valid;
    sc_out<bool>   in_ready;
    sc_in<bool>    frame_start;
    sc_in<bool>    frame_end;

    // ------------------------------------------------------------
    // DMA Master Interface (abstract)
    // ------------------------------------------------------------
    sc_out<sc_uint<32>> dma_addr;
    sc_out<sc_uint<32>> dma_wdata;
    sc_out<bool>        dma_wvalid;
    sc_in<bool>         dma_wready;
    sc_out<bool>        dma_start;
    sc_in<bool>         dma_done;

    // ------------------------------------------------------------
    // Interrupt
    // ------------------------------------------------------------
    sc_out<bool> irq_out;

    // ------------------------------------------------------------
    // Internal Registers
    // ------------------------------------------------------------
    sc_signal<sc_uint<32>> reg_CTRL;        // 0x00
    sc_signal<sc_uint<32>> reg_STATUS;      // 0x04
    sc_signal<sc_uint<32>> reg_FRAME_WIDTH; // 0x08
    sc_signal<sc_uint<32>> reg_FRAME_HEIGHT;// 0x0C
    sc_signal<sc_uint<32>> reg_DMA_BASE;    // 0x10
    sc_signal<sc_uint<32>> reg_DMA_SIZE;    // 0x14
    sc_signal<sc_uint<32>> reg_IRQ_ENABLE;  // 0x18

    // CTRL bits
    enum {
        CTRL_ENABLE      = 1 << 0,
        CTRL_START_FRAME = 1 << 1
    };

    // STATUS bits
    enum {
        STATUS_BUSY       = 1 << 0,
        STATUS_FRAME_DONE = 1 << 1,
        STATUS_ERROR      = 1 << 2
    };

    // ------------------------------------------------------------
    // Submodules
    // ------------------------------------------------------------
    HeaderGen*      header;
    BlockMaker*     bm;
    DCTQ*           dctq;
    Entropy*        entropy;
    BitstreamFifo*  bs_fifo;
    DmaEngine*      dma;

    // ------------------------------------------------------------
    // Processes
    // ------------------------------------------------------------
    void reg_if();
    void irq_logic();

    SC_CTOR(EncoderTop) {
        // Register interface
        SC_CTHREAD(reg_if, clk.pos());
        async_reset_signal_is(rst_n, false);

        // IRQ logic
        SC_CTHREAD(irq_logic, clk.pos());
        async_reset_signal_is(rst_n, false);

        // Instantiate submodules
        header   = new HeaderGen("HeaderGen");
        bm       = new BlockMaker("BlockMaker");
        dctq     = new DCTQ("DCTQ");
        entropy  = new Entropy("Entropy");
        bs_fifo  = new BitstreamFifo("BitstreamFifo");
        dma      = new DmaEngine("DmaEngine");
    }

    ~EncoderTop() {
        delete header;
        delete bm;
        delete dctq;
        delete entropy;
        delete bs_fifo;
        delete dma;
    }
};
