static:
	make -C ../zad1 static
	gcc -Wall -c wc_repl.c
	gcc -Wall wc_repl.o ../zad1/libwclib.a -o wc_repl_static

shared:
	make -C ../zad1 shared
	gcc -Wall -c wc_repl.c
	gcc -Wall wc_repl.o -L ../zad1 -lwclib -Wl,-rpath=../zad1 -o wc_repl_shared

dynamic:
	make -C ../zad1 shared
	gcc -Wall wc_repl_dynamic.c -o wc_repl_dynamic -ldl

clean:
	make -C ../zad1 clean
	rm -f wc_repl.o wc_repl_dynamic wc_repl_shared wc_repl_static