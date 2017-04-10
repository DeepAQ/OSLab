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
    iobuf: resb 1
    numa: resb 128
    numb: resb 128
    
section .text
global main

main: ;int main()
    mov rax, 1 ;show prompt
    mov rdi, 1
    mov rsi, prompt
    mov rdx, len_prompt
    syscall
    mov rbx, -1 ;store number
    xor rbp, rbp ;numbers count
loop_input:
    call getc
    mov r12, rax
    cmp r12, 10
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
    mov rdi, rbx
    call fib
    mov rdi, rbp
    call print_num
    cmp r12, 10
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
    xor rax, rax ;return 0
    ret

fib: ;void fib(int n)
    xor rax, rax
fib_clean: ;clear memory first, total 8Byte * 32 = 128Byte
    mov qword [numa + rax * 8], 0
    inc rax
    cmp rax, 32
    jl fib_clean
    mov rax, -1 ;count
    xor rsi, rsi ;carry
    mov byte [numa], 1 ;let a = 1
fib_loop:
    inc rax
    cmp rax, rdi
    jg fib_end
    xor rcx, rcx ;bias for numa/numb
fib_add: ;numa + numb -> numb
    xor r8, r8 ;digit in numa
    xor r9, r9 ;digit in numb
    mov byte r8b, [numa + rcx]
    mov byte r9b, [numb + rcx]
    mov r10, r8
    add r10, r9
    add r10, rsi
    xor rsi, rsi ;clear carry
    cmp r10, 10
    jl fib_added ;no carry
    mov rsi, 1 ;has carry
    sub r10, 10
fib_added:
    mov byte [numa + rcx], r9b
    mov byte [numb + rcx], r10b
    inc rcx
    cmp rcx, 128
    jl fib_add
    jmp fib_loop
fib_end:
    xor rax, rax
    ret

getc: ;char getc()
    mov rax, 0
    mov rdi, 1
    mov rsi, iobuf
    mov rdx, 1
    syscall
    xor rax, rax
    mov byte al, [iobuf]
    ret

print_num: ;void print_num(int number)
    mov rsi, rdi
    and rsi, 11b ;rsi %= 4
    sal rsi, 1 ;rsi *= 2
    add rsi, colors
    push rsi
    mov rax, 1
    mov rdi, 1
    mov rsi, format1
    mov rdx, length1
    syscall
    mov rax, 1
    pop rsi
    mov rdx, 2
    syscall
    mov rax, 1
    mov rsi, format2
    mov rdx, length2
    syscall
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
    xor rdx, rdx
    mov byte dl, [rbx]
    add rdx, 48
    mov byte [iobuf], dl
    mov rax, 1
    mov rsi, iobuf
    mov rdx, 1
    syscall
    dec rbx
    jmp print_loop1
print_loop_end:
    mov rax, 1
    mov rsi, format3
    mov rdx, length3
    syscall
    xor rax, rax
    ret
