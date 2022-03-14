#include "rpi.h"
#include "libc/bit-support.h"

#define SCREEN_WIDTH 250
#define SCREEN_HEIGHT 180

#define BALL_WIDTH 10
#define BALL_HEIGHT 10

#define PADDLE_OFFSET 10
#define PADDLE_WIDTH 10
#define PADDLE_HEIGHT 30
#define PADDLE_Y 90

#define WIN_SCORE 8

#define CONTROLLER_UP 21
#define CONTROLLER_DOWN 20

#define DISPLAY_ACK 26
#define DISPLAY_CLOCK 16

#define WRITE_DELAY 1500

#define SPEED 2
#define MAXSPEED 15

int16_t display_xmits[] = {
    19,
    13,
    12,
    6,
    5,
    7
};

typedef struct ball {
    int16_t x, y;
    int16_t dx, dy;
} ball_t;

typedef struct paddle {
    int16_t x, y;
} paddle_t;

ball_t ball;
paddle_t paddles[2];

int16_t player_score;
int16_t ai_score;

int16_t towards;

void game_init() {
    player_score = 0;
    ai_score = 0;
    towards = -SPEED;
}

/* towards = -1 for towards player and 1 for towards AI */
void point_start(int16_t towards) {
    ball.x = SCREEN_WIDTH / 2;
    ball.y = SCREEN_HEIGHT / 2;

    paddles[0].x = 0;
    paddles[0].y = PADDLE_Y;

    paddles[1].x = SCREEN_WIDTH - PADDLE_WIDTH;
    paddles[1].y = PADDLE_Y;

    ball.dx = towards;
    ball.dy = 0;
}

int check_ball_paddle_collision(paddle_t* paddle) {
    if (ball.x > paddle->x + PADDLE_WIDTH) {
        return 0;
    }

    if (ball.x + BALL_WIDTH < paddle->x) {
        return 0;
    }

    if (ball.y > paddle->y + PADDLE_HEIGHT) {
        return 0;
    }

    if (ball.y + BALL_HEIGHT < paddle->y) {
        return 0;
    }

    return 1;
}

void move_ball() {
    if (ball.y <= 0 || ball.y >= SCREEN_HEIGHT - BALL_HEIGHT)
        ball.dy *= -1;

    if (ball.y < 0)
        ball.y = 0;
    if (ball.y + BALL_HEIGHT > SCREEN_HEIGHT)
        ball.y = SCREEN_HEIGHT - BALL_HEIGHT;
    
    for (int i = 0; i < 2; i++) {
        if (check_ball_paddle_collision(&paddles[i])) {
            output("COLLISION DETECTED ON PADDLE %d\n", i);
            if (i == 0)
                ball.x = paddles[0].x + PADDLE_WIDTH + 1;
            else
                ball.x = paddles[1].x - BALL_WIDTH - 1;
            ball.dx += (ball.dx >= 0) ? 1 : -1;
            ball.dx *= -1;

            if (ball.dx < -MAXSPEED)
                ball.dx = -MAXSPEED;
            
            if (ball.dx > MAXSPEED)
                ball.dx = MAXSPEED;

            int16_t hit_pos = paddles[i].y + PADDLE_HEIGHT/2 - ball.y - BALL_HEIGHT/2;

            // gross
            ball.dy = -(hit_pos + 2) / 4;            
        }
    }

    if (ball.x < paddles[0].x + PADDLE_WIDTH && ball.dx < 0) {
        output("AI WINS: ball %d %d, paddle %d %d\n", ball.x, ball.y, paddles[0].x, paddles[0].y);
        ai_score++;
        towards *= -1;
        point_start(towards);
        return;
    }

    if (ball.x + BALL_WIDTH > paddles[1].x && ball.dx > 0) {
        output("PLAYER WINS: ball %d %d, paddle %d %d\n", ball.x, ball.y, paddles[1].x, paddles[1].y);
        player_score++;
        towards *= -1;
        point_start(towards);
        return;
    }

    ball.x += ball.dx;
    if (ball.x <= paddles[0].x + PADDLE_WIDTH - 2)
        ball.x = paddles[0].x + PADDLE_WIDTH - 2;
    if (ball.x >= paddles[1].x - PADDLE_WIDTH + 2)
        ball.x = paddles[1].x - PADDLE_WIDTH + 2;
    ball.y += ball.dy;
}

void ai_move_paddle() {
    int16_t paddle_ctr = paddles[1].y + (PADDLE_HEIGHT / 2);
    int16_t screen_ctr = SCREEN_HEIGHT / 2;
    int16_t ball_ctr = ball.y + (BALL_HEIGHT / 2);

    int16_t spd = 3;

    if (ball.dx < 0) {
        paddles[1].y += (paddle_ctr < screen_ctr) ? spd : -spd;
    } else {
        paddles[1].y += (paddle_ctr < ball_ctr) ? spd : -spd;
        
        if (paddles[1].y >= SCREEN_HEIGHT - PADDLE_HEIGHT)
            paddles[1].y = SCREEN_HEIGHT - PADDLE_HEIGHT;
        
        if (paddles[1].y <= 0)
            paddles[1].y = 0;
    }
}

/* -1 down, 1 up */
void move_paddle(int16_t dir) {
    if (dir == -1) {
        if (paddles[0].y >= SCREEN_HEIGHT - PADDLE_HEIGHT)
            paddles[0].y = SCREEN_HEIGHT - PADDLE_HEIGHT;
        else
            paddles[0].y += 5;
    } else {
        if (paddles[0].y <= 0)
            paddles[0].y = 0;
        else
            paddles[0].y -= 5;
    }
}

void notmain() {
    game_init();
    point_start(-SPEED);

    for (int i = 0; i < 6; i++)
        gpio_set_output(display_xmits[i]);

    gpio_set_output(DISPLAY_CLOCK);
    gpio_set_input(DISPLAY_ACK);

    gpio_write(DISPLAY_CLOCK, 0);

    gpio_set_on(DISPLAY_CLOCK);
    while (gpio_read(DISPLAY_ACK) == 0);
    gpio_set_off(DISPLAY_CLOCK);

    while (1) {
        uint8_t up_signal = gpio_read(CONTROLLER_UP);
        uint8_t down_signal = gpio_read(CONTROLLER_DOWN);

        if (up_signal) {
            move_paddle(1);
        } else if (down_signal) {
            move_paddle(-1);
        }
        
        if (player_score == 8 || ai_score == 8) {
            game_init();
            point_start(-1);
            continue;
        }

        ai_move_paddle();
        move_ball();

        gpio_write(DISPLAY_CLOCK, 1);
        delay_us(WRITE_DELAY);
        gpio_write(DISPLAY_CLOCK, 0);

        uint8_t state[] = {
            ball.x,
            ball.y,
            paddles[0].y,
            paddles[1].y,
            player_score,
            ai_score
        };

        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 6; j++) {
                gpio_write(display_xmits[j], bit_isset(state[j], i));
            }
            delay_us(WRITE_DELAY);
            gpio_write(DISPLAY_CLOCK, 1);
            delay_us(WRITE_DELAY);
            gpio_write(DISPLAY_CLOCK, 0);
        }

        for (int i = 0; i < 6; i++)
            gpio_write(display_xmits[i], 0);

        while (gpio_read(DISPLAY_ACK) == 0);
        while (gpio_read(DISPLAY_ACK) == 1);
    }
}