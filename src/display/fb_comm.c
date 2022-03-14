#include "gl.h"
#include "rpi.h"
#include "nrf.h"
#include "fb_comm.h"
#include "libc/bit-support.h"
//#include "libuart.h"

game_info_t info;

uint8_t data_pins[] = {
    20, // orange
    19, // white
    16, // brown
    13, // red
    12, // yellow
    6   // grey
};

void get_info(void) {
    //output("Waiting for info...\n");
    while (gpio_read(CLOCK_PIN) == 0);
    while (gpio_read(CLOCK_PIN) == 1);
    //output("Received start signal. Reading info...\n");
    //delay_us(READ_DELAY);
    uint8_t data[] = {0, 0, 0, 0, 0, 0};
    //gl_clear(GL_BLACK);
    uint32_t buf[] = {0, 0, 0, 0, 0, 0};
    for (int i = 0; i < 8; i++) {
        while (gpio_read(CLOCK_PIN) == 0);
        for (int j = 0; j < 6; j++) {
            if (gpio_read(data_pins[j])) buf[j] = bit_set(buf[j], i);
        }
        while (gpio_read(CLOCK_PIN) == 1);

            /*gpio_write(OUTPUT_PIN, 1);
            delay_us(READ_DELAY); 
            gpio_write(OUTPUT_PIN, 0);*/
            //delay_us(READ_DELAY);
            //output("Read i=%d, j=%d\n", i, j);
    }
    for (int i = 0; i < 6; i++) {
        data[i] = (uint8_t) buf[i];
    }
    //output("Received all data.\n");
    info.ball_x = data[0];
    info.ball_y = data[1];
    info.player_y = data[2];
    info.comp_y = data[3];
    info.player_score = data[4];
    info.comp_score = data[5];
}

void create_gl() {
    gl_init(WIDTH, HEIGHT, GL_DOUBLEBUFFER);
    gl_clear(GL_BLACK);
    gl_swap_buffer();
}

void test_fb_loop(void) {
    create_gl();
    unsigned i = 0;
    while (1) {
        gl_clear(GL_BLACK);
        gl_draw_char(50, 50, ((i++) % 10) + '0', GL_WHITE);
        gl_swap_buffer(); 
        delay_us(1000000);
    }
}

void receive_msgs_and_draw(void) {
    //create_gl();
    /*gl_draw_char(100, 10, 'w', GL_WHITE);
    gl_swap_buffer();

    delay_ms(10000);*/

    while (1) {
        get_info();

        gl_clear(GL_BLACK);
        
        gl_draw_rect(WIDTH - SCORE_WIDTH + 5, 0, 1, HEIGHT, GL_WHITE);
        gl_draw_rect(0, 0, 1, HEIGHT, GL_WHITE);
        gl_draw_rect(0, 0, WIDTH - SCORE_WIDTH + 5, 1, GL_WHITE);
        gl_draw_rect(0, HEIGHT-1, WIDTH - SCORE_WIDTH + 5, 1, GL_WHITE);
        
        gl_draw_rect(info.ball_x, info.ball_y, BALL_SIZE, BALL_SIZE, GL_WHITE);
        gl_draw_rect(0, info.player_y, PADDLE_WIDTH, PADDLE_HEIGHT, GL_CARDINAL);
        gl_draw_rect(WIDTH - PADDLE_WIDTH - SCORE_WIDTH, info.comp_y, PADDLE_WIDTH, PADDLE_HEIGHT, GL_STANFORD_GREEN);

        gl_draw_char(WIDTH - SCORE_WIDTH + 10, HEIGHT / 4, info.player_score + '0', GL_CARDINAL);
        gl_draw_char(WIDTH - SCORE_WIDTH + 10, 3 * HEIGHT / 4, info.comp_score + '0', GL_STANFORD_GREEN);

        gl_swap_buffer();

        /*output("----------------\n");
        output("ball x: %d = %b\n", info.ball_x, info.ball_x);
        output("ball y: %d = %b\n", info.ball_y, info.ball_y);
        output("player y: %d = %b\n", info.player_y, info.player_y);
        output("comp y: %d = %b\n", info.comp_y, info.comp_y);
        output("player score: %d = %b\n", info.player_score, info.player_score);
        output("comp score: %d = %b\n", info.comp_score, info.comp_score);
        output("----------------\n");*/

        /*gl_draw_char(50, 10, (info.ball_x % 10) + '0', GL_WHITE);
        gl_draw_char(50, 30, (info.ball_y % 10) + '0', GL_WHITE);
        gl_draw_char(50, 50, (info.player_y % 10) + '0', GL_WHITE);
        gl_draw_char(50, 70, (info.comp_y % 10) + '0', GL_WHITE);
        gl_draw_char(50, 90, (info.player_score % 10) + '0', GL_WHITE);
        gl_draw_char(50, 110, (info.comp_score % 10) + '0', GL_WHITE);*/

        //gl_swap_buffer();

        gpio_write(OUTPUT_PIN, 1);
        delay_us(READ_DELAY); 
        gpio_write(OUTPUT_PIN, 0);
    }
}

void notmain(void) {
    gpio_set_output(OUTPUT_PIN);
    gpio_set_input(CLOCK_PIN);
    for (int i = 0; i < 6; i++)
        gpio_set_input(data_pins[i]);
    //test_fb_loop();
    create_gl();
    gpio_write(OUTPUT_PIN, 1);
    while (gpio_read(CLOCK_PIN) == 0);
    gpio_write(OUTPUT_PIN, 0);
    receive_msgs_and_draw();

}

