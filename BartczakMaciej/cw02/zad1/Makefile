CC=gcc
CFLAGS=-Wall
TEST_SIZE=4M
TEST_COUNT=10

all: std sys

std: replace.c replace.h
	$(CC) $(CFLAGS) replace.c -o replace_std

sys: replace.c replace.h
	$(CC) $(CFLAGS) replace.c -o replace_sys -D SYS

test: all
	base64 /dev/urandom | head -c $(TEST_SIZE) > input.txt
	./replace_sys A B input.txt output.txt > pomiar_zad_1.txt
	tr A B < input.txt > output_tr.txt
	diff -s output_tr.txt output.txt
	./replace_std A B input.txt output.txt >> pomiar_zad_1.txt
	diff -s output_tr.txt output.txt
	rm -f input.txt output.txt output_tr.txt replace_std replace_sys

clean:
	rm -f replace_std replace_sys

