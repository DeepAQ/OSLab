build:
	@nasm -f elf64 -o my_print.o my_print.asm
	@gcc -m64 -o FAT12Reader main.c my_print.o

run: build
	@./FAT12Reader
