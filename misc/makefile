CC = clang
CFLAGS = -std=c99 -Wall -Wextra -ggdb #-pedantic



tests: clean build/hashmap_test build/file_test


build/hashmap_test: build hashmap_test.c hashmap.h
	$(CC) $(CFLAGS) hashmap_test.c -o build/hashmap_test

build/file_test: build file_test.c file.h
	$(CC) $(CFLAGS) file_test.c -o build/file_test


# inner dependencies
file.h: dynamic_array.h ints.h String_View.h
hashmap.h: ints.h
String_View.h: ints.h


build:
	mkdir -p build/

clean:
	rm -rf build/
