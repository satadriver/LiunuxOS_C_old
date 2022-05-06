#pragma once

#include "def.h"

#pragma pack(1)
typedef struct
{
	DWORD addr;
	DWORD interval;
	DWORD param;
}CMOSALARM_PROCESS_LIST;
#pragma pack()

unsigned char readCmosPort(unsigned char port);

void writeCmosPort(unsigned char port, unsigned char value);

unsigned char bcd2b(unsigned char bcd);
unsigned char b2bcd(unsigned char b);

unsigned short makehalf(unsigned char low, unsigned char high);

extern "C" __declspec(dllexport) void __kCmosAlarmProc();

extern "C" __declspec(dllexport) int __kAddCmosAlarm(DWORD interval, DWORD addr, DWORD param);

extern "C" __declspec(dllexport) void __kRemoveCmosAlarm();

int isLeapYear(int year);

int getDayOfMonth(int year, int month);

extern "C" __declspec(dllexport) void __doAlarmTask(DWORD  param);

//void __doAlarmTask(DWORD retaddr, DWORD pid, char * fname, char * funcname, DWORD param);

