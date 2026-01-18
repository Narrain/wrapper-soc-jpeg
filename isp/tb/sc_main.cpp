#include <systemc>
#include "tb_isp.cpp"

int sc_main(int argc, char* argv[]) {
    TB_ISP tb("tb");
    sc_core::sc_start();
    return 0;
}
