[BITS 32]

global context_switch

section .text

context_switch:
    mov eax, [esp + 4]
    mov edx, [esp + 8]

    pushfd
    pushad

    mov [eax], esp
    mov esp, edx

    popad
    popfd

    ret
