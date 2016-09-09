hellomake: 
	mkdir include lib tmp
	cp pandora.h include/pandora.h
	gcc -c pandora.c -o tmp/libpandora.o
	ar rcs lib/libpandora.a tmp/libpandora.o
	rm -rf tmp

install:
	mv include/pandora.h /usr/include/pandora.h
	mv lib/libpandora.a /usr/lib/libpandora.a
	rm -rf include lib
