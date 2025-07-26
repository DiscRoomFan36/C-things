
CC = clang
CFLAGS = -Wall -Wextra -ggdb -pedantic -Werror
# CFLAGS += -O2
# CFLAGS += -Weverything -ferror-limit=0 -Wno-declaration-after-statement -Wno-unsafe-buffer-usage

run: clean main
	./main

main: main.c
	$(CC) $(CFLAGS) -o main main.c

clean:
	rm -f main
