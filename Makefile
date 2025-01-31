CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
TARGET = mines
SRC = mines.c
HEADERS = mines.h termbox2.h

all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) -DTB_IMPL -o $@ $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all clean
