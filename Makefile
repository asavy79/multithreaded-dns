CC = gcc
CFLAGS = -Wall -Wextra -pthread
LDFLAGS = -pthread

all:
	$(CC) $(CFLAGS) multi-lookup.c array.c -o multi-lookup

clean:
	rm -f multi-lookup