#pragma once
#include "video.h"

void pauseBreak();

#ifdef DLL_EXPORT

extern "C" __declspec(dllexport) int initScreenProtect();
extern "C" __declspec(dllexport) int stopScreenProtect();

extern "C" __declspec(dllexport) int __kPrintScreen();

extern "C" __declspec(dllexport) void __kScreenProtect();

extern "C"  void setCursor(int enable, DWORD *x, DWORD *y,unsigned int color);
#else

extern "C" __declspec(dllimport) int initScreenProtect();
extern "C" __declspec(dllimport) int stopScreenProtect();

extern "C" __declspec(dllimport) int __kPrintScreen();

extern "C" __declspec(dllimport) void __kScreenProtect();

extern "C" __declspec(dllimport) void setCursor(int enable, DWORD *x, DWORD *y, unsigned int color);
#endif

int drawCursor();

int restoreCursor();
