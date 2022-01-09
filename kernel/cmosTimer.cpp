
#include "cmosTimer.h"
#include "cmosAlarm.h"
#include "Utils.h"
#include "video.h"
#include "ListEntry.h"


char * dw2str(int dw) {
	if (dw == 0)
	{
		return "Sunday";
	}else if (dw == 1)
	{
		return "Monday";
	}
	else if (dw == 2)
	{
		return "Tuesday";
	}
	else if (dw == 3)
	{
		return "Wednesday";
	}
	else if (dw == 4)
	{
		return "Thursday";
	}
	else if (dw == 5)
	{
		return "Friday";
	}
	else if (dw == 6)
	{
		return "Saturday";
	}
	else {
		return "Other";
	}
}

#define SHUTDOWN_SCREEN_DELAY 360

unsigned char bcd2binary(char bcd) {
	int low = (bcd & 0xf) ;
	int high = (bcd >> 4)*10;
	return low + high;
}


unsigned short bcd2asc(char bcd) {
	int low = (bcd & 0xf) + 0x30;
	int high = (bcd >> 4) + 0x30;
	return (low << 8) + high;
}

void __kCmosTimer() {
	char c = readCmosPort(0x32);
	char y = readCmosPort(9);
	char m = readCmosPort(8);
	char d = readCmosPort(7);
	char hour = readCmosPort(4);
	char minute = readCmosPort(2);
	char second = readCmosPort(0);

	int strc = bcd2asc(c);
	int stry = bcd2asc(y);
	int strm = bcd2asc(m);
	int strd = bcd2asc(d);
	int strhour = bcd2asc(hour);
	int strminute = bcd2asc(minute);
	int strsecond = bcd2asc(second);

	char dw = readCmosPort(6);
	char *strdw = dw2str(dw);

	char szout[256];
	
	char*  singlefmt = "%s%s/%s/%s [%s] %s:%s:%s";
	char* doublefmt =  "%s%s-%s-%s [%s] %s:%s:%s";
	char*  thirdfmt =  "%s%s\\%s\\%s [%s] %s:%s:%s";
	char * fmt = 0;

	int fontcolor = 0;

	int binsec = bcd2binary(second);
	if ((binsec % 3) == 0)
	{
		fmt = singlefmt;
		fontcolor = CMOS_TIMESTAMP_SINGLE_COLOR;
	}
	else if ((binsec % 3) == 1)
	{
		fmt = doublefmt;
		fontcolor = CMOS_TIMESTAMP_DOUBLE_COLOR;
	}else if ((binsec % 3) == 2)
	{
		fmt = thirdfmt;
		fontcolor = CMOS_TIMESTAMP_THIRD_COLOR;
	}

	__printf(szout,fmt , &strc, &stry, &strm, &strd, strdw, &strhour, &strminute, &strsecond);
	__strcpy((char*)CMOS_DATETIME_STRING, szout);

	DWORD * lptickcnt = (DWORD*)CMOS_SECONDS_TOTAL;
	(*lptickcnt)++;
	if (*lptickcnt >= SHUTDOWN_SCREEN_DELAY)
	{
		__asm {
			mov eax, 8
			int 80h
		}
		*lptickcnt = 0;
	}

	DWORD pos = (gVideoHeight - GRAPHCHAR_HEIGHT) * gVideoWidth * gBytesPerPixel;
	__drawGraphChar((unsigned char*)szout, fontcolor, pos, TASKBARCOLOR);
}


#define TIMER_PROC_MAX 16

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

TIMER_PROC_PARAM gCmosTimerDPC[TIMER_PROC_MAX] = { 0 };

void initDPC() {
	__memset((char*)gCmosTimerDPC, 0, TIMER_PROC_MAX * sizeof(TIMER_PROC_PARAM));
}


int __kAddCmosTimer(DWORD addr,DWORD delay,DWORD param1,DWORD param2,DWORD param3,DWORD param4) {
	if (addr == 0 || delay == 0)
	{
		return -1;
	}

//  	char szout[1024];
// 	__printf(szout, "__kAddCmosTimer addr:%x,delay:%d,param1:%x,param2:%x,param3:%x,param4:%x\r\n", addr,delay,param1,param2,param3,param4);
// 	__drawGraphChars((unsigned char*)szout, 0);

	DWORD * lptickcnt = (DWORD*)CMOS_TICK_COUNT;

	DWORD ticks = delay / 15;

	int i = 0;
	for ( i = 0;i < TIMER_PROC_MAX;i ++)
	{
		if (gCmosTimerDPC[i].func == 0 && gCmosTimerDPC[i].tickcnt == 0)
		{
			gCmosTimerDPC[i].func = addr;
			gCmosTimerDPC[i].ticks = ticks;
			gCmosTimerDPC[i].tickcnt = *lptickcnt + ticks;
			gCmosTimerDPC[i].param1 = param1;
			gCmosTimerDPC[i].param2 = param2;
			gCmosTimerDPC[i].param3 = param3;
			gCmosTimerDPC[i].param4 = param4;
			break;
		}
	}

	return i;
}



void __kRemoveCmosTimer(int no) {
	if (no >= 0 && no < TIMER_PROC_MAX)
	{
		gCmosTimerDPC[no].func = 0;
		gCmosTimerDPC[no].tickcnt = 0;
	}
}



void __kCmosExactTimerProc() {

	int result = 0;
	DWORD * lptickcnt = (DWORD*)CMOS_TICK_COUNT;
	DWORD tickcnt = *lptickcnt;
	(*lptickcnt)++;

	for (int i = 0;i < TIMER_PROC_MAX;i ++)
	{
		if (gCmosTimerDPC[i].func)
		{
			if (gCmosTimerDPC[i].tickcnt <= *lptickcnt)
			{
// 				char szout[1024];
// 				__printf(szout,"__kCmosExactTimerProc\r\n");
// 				__drawGraphChars((unsigned char*)szout, 0);

				gCmosTimerDPC[i].tickcnt = *lptickcnt + gCmosTimerDPC[i].ticks;

				typedef int(*ptrfunction)(DWORD param1,DWORD param2,DWORD param3,DWORD param4);
				ptrfunction lpfunction = (ptrfunction)gCmosTimerDPC[i].func;
				result = lpfunction(gCmosTimerDPC[i].param1, gCmosTimerDPC[i].param2,
					gCmosTimerDPC[i].param3, gCmosTimerDPC[i].param4 );
			}
		}
	}
}