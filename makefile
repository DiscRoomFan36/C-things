CC = clang
CFLAGS = -std=c99 -Wall -Wextra -ggdb #-pedantic


tests: build/hashmap_test


build/hashmap_test: build tests/hashmap_test.c src/hashmap.c src/ints.h
	$(CC) $(CFLAGS) tests/hashmap_test.c -o build/hashmap_test


build:
	mkdir -p build/

clean:
	rm -rf build/
