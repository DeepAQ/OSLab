default rel

section .bss
    iobuf: resb 1

section .text
global my_print
global my_print_char
; void my_print(char* c);
my_print:
    mov rcx, rdi
    xor r8, r8
my_print_loop:
    xor r9, r9
    mov byte r9b, [rcx + r8]
    cmp byte r9b, 0
    je my_print_end
    mov byte [iobuf], r9b
    push rcx
    mov rax, 1
    mov rdi, 1
    mov rsi, iobuf
    mov rdx, 1
    syscall
    pop rcx
    inc r8
    jmp my_print_loop
my_print_end:
    xor rax, rax
    ret

; void my_print_char(char c);
my_print_char:
    mov byte [iobuf], dil
    mov rax, 1
    mov rdi, 1
    mov rsi, iobuf
    mov rdx, 1
    syscall
    xor rax, rax
    ret