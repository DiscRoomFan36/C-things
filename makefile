CC = clang
CFLAGS = -std=c99 -Wall -Wextra -ggdb #-pedantic


mytb.h: build src/mytb_unsqueezed.h
	python3 juicer.py build/mytb.h


tests: build/hashmap_test


build/hashmap_test: build tests/hashmap_test.c src/hashmap.c src/ints.h
	$(CC) $(CFLAGS) tests/hashmap_test.c -o build/hashmap_test


build:
	mkdir -p build/

clean:
	rm -rf build/
