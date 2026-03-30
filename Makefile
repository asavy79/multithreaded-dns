CC = gcc
CFLAGS = -Wall -Wextra -pthread
LDFLAGS = -pthread

TARGET = multi-lookup
SRCS = multi-lookup.c array.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
