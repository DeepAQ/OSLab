build:
	@nasm -felf64 -o fib.o FibonacciHR.asm
	@gcc -m64 -o fib fib.o

run: build
	@./fib
