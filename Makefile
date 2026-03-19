CC = gcc
CFLAGS = -Wall
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

all: farm_game

farm_game: main.c farm.c
	$(CC) $(CFLAGS) main.c farm.c -o farm_game $(LDFLAGS)

clean:
	rm -f farm_game save.dat