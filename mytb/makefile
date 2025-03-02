CC = clang
CFLAGS = -std=c99 -Wall -Wextra -ggdb #-pedantic


mytb.h: build mytb/mytb_unsqueezed.h
	python3 juicer.py build/mytb.h


tests: build/hashmap_test build/file_test

build/hashmap_test: build tests/hashmap_test.c mytb/hashmap.h
	$(CC) $(CFLAGS) tests/hashmap_test.c -o build/hashmap_test

build/file_test: build tests/file_test.c mytb/file.h
	$(CC) $(CFLAGS) tests/file_test.c -o build/file_test


# inner dependencies
mytb/file.h: mytb/dynamic_array.h mytb/ints.h mytb/String_View.h
mytb/hashmap.h: mytb/ints.h
mytb/String_View.h: mytb/ints.h


build:
	mkdir -p build/

clean:
	rm -rf build/
