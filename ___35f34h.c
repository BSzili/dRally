#include "drally.h"
#include "drally_structs_fixed.h"

	extern __BYTE__ ___1a0ef8h[];
	extern __BYTE__ ___1a01e0h[];

void ___35f34h_cdecl(__DWORD__ A1, __DWORD__ A2, __DWORD__ A3){

	__DWORD__ 	eax, ebx, ecx, edx, edi, esi, ebp;
	__BYTE__ 	esp[0x14];
	racer_t * 	s_6c;

	s_6c = (racer_t *)___1a01e0h;
	eax = A1;
	edx = A2;
	ebx = A3;

		D(esp+0xc) = eax;
		D(esp+0x10) = edx;
		D(esp+0x8) = ebx;
___35f51h:
		esi = D(esp+0x10);
		ecx = D(esp+0xc);
		edx = ecx+esi;
		eax = edx;
		edx = (int)edx>>0x1f;
		eax -= edx;
		eax = (int)eax>>1;
		ebp = D(esp+0x8);
		ebp <<= 2;
		edi = B(eax+ebp+___1a0ef8h);
		edi = 0x6c*edi;
		D(esp+0x4) = ebp;
		edi = s_6c[edi/0x6c].car;
___35f81h:
		L(eax) = B(ecx+ebp+___1a0ef8h);
		eax &= 0xff;
		eax = 0x6c*eax;
		if((int)edi <= (int)s_6c[eax/0x6c].car) goto ___35f9bh;
		ecx++;
		goto ___35f81h;
___35f9bh:
		eax = D(esp+0x4);
		eax += esi;
___35fa1h:
		ebx ^= ebx;
		L(ebx) = B(eax+___1a0ef8h);
		ebx = 0x6c*ebx;
		edx = esi-1;
		if((int)edi >= (int)s_6c[ebx/0x6c].car) goto ___35fbch;
		esi = edx;
		eax--;
		goto ___35fa1h;
___35fbch:
		if((int)ecx > (int)esi) goto ___35fe9h;
		esi = D(esp+0x4);
		ebx ^= ebx;
		L(ebx) = B(ecx+esi+___1a0ef8h);
		D(esp) = ebx;
		L(ebx) = B(eax+___1a0ef8h);
		B(ecx+esi+___1a0ef8h) = L(ebx);
		ecx++;
#ifdef __AMIGA__
		L(ebx) = D(esp);
#else
		L(ebx) = B(esp);
#endif
		esi = edx;
		B(eax+___1a0ef8h) = L(ebx);
___35fe9h:
		if((int)ecx < (int)esi) goto ___35f81h;
		if((int)esi <= (int)D(esp+0xc)) goto ___36002h;
		ebx = D(esp+0x8);
		eax = D(esp+0xc);
		edx = esi;
		___35f34h_cdecl(eax, edx, ebx);
___36002h:
		if((int)ecx >= (int)D(esp+0x10)) goto ___36011h;
		D(esp+0xc) = ecx;
		goto ___35f51h;
___36011h:
		return;
}
