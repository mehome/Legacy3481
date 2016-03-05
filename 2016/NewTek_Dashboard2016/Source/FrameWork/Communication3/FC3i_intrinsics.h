#pragma once

#ifdef _M_IX86 

__forceinline __int64 __cdecl _InterlockedCompareExchange64( __int64 volatile *Destination, __int64 Exchange, __int64 Comparand )
{	__int64 Result_;

	__asm
	{	mov  edx, DWORD PTR [Comparand+4]
		mov  eax, DWORD PTR [Comparand+0]
		mov  ecx, DWORD PTR [Exchange+4]
		mov  ebx, DWORD PTR [Exchange+0]
		mov  edi, Destination

		lock cmpxchg8b qword ptr [edi]

		mov  DWORD PTR [Result_+0], eax
		mov  DWORD PTR [Result_+4], edx
	}

	return Result_;
} 

__forceinline __int64 _InterlockedExchange64( __int64 volatile *Target, __int64 Value )
{
    __int64 Old;

    do {
        Old = *Target;
    } while( _InterlockedCompareExchange64(Target, Value, Old) != Old);

    return Old;
}

#endif
