SHELL=/bin/sh
.SUFFIXES:
.SUFFIXES: .c .o
lib_util = ../util/
libraries = ../Library/libutil.so
sources = $(lib_util)
SRC = src/data_key.c src/data_value.c
OBJ = $(SRC:.c=.o)
mypath = ../Library

all: $(OBJ) $(libraries)
	gcc -shared -I$(sources)/include -Iinclude/ -o $(mypath)/libdata.so $(OBJ) $(libraries)
$(libraries) : $(sources)
	$(MAKE) --directory=$<
%.o : %.c
	gcc -c -Wall -g -fPIC -Iinclude/ -I$(lib_util)/include $< -o $@

clean: clean_subs clean_so clean_local

clean_subs : $(sources)
	$(MAKE) --directory=$< clean
clean_local:
	-rm src/*.o

clean_so:
	-rm $(libraries)
