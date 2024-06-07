global idt_load

section .text
idt_load:
    jmp pass
    pointer: dw 0
    dq 0
    pass:
    mov edx,[esp + 4]
    lea eax, pointer
    lidt[edx]
    sidt [eax]
    ret
;.end