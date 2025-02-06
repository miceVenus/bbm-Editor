CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99

SRC = src/appendbuffer.c src/terminal.c src/input.c src/file.c src/editor.c src/main.c
OBJ = $(SRC:src/%.c=output/%.o)
TARGET = bbmEditor

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

output/%.o: src/%.c
	@mkdir -p output
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean