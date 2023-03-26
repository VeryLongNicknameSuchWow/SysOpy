CC=gcc
CFLAGS=-Wall

signal: signal.c
	$(CC) $(CFLAGS) signal.c -o signal

test: signal
	-./signal ignore > ignore.txt
	-./signal handler > handler.txt
	-./signal mask > mask.txt
	-./signal pending > pending.txt
	rm -f signal


clean:
	rm -f signal *.out *.txt