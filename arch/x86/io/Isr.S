section .text

extern ih_keyboard
global asm_ih_keyboard
asm_ih_keyboard:
    push ax;
	call ih_keyboard
    pop ax;
iret
global pause
pause:
	hlt
	ret
;.end