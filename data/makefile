SHELL=/bin/sh
.SUFFIXES:
.SUFFIXES: .c .o


mm = memory
ut = util
everything = $(mm) $(ut)
SRC = src/data_value.c src/data_key.c
OBJ = $(SRC:.c=.o)

sources_path = $(foreach p, $(everything), ../$p/)
libraries = $(foreach l, $(everything), ../Library/lib$l.so)

include_path = $(foreach d, $(sources_path), -I$dinclude/)


all: $(OBJ)
	gcc -shared $(OBJ) -o ../Library/libdata.so
%.o : %.c
	gcc -c -Wall -g -fPIC -Iinclude/ $(include_path) $< -o $@

clean: clean_so clean_local

clean_local:
	-rm src/*.o
clean_so:
	-rm ../Library/libdata.so
