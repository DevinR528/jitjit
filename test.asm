
.model flat, c
.stack 4096

ExitProcess PROTO

.data

.code
main PROC
	push ebp
	mov ebp, esp
	sub esp, 20h
	mov esp, ebp
	pop ebp
	ret
main ENDP

END
