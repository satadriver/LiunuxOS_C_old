#pragma once
#include "def.h"


#pragma pack(1)

typedef struct {
	DWORD no;
	DWORD ss0;
	DWORD gs;	//4
	DWORD fs;	//8
	DWORD es;	//12
	DWORD ds;	//16
	DWORD edi;	//20
	DWORD esi;	//24
	DWORD ebp;	//28
	DWORD esp0;	//32
	DWORD ebx;	//36
	DWORD edx;	//40
	DWORD ecx;	//44
	DWORD eax;	//48

	DWORD		errcode;	//52
	DWORD		eip;		//56
	DWORD		cs;			//60
	DWORD		eflags;		//64
	DWORD		esp3;		//68
	DWORD		ss3;		//72
}EXCEPTIONCODESTACK, *LPEXCEPTIONCODESTACK;

typedef struct {
	DWORD no;
	DWORD ss0;
	DWORD gs;	//4
	DWORD fs;	//8
	DWORD es;	//12
	DWORD ds;	//16
	DWORD edi;	//20
	DWORD esi;	//24
	DWORD ebp;	//28
	DWORD esp0;	//32
	DWORD ebx;	//36
	DWORD edx;	//40
	DWORD ecx;	//44
	DWORD eax;	//48

	DWORD		eip;		//52
	DWORD		cs;			//56
	DWORD		eflags;		//60
	DWORD		esp3;		//64
	DWORD		ss3;		//68
}EXCEPTIONSTACK, *LPEXCEPTIONSTACK;

#pragma pack()

#ifdef DLL_EXPORT

extern "C" __declspec(dllexport) void __kException(DWORD);
#else

extern "C" __declspec(dllimport) void __kException(DWORD);
#endif