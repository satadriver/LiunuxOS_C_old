#pragma once
#include "def.h"

#define KBD_OUTPUT			1
#define KBD_INPUT			2
#define MOUSE_OUTPUT		3
#define GRAPH_CHAR_OUTPUT	4
#define RANDOM				5
#define SLEEP				6
#define TURNON_SCREEN		7
#define TURNOFF_SCREEN		8
#define CPU_MANUFACTORY		9
#define TIMESTAMP			10
#define SWITCH_SCREEN		11
#define CPUINFO				12
#define DRAW_MOUSE			13
#define RESTORE_MOUSE		14
#define SET_VIDEOMODE		15

// #define DRAW_MOUSE					13
// #define RESTORE_MOUSE				14


extern "C"  __declspec(dllexport) DWORD __kServicesProc(DWORD no,DWORD * params);