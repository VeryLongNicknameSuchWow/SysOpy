CC=gcc
CFLAGS=-Wall

comms: comms.c
	$(CC) $(CFLAGS) comms.c -o comms

test: comms
	./comms bundled 0 1 2 3 4 5

clean:
	rm -f comms