CC = gcc
CFLAGS = -Wall -Wextra -g

all: main main_macro

main: main.c hashmap.c
	$(CC) $(CFLAGS) main.c -o main

main_macro: main_macro.c hashmap_macro.c
	$(CC) $(CFLAGS) main_macro.c -o main_macro

clean:
	rm -f core main main_macro *.o *~
