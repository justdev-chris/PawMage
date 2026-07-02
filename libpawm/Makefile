CC = gcc
CFLAGS = -Wall -Wextra -O2 -fPIC
AR = ar
ARFLAGS = rcs

SRCS = src/pawm.c src/compression.c src/metadata.c
OBJS = $(SRCS:.c=.o)

# Detect OS
ifeq ($(OS),Windows_NT)
    TARGET_STATIC = libpawm.a
    TARGET_SHARED = libpawm.dll
    SHARED_FLAGS = -shared -Wl,--out-implib,libpawm.dll.a
else
    TARGET_STATIC = libpawm.a
    TARGET_SHARED = libpawm.so
    SHARED_FLAGS = -shared
endif

all: $(TARGET_STATIC) $(TARGET_SHARED)

$(TARGET_STATIC): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(TARGET_SHARED): $(OBJS)
	$(CC) $(SHARED_FLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET_STATIC) $(TARGET_SHARED) libpawm.dll.a

install: all
	cp $(TARGET_STATIC) /usr/local/lib/
	cp $(TARGET_SHARED) /usr/local/lib/
	cp -r include/pawm.h /usr/local/include/

uninstall:
	rm -f /usr/local/lib/$(TARGET_STATIC)
	rm -f /usr/local/lib/$(TARGET_SHARED)
	rm -f /usr/local/include/pawm.h

.PHONY: all clean install uninstall