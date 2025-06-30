
CC = clang
CFLAGS = -Wall -Wextra -ggdb -pedantic -Werror -Wmissing-prototypes #-Weverything
# CFLAGS += -O2

run: clean main
	./main

main:
	$(CC) $(CFLAGS) -o main main.c

clean:
	rm -rf main
