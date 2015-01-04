#CC = gcc-4.8
CFLAGS = -std=gnu99 -Wall -pedantic

all: mbr grub-kernel

grub-kernel: grub-kernel.o

mbr: mbr.o

mbr.o: mbr.c

mbr.o: grub-kernel.c

clean:
	rm -f grub-kernel mbr ./*.o
