SRC = src/fl_alloc.c
LIB = include/
OBJ = $(SRC:.c=.o)
mypath = ../Library/

all: $(OBJ)
	gcc -shared $(OBJ) -o $(mypath)libmemory.so
%.o : %.c
	gcc -c -Wall -g -fPIC -I$(LIB) $< -o $@

clean: 
	-rm src/*.o

