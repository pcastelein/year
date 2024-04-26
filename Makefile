CC = gcc
CFLAGS = -std=c17 -O2
LDFLAGS = -lglfw -lvulkan

Bench: main.c
	$(CC) -o bench main.c $(LDFLAGS)

.PHONY: test clean

test: Bench
	./bench

clean:
	rm -f bench