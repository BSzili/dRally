cpu 386
%include "macros.inc"

	extern	__CHK
	extern	___5a101h
	extern	___5ec04h_freeMemPool
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
		call    near ___5a101h
		call    near ___5ec04h_freeMemPool
		call    near VGA3_SETMODE
		push    ___183e84h
		call    near printf_
		mov     edx, [___19bd60h]
		add     esp, byte 4
		test    edx, edx
		je      short ___4932ch
		call    near ___623d4h
___4932ch:
		mov     eax, 70h
		call    near exit_
		pop     edx
		pop     ecx
		pop     ebx
		retn    
