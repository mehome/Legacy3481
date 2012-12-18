__asm {
	mov eax,[request]
	sub eax,minimum
	//The carry flag will either set this to zero or -1
	sbb edx,edx
	and eax,edx
	add eax,minimum
	}
