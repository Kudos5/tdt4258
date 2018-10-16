CC=gcc # What compiler to use
EXEC = showcase
CFLAGS=-ggdb -Wall

DEPS = sound.h 
OBJ = showcase.c sound.c seqs.c sweep.c

LIBS = -lsndfile -lm 

all: $(EXEC)	
	
$(EXEC): $(OBJ)
	$(CC) -DSHOWCASE -o $(EXEC) $(LIBS) $(OBJ)

%.o: %.c $(DEPS)
	$(CC) -c $(CFLAGS) $< -o $@  

.PHONY: clean
clean: 
	rm *.wav $(EXEC) 
