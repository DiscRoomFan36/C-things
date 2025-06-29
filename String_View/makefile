
CC = clang
CFLAGS = -Wall -Wextra -ggdb -pedantic #-Weverything
# CFLAGS += -O2

main: clean main.c String_View.h
	$(CC) $(CFLAGS) -o main main.c

clean:
	rm -rf main
