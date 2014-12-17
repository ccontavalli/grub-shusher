all: shush-mbr shush-kernel

shush-mbr:
	g++ shush-mbr.cpp -o shush-mbr

shush-kernel:
	g++ shush-kernel.cpp -o shush-kernel

clean:
	rm shush-kernel shush-mbr
