CC=gcc
LIB_PATH=../zad1

ifeq ($(CFLAGS),)
CFLAGS=-Wall
endif

all: static shared dynamic

static: wc_repl.c
	make -C $(LIB_PATH) static
	$(CC) $(CFLAGS) -c wc_repl.c
	$(CC) -Wall wc_repl.o ../zad1/libwclib.a -o wc_repl_static

shared: wc_repl.c
	make -C $(LIB_PATH) shared
	$(CC) $(CFLAGS) -c wc_repl.c
	$(CC) $(CFLAGS) wc_repl.o -L $(LIB_PATH) -lwclib -Wl,-rpath=$(LIB_PATH) -o wc_repl_shared

dynamic: wc_repl_dynamic.c
	make -C $(LIB_PATH) shared
	$(CC) $(CFLAGS) wc_repl_dynamic.c -o wc_repl_dynamic -ldl

clean:
	make -C $(LIB_PATH) clean
	rm -f *.a *.o *.so wc_repl_dynamic wc_repl_shared wc_repl_static
