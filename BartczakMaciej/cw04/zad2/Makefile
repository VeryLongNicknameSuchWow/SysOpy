CC=gcc
CFLAGS=-Wall

sigaction: sigaction.c
	$(CC) $(CFLAGS) sigaction.c -o sigaction

test: sigaction
	-./sigaction
	make clean

clean:
	rm -f sigaction