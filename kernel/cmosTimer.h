#pragma once
#include "def.h"

#define TIMER_PROC_MAX			16

#define SHUTDOWN_SCREEN_DELAY	360

#pragma pack(1)
typedef struct
{
	DWORD func;
	DWORD ticks;
	DWORD tickcnt;
	DWORD param1;
	DWORD param2;
	DWORD param3;
	DWORD param4;
}TIMER_PROC_PARAM;
#pragma pack()

char * dw2str(int dw);

unsigned short bcd2asc(char bcd);

unsigned char bcd2binary(char bcd);

int __kAddCmosTimer(DWORD addr, DWORD delay, DWORD param1, DWORD param2, DWORD param3, DWORD param4);

void __kRemoveCmosTimer(int no);

void initDPC();

extern "C" __declspec(dllexport) void __kCmosTimer();

extern "C" __declspec(dllexport) void __kCmosExactTimerProc();
