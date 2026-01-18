#include <systemc>
#include "tb_encoder.cpp"

int sc_main(int argc, char* argv[]) {
    TB tb("tb");
    sc_start();
    return 0;
}
