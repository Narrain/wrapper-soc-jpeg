#include "soc_top.h"
using namespace sc_core;

SC_CTOR(SocTop) {
    isp = new ISP("isp");
    enc = new Encoder("encoder");
    mcu = new MCU("mcu");
    mem = new MemCtrl("mem");
    i2c = new I2C("i2c");

    isp->clk(clk); isp->rst_n(rst_n);
    enc->clk(clk); enc->rst_n(rst_n);
    mcu->clk(clk); mcu->rst_n(rst_n);
    mem->clk(clk); mem->rst_n(rst_n);
    i2c->clk(clk); i2c->rst_n(rst_n);
}
