CC=gcc
CFLAGS=-Wall

all: dudir_r

dudir_r: dudir_r.c
	$(CC) $(CFLAGS) dudir_r.c -o dudir_r

test: dudir_r
	-find . | xargs wc --bytes
	#-------------------------------------------------
	./dudir_r .
	rm -f dudir_r

clean:
	rm -f dudir_r