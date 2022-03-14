#include "rpi.h"
#include "nes_controller.h"

#define DATA_PIN 24
#define CLOCK_PIN 25
#define LATCH_PIN 8

void notmain(void) {
	gpio_set_output(CLOCK_PIN);
	gpio_set_output(LATCH_PIN);
	gpio_set_input(DATA_PIN);
	
	nes_dev_t dev = {
		.data = DATA_PIN,
		.clock = CLOCK_PIN,
		.latch = LATCH_PIN
	};

	for (int i = 0; i < 10000; i++) {
		nes_input_t input = read_input(&dev);
		
		print_input(input);

		delay_ms(1000);
	}

	clean_reboot();
}
