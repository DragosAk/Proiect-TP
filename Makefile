CC = gcc
CFLAGS = -Wall -O2
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

all: farming_simulator

farming_simulator: main.o farm.o
	$(CC) $(CFLAGS) main.o farm.o -o farming_simulator $(LDFLAGS)

main.o: main.c farm.h
	$(CC) $(CFLAGS) -c main.c

farm.o: farm.c farm.h
	$(CC) $(CFLAGS) -c farm.c

clean:
	rm -f farming_simulator *.o savegame.dat

run: all
	./farming_simulator