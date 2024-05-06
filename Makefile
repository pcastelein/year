CC = clang
CFLAGS = -std=c17 -g3 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wdouble-promotion -Wconversion -Wno-sign-conversion -fsanitize=undefined -fsanitize-trap -DDEBUG #def for validation layers
LDFLAGS = -lglfw -lvulkan 

Build: main.c
	$(CC) $(CFLAGS) main.c -o year $(LDFLAGS)

# ps ax | grep year #to find pid
# cat/proc/[pid]/maps > maps.dump #memory address dumper
membug: CFLAGS = -std=c17 -g3 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wdouble-promotion -Wconversion -Wno-sign-conversion -fsanitize=address,undefined -fsanitize-trap -fno-omit-frame-pointer -DDEBUG #def for validation layers
membug: Build
	ASAN_OPTIONS=symbolize=1,fast_unwind_on_malloc=0,log_path=asan.log LSAN_OPTIONS=suppressions=myasan.supp ./year

.PHONY: test clean

test: Build
	./year

clean:
	rm -f year