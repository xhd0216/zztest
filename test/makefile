SHELL=/bin/sh
.SUFFIXES:
.SUFFIXES: .c .o


ut = util
da = data
everything = $(ut) $(da)

sources_path = $(foreach p, $(everything), ../$p/)
libraries = $(foreach l, $(everything), ../Library/lib$l.so)

include_path = $(foreach d, $(sources_path), -I$dinclude/)
all: src/test.c  $(libraries)
	gcc -Wall -g  $(include_path) -Iinclude/ -o test.o src/test.c $(libraries)

../Library/libdata.so : ../data ../Library/libutil.so
	$(MAKE) -C ../data
../Library/libutil.so : ../util
	$(MAKE) -C ../util


clean: clean_subs clean_so clean_local

clean_subs : $(sources)
	$(foreach k, $(sources_path), $(MAKE) -C $k clean)
clean_local: test.o
	-rm test.o
clean_so:
	-rm ../Library/*.so
