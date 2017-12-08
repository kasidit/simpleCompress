zzz: zzz.o
	gcc -o zzz zzz.o -lz
zzz.o: zzz.c
	gcc -c zzz.c -lz
zzz2: zzz2.o
	gcc -o zzz2 zzz2.o -lz
zzz2.o: zzz2.c
	gcc -c zzz2.c -lz
