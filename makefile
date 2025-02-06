CC = clang
CFLAGS = -Wall -Wextra -g #-pedantic

all: hashmap_test

hashmap_test: hashmap_test.c hashmap.c ints.h
	$(CC) $(CFLAGS) hashmap_test.c -o hashmap_test

clean:
	rm -f core hashmap_test *.o *~
