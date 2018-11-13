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
#define SNAKE_MAX_LENGTH 20
#define SNAKE_START_LENGTH 5
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define DELTA_X 10
#define DELTA_Y 10
#define FOOD_SIZE 10

enum { UP, DOWN, LEFT, RIGHT };

struct Player {
    struct fb_copyarea snake[SNAKE_MAX_LENGTH];	// Array of entire snake (body and head)
    uint16_t head_index;			// Position of the snake head
    uint16_t length;				// Array of entire snake (body and head)
    uint16_t color;
    int direction;				// Keep track of which way snake is moving
    int unsigned score;
};
 
static struct fb_copyarea game_food = {
    .width = FOOD_SIZE,
    .height = FOOD_SIZE,
};
   
static struct Player players[NUM_PLAYERS];

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

// Function declarations
static void SetArea(struct fb_copyarea * area, uint16_t colour);

static void GameOver() {
    printf("Game over\n");
    for ( size_t i = 0; i < NUM_PLAYERS; ++i ) {
        printf("Player %u score: %u\n", i, players[i].score);
    }
}

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

static int DetectCollision(struct fb_copyarea * area_a, struct fb_copyarea * area_b) {
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

static int DetectCollisionWithEdgeOfScreen() {
    for ( size_t i = 0; i < NUM_PLAYERS; ++i ) {
        struct Player * player = &players[i];
        printf("%d\n", player->direction);
        switch (player->direction) {
            struct fb_copyarea * snake_head = &(player->snake[player->head_index]);
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
    }
    return 0;
}

static int DetectCollisionWithSelf() {
    for ( size_t i = 0; i < NUM_PLAYERS; ++i ) {
        struct Player * player = &players[i];
        struct fb_copyarea * snake_head = &player->snake[player->head_index];
        for ( size_t j = 0; j < player->length; ++j ) {
            // Make sure to skip the head
            if ( j == player->head_index ) {
                continue;
            }
            if ( DetectCollision(snake_head, &player->snake[j]) ) {
                return 1;
            }
        }
    }
    return 0;
}

static int DetectCollisionBetweenSnakes() {
    // TODO
    return 0;
}

static int DetectCollisionWithFood(size_t * player_id) {
    for ( size_t i = 0; i < NUM_PLAYERS; ++i ) {
        struct Player * player = &players[i];
        struct fb_copyarea * snake_head = &player->snake[player->head_index];
        if ( DetectCollision(snake_head, &game_food) ) {
            *player_id = i;
            return 1;
        }
    }
    return 0;
}
void SpawnFood() {
    // TODO: Do not spawn food on top of the snake
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
    
static struct Player players[NUM_PLAYERS];

void SetupPlayers() {
    for (int p = 0; p < NUM_PLAYERS; p++) {
	    players[p].head_index = 0;
	    players[p].length = SNAKE_START_LENGTH;
	/* Initialize snake blocks */
        for (int sb = 0; sb < SNAKE_MAX_LENGTH; sb++) {
            players[p].snake[sb].dx = 0;	// TODO : Do something smart here, to avoid...
            players[p].snake[sb].dy = 0;	// ...overwriting blocks that we don't want to clear
            players[p].snake[sb].width = DELTA_X;  // Also, consider making the position random.
            players[p].snake[sb].height = DELTA_Y; // Or just make eash snake start in its own corner
        }
	players[p].direction = DOWN;
	players[p].color = 0xFFFF;
    players[p].length = SNAKE_START_LENGTH;
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


static inline void move_snake(struct Player* p) 
{
    int new_x = p->snake[p->head_index].dx;
    int new_y = p->snake[p->head_index].dy;
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
    if (new_x < 0 || (new_x + p->snake[p->head_index].width) > SCREEN_WIDTH)
        return;
    if (new_y < 0 || (new_y + p->snake[p->head_index].height) > SCREEN_HEIGHT)
        return;
    /* Set head to the oldest block (for overwriting) */
    p->head_index = (p->head_index + 1) % p->length;
    /* Clear the oldest block in the snake array */
    // TODO : Right now ClearArea will update the screen, which is unnecessary
    ClearArea(&(p->snake[p->head_index]));
    /* Update the snake head */
    p->snake[p->head_index].dx = new_x;
    p->snake[p->head_index].dy = new_y;
    /* Draw the new block, i.e. the game_cursor */
    SetArea(&(p->snake[p->head_index]), p->color);
    
}

static inline void update_snakes()
{
    for (int p = 0; p < NUM_PLAYERS; p++) {
        move_snake(&players[p]);
    }
    /* TODO: Put collision detection here */
}


int SnakeGrow(struct Player* p) {
    /* Victory! Don't grow */
    if (++(p->length) >= SNAKE_MAX_LENGTH) {
        return 0;
    }
    return 1;
    
    /*
    if (p->length >= SNAKE_MAX_LENGTH){
	// victory
    }
    // If head is last block, everything's good
    if (p->head_index+2 >= p->length-1) 
	return;
    // Unless head is the last block, shift everything right of the head to the right
    for (uint16_t i = p->length-1; i != p->head_index+2; i--) {
	p->snake[i] = p->snake[i-1];
    } */
}


void PrintState() {
    for ( size_t i = 0; i < NUM_PLAYERS; ++i ) {
        printf("Player %u score: %u\n", i, players[i].score);
    }
}

int main()
{
	printf("Game started\n");
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
            if ( DetectCollisionWithEdgeOfScreen() ) {
                printf("Collided with edge\n");
                GameOver();
                return 0;
            }
            if ( DetectCollisionWithSelf() ) {
                printf("Collided with self\n");
                GameOver();
                return 0;
            }
            if ( DetectCollisionBetweenSnakes() ) {
                printf("Collision with other player\n");
                GameOver();
                return 0;
            }
            size_t player_id;
            if ( DetectCollisionWithFood(&player_id) ) {
                ++players[player_id].score;
                game_food_eaten = 1;
                PrintState();
                /* If snake has reached SNAKE_MAX_LENGTH*/
                if (!SnakeGrow(&players[0])) {
                    printf("You win\n");
                    GameOver();
                    break;
                }
            }
            UpdateScreen();
            flag_update_screen_timer = 0;
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
