cpu 386
%include "macros.inc"

	extern	__CHK
	extern	dRally_System_clean
	extern	VGA3_SETMODE
	extern	___183e84h
	extern	printf_
	extern	___19bd60h
	extern	___623d4h
	extern	exit_

section .text

__GDECL(___492f4h)
		push    14h
		call    near __CHK
		push    ebx
		push    ecx
		push    edx

	push 	eax
	push 	ecx
	push 	edx
		call    dRally_System_clean
	pop 	edx
	pop 	ecx
	pop 	eax
	
		call    near VGA3_SETMODE
		push    ___183e84h
		call    near printf_
		mov     edx, [___19bd60h]
		add     esp, byte 4
		test    edx, edx
		je      short ___4932ch

	push 	edx
	push 	ecx
	push 	eax
		call    ___623d4h
	pop 	eax
	pop 	ecx
	pop 	edx

___4932ch:
		mov     eax, 70h
		call    near exit_
		pop     edx
		pop     ecx
		pop     ebx
		retn    
