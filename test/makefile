SHELL=/bin/sh
.SUFFIXES:
.SUFFIXES: .c .o
lib_util = ../util/
libraries = ../Library/libutil.so
sources = $(lib_util)

all: src/test.c $(sources) $(libraries)
	gcc -Wall -I$(sources)/include -Iinclude/ -o test.o src/test.c $(libraries)
$(libraries) : $(sources)
	$(MAKE) --directory=$<

clean: clean_subs clean_so clean_local

clean_subs : $(sources)
	$(MAKE) --directory=$< clean
clean_local:
	-rm test.o

clean_so:
	-rm $(libraries)
