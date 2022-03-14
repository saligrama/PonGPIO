#include "rpi.h"
#include "nes_controller.h"

#define DATA_PIN 24
#define CLOCK_PIN 25
#define LATCH_PIN 8

#define UP_PIN 21
#define DOWN_PIN 20

void notmain(void) {
	gpio_set_output(CLOCK_PIN);
	gpio_set_output(LATCH_PIN);
	gpio_set_input(DATA_PIN);

	gpio_set_output(UP_PIN);
	gpio_set_output(DOWN_PIN);
	
	nes_dev_t dev = {
		.data = DATA_PIN,
		.clock = CLOCK_PIN,
		.latch = LATCH_PIN
	};

	while (1) {
		nes_input_t input = read_input(&dev);
		
		if (input.up)
			gpio_set_on(UP_PIN);
		else
			gpio_set_off(UP_PIN);

		if (input.down)
			gpio_set_on(DOWN_PIN);
		else
			gpio_set_off(DOWN_PIN);

		if (input.start)
			clean_reboot();
	}
}
