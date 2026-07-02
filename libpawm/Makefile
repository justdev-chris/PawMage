# libpawm/Makefile
CC = gcc
CFLAGS = -Wall -Wextra -O2 -fPIC
AR = ar
ARFLAGS = rcs

TARGET_STATIC = libpawm.a
TARGET_SHARED = libpawm.so
SRCS = src/pawm.c src/compression.c src/metadata.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET_STATIC) $(TARGET_SHARED)

$(TARGET_STATIC): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(TARGET_SHARED): $(OBJS)
	$(CC) -shared -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET_STATIC) $(TARGET_SHARED)

install: all
	cp $(TARGET_STATIC) /usr/local/lib/
	cp $(TARGET_SHARED) /usr/local/lib/
	cp -r include/pawm.h /usr/local/include/

uninstall:
	rm -f /usr/local/lib/$(TARGET_STATIC)
	rm -f /usr/local/lib/$(TARGET_SHARED)
	rm -f /usr/local/include/pawm.h

.PHONY: all clean install uninstall