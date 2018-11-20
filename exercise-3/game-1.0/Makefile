#
# Simple development makefile
#

#
# add more flags here,
# but don't remove the "+="
#
CFLAGS += -O2 -g -Wall -Wsign-compare -Wfloat-equal -Wformat-security -Wall -Wextra -pedantic -std=c99 -I.. #-Werror
CPPFLAGS +=
LDFLAGS += 


all: game

clean:
	-rm -f game

game: snake.c game.c

install:


.PHONY: all install clean

# end of development makefile
