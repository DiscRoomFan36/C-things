CC = gcc
CFLAGS = -Wall -Wextra -pedantic -g

run: main

main: main.o 
	$(CC) $(CFLAGS) main.o -o main

main.o: main.c hashmap.c
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f core main *.o *~
