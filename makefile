CC = clang
CFLAGS = -std=c99 -Wall -Wextra -ggdb #-pedantic


mytb.h: build src/mytb_unsqueezed.h
	python3 juicer.py build/mytb.h


tests: build/hashmap_test build/file_test

build/hashmap_test: build tests/hashmap_test.c src/hashmap.h
	$(CC) $(CFLAGS) tests/hashmap_test.c -o build/hashmap_test

build/file_test: build tests/file_test.c src/file.h
	$(CC) $(CFLAGS) tests/file_test.c -o build/file_test


# inner dependencies
src/file.h: src/dynamic_array.h src/ints.h
src/hashmap.h: src/ints.h


build:
	mkdir -p build/

clean:
	rm -rf build/
