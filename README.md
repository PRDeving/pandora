gcc -c pandora.c -o libpandora.o
ar rcs libpandora.a libpandora.o
gcc -static client.c -L. -lpandora

