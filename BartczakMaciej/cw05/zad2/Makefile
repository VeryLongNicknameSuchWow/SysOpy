CC=gcc
CFLAGS=-Wall

test: integral
	echo "dx=1e-9 n=2:" > raport2.txt
	./integral 1e-9 2 >> raport2.txt
	echo "dx=1e-9 n=4:" >> raport2.txt
	./integral 1e-9 4 >> raport2.txt
	echo "dx=1e-9 n=8:" >> raport2.txt
	./integral 1e-9 8 >> raport2.txt
	echo "dx=1e-9 n=16:" >> raport2.txt
	./integral 1e-9 16 >> raport2.txt
	echo "dx=1e-9 n=32:" >> raport2.txt
	./integral 1e-9 32 >> raport2.txt
	echo "dx=1e-10 n=2:" >> raport2.txt
	./integral 1e-10 2 >> raport2.txt
	echo "dx=1e-10 n=4:" >> raport2.txt
	./integral 1e-10 4 >> raport2.txt
	echo "dx=1e-10 n=8:" >> raport2.txt
	./integral 1e-10 8 >> raport2.txt
	echo "dx=1e-10 n=16:" >> raport2.txt
	./integral 1e-10 16 >> raport2.txt
	echo "dx=1e-10 n=32:" >> raport2.txt
	./integral 1e-10 32 >> raport2.txt
	make clean

integral: integral.c
	$(CC) $(CFLAGS) integral.c -o integral

clean:
	rm -f integral