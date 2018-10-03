CC=gcc # What compiler to use
EXEC = seq
CFLAGS=-ggdb -Wall

DEPS = sound.h 
OBJ = showcase.c sound.c seqs.c 

LIBS = -lsndfile -lm 

all: $(EXEC)	
	
$(EXEC): $(OBJ)
	$(CC) -o $(EXEC) $(LIBS) $(OBJ)

%.o: %.c $(DEPS)
	$(CC) -c $(CFLAGS) $< -o $@  

.PHONY: clean
clean: 
	rm *.wav $(EXEC) 
