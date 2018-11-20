#include <stdlib.h>
#include <stdio.h>
#include <linux/fb.h>
#include <sys/stat.h>
#include <linux/fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <signal.h>
#include <sys/mman.h>
#include <linux/fs.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

#include "driver-gamepad-1.0/driver-gamepad.h"
#include "snake.h"

#define GAME_TIMER_MAX 250
#define GAME_TIMER_MIN 50

static struct fb_copyarea game_food = {
    .width = FOOD_SIZE,
    .height = FOOD_SIZE,
};

static int gp_fd;
static int fb_fd;

static uint16_t * game_screen;

static int flag_button_pressed;
static int flag_update_screen_timer;

static int game_button_state;

static uint16_t const game_background_colour = 0x0000;
static uint16_t const game_cursor_colour = 0xFFFF;
static uint16_t const game_food_colour = 0xF000;
static int unsigned game_timer_ms = 250;
static int game_food_eaten = 1;

// Function declarations
static void set_area(struct fb_copyarea * area, uint16_t colour);

static void game_over() {
    printf("Game over\n");
    printf("Score: %u\n", get_score());
}

static void setup_timer(int ms) {
    int ret;
    struct timeval time_val = {
        .tv_sec = 0,
        .tv_usec = ms*1000,
    };
    struct itimerval itimer_val = {
        .it_interval = time_val,
        .it_value = time_val,
    };
    if ( (ret = setitimer(ITIMER_REAL, &itimer_val, NULL)) != 0 ) {
        printf("failed to setitimer\n");
    }
}

static int detect_collision(struct fb_copyarea * area_a, struct fb_copyarea * area_b) {
    int a_x0 = area_a->dx;
    int a_x1 = area_a->dx + area_a->width;;
    int a_y0 = area_a->dy;
    int a_y1 = area_a->dy + area_a->height;;
    int b_x0 = area_b->dx;
    int b_x1 = area_b->dx + area_a->width;;
    int b_y0 = area_b->dy;
    int b_y1 = area_b->dy + area_a->height;;
    // b is to the left of a
    if ( b_x1 <= a_x0 ) {
        return 0;
    }
    // a is to the left of b
    else if ( a_x1 <= b_x0 ) {
        return 0;
    }
    // b is above a
    if ( b_y1 <= a_y0 ) {
        return 0;
    }
    // a is above b
    else if ( a_y1 <= b_y0 ) {
        return 0;
    }
    return 1;
}
static int detect_collision_edge() {
    struct fb_copyarea * snake_head = get_snake_head();
    switch (get_snake_direction()) {
        case UP:
            return snake_head->dy == 0;
            break;
        case DOWN:
            return snake_head->dy >= (SCREEN_HEIGHT - DELTA_Y);
            break;
        case LEFT:
            return snake_head->dx == 0;
            break;
        case RIGHT:
            return snake_head->dx >= (SCREEN_WIDTH - DELTA_X);
            break;
    };
    return 0;
}

static int detect_collision_self() {
    struct fb_copyarea * snake_head = get_snake_head();
    for ( size_t j = tail_index(); j != head_index(); j = ((j+1)%SNAKE_MAX_LENGTH) ) {
        // Make sure to skip the head
        if ( detect_collision(snake_head, get_body_part(j)) ) {
            return 1;
        }
    }
    return 0;
}

static int detect_collision_food() {
    struct fb_copyarea * snake_head = get_snake_head();
    return detect_collision(snake_head, &game_food);
}

void spawn_food() {
    // TODO: Do not spawn food on top of the snake
    int dx = rand() % (SCREEN_WIDTH-FOOD_SIZE);
    int dy = rand() % (SCREEN_HEIGHT-FOOD_SIZE);
    // Snap the position to a 10x10 grid
    dx = (dx/10)*10;
    dy = (dy/10)*10;
    game_food.dx = dx;
    game_food.dy = dy;
    set_area(&game_food, game_food_colour);
}

void update_screen() {
    if ( game_food_eaten ) {
        spawn_food();
        game_food_eaten = 0;
    }
}

void alarm_handler(int signum) {
    // Just to avoid warnings
    signum = signum;
    flag_update_screen_timer = 1;
}

void input_handler(int signum) {
    signum = signum;
    game_button_state = ioctl(gp_fd, GP_IOCTL_GET_BUTTON_STATE);
    flag_button_pressed = 1;
}

void setup_gamepad(void) {
    int oflags;
    gp_fd = open("/dev/gamepad", O_RDWR);
    if ( gp_fd < 0 ) {
        printf("Failed to open file\n");
        exit(EXIT_FAILURE);
    }

    // Subscribe to signals from gamepad
    signal(SIGIO, &input_handler);
    fcntl(gp_fd, F_SETOWN, getpid( ));
    oflags = fcntl(gp_fd, F_GETFL);
    fcntl(gp_fd, F_SETFL, oflags | FASYNC);
}

void set_area(struct fb_copyarea * area, uint16_t colour) {
    int x_max = area->dx + area->width;
    int y_max = area->dy + area->height;
    for ( int y = area->dy; y < y_max; ++y ) {
        for ( int x = area->dx; x < x_max; ++x ) {
            size_t index = (y*SCREEN_WIDTH + x);
            game_screen[index] = colour;
        }
    }
    ioctl(fb_fd, 0x4680, area);
}

void clear_area(struct fb_copyarea * area) {
    set_area(area, game_background_colour);
}

void clear_screen() {
    struct fb_copyarea rect;
    rect.dx = 0;
    rect.dy = 0;
    rect.width = SCREEN_WIDTH;
    rect.height = SCREEN_HEIGHT;
    clear_area(&rect);
}

#define BTN_L_LEFT(btn_state) ((btn_state) & 0x01)
#define BTN_L_UP(btn_state) ((btn_state) & 0x02)
#define BTN_L_RIGHT(btn_state) ((btn_state) & 0x04)
#define BTN_L_DOWN(btn_state) ((btn_state) & 0x08)


static inline void button_action(int button_state) {
    static int prev_button_state = 0xFF;

    int newly_pressed = (prev_button_state ^ button_state) & prev_button_state;
    prev_button_state = button_state;

    if (BTN_L_LEFT(newly_pressed))
	change_direction(LEFT);
    if (BTN_L_RIGHT(newly_pressed))
	change_direction(RIGHT);
    if (BTN_L_UP(newly_pressed))
	change_direction(UP);
    if (BTN_L_DOWN(newly_pressed))
	change_direction(DOWN);
}

int main()
{
    srand(time(NULL));

    fb_fd = open("/dev/fb0", O_RDWR);
    if ( fb_fd == -1 ) {
        printf("Failed to open file\n");
        exit(EXIT_FAILURE);
    }

    // Let's map the file descriptor to a memory location
    game_screen = mmap(NULL, SCREEN_WIDTH*SCREEN_HEIGHT*2, PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if ( game_screen == MAP_FAILED ) {
        printf("Failed to map memory: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    setup_gamepad();
    // Setup a signal handler for the alarm signal which is used to periodically update
    // the screen
    setup_timer(GAME_TIMER_MAX);
    signal(SIGALRM, &alarm_handler);
    clear_screen();
    setup_snake();
	printf("Game started\n");
    while (1) {
        pause();
        if ( flag_button_pressed ) {
            button_action(game_button_state);
            flag_button_pressed = 0;
        } 
        if (flag_update_screen_timer) {
            if ( detect_collision_edge() ) {
                printf("Collided with edge\n");
                game_over();
                break;
            }
            clear_area(get_snake_tail());
            move_snake();
            set_area(get_snake_head(), SNAKE_COLOR);
            if ( detect_collision_self() ) {
                printf("Collided with self\n");
                game_over();
                break;
            }
            if ( detect_collision_food() ) {
                game_food_eaten = 1;
                print_state();
                /* If snake has reached SNAKE_MAX_LENGTH*/
                if (!snake_grow()) {
                    printf("You win\n");
                    game_over();
                    break;
                }
                if ( game_timer_ms > GAME_TIMER_MIN ) {
                    game_timer_ms = GAME_TIMER_MAX - get_score()*10;
                    setup_timer(game_timer_ms);
                }
            }
            update_screen();
            flag_update_screen_timer = 0;
        }
    }

    // Clean up
    int ret;
    ret = close(fb_fd);
    if ( ret == -1 ) {
        printf("Failed to close file\n");
        exit(EXIT_FAILURE);
    }

    ret = close(gp_fd);
    if ( ret == -1 ) {
        printf("Failed to close file\n");
        exit(EXIT_FAILURE);
    }

	exit(EXIT_SUCCESS);
}
