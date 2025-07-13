
CC = clang
CFLAGS = -Wall -Wextra -ggdb -pedantic
# CFLAGS += -O2

run: clean main
	./main

main: main.c
	$(CC) $(CFLAGS) -o main main.c

clean:
	rm -f main
