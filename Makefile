zzz: zzz.o
	gcc -o zzz zzz.o -lz
zzz.o: zzz.c
	gcc -c zzz.c -lz
