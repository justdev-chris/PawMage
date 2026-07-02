# pawm-convert/Makefile
CC = gcc
CFLAGS = -Wall -Wextra -O2 -I../libpawm/include
LDFLAGS = -L../libpawm -lpawm -lm

TARGET = pawm-convert
SRCS = main.c
OBJS = $(SRCS:.c=.o)

# stb_image and stb_image_write are header-only - no extra files needed

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

uninstall:
	rm -f /usr/local/bin/$(TARGET)

.PHONY: all clean install uninstall
