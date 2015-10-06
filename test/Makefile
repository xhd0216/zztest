lib_util = ../util
libraries = ../Library/*.so
sources = $(lib_util)

all: src/test.c $(sources) $(libraries)
	gcc -Wall -I$(sources)/include -o test.o src/test.c $(libraries)
$(libraries) : $(sources)
$(sources):
	$(MAKE) --directory=$@

clean: $(sources)
	$(MAKE) --directory=$< clean
	
