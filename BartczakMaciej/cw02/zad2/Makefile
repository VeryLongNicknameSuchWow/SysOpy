CC=gcc
CFLAGS=-Wall
TEST_SIZE=1M

all: 1 1024

1: reverse.c
	$(CC) $(CFLAGS) reverse.c -o reverse_1 -D BLOCK_SIZE=1

1024: reverse.c
	$(CC) $(CFLAGS) reverse.c -o reverse_1024 -D BLOCK_SIZE=1024

test: all
	base64 /dev/urandom | head -c $(TEST_SIZE) > input.txt
	echo "test" >> input.txt
	tac < input.txt | rev > output_rev.txt
	./reverse_1 input.txt output_1.txt > pomiar_zad_2.txt
	-diff -q -s output_1.txt output_rev.txt
	./reverse_1024 input.txt output_1024.txt >> pomiar_zad_2.txt
	-diff -q -s output_1024.txt output_rev.txt
	rm -f input.txt output_1.txt output_1024.txt output_rev.txt reverse_1 reverse_1024

clean:
	rm -f reverse_1 reverse_1024

