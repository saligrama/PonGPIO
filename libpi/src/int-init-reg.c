#include "rpi.h"
#include "rpi-interrupts.h"
#include "vector-base.h"

void int_init_reg(void *int_vector_addr) {
    // put interrupt flags in known state. 
    //  BCM2835 manual, section 7.5
    dev_barrier();
    PUT32(Disable_IRQs_1, 0xffffffff);
    PUT32(Disable_IRQs_2, 0xffffffff);
    dev_barrier();

    vector_base_set(int_vector_addr);
}