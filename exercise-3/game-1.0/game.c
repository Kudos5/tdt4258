#include <stdio.h>
#include <stdlib.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <sys/mman.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240


int main(int argc, char *argv[])
{
    int ret;
	printf("Hello World, I'm game!\n");

    // FILE * fb_file = fopen("/dev/fb0", "wb");
    int fb_file = open("/dev/fb0", O_RDWR);
    if ( fb_file == -1 ) {
        printf("Failed to open file\n");
        exit(EXIT_FAILURE);
    }

    // Get file stat
    struct stat fb_stat;
    ret = fstat(fb_file, &fb_stat);
    if ( ret == -1 ) {
        printf("Failed to state file\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Total size in bytes: %lu\n", fb_stat.st_size);
    }

    // Let's map the file descriptor to a memory location
    uint16_t buffer[SCREEN_WIDTH*SCREEN_HEIGHT];
    uint16_t * screen = mmap(buffer, sizeof(buffer), PROT_WRITE, MAP_SHARED, fb_file, 0);
    if ( screen == MAP_FAILED ) {
        printf("Failed to map memory: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    printf("I will now clear the screen\n");
    for ( int i = 0; i < SCREEN_WIDTH*SCREEN_HEIGHT; ++i ) {
        screen[i] = 0;
        // buffer[i] = 0;
    }
    struct fb_copyarea rect;
    rect.dx = 0;
    rect.dy = 0;
    rect.width = SCREEN_WIDTH;
    rect.height = SCREEN_HEIGHT;
    ioctl(fb_file, 0x4680, &rect);

    ret = close(fb_file);
    if ( ret == -1 ) {
        printf("Failed to close file\n");
        exit(EXIT_FAILURE);
    }

	exit(EXIT_SUCCESS);
}
