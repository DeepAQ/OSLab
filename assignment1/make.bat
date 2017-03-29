@echo off
nasm -fwin64 -o fib_win.o FibonacciWin.asm
gcc -m64 -o fib.exe fib_win.o
fib.exe
