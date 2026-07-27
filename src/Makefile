CC      = gcc
CFLAGS  = -Wall -Werror -std=gnu11 -g3 -O2
LDFLAGS = -lm
TARGET  = highway
SRCS    = main.c highway.c
OBJS    = $(SRCS:.c=.o)

.PHONY: all debug clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# debug: no optimisation, AddressSanitizer + UBSanitizer enabled
debug: CFLAGS = -Wall -Werror -std=gnu11 -g3 -O0 -fsanitize=address,undefined
debug: $(TARGET)

%.o: %.c highway.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
