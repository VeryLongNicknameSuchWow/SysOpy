CC=gcc
CFLAGS=-Wall

all: dudir

dudir: dudir.c
	$(CC) $(CFLAGS) dudir.c -o dudir

test: dudir
	./dudir .
	rm -f dudir

clean:
	rm -f dudir