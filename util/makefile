SRC = src/hash_map.c src/lru_cache.c
LIB = include/*.h
OBJ = $(SRC:.c=.o)
mypath = ~/codes/zz/src/libs/Library/

all: $(OBJ)
	gcc -shared $(OBJ) -o $(mypath)libutil.so
%.o : %.c
	gcc -c -Wall -g -fPIC -Iinclude/ $< -o $@

clean:
	rm src/*.o


