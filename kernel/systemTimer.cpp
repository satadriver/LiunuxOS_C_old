#include "systemTimer.h"
#include "hpet.h"

#define DOS_SYSTIMER_ADDR 0X46C

#define SYSTEM_TIMER_FACTOT 11932


void systimerProc() {
	DWORD * lptickcnt = (DWORD *)TIMER0_TICK_COUNT;
	(*lptickcnt)++;
	if (*lptickcnt >= 8640000)
	{
		*lptickcnt = 0;
	}

	DWORD * pdoscounter = (DWORD *)DOS_SYSTIMER_ADDR;
	*pdoscounter = *lptickcnt;

	heptEOI();
}