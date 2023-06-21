CC = gcc
CFLAGS = -Wall

all: srv cli

srv: srv.c
	$(CC) $(CFLAGS) srv.c -o srv

cli: cli.c
	$(CC) $(CFLAGS) cli.c -o cli

clean:
	rm -f srv cli
