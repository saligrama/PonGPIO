#ifndef _NES_CONTROLLER_H_
#define _NES_CONTROLLER_H_

/*
 * On the NES controller I have:
 * orange = data
 * blue = clock
 * yellow = latch
 */

// holds input pattern
typedef struct nes_input {
	unsigned char a:1;
	unsigned char b:1;
	unsigned char select:1;
	unsigned char start:1;
	unsigned char up:1;
	unsigned char down:1;
	unsigned char left:1;
	unsigned char right:1;
} nes_input_t;

// holds gpio pins
typedef struct nes_dev {
	unsigned data;
	unsigned clock;
	unsigned latch;
} nes_dev_t;

// important defines
#define LATCH_DELAY 12
#define CLOCK_DELAY 6

nes_input_t read_input(nes_dev_t* dev);

void print_input(nes_input_t input);

#endif
