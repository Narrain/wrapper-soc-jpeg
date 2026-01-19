#include "SoCTop.h"

void SoCTop::reset_gen() {
    rst_n.write(false);
    wait(5, SC_NS);
    rst_n.write(true);
}
