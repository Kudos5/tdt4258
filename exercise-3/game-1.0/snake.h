#include <stdlib.h>

#define SNAKE_MAX_LENGTH 10
#define SNAKE_START_LENGTH 5
#define SNAKE_COLOR 0xFFFF
#define DELTA_X 10
#define DELTA_Y 10
#define FOOD_SIZE 10

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

enum { UP, DOWN, LEFT, RIGHT };



uint16_t get_snake_length();
uint16_t get_snake_direction();
struct fb_copyarea* get_snake_head();
struct fb_copyarea* get_snake_tail();
uint16_t head_index();
uint16_t tail_index();
struct fb_copyarea * get_body_part(size_t body_index);
unsigned int get_score();

void setup_snake();
void move_snake();
int snake_grow();
void change_direction(int change_dir);

void print_state();
