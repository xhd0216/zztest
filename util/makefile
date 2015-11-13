SRC = src/hash_map.c src/lru_cache.c src/queue.c
LIB = include/
OBJ = $(SRC:.c=.o)
mypath = ../Library/

all: $(OBJ)
	gcc -shared $(OBJ) -o $(mypath)libutil.so
%.o : %.c
	gcc -c -Wall -g -fPIC -I$(LIB) $< -o $@

clean: 
	-rm src/*.o

