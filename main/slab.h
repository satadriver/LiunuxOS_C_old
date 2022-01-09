#pragma once

#include "def.h"
#include "ListEntry.h"

#pragma pack(1)

typedef struct 
{
	LIST_ENTRY list;
	DWORD addr;
	DWORD size;
	DWORD pid;
	DWORD vaddr;
}MEMALLOCINFO,*LPMEMALLOCINFO;

typedef struct
{
	unsigned int BaseAddrLow;
	unsigned int BaseAddrHigh;
	unsigned int LengthLow;
	unsigned int LengthHigh;
	unsigned int Type;
}ADDRESS_RANGE_DESCRIPTOR_STRUCTURE;
#pragma pack()


LPMEMALLOCINFO checkAddrExist(DWORD addr);

LPMEMALLOCINFO getFreeMemItem();


int initMemory();

DWORD getAlignSize(DWORD s, int max);

DWORD __kProcessMalloc(DWORD s, DWORD *retsize, int pid, DWORD vaddr);

void freeProcessMemory();

#ifdef DLL_EXPORT

extern "C"  __declspec(dllexport) int formatProcMem(int pid, char * szout);
extern "C"  __declspec(dllexport) int __free(DWORD addr);
extern "C"  __declspec(dllexport) DWORD __malloc(DWORD s);

extern "C"  __declspec(dllexport) DWORD __kMalloc(DWORD size);

extern "C"  __declspec(dllexport) int __kFree(DWORD buf);
#else
extern "C"  __declspec(dllimport) int formatProcMem(int pid, char * szout);
extern "C"  __declspec(dllimport) int __free(DWORD addr);
extern "C"  __declspec(dllimport) DWORD __malloc(DWORD s);
extern "C"  __declspec(dllimport) DWORD __kMalloc(DWORD size);

extern "C"  __declspec(dllimport) int __kFree(DWORD buf);
#endif

