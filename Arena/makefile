
CC = clang
CFLAGS = -Wall -Wextra -ggdb -pedantic #-Weverything

run: clean main
	./main

main: main.c arena.h
	$(CC) $(CFLAGS) -o main main.c

clean:
	rm main
