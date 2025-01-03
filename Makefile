CC = gcc
CFLAGS = -Wall -fPIC -g
LDFLAGS = -shared

LIB_EXAMPLE = libexample.so

trampoline: trampoline.o $(LIB_EXAMPLE)
	$(CC) -o $@ trampoline.o -L.

trampoline.o: trampoline.c
	$(CC) $(CFLAGS) -c $<

dlopen_load: load_as_dlopen.o $(LIB_EXAMPLE)
	$(CC) -o $@ load_as_dlopen.o -L.

load_as_dlopen.o: load_as_dlopen.c
	$(CC) $(CFLAGS) -c $<

init_load: main.o $(LIB_EXAMPLE)
	$(CC) -o $@ main.o -L. -lexample

main.o: main.c
	$(CC) $(CFLAGS) -c $<

$(LIB_EXAMPLE): libexample.o
	$(CC) $(LDFLAGS) -o $@ $^

libexample.o: libexample.c
	$(CC) $(CFLAGS) -c $<


clean:
	rm -f *.o trampoline dlopen_load init_load $(LIB_EXAMPLE)
