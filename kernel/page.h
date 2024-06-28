#pragma once

#include "def.h"
#include "ListEntry.h"
#include "malloc.h"


#define PAGE_PRESENT		1
#define PAGE_READWRITE		2
#define PAGE_USERPRIVILEGE	4
#define PAGE_ACCESSED		0X20
#define PAGE_DIRT			0X40


void initPage();

LPMEMALLOCINFO checkPageExist(DWORD addr);

LPMEMALLOCINFO getFreePageItem();

void freeProcessPages();

void enablePage();


#ifdef DLL_EXPORT

extern "C"  __declspec(dllexport) void __kPageFaultProc();

extern "C"  __declspec(dllexport) DWORD __kPageAlloc(int size);

extern "C"  __declspec(dllexport) int __kFreePage(DWORD buf);
#else
extern "C"  __declspec(dllimport) DWORD __kPageAlloc(int size);

extern "C"  __declspec(dllimport) int __kFreePage(DWORD buf);
#endif

