section .data
    format1: db 27, '['
    length1: equ $-format1
    format2: db 'm'
    length2: equ $-format2
    format3: db 27, '[0m', 10
    length3: equ $-format3
    colors: db '31333536'
    
section .bss
    nstr: resb 20
    
section .text
global main

main:
    push rbx
    xor rbx, rbx ;store number
    xor rcx, rcx ;count
loop_input:
    push rcx
    call getc
    pop rcx
    cmp rax, 10
    jne not_return
    ;return
    cmp rbx, 0
    jne calc
    je end
not_return:
    cmp rax, 32
    jne not_blank
    ;space
    cmp rbx, 0
    je loop_input
calc:
    mov rdi, rbx
    push rax
    push rcx
    call fib
    pop rcx
    mov rdi, rax
    mov rsi, rcx
    push rcx
    call print_num
    pop rcx
    pop rax
    cmp rax, 10
    je end
    xor rbx, rbx
    add rcx, 1
    jmp loop_input
not_blank:
    cmp rax, 48
    jl loop_input
    cmp rax, 57
    jg loop_input
    ; a number
    imul rbx, 10
    add rbx, rax
    sub rbx, 48
    jmp loop_input
end:
    xor rax, rax
    pop rbx
    ret

fib:
    push rbx
    mov rax, 1
    xor rbx, rbx
    mov rcx, 1
fib_loop:
    inc rax
    cmp rax, rdi
    jg fib_end
    mov rdx, rcx
    add rcx, rbx
    mov rbx, rdx
    jmp fib_loop
fib_end:
    mov rax, rcx
    pop rbx
    ret

getc:
    mov rax, 0
    mov rdi, 1
    mov rsi, nstr
    mov rdx, 1
    syscall
    xor rax, rax
    mov byte al, [nstr]
    ret

print_num:
    mov r8, nstr
    add r8, 19
    mov byte [r8], 0
pn_loop:
    mov rax, rdi
    xor rdx, rdx
    mov rcx, 10
    idiv rcx
    imul rcx, rax, 10
    sub rdi, rcx
pn_end1:
    add rdi, 48
    dec r8
    mov byte [r8], dil
    cmp rax, 0
    jle pn_end
    mov rdi, rax
    jmp pn_loop
pn_end:
    and rsi, 11b
    sal rsi, 1
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
    mov rax, 1
    mov rsi, r8
    neg r8
    add r8, nstr + 19
    mov rdx, r8
    syscall
    mov rax, 1
    mov rsi, format3
    mov rdx, length3
    syscall
    xor rax, rax
    ret
