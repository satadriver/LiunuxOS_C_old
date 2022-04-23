#pragma once
#include "def.h"

#define DATA_BREAKPOINT 		 1
#define PORT_BREAKPOINT			 2
#define INSTRUCTION_BREAKPOINT 	 3
#define BD_BREAKPOINT 			 4
#define BS_BREAKPOINT 			 5
#define BT_BREAKPOINT 			 6
#define INT3_BREAKPOINT 		 7

void initDebugger();



#ifdef DLL_EXPORT

extern "C" __declspec(dllexport) int codeBreakPoint(unsigned int * addr, int len);
extern "C" __declspec(dllexport) int ioBreakPoint(unsigned int * addr, int len);
extern "C" __declspec(dllexport) int dataWBreakPoint(unsigned int * addr, int len);
extern "C" __declspec(dllexport) int dataRWBreakPoint(unsigned int * addr, int len);

extern "C" __declspec(dllexport) int enterSingleStep();
extern "C" __declspec(dllexport) int clearSingleStep();
extern "C" __declspec(dllexport) int enterGdDebugger();
extern "C" __declspec(dllexport) int clearGdDebugger();

extern "C" __declspec(dllexport) void __kdBreakPoint();

extern "C" __declspec(dllexport) void __kBreakPoint(unsigned int * regs);

extern "C" __declspec(dllexport) void __kDebugger(unsigned int * regs);
#else
extern "C" __declspec(dllimport) int codeBreakPoint(unsigned int * addr, int len);
extern "C" __declspec(dllimport) int ioBreakPoint(unsigned int * addr, int len);
extern "C" __declspec(dllimport) int dataWBreakPoint(unsigned int * addr, int len);
extern "C" __declspec(dllimport) int dataRWBreakPoint(unsigned int * addr, int len);

extern "C" __declspec(dllimport) int enterSingleStep();
extern "C" __declspec(dllimport) int clearSingleStep();
extern "C" __declspec(dllimport) int enterGdDebugger();
extern "C" __declspec(dllimport) int clearGdDebugger();

extern "C" __declspec(dllimport) void __kdBreakPoint();

extern "C" __declspec(dllimport) void __kdBreakPoint();

extern "C" __declspec(dllimport) void __kBreakPoint(unsigned int * regs);

extern "C" __declspec(dllimport) void __kDebugger(unsigned int * regs);
#endif