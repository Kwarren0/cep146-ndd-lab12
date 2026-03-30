CC = gcc
CFLAGS = -Wall -Wextra -g

TARGET = lab12

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c

clean:
	rm -f $(TARGET)
