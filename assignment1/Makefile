build:
	@echo [1] Assembling
	@nasm -felf64 -o fib.o Fibonacci64.asm
	@echo [2] Linking
	@gcc -m64 -o fib fib.o

run: build
	@echo [3] Running program
	@./fib

