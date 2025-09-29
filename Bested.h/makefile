
CC = clang
CFLAGS  = -std=gnu11
CFLAGS += -Wall -Wextra -ggdb -Werror
CFLAGS += -pedantic -Wno-gnu-statement-expression
CFLAGS += -Wno-gnu-alignof-expression -Wno-gnu-zero-variadic-macro-arguments
# CFLAGS += -O2
# CFLAGS += -Weverything -ferror-limit=0 -Wno-declaration-after-statement -Wno-unsafe-buffer-usage

run: all
	valgrind ./build/arena_test
	valgrind ./build/pool_test
	./build/string_builder_test
	./build/array_test
	./build/hashmap_test

all: arena_test pool_test string_builder_test array_test hashmap_test

arena_test:                               | build
	$(CC) $(CFLAGS) -o ./build/arena_test tests/arena_test.c

pool_test:                                | build
	$(CC) $(CFLAGS) -o ./build/pool_test tests/pool_test.c

string_builder_test:                      | build
	$(CC) $(CFLAGS) -o ./build/string_builder_test tests/string_builder_test.c

array_test:                               | build
	$(CC) $(CFLAGS) -o ./build/array_test tests/array_test.c

hashmap_test:                             | build
	$(CC) $(CFLAGS) -o ./build/hashmap_test tests/hashmap_test.c


build:
	mkdir -p ./build

clean:
	rm -rf ./build/
