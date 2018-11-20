#include <stdint.h>
#include <linux/fb.h>
#include "snake.h"
#include <stdio.h>

struct snake_t {
    struct fb_copyarea body[SNAKE_MAX_LENGTH];	// Array of entire snake (body and head)
    uint16_t head_index;			// Position of the snake head
    uint16_t tail_index;			// Position of the snake tail
    uint16_t length;				// Array of entire snake (body and head)
    uint16_t direction;			// Keep track of which way snake is moving
    int unsigned score;
};

static struct snake_t snake;
static unsigned int new_direction;


void setup_snake() {
    snake.length = SNAKE_START_LENGTH;
    snake.tail_index = 0;
    snake.head_index = SNAKE_START_LENGTH - 1;
/* Initialize snake blocks */
    for (int sb = 0; sb < SNAKE_MAX_LENGTH; sb++) {
        snake.body[sb].dx = SCREEN_WIDTH/2;	// TODO : Do something smart here, to avoid...
        snake.body[sb].dy = SCREEN_HEIGHT/2;	// ...overwriting blocks that we don't want to clear
        snake.body[sb].width = DELTA_X;  // Also, consider making the position random.
        snake.body[sb].height = DELTA_Y; // Or just make eash snake start in its own corner
    }
    snake.length = SNAKE_START_LENGTH;
    snake.direction = DOWN;
    new_direction = DOWN;
}


/**
* Calculate the new position of the snake head based on the direction and 
* previous head position.
*/
void move_snake() {
    int new_x = snake.body[snake.head_index].dx;
    int new_y = snake.body[snake.head_index].dy;
    snake.direction = new_direction;
    switch(snake.direction) {
    case LEFT:
        new_x -= DELTA_X;
        break;
    case RIGHT:
        new_x += DELTA_X;
        break;
    case UP:
        new_y -= DELTA_Y;
        break;
    case DOWN:
        new_y += DELTA_Y;
        break;
    }

    /* Update the snake head (index and block) and tail  (index only) */
    snake.tail_index = (snake.tail_index + 1) % SNAKE_MAX_LENGTH;
    snake.head_index = (snake.head_index + 1) % SNAKE_MAX_LENGTH;
    snake.body[snake.head_index].dx = new_x;
    snake.body[snake.head_index].dy = new_y;
    
}

int snake_grow() {
    ++snake.score;
    /* Victory! Don't grow */
    if (++(snake.length) >= SNAKE_MAX_LENGTH) {
        return 0;
    }
    /* Set tail to tail-1, so that we don't overwrite the tail on next move_snake */
    snake.tail_index = ( snake.tail_index == 0 ? SNAKE_MAX_LENGTH-1 : snake.tail_index-1 );
    return 1;
}


 void change_direction(int change_dir) {
    // Don't allow changing direction 180 degrees
    switch (snake.direction) {
    case LEFT:
	new_direction = (change_dir == RIGHT ? LEFT : change_dir);
	break;
    case RIGHT:
	new_direction = (change_dir == LEFT ? RIGHT : change_dir);
	break;
    case UP:
	new_direction = (change_dir == DOWN ? UP : change_dir);
	break;
    case DOWN:
	new_direction = (change_dir == UP ? DOWN : change_dir);
	break;
    default:
  	new_direction = DOWN;
    }
}


void print_state() {
    printf("score: %u\n", snake.score);
}

/**
 *Getter functions 
 */
unsigned int get_score() {
    return snake.score;
}
uint16_t get_snake_length() {
    return snake.length;
}
uint16_t get_snake_direction() {
    return new_direction;
}
struct fb_copyarea* get_snake_head() {
    return &snake.body[snake.head_index];
}
 struct fb_copyarea* get_snake_tail() {
    return &snake.body[snake.tail_index];
}
uint16_t head_index() {
    return snake.head_index;
}
uint16_t tail_index() {
    return snake.tail_index;
}
struct fb_copyarea * get_body_part(size_t body_index) {
    if (body_index < SNAKE_MAX_LENGTH)
        return &snake.body[body_index];
    return NULL;
}
   
