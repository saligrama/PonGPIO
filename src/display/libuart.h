#ifndef __LIBUART_H__
#define __LIBUART_H__

// never use this directly!
static inline struct aux_periphs *libuart_get(void) { return (void*)0x20215040; }

// initialize
void libuart_init(void);
// disable
void libuart_disable(void);

// get one byte from the libuart
int libuart_getc(void);
// put one byte on the libuart
void libuart_putc(unsigned c);

// returns -1 if no byte, the value otherwise.
int libuart_getc_async(void);

// 0 = no data, 1 = at least one byte
int libuart_has_data(void);
// 0 = no space, 1 = space for at least 1 byte
int libuart_can_putc(void);

// flush out the tx fifo
void libuart_flush_tx(void);

#endif
