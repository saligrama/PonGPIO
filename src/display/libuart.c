// implement:
//  void libuart_init(void)
//
//  int libuart_can_getc(void);
//  int libuart_getc(void);
//
//  int libuart_can_putc(void);
//  void libuart_putc(unsigned c);
//
//  int libuart_tx_is_empty(void) {
//
// see that hello world works.
//
//
#include "rpi.h"
#include "libuart.h"

#define LIBUART_RX_PIN 20
#define LIBUART_TX_PIN 21
#define LIBUART_AUX_ENABLES 0x20215004
#define LIBUART_AUX_MU_IO_REG 0x20215040
#define LIBUART_AUX_MU_IER_REG 0x20215044
#define LIBUART_AUX_MU_IIR_REG 0x20215048
#define LIBUART_AUX_MU_LCR_REG 0x2021504C
#define LIBUART_AUX_MU_LSR_REG 0x20215054
#define LIBUART_AUX_MU_CNTL_REG 0x20215060
#define LIBUART_AUX_MU_STAT_REG 0x20215064
#define LIBUART_AUX_MU_BAUD_REG 0x20215068

#define LIBUART_115200_BAUD 270

// called first to setup lbuart to 8n1 115200  baud,
// no interrupts.
//  - you will need memory barriers, use <dev_barrier()>
//
//  later: should add an init that takes a baud rate.
void libuart_init(void) {
    dev_barrier();

    gpio_set_function(LIBUART_RX_PIN, GPIO_FUNC_ALT5);
    gpio_set_function(LIBUART_TX_PIN, GPIO_FUNC_ALT5);

    dev_barrier();

    // turn the LIBUART on using LIBUART_AUX_ENABLES
    PUT32(LIBUART_AUX_ENABLES, GET32(LIBUART_AUX_ENABLES) | 0b1);

    dev_barrier();

    // disable TX and RX (AUX_MU_CNTL_REG)
    PUT32(LIBUART_AUX_MU_CNTL_REG, GET32(LIBUART_AUX_MU_CNTL_REG) & ~0b11);

    // clear TX and RX (AUX_MU_IIR_REG)
    PUT32(LIBUART_AUX_MU_IIR_REG, 0b110);

    // disable interrupts (AUX_MU_IER_REG)
    PUT32(LIBUART_AUX_MU_IER_REG, 0b0);

    // set baud rate to 115200 (AUX_MU_BAUD_REG)
    PUT32(LIBUART_AUX_MU_BAUD_REG, LIBUART_115200_BAUD);

    // set data size to 8 bits (AUX_MU_LCR_REG)
    PUT32(LIBUART_AUX_MU_LCR_REG, 0b11);

    // assumed that 1 start bit, 1 stop bit, no flow control

    // enable TX and RX (AUX_MU_CNTL_REG)
    PUT32(LIBUART_AUX_MU_CNTL_REG, GET32(LIBUART_AUX_MU_CNTL_REG) | 0b11);

    dev_barrier();
}

// disable the libuart.
void libuart_disable(void) {
    dev_barrier();

    // check LIBUART is empty and not idle
    libuart_flush_tx();

    PUT32(LIBUART_AUX_ENABLES, GET32(LIBUART_AUX_ENABLES) & ~0b1);

    dev_barrier();
}

// 1 = at least one byte on rx queue, 0 otherwise
static int libuart_can_getc(void) {
    dev_barrier();

    int can_getc = GET32(LIBUART_AUX_MU_LSR_REG) & 0b1;

    dev_barrier();

    return can_getc;
}

// returns one byte from the rx queue, if needed
// blocks until there is one.
int libuart_getc(void) {
	dev_barrier();

    while (!libuart_can_getc());

    int retval = GET32(LIBUART_AUX_MU_IO_REG) & 0xff;

    dev_barrier();

    return retval;
}

// 1 = space to put at least one byte, 0 otherwise.
int libuart_can_putc(void) {
    dev_barrier();

    int can_putc = (GET32(LIBUART_AUX_MU_LSR_REG) & 0b100000) != 0;

    dev_barrier();

    return can_putc;
}

// put one byte on the tx qqueue, if needed, blocks
// until TX has space.
void libuart_putc(unsigned c) {
    dev_barrier();

    while (!libuart_can_putc());

    PUT32(LIBUART_AUX_MU_IO_REG, c & 0xff);

    dev_barrier();
}

// simple wrapper routines useful later.

// a maybe-more intuitive name for clients.
int libuart_has_data(void) {
    return libuart_can_getc();
}


// return -1 if no data, otherwise the byte.
int libuart_getc_async(void) { 
    if(!libuart_has_data())
        return -1;
    return libuart_getc();
}

// 1 = tx queue empty, 0 = not empty.
int libuart_tx_is_empty(void) {
    dev_barrier();

    int tx_is_empty = (GET32(LIBUART_AUX_MU_LSR_REG) & 0b1000000) != 0;

    dev_barrier();

    return tx_is_empty;
}

// flush out all bytes in the libuart --- we use this when 
// turning it off / on, etc.
void libuart_flush_tx(void) {
    while(!libuart_tx_is_empty())
        ;
}
