CC = gcc
CFLAGS = -g -O2 -Wall -Wextra -Iincludes

LIB_SRC = src/dict.c src/dict_err.c src/hash.c
LIB_OBJ = $(patsubst src/%.c,build/%.o,$(LIB_SRC))

TEST_SRC = test.c
TEST_OBJ = build/test.o

.PHONY: all clean check

all: $(LIB_OBJ)

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/test.o: tests/test.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

check: $(LIB_OBJ) $(TEST_OBJ)
	$(CC) $(LIB_OBJ) $(TEST_OBJ) -o build/test
	./build/test
	
clean:
	rm -rf build