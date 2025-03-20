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


typedef struct  
{
	DWORD addr;
	DWORD size;

}MS_HEAP_STRUCT;



#pragma pack()

DWORD getBorderAddr();

int setMemAllocInfo(LPMEMALLOCINFO item, DWORD addr, DWORD vaddr, int size, int pid);

void resetAllMemAllocInfo();

int resetMemAllocInfo(LPMEMALLOCINFO item);

LPMEMALLOCINFO getMemAllocInfo();

int getAlignSize(int size, int allignsize);

LPMEMALLOCINFO isAddrExist(DWORD addr, int size);

LPMEMALLOCINFO findAddr(DWORD addr);

int initMemory();

DWORD pageAlignSize(DWORD size,int max);

DWORD __kProcessMalloc(DWORD s, DWORD *retsize, int pid, DWORD vaddr,int tag);

void freeProcessMemory(int pid);

#ifdef DLL_EXPORT

extern "C"  __declspec(dllexport) int getProcMemory(int pid, char * szout);
extern "C"  __declspec(dllexport) int __free(DWORD addr);
extern "C"  __declspec(dllexport) DWORD __malloc(DWORD s);

extern "C"  __declspec(dllexport) DWORD __kMalloc(DWORD size);

extern "C"  __declspec(dllexport) int __kFree(DWORD buf);
#else
extern "C"  __declspec(dllimport) int getProcMemory(int pid, char * szout);
extern "C"  __declspec(dllimport) int __free(DWORD addr);
extern "C"  __declspec(dllimport) DWORD __malloc(DWORD s);
extern "C"  __declspec(dllimport) DWORD __kMalloc(DWORD size);

extern "C"  __declspec(dllimport) int __kFree(DWORD buf);
#endif

