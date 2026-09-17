CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
TARGET = bin/shell
SRC = src/shell.c

all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p bin
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -rf bin

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
