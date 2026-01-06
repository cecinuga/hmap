CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
CDFLAGS = -g -O0 -Wall -Wextra -Iinclude
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c,build/%.o,$(SRC))

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CDFLAGS) -c $< -o $@

app: $(OBJ)
	$(CC) $(OBJ) -o build/app

clean:
	rm -rf build app