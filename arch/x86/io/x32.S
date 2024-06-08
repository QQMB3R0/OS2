section .text

global read_port
global write_port
global inw_asm

read_port:
	mov edx, [esp + 4]
				;al is the lower 8 bits of eax
	in al, dx	;dx is the lower 16 bits of edx
	ret

write_port:
	mov   edx, [esp + 4]    
	mov   al, [esp + 4 + 4]  
	out   dx, al  
	ret
inw_asm:
	mov edx,[esp + 4]
	mov	edx, eax
	in 	ax, dx
	pop edx
	ret