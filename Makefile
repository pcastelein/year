CC = gcc
CFLAGS = -std=c17 -O2 -DDEBUG #def for validation layers
LDFLAGS = -lglfw -lvulkan -fsanitize=undefined

Bench: main.c
	$(CC) $(CFLAGS) main.c -o bench $(LDFLAGS)

.PHONY: test clean

test: Bench
	./bench

clean:
	rm -f bench