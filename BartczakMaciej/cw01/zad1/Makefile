static:
	gcc -Wall -c wclib.c
	ar rcs libwclib.a wclib.o

shared:
	gcc -Wall -fPIC -c wclib.c
	gcc -Wall -shared wclib.o -o libwclib.so

clean:
	rm -f libwclib.a wclib.o libwclib.so