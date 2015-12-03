SHELL=/bin/sh
.SUFFIXES:
.SUFFIXES: .c .o


ut = util
da = data
everything = $(ut) $(da)

sources_path = $(foreach p, $(everything), ../$p/)
libraries = $(foreach l, $(everything), ../Library/lib$l.so)

include_path = $(foreach d, $(sources_path), -I$dinclude/)
all: src/srvr.c  src/single_thread_server.c $(libraries)
	gcc -Wall -Werror -pthread -g  $(include_path) -Iinclude/ -o srvr.o src/srvr.c $(libraries)
	gcc -Wall -Werror -pthread -g $(include_path) -Iinclude/ -o single.o src/single_thread_server.c $(libraries)
../Library/libdata.so : ../data ../Library/libutil.so
	$(MAKE) -C ../data
../Library/libutil.so : ../util
	$(MAKE) -C ../util


clean: clean_subs clean_so clean_local

clean_subs : 
	$(MAKE) -C ../data clean
#clean_subs : $(sources)
#	$(foreach k, $(sources_path), $(MAKE) -C $k clean)
clean_local: srvr.o single.o
	-rm srvr.o
	-rm single.o
clean_so:
	-rm ../Library/*.so
