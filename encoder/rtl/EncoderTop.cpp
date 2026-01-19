#include "EncoderTop.h"

// ================================================================
// Internal Signals
// ================================================================
sc_signal<bool>        sig_header_valid;
sc_signal<bool>        sig_header_ready;
sc_signal<sc_uint<32>> sig_header_data;

sc_signal<bool>        sig_bm_valid;
sc_signal<sc_int<16>>  sig_bm_block[64];
sc_signal<sc_uint<2>>  sig_bm_type;
sc_signal<bool>        sig_bm_restart;

sc_signal<bool>        sig_dct_valid;
sc_signal<sc_int<16>>  sig_dct_coeff[64];

sc_signal<bool>        sig_entropy_valid;
sc_signal<sc_uint<32>> sig_entropy_bits;

sc_signal<bool>        sig_fifo_valid;
sc_signal<bool>        sig_fifo_ready;
sc_signal<sc_uint<32>> sig_fifo_data;


// ================================================================
// Register Interface + Control Glue
// ================================================================
void EncoderTop::reg_if() {
    // Reset
    reg_ready.write(false);
    reg_CTRL.write(0);
    reg_STATUS.write(0);
    reg_FRAME_WIDTH.write(0);
    reg_FRAME_HEIGHT.write(0);
    reg_DMA_BASE.write(0);
    reg_DMA_SIZE.write(0);
    reg_IRQ_ENABLE.write(0);

    dma_addr.write(0);
    dma_wdata.write(0);
    dma_wvalid.write(false);
    dma_start.write(false);

    wait();

    while (true) {
        reg_ready.write(false);

        // ----------------------------
        // Writes
        // ----------------------------
        if (reg_write.read()) {
            sc_uint<8>  addr = reg_addr.read();
            sc_uint<32> w    = reg_wdata.read();

            switch (addr) {
                case 0x00: // CTRL
                    reg_CTRL.write(w);
                    break;
                case 0x08: // FRAME_WIDTH
                    reg_FRAME_WIDTH.write(w);
                    break;
                case 0x0C: // FRAME_HEIGHT
                    reg_FRAME_HEIGHT.write(w);
                    break;
                case 0x10: // DMA_BASE_ADDR
                    reg_DMA_BASE.write(w);
                    break;
                case 0x14: // DMA_SIZE
                    reg_DMA_SIZE.write(w);
                    break;
                case 0x18: // IRQ_ENABLE
                    reg_IRQ_ENABLE.write(w);
                    break;
                default:
                    break;
            }

            reg_ready.write(true);
        }

        // ----------------------------
        // Reads
        // ----------------------------
        if (reg_read.read()) {
            sc_uint<8>  addr = reg_addr.read();
            sc_uint<32> r    = 0;

            switch (addr) {
                case 0x00: r = reg_CTRL.read();        break;
                case 0x04: r = reg_STATUS.read();      break;
                case 0x08: r = reg_FRAME_WIDTH.read(); break;
                case 0x0C: r = reg_FRAME_HEIGHT.read();break;
                case 0x10: r = reg_DMA_BASE.read();    break;
                case 0x14: r = reg_DMA_SIZE.read();    break;
                case 0x18: r = reg_IRQ_ENABLE.read();  break;
                default:   r = 0;                      break;
            }

            reg_rdata.write(r);
            reg_ready.write(true);
        }

        // ----------------------------
        // Control FSM
        // ----------------------------
        sc_uint<32> ctrl   = reg_CTRL.read();
        sc_uint<32> status = reg_STATUS.read();

        bool enable      = ctrl & CTRL_ENABLE;
        bool start_frame = ctrl & CTRL_START_FRAME;

        // Start of frame: clear FRAME_DONE, set BUSY
        if (enable && start_frame) {
            status &= ~STATUS_FRAME_DONE;
            status |= STATUS_BUSY;

            // Program DMA start address
            dma_addr.write(reg_DMA_BASE.read());
            dma_start.write(true);
        } else {
            dma_start.write(false);
        }

        // DMA done → clear BUSY, set FRAME_DONE
        if (dma_done.read()) {
            status &= ~STATUS_BUSY;
            status |= STATUS_FRAME_DONE;
        }

        // TODO: hook real error flags into STATUS_ERROR
        reg_STATUS.write(status);

        // ----------------------------
        // Drive config into submodules
        // ----------------------------
        // BlockMaker config
        bm->cfg_width(reg_FRAME_WIDTH.read().range(15,0));
        bm->cfg_height(reg_FRAME_HEIGHT.read().range(15,0));
        // bm->cfg_subsampling(...)       // when you add a reg for it
        // bm->cfg_restart_interval(...); // when you add a reg for it

        // HeaderGen config (adapt to your ports)
        // header->cfg_width(reg_FRAME_WIDTH.read().range(15,0));
        // header->cfg_height(reg_FRAME_HEIGHT.read().range(15,0));

        wait();
    }
}


// ================================================================
// IRQ Logic
// ================================================================
void EncoderTop::irq_logic() {
    irq_out.write(false);
    wait();

    while (true) {
        bool irq = false;

        // Frame-done IRQ
        if (dma_done.read() && (reg_IRQ_ENABLE.read() & 0x1))
            irq = true;

        // TODO: add error IRQs if you want

        irq_out.write(irq);
        wait();
    }
}


// ================================================================
// Constructor Wiring
// ================================================================
EncoderTop::EncoderTop(sc_module_name name)
: sc_module(name)
{
    // CTHREADs
    SC_CTHREAD(reg_if, clk.pos());
    async_reset_signal_is(rst_n, false);

    SC_CTHREAD(irq_logic, clk.pos());
    async_reset_signal_is(rst_n, false);

    // Instantiate submodules
    header   = new HeaderGen("HeaderGen");
    bm       = new BlockMaker("BlockMaker");
    dctq     = new DCTQ("DCTQ");
    entropy  = new Entropy("Entropy");
    bs_fifo  = new BitstreamFifo("BitstreamFifo");
    dma      = new DmaEngine("DmaEngine");

    // ----------------------
    // Pixel path: ISP → BlockMaker
    // ----------------------
    bm->clk(clk);
    bm->rst_n(rst_n);

    bm->in_pixel(in_pixel);
    bm->in_valid(in_valid);
    bm->in_ready(in_ready);
    // if you have frame_start/frame_end in BlockMaker, wire them:
    // bm->frame_start(frame_start);
    // bm->frame_end(frame_end);

    // ----------------------
    // BlockMaker → DCTQ
    // ----------------------
    for (int i = 0; i < 64; ++i) {
        bm->out_block[i](sig_bm_block[i]);
        dctq->in_block[i](sig_bm_block[i]);
    }
    bm->out_valid(sig_bm_valid);
    bm->out_type(sig_bm_type);
    bm->restart_mcu_boundary(sig_bm_restart);

    dctq->clk(clk);
    dctq->rst_n(rst_n);
    dctq->in_valid(sig_bm_valid);
    dctq->in_type(sig_bm_type);
    dctq->restart_mcu_boundary(sig_bm_restart);

    for (int i = 0; i < 64; ++i) {
        dctq->out_coeff[i](sig_dct_coeff[i]);
    }
    dctq->out_valid(sig_dct_valid);

    // ----------------------
    // DCTQ → Entropy
    // ----------------------
    entropy->clk(clk);
    entropy->rst_n(rst_n);

    for (int i = 0; i < 64; ++i) {
        entropy->in_coeff[i](sig_dct_coeff[i]);
    }
    entropy->in_valid(sig_dct_valid);
    entropy->in_type(sig_bm_type);
    entropy->restart_mcu_boundary(sig_bm_restart);

    entropy->out_bits(sig_entropy_bits);
    entropy->out_valid(sig_entropy_valid);

    // ----------------------
    // HeaderGen → FIFO
    // ----------------------
    header->clk(clk);
    header->rst_n(rst_n);

    header->out_data(sig_header_data);
    header->out_valid(sig_header_valid);
    header->out_ready(sig_header_ready);

    // ----------------------
    // FIFO Input Arbitration (Header + Entropy)
    // ----------------------
    bs_fifo->clk(clk);
    bs_fifo->rst_n(rst_n);

    // Simple priority: header first, then entropy
    bs_fifo->in_data( sig_header_valid.read() ? sig_header_data.read()
                                              : sig_entropy_bits.read() );
    bs_fifo->in_valid( sig_header_valid | sig_entropy_valid );
    sig_header_ready = bs_fifo->in_ready; // backpressure to header
    // entropy has no backpressure in your current design

    // ----------------------
    // FIFO → DMA
    // ----------------------
    bs_fifo->out_data(sig_fifo_data);
    bs_fifo->out_valid(sig_fifo_valid);
    bs_fifo->out_ready(sig_fifo_ready);

    dma->clk(clk);
    dma->rst_n(rst_n);

    dma->in_data(sig_fifo_data);
    dma->in_valid(sig_fifo_valid);
    dma->in_ready(sig_fifo_ready);

    dma->dma_addr(dma_addr);
    dma->dma_wdata(dma_wdata);
    dma->dma_wvalid(dma_wvalid);
    dma->dma_wready(dma_wready);
    dma->dma_start(dma_start);
    dma->dma_done(dma_done);
}
