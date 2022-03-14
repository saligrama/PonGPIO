#include "nes_controller.h"
#include "rpi.h"

nes_input_t read_input(nes_dev_t* dev) {
	// pulse latch to tell controller we want data
	gpio_set_on(dev->latch);
	delay_us(LATCH_DELAY);
	gpio_set_off(dev->latch);

	unsigned char ret = 0;
	for (int i = 0; i < 8; i++) {
		ret |= gpio_read(dev->data) << i;

		delay_us(CLOCK_DELAY);
		gpio_set_on(dev->clock);
		delay_us(CLOCK_DELAY);
		gpio_set_off(dev->clock);
	}

	ret = ~ret;
	return *(nes_input_t*)&ret;
}

void print_input(nes_input_t input) {
	output("---INPUT---\n");
	output(" A:      %d\n", input.a);
	output(" B:      %d\n", input.b);
	output(" Select: %d\n", input.select);
	output(" Start:  %d\n", input.start);
	output(" Up:     %d\n", input.up);
	output(" Down:   %d\n", input.down);
	output(" Left:   %d\n", input.left);
	output(" Right:  %d\n", input.right);
	output("-----------\n");
}
