// engler, cs140 put your gpio implementations in here.
#include "rpi.h"

#define GPIO_IRQ_PENDING_REG 0x2000B208
#define GPIO_IRQ_ENABLES_REG 0x2000B214
#define GPIO_GPEDS0 0x20200040
#define GPIO_GPREN0 0x2020004C
#define GPIO_GPFEN0 0x20200058

// gpio_int_rising_edge and gpio_int_falling_edge (and any other) should
// call this routine (you must implement) to setup the right GPIO event.
// as with setting up functions, you should bitwise-or in the value for the 
// pin you are setting with the existing pin values.  (otherwise you will
// lose their configuration).  you also need to enable the right IRQ.   make
// sure to use device barriers!!
int is_gpio_int(unsigned gpio_int) {
    if (
        gpio_int != GPIO_INT0 &&
        gpio_int != GPIO_INT1 &&
        gpio_int != GPIO_INT2 &&
        gpio_int != GPIO_INT3
    ) {
        panic("bad gpio_int parameter %d", gpio_int);
    }

    dev_barrier();

    unsigned irq = GET32(GPIO_IRQ_PENDING_REG);

    dev_barrier();

    return (irq & (1 << (gpio_int - 32))) != 0;
}


// p97 set to detect rising edge (0->1) on <pin>.
// as the broadcom doc states, it  detects by sampling based on the clock.
// it looks for "011" (low, hi, hi) to suppress noise.  i.e., its triggered only
// *after* a 1 reading has been sampled twice, so there will be delay.
// if you want lower latency, you should us async rising edge (p99)
void gpio_int_rising_edge(unsigned pin) {
    if (pin >= 32)
            return;

    dev_barrier();
    PUT32(GPIO_GPREN0, GET32(GPIO_GPREN0) | (1 << pin));
    dev_barrier();
    PUT32(GPIO_IRQ_ENABLES_REG, GET32(GPIO_IRQ_ENABLES_REG) | (1 << (GPIO_INT0 - 32)));
    dev_barrier();
}

// p98: detect falling edge (1->0).  sampled using the system clock.  
// similarly to rising edge detection, it suppresses noise by looking for
// "100" --- i.e., is triggered after two readings of "0" and so the 
// interrupt is delayed two clock cycles.   if you want  lower latency,
// you should use async falling edge. (p99)
void gpio_int_falling_edge(unsigned pin) {
    if (pin >= 32)
        return;

    dev_barrier();
    PUT32(GPIO_GPFEN0, GET32(GPIO_GPFEN0) | (1 << pin));
    dev_barrier();
    PUT32(GPIO_IRQ_ENABLES_REG, GET32(GPIO_IRQ_ENABLES_REG) | (1 << (GPIO_INT0 - 32)));
    dev_barrier();
}

// p96: a 1<<pin is set in EVENT_DETECT if <pin> triggered an interrupt.
// if you configure multiple events to lead to interrupts, you will have to 
// read the pin to determine which caused it.
int gpio_event_detected(unsigned pin) {
    if (pin >= 32)
        return -1;

    dev_barrier();
    int retval = (GET32(GPIO_GPEDS0) & (1 << pin)) != 0;
    dev_barrier();
    return retval;
}

// p96: have to write a 1 to the pin to clear the event.
void gpio_event_clear(unsigned pin) {
    if (pin >= 32)
        return;

    dev_barrier();
    PUT32(GPIO_GPEDS0, 1 << pin);
    dev_barrier();
}
