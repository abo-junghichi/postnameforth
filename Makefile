CC=gcc -Wall -pedantic -Wextra -Wstrict-aliasing=1
NOSTDLIB=-static -nostdlib -fno-builtin -fno-pie
clean:
	rm forth.out forth.out.trunc *.s words.o
all: forth.out.trunc
forth.out: forth.c
	$(CC) $(NOSTDLIB) -Os forth.c -o forth.out
	#$(CC) -g -O  forth.c -o forth.out
forth.out.trunc: forth.c
	sh build-dirty.sh
words.o: words.c
	$(CC) $(NOSTDLIB) -O3 -c words.c
