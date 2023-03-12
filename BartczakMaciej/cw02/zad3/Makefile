CC=gcc
CFLAGS=-Wall

all: dudir

dudir: dudir.c
	$(CC) $(CFLAGS) dudir.c -o dudir

test: dudir
	wc --bytes *
	#-------------------------------------------------
	./dudir .
	rm -f dudir

clean:
	rm -f dudir