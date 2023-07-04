CC = gcc
CFLAGS = -Wall

all: client server

client: client.c
	$(CC) $(CFLAGS) client.c -o client
	cp client cli_test/

server: server.c
	$(CC) $(CFLAGS) server.c -o server
	cp server srv_test/

clean:
	rm -f server client
