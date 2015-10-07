SHELL=/bin/sh
.SUFFIXES:
.SUFFIXES: .c .o
lib_util = ../util/
libraries = ../Library/libutil.so
sources = $(lib_util)

all: src/test.c $(sources) $(libraries)
	gcc -Wall -I$(sources)/include -o test.o src/test.c $(libraries)
$(libraries) : $(sources)
	$(MAKE) --directory=$<

clean: clean_subs clean_so clean_local

clean_subs : $(sources)
	$(MAKE) --directory=$< clean
clean_local:
	if [ -a test.o]; \
	then \
		rm test.o; \
	fi;
clean_so: $(libraries)
	if [ -a $(library)]; \
	then \
		rm $(libraries); \
	fi;
	
