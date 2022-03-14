#define WIDTH 280
#define HEIGHT 180

#define PADDLE_HEIGHT 30
#define PADDLE_WIDTH 10
#define BALL_SIZE 10

#define SCORE_WIDTH 30

//#define INPUT_PIN 20
#define OUTPUT_PIN 21
#define CLOCK_PIN 26
//#define SEND_ACK 26
//#define RECEIVE_ACK 19
//#define READY_PIN 12
#define READ_DELAY 1500

typedef struct {
    uint8_t ball_x; // left
    uint8_t ball_y; // top
    uint8_t player_y; // top
    uint8_t comp_y; // top
    uint8_t player_score;
    uint8_t comp_score;
} game_info_t;
