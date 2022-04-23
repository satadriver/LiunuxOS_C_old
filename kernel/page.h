#pragma once

#include "def.h"
#include "ListEntry.h"
#include "slab.h"




void initPage();

LPMEMALLOCINFO checkPageExist(DWORD addr);

LPMEMALLOCINFO getFreePageItem();

void freeProcessPages();


#ifdef DLL_EXPORT

extern "C"  __declspec(dllexport) void __kPageFaultProc();

extern "C"  __declspec(dllexport) DWORD __kPageAlloc(int size);

extern "C"  __declspec(dllexport) int __kFreePage(DWORD buf);
#else
extern "C"  __declspec(dllimport) DWORD __kPageAlloc(int size);

extern "C"  __declspec(dllimport) int __kFreePage(DWORD buf);
#endif

