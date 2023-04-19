CC=gcc
CFLAGS=-Wall

test: all
	echo "dx=1e-9 n=2:" > raport3.txt
	./parent 1e-9 2 >> raport3.txt
	echo "dx=1e-9 n=4:" >> raport3.txt
	./parent 1e-9 4 >> raport3.txt
	echo "dx=1e-9 n=8:" >> raport3.txt
	./parent 1e-9 8 >> raport3.txt
	echo "dx=1e-9 n=16:" >> raport3.txt
	./parent 1e-9 16 >> raport3.txt
	echo "dx=1e-9 n=32:" >> raport3.txt
	./parent 1e-9 32 >> raport3.txt
	echo "dx=1e-10 n=2:" >> raport3.txt
	./parent 1e-10 2 >> raport3.txt
	echo "dx=1e-10 n=4:" >> raport3.txt
	./parent 1e-10 4 >> raport3.txt
	echo "dx=1e-10 n=8:" >> raport3.txt
	./parent 1e-10 8 >> raport3.txt
	echo "dx=1e-10 n=16:" >> raport3.txt
	./parent 1e-10 16 >> raport3.txt
	echo "dx=1e-10 n=32:" >> raport3.txt
	./parent 1e-10 32 >> raport3.txt
	make clean

all: parent child

parent: parent.c
	$(CC) $(CFLAGS) parent.c -o parent

child: child.c
	$(CC) $(CFLAGS) child.c -o child

clean:
	rm -f child parent