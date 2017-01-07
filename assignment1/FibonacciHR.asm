section .data
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

main:
    push rbx
    mov rbx, -1 ;store number
    xor rcx, rcx ;numbers count
loop_input:
    push rcx
    call getc
    pop rcx
    cmp rax, 10
    jne not_return
    ;return key
    cmp rbx, 0
    jge calc
    je end
not_return:
    cmp rax, 32
    jne not_blank
    ;space key
    cmp rbx, 0
    jl loop_input
calc:
    mov rdi, rbx
    push rax
    push rcx
    call fib
    pop rcx
    mov rdi, rcx
    push rcx
    call print_num
    pop rcx
    pop rax
    cmp rax, 10
    je end
    mov rbx, -1
    add rcx, 1
    jmp loop_input
not_blank:
    cmp rax, 48
    jl loop_input
    cmp rax, 57
    jg loop_input
    ; a number
    cmp rbx, 0
    jge number
    xor rbx, rbx
number:
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
    ;clear memory first
    xor rax, rax
fib_clean:
    mov qword [numa + rax * 8], 0
    inc rax
    cmp rax, 32
    jl fib_clean
    ;clear end
    xor rax, rax ;count
    xor rbx, rbx ;carry
    mov byte [numa], 1 ;let a = 1
fib_loop:
    inc rax
    cmp rax, rdi
    jg fib_end
    ;add numa + numb -> numb
    xor rcx, rcx ;bias for numa/numb
fib_add:
    xor r8, r8 ;digit in numa
    xor r9, r9 ;digit in numb
    mov byte r8b, [numa + rcx]
    mov byte r9b, [numb + rcx]
    mov r10, r8
    add r10, r9
    add r10, rbx
    xor rbx, rbx ;clear carry
    cmp r10, 10
    jl fib_added
    mov rbx, 1
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
    pop rbx
    ret

getc:
    mov rax, 0
    mov rdi, 1
    mov rsi, iobuf
    mov rdx, 1
    syscall
    xor rax, rax
    mov byte al, [iobuf]
    ret

print_num:
    mov rsi, rdi
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
    ;print numb
    mov rcx, numb + 128
print_loop:
    dec rcx
    cmp rcx, numb
    jle print_loop1
    cmp byte [rcx], 0
    jne print_loop1
    jmp print_loop
print_loop1:
    cmp rcx, numb
    jl print_loop_end
    xor rdx, rdx
    mov byte dl, [rcx]
    add rdx, 48
    mov byte [iobuf], dl
    push rcx
    mov rax, 1
    mov rsi, iobuf
    mov rdx, 1
    syscall
    pop rcx
    dec rcx
    jmp print_loop1
print_loop_end:
    mov rax, 1
    mov rsi, format3
    mov rdx, length3
    syscall
    xor rax, rax
    ret
