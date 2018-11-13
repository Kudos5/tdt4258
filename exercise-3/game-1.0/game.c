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

#include "driver-gamepad-1.0/driver-gamepad.h"

#define NUM_PLAYERS 1	// TODO : Consider making this dynamic, so that # players can be decided in a menu?
#define SNAKE_LENGTH 5
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define DELTA_X 10
#define DELTA_Y 10

static int gp_fd;
static int fb_fd;

static uint16_t * game_screen;

static int flag_button_pressed;
static int flag_update_screen_timer;

static int game_button_state;

static uint16_t const game_background_colour = 0x0000;
static uint16_t const game_cursor_colour = 0xFFFF;
static uint16_t const game_food_colour = 0xF000;
static int game_food_eaten = 1;
static int game_cursor_direction;

static unsigned int snake_array_index;

#define FOOD_SIZE 10

// Function declarations
static void SetArea(struct fb_copyarea * area, uint16_t colour);

static void SetupTimer() {
    int ret;
    struct timeval time_val = {
        .tv_sec = 0,
        .tv_usec = 250*1000,
    };
    struct itimerval itimer_val = {
        .it_interval = time_val,
        .it_value = time_val,
    };
    if ( (ret = setitimer(ITIMER_REAL, &itimer_val, NULL)) != 0 ) {
        printf("failed to setitimer\n");
    }
}

static int DetectCollision(struct fb_copyarea area_a, struct fb_copyarea area_b) {
    int a_x0 = area_a.dx;
    int a_x1 = area_a.dx + area_a.width;;
    int a_y0 = area_a.dy;
    int a_y1 = area_a.dy + area_a.height;;
    int b_x0 = area_b.dx;
    int b_x1 = area_b.dx + area_a.width;;
    int b_y0 = area_b.dy;
    int b_y1 = area_b.dy + area_a.height;;
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

static struct fb_copyarea game_food = {
    .width = FOOD_SIZE,
    .height = FOOD_SIZE,
};

void SpawnFood() {
    int dx = rand() % (SCREEN_WIDTH-FOOD_SIZE);
    int dy = rand() % (SCREEN_HEIGHT-FOOD_SIZE);
    // Snap the position to a 10x10 grid
    dx = (dx/10)*10;
    dy = (dy/10)*10;
    game_food.dx = dx;
    game_food.dy = dy;
    SetArea(&game_food, game_food_colour);
}

void UpdateScreen() {
    if ( game_food_eaten ) {
        SpawnFood();
        game_food_eaten = 0;
    }
}

void alarm_handler(int signum) {
    // Just to avoid warnings
    signum = signum;
    flag_update_screen_timer = 1;
}

enum { UP, DOWN, LEFT, RIGHT };
struct Player {
    struct fb_copyarea snake_head;		// Position of the snake head
    struct fb_copyarea snake[SNAKE_LENGTH];	// Array of entire snake (body and head)
    uint16_t color;
    int direction;				// Keep track of which way snake is moving
};
    
static struct Player players[NUM_PLAYERS];

void SetupPlayers() {
    /* This one looks messy when using fb_copyareas for all snake blocks, 
     * but I think it makes things easier other places in the code  */
    for (int p = 0; p < NUM_PLAYERS; p++) {
	    players[p].snake_head.width = DELTA_X;
	    players[p].snake_head.height = DELTA_Y;
	    players[p].snake_head.dx = 0;	/* TODO : Make the position random */
	    players[p].snake_head.dy = 0;	/* OR just make each snake start in its own corner */
	/* Initialize snake blocks */
        for (int sb = 0; sb < SNAKE_LENGTH; sb++) {
            players[p].snake[sb].dx = 0;	// TODO : Do something smart here, to avoid...
            players[p].snake[sb].dy = 0;	// ...overwriting blocks that we don't want to clear
            players[p].snake[sb].width = DELTA_X;
            players[p].snake[sb].height = DELTA_Y;
        }
	players[p].direction = DOWN;
	players[p].color = 0xFFFF;
    }
}

void input_handler(int signum) {
    signum = signum;
    game_button_state = ioctl(gp_fd, GP_IOCTL_GET_BUTTON_STATE);
    flag_button_pressed = 1;
}

void SetupGamepad(void) {
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

void SetArea(struct fb_copyarea * area, uint16_t colour) {
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

void ClearArea(struct fb_copyarea * area) {
    SetArea(area, game_background_colour);
}

void ClearScreen() {
    struct fb_copyarea rect;
    rect.dx = 0;
    rect.dy = 0;
    rect.width = SCREEN_WIDTH;
    rect.height = SCREEN_HEIGHT;
    ClearArea(&rect);
}

void DrawBackground() {
    ClearScreen();
}


#define BTN_L_LEFT(btn_state) ((btn_state) & 0x01)
#define BTN_L_UP(btn_state) ((btn_state) & 0x02)
#define BTN_L_RIGHT(btn_state) ((btn_state) & 0x04)
#define BTN_L_DOWN(btn_state) ((btn_state) & 0x08)

static inline void change_direction(struct Player* player, int change_dir) {
    // Don't allow changing direction 180 degrees
    printf("Current dir: %d\n", player->direction);
    switch (player->direction) {
    case LEFT:
	player->direction = (change_dir == RIGHT ? LEFT : change_dir);
	break;
    case RIGHT:
	player->direction = (change_dir == LEFT ? RIGHT : change_dir);
	break;
    case UP:
	player->direction = (change_dir == DOWN ? UP : change_dir);
	break;
    case DOWN:
	player->direction = (change_dir == UP ? DOWN : change_dir);
	break;
    default:
  	player->direction = DOWN;
    }
    printf("New dir: %d\n", player->direction);
}

/* TODO? Make button to player mapping dynamic  */
static inline void button_action(int button_state)
{
    static int prev_button_state = 0xFF;

    int newly_pressed = (prev_button_state ^ button_state) & prev_button_state;
    prev_button_state = button_state;

    if (BTN_L_LEFT(newly_pressed))
	change_direction(&players[0], LEFT);
    if (BTN_L_RIGHT(newly_pressed))
	change_direction(&players[0], RIGHT);
    if (BTN_L_UP(newly_pressed))
	change_direction(&players[0], UP);
    if (BTN_L_DOWN(newly_pressed))
	change_direction(&players[0], DOWN);
} 

/* To be called every clock cycle */
/** pi - player index
 **/
/*
static inline void MoveCursor(int pi)
{
    int new_x = players[pi].snake_head.dx;
    int new_y = players[pi].snake_head.dy;
    switch(players[pi].direction) {
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
    // TODO : Game over when hitting wall
    if (new_x < 0 || (new_x + game_cursor.width) > SCREEN_WIDTH)
        return;
    if (new_y < 0 || (new_y + game_cursor.height) > SCREEN_HEIGHT)
        return;
    // TODO : Right now ClearArea will update the screen, which is unnecessary
    //ClearArea(&game_cursor);	
    // Move the cursor if 
    players[pi].snake_head.dx = new_x;
    players[pi].snake_head.dy = new_y;
    // Clear the oldest block in the snake array
    ClearArea(&(players[pi].snake[snake_array_index]));
    // Copy the position of the game_cursor to the snake array
    players[pi].snake[snake_array_index].dx = new_x;
    players[pi].snake[snake_array_index].dy = new_y;
    // Draw the new block, i.e. the game_cursor 
    SetArea(&players[pi].snake_head, players[pi].color);
    snake_array_index = (snake_array_index + 1) % SNAKE_LENGTH;
}
*/

static inline void move_snake(struct Player* p) 
{
    int new_x = p->snake_head.dx;
    int new_y = p->snake_head.dy;
    switch(p->direction) {
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
    /* TODO : Game over when hitting wall */
    if (new_x < 0 || (new_x + p->snake_head.width) > SCREEN_WIDTH)
        return;
    if (new_y < 0 || (new_y + p->snake_head.height) > SCREEN_HEIGHT)
        return;
    // TODO : Right now ClearArea will update the screen, which is unnecessary
    /* Move the cursor if */
    p->snake_head.dx = new_x;
    p->snake_head.dy = new_y;
    /* Clear the oldest block in the snake array */
    ClearArea(&(p->snake[snake_array_index]));
    /* Copy the position of the game_cursor to the snake array */
    p->snake[snake_array_index].dx = new_x;
    p->snake[snake_array_index].dy = new_y;
    /* Draw the new block, i.e. the game_cursor */
    SetArea(&(p->snake_head), p->color);
    
}

static inline void update_snakes()
{
    for (int p = 0; p < NUM_PLAYERS; p++) {
	move_snake(&players[p]);
    }
    /* TODO: Put collision detection here */
    snake_array_index = (snake_array_index + 1) % SNAKE_LENGTH;
}

int main()
{
	printf("Hello World, I'm game!\n");
    int ret;

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
    SetupGamepad();
    // Setup a signal handler for the alarm signal which is used to periodically update
    // the screen
    SetupTimer();
    signal(SIGALRM, &alarm_handler);
    DrawBackground();
    SetupPlayers();
    while (1) {
        pause();
        if ( flag_button_pressed ) {
            button_action(game_button_state);
            flag_button_pressed = 0;
        } 
        if (flag_update_screen_timer) {
            update_snakes();
            UpdateScreen();
            flag_update_screen_timer = 0;
            if ( DetectCollision(game_food, game_cursor) ) {
                game_food_eaten = 1;
            }
        }
    }

    // Clean up
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
