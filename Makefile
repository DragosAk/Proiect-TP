CC = gcc
CFLAGS = -Wall
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

all: farming_simulator

farming_simulator: main.c farm.c
	$(CC) $(CFLAGS) main.c farm.c -o farming_simulator $(LDFLAGS)

clean:
	rm -f farming_simulator save.dat