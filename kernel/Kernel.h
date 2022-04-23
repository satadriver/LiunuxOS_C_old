#pragma once

#include "descriptor.h"
#include "def.h"
#include "video.h"





#ifdef DLL_EXPORT

extern "C" __declspec(dllexport)  LPSYSDESCRIPTOR glpCallGate;
extern "C" __declspec(dllexport)  LPSEGDESCRIPTOR glpLdt;
extern "C" __declspec(dllexport)  LPSEGDESCRIPTOR glpGdt;

extern "C" __declspec(dllexport)  LPSYSDESCRIPTOR glpIdt;

extern "C" __declspec(dllexport)  DWORD gV86VMIEntry;

extern "C" __declspec(dllexport)  DWORD gV86VMLeave;

extern "C" __declspec(dllexport)  DWORD gKernel16;
extern "C" __declspec(dllexport)  DWORD gKernel32;
extern "C" __declspec(dllexport)  DWORD gKernelData;
extern "C" __declspec(dllexport)  DWORD gAsmTsses;

extern "C" __declspec(dllexport) void __kKernelMain(DWORD retaddr, int pid, char * pname,char * funcname,DWORD param);

extern "C" __declspec(dllexport) int __kernelEntry( LPVESAINFORMATION vesa, DWORD fontbase, 
	DWORD v86Proc, DWORD v86Leave,DWORD kerneldata, DWORD kernel16, DWORD kernel32, DWORD lpasmTsses);
#else
extern "C" __declspec(dllimport)  LPSYSDESCRIPTOR glpCallGate;
extern "C" __declspec(dllimport)  LPSEGDESCRIPTOR glpLdt;
extern "C" __declspec(dllimport)  LPSEGDESCRIPTOR glpGdt;

extern "C" __declspec(dllimport)  LPSYSDESCRIPTOR glpIdt;

extern "C" __declspec(dllimport)  DWORD gV86VMIEntry;

extern "C" __declspec(dllimport)  DWORD gV86VMLeave;

extern "C" __declspec(dllimport)  DWORD gKernel16;
extern "C" __declspec(dllimport)  DWORD gKernel32;
extern "C" __declspec(dllimport)  DWORD gKernelData;

extern "C" __declspec(dllimport) void __kKernelMain(DWORD retaddr, int pid, char * pname, char * funcname, DWORD param);


extern "C" __declspec(dllimport) int __kernelEntry(LPVESAINFORMATION vesa, DWORD fontbase, DWORD v86Proc, DWORD v86Leave,
	DWORD kerneldata, DWORD kernel16, DWORD kernel32, DWORD lpasmTsses);
#endif






