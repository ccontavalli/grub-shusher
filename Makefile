CC = gcc-4.8
CFLAGS = -std=gnu99

mbr: mbr.o

mbr.o: mbr.c

clean:
	rm -f mbr ./*.o
