SRC = src/hash_map.c src/lru_cache.c
LIB = include/
OBJ = $(SRC:.c=.o)
mypath = ../Library/

all: $(OBJ)
	gcc -shared $(OBJ) -o $(mypath)libutil.so
%.o : %.c
	gcc -c -Wall -g -fPIC -I$(LIB) $< -o $@

clean: $(OBJ)
	for f in $(OBJ); do\
		if [ -e f ] ; then\
			echo "processing $f"; \
			rm f; \
		fi; \
	done;


