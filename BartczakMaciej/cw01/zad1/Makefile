CC=gcc

ifeq ($(CFLAGS),)
CFLAGS=-Wall
endif

all: static shared

static:	wclib.c wclib.h
	$(CC) $(CFLAGS) -c wclib.c
	ar rcs libwclib.a wclib.o

shared:	wclib.c wclib.h
	$(CC) $(CFLAGS) -c -fPIC wclib.c
	$(CC) $(CFLAGS) -shared wclib.o -o libwclib.so

clean:
	rm -f *.a *.o *.so