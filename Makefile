CC      = gcc
CFLAGS  = -Wall -Wextra -pedantic -std=c99 -g
TARGET  = lab12

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c

clean:
	rm -f $(TARGET)
