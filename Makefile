CC = gcc
CFLAGS = -std=c17 -O2 -DDEBUG #def for validation layers
LDFLAGS = -lglfw -lvulkan -fsanitize=undefined

Build: main.c
	$(CC) $(CFLAGS) main.c -o year $(LDFLAGS)

.PHONY: test clean

test: Build
	./year

clean:
	rm -f year