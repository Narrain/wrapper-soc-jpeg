#include "ConfigRegs.h"
#include <iostream>

void ConfigRegs::run() {
    width_reg            = 0;
    height_reg           = 0;
    subsampling_reg      = 0;
    restart_interval_reg = 0;
    quality_reg          = 75;
    start_reg            = false;
    dma_base_reg         = 0;
    irq_mask_reg         = 0;

    width.write(0);
    height.write(0);
    subsampling.write(0);
    restart_interval.write(0);
    quality.write(75);
    start.write(false);
    dma_base.write(0);
    irq_mask.write(0);

    wait();

    while (true) {
        if (wr_en.read()) {
            sc_uint<8>  addr = wr_addr.read();
            sc_uint<32> data = wr_data.read();

            switch (addr) {
                case 0x00: width_reg            = data & 0xFFFF; break;
                case 0x04: height_reg           = data & 0xFFFF; break;
                case 0x08: subsampling_reg      = data & 0x3;    break;
                case 0x0C: restart_interval_reg = data & 0xFFFF; break;
                case 0x10: quality_reg          = data & 0xFF;   break;
                case 0x14: start_reg            = (data & 0x1);  break;
                case 0x18: dma_base_reg         = data;          break;
                case 0x1C: irq_mask_reg         = data & 0xFF;   break;
                default:
                    std::cout << sc_time_stamp()
                              << " [CFG] Write to unknown addr 0x"
                              << std::hex << addr << std::dec << "\n";
                    break;
            }
        }

        width.write(width_reg);
        height.write(height_reg);
        subsampling.write(subsampling_reg);
        restart_interval.write(restart_interval_reg);
        quality.write(quality_reg);
        start.write(start_reg);
        dma_base.write(dma_base_reg);
        irq_mask.write(irq_mask_reg);

        wait();
    }
}
