#pragma once
#include "def.h"

char * dw2str(int dw);

unsigned short bcd2asc(char bcd);

unsigned char bcd2binary(char bcd);

int __kAddCmosTimer(DWORD addr, DWORD delay, DWORD param1, DWORD param2, DWORD param3, DWORD param4);

void __kRemoveCmosTimer(int no);

void initDPC();

extern "C" __declspec(dllexport) void __kCmosTimer();

extern "C" __declspec(dllexport) void __kCmosExactTimerProc();
