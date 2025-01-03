CC = gcc
CFLAGS = -Wall -fPIC
LDFLAGS = -shared

LIB_EXAMPLE = libexample.so


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
	rm -f *.0 $(TARGET) $(LIB_EXAMPLE)
