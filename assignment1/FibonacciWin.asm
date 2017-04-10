section .data
    prompt: db 'OS_Lab_1 sample Fibonacci Sequence', 10, 'Please input n: '
    len_prompt: equ $-prompt
    format1: db 27, '['
    length1: equ $-format1
    format2: db 'm'
    length2: equ $-format2
    format3: db 27, '[0m', 10
    length3: equ $-format3
    colors: db '31333536'
    
section .bss
    stdin: resq 1
    stdout: resq 1
    iobuf: resb 1
    charnum: resd 1
    numa: resb 128
    numb: resb 128
    
section .text
extern GetStdHandle, ReadConsoleA, WriteConsoleA, ExitProcess
global main

main: ;int main()
    mov rcx, -10 ;get stdin handle
    call GetStdHandle
    mov [stdin], rax
    mov rcx, -11 ;get stdout handle
    call GetStdHandle
    mov [stdout], rax
    mov rcx, [stdout] ;show prompt
    mov rdx, prompt
    mov r8, len_prompt
    mov r9, charnum
    sub rsp, 8 ;align rsp
    call WriteConsoleA
    add rsp, 8
    mov rbx, -1 ;store number
    xor rbp, rbp ;numbers count
loop_input:
    call getc
    mov r12, rax
    cmp r12, 13
    jne not_return
    cmp rbx, 0 ;return key
    jge calc
    jmp end
not_return:
    cmp r12, 32
    jne not_blank
    cmp rbx, 0 ;space key
    jl loop_input
calc:
    mov rcx, rbx
    call fib
    mov rcx, rbp
    call print_num
    cmp r12, 13
    je end
    mov rbx, -1
    inc rbp
    jmp loop_input
not_blank:
    cmp r12, 48
    jl invalid
    cmp r12, 57
    jg invalid
    cmp rbx, 0 ;a number
    jge number
    xor rbx, rbx
number:
    imul rbx, 10
    sub r12, 48
    add rbx, r12
    jmp loop_input
invalid:
    mov rbx, -1
    jmp loop_input
end:
    xor rcx, rcx ;return 0
    call ExitProcess

fib: ;void fib(int n)
    xor rax, rax
fib_clean: ;clear memory first, total 8Byte * 32 = 128Byte
    mov qword [numa + rax * 8], 0
    inc rax
    cmp rax, 32
    jl fib_clean
    mov rax, -1 ;count
    xor r11, r11 ;carry
    mov byte [numa], 1 ;let a = 1
fib_loop:
    inc rax
    cmp rax, rcx
    jg fib_end
    xor rdx, rdx ;bias for numa/numb
fib_add: ;numa + numb -> numb
    movzx r8, byte [numa + rdx] ;digit in numa
    movzx r9, byte [numb + rdx] ;digit in numb
    mov r10, r8
    add r10, r9
    add r10, r11
    xor r11, r11 ;clear carry
    cmp r10, 10
    jl fib_added ;no carry
    mov r11, 1 ;has carry
    sub r10, 10
fib_added:
    mov byte [numa + rdx], r9b
    mov byte [numb + rdx], r10b
    inc rdx
    cmp rdx, 128
    jl fib_add
    jmp fib_loop
fib_end:
    xor rax, rax
    ret

getc: ;char getc()
    sub rsp, 48
    mov rcx, [stdin]
    mov rdx, iobuf
    mov r8, 1
    mov r9, charnum
    mov qword [rsp+0x20], 0
    call ReadConsoleA
    movzx rax, byte [iobuf]
    add rsp, 48
    ret

print_num: ;void print_num(int number)
    push rbx
    sub rsp, 40
    mov rbx, rcx
    mov rcx, [stdout]
    mov rdx, format1
    mov r8, length1
    mov r9, charnum
    call WriteConsoleA
    and rbx, 11b ;r13 %= 4
    sal rbx, 1 ;r13 *= 2
    add rbx, colors
    mov rcx, [stdout]
    mov rdx, rbx
    mov r8, 2
    mov r9, charnum
    call WriteConsoleA
    mov rcx, [stdout]
    mov rdx, format2
    mov r8, length2
    mov r9, charnum
    call WriteConsoleA
    mov rbx, numb + 128
print_loop: ;print numb
    dec rbx
    cmp rbx, numb
    jle print_loop1
    cmp byte [rbx], 0
    jne print_loop1
    jmp print_loop
print_loop1:
    cmp rbx, numb
    jl print_loop_end
    movzx rdx, byte [rbx]
    add rdx, 48
    mov byte [iobuf], dl
    mov rcx, [stdout]
    mov rdx, iobuf
    mov r8, 1
    mov r9, charnum
    call WriteConsoleA
    dec rbx
    jmp print_loop1
print_loop_end:
    mov rcx, [stdout]
    mov rdx, format3
    mov r8, length3
    mov r9, charnum
    call WriteConsoleA
    xor rax, rax
    add rsp, 40
    pop rbx
    ret
