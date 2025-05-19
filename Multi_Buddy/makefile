
CC = clang
CFLAGS = -Wall -Wextra -pedantic -Werror -ggdb -std=gnu11
CFLAGS += -O2

all: main main_unity

main_unity: main.c multi_buddy.h
	$(CC) $(CFLAGS) -lpthread -lm -o main_unity main.c


main: main.o multi_buddy.o
	$(CC) $(CFLAGS) -lpthread -lm -o main main.o multi_buddy.o

main.o: main.c multi_buddy.h
	$(CC) $(CFLAGS) -DNO_UNITY -c -o main.o main.c

multi_buddy.o: multi_buddy.h
	$(CC) $(CFLAGS) -x c -DMULTI_BUDDY_IMPLEMENTATION -c -o multi_buddy.o multi_buddy.h


clean:
	rm main *.o
