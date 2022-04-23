#pragma once

#include "def.h"

#define DOS_SYSTIMER_ADDR		0X46C

#define TIMER0_FREQUENCY		1193182		//1193181.6

extern int SYSTEM_TIMER0_FACTOR;	

extern int g_timeslip;

void systimerProc();