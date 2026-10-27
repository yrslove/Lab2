CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror -O2
TARGET = bin/readpart
SOURCE = $(wildcard src/*.c)

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SOURCE)
	mkdir -p bin
	$(CC) $(CFLAGS) -Iinclude $(SOURCE) -o $(TARGET)

run: all
	$(TARGET) --help

clean:
	rm -rf bin
