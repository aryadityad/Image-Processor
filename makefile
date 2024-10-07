# Makefile

CC = gcc
CFLAGS = -I./include -pthread -lm
SRC = src/image_processor.c
TARGET = image_processor

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS)

clean:
	rm -f $(TARGET) *.jpg

.PHONY: all clean
