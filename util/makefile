SRC = src/hash_map.c src/lru_cache.c src/queue.c
OBJ = $(SRC:.c=.o)
mypath = ../Library/

mm = memory
everything = $(mm)
libs = $(foreach l, $(everything), $(mypath)lib$l.so)
LIB = inlcude/
#so_path = $(foreach d, $(libs), -I$d)
path_include = $(foreach d, $(everything), -I../$d/include)

all: $(OBJ)
	gcc -shared $(OBJ) -o $(mypath)libutil.so
%.o : %.c
	gcc -c -Wall -g -fPIC -Iinclude/ $(path_include) $< -o $@

clean: 
	-rm src/*.o

