#include <stdio.h>
#include <stdlib.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <signal.h>
#include <sys/mman.h>
#include <linux/fs.h>
#include <time.h>

#include "driver-gamepad-1.0/driver-gamepad.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define SNAKE_LENGTH 5

static int gp_fd;
static int fb_fd;

static uint16_t * game_screen;

static int flag_button_pressed;
static int flag_update_screen_timer;

static int game_button_state;

static uint16_t game_background_colour = 0x0000;
static uint16_t game_cursor_colour = 0xFFFF;

static struct fb_copyarea game_cursor;
static struct fb_copyarea snake[SNAKE_LENGTH];
static unsigned int snake_array_index;

void UpdateScreen() {
}

void alarm_handler(int signum) {
    // Just to avoid warnings
    signum = signum;
    flag_update_screen_timer = 1;
    // Trigger a new alarm
    alarm(1);
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
    // TODO: Make this nice, instead of just setting everything to black
    ClearScreen();
}

#define DELTA_X 10
#define DELTA_Y 10
void SetupCursor() {
    game_cursor.width = DELTA_X;
    game_cursor.height = DELTA_Y;
    game_cursor.dx = 0;
    game_cursor.dy = 0;
    for (int i = 0; i < SNAKE_LENGTH; i++) {
        snake[i].dx = 0;
        snake[i].dy = 0;
        snake[i].width = DELTA_X;
        snake[i].height = DELTA_Y;
    }
}

void DrawCursor() {
    SetArea(&game_cursor, game_cursor_colour);
}

enum { BTN_L_LEFT, BTN_L_RIGHT, BTN_L_UP, BTN_L_DOWN,
       BTN_R_LEFT, BTN_R_RIGHT, BTN_R_UP, BTN_R_DOWN };

static inline int decode_button_state(int button_state)
{
    switch(button_state) {
    case 0xFE : return BTN_L_LEFT;
    case 0xFB : return BTN_L_RIGHT;
    case 0xFD : return BTN_L_UP;
    case 0xF7 : return BTN_L_DOWN;
    case 0xEF : return BTN_R_LEFT;
    case 0xBF : return BTN_R_RIGHT;
    case 0xDF : return BTN_R_UP;
    case 0x7F : return BTN_R_DOWN;
    }
}

static inline void MoveCursor(int cursor_direction)
{
    int new_x = game_cursor.dx;
    int new_y = game_cursor.dy;
    switch(cursor_direction) {
    case BTN_L_LEFT:
    case BTN_R_LEFT:
        new_x -= DELTA_X;
        break;
    case BTN_L_RIGHT:
    case BTN_R_RIGHT:
        new_x += DELTA_X;
        break;
    case BTN_L_UP:
    case BTN_R_UP:
        new_y -= DELTA_Y;
        break;
    case BTN_L_DOWN:
    case BTN_R_DOWN:
        new_y += DELTA_Y;
        break;
    }
    /* TODO : Game over when hitting wall */
    if (new_x < 0 || (new_x + game_cursor.width) > SCREEN_WIDTH)
        return;
    if (new_y < 0 || (new_y + game_cursor.height) > SCREEN_HEIGHT)
        return;
    // TODO : Right now ClearArea will update the screen, which is unnecessary
    //ClearArea(&game_cursor);	
    /* Move the cursor if */
    game_cursor.dx = new_x;
    game_cursor.dy = new_y;
    /* Clear the oldest block in the snake array */
    ClearArea(&snake[snake_array_index]);
    /* Copy the position of the game_cursor to the snake array */
    snake[snake_array_index].dx = new_x;
    snake[snake_array_index].dy = new_y;
    /* Draw the new block i.e. the game_cursor */
    DrawCursor();
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
    signal(SIGALRM, &alarm_handler);
    DrawBackground();
    SetupCursor();
    DrawCursor();
    alarm(1);
    while (1) {
        pause();
        if ( flag_button_pressed ) {
            // DrawBackground();
            int cursor_direction = decode_button_state(game_button_state);
            MoveCursor(cursor_direction);
            printf("Button pressed\n");
            flag_button_pressed = 0;
        } 
        if (flag_update_screen_timer) {
            printf("Updating screen\n");
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
