#pragma once
#include "video.h"

void pauseBreak();


extern int gScreenProtectWindowID ;


#ifdef DLL_EXPORT
extern "C" __declspec(dllexport) void SpiralVectorGraph();

extern "C" __declspec(dllexport) void SnowScreenShow();

extern "C" __declspec(dllexport) void CubeVectorGraph();
extern "C" __declspec(dllexport) void EllipseScreenColor();
extern "C" __declspec(dllexport) int initScreenProtect();
extern "C" __declspec(dllexport) int stopScreenProtect();

extern "C" __declspec(dllexport) int __kPrintScreen();

extern "C" __declspec(dllexport) void __kScreenProtect(int p1, int p2, int p3, int p4);

extern "C" __declspec(dllexport) void initVectorGraph();

extern "C" __declspec(dllexport)  void VectorGraph(DWORD p1, DWORD p2, DWORD p3, DWORD p4);

extern "C" __declspec(dllexport) void initTrajectory();

extern "C" __declspec(dllexport) void TrajectoryProc(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
#else
extern "C" __declspec(dllimport) void SpiralVectorGraph();
extern "C" __declspec(dllimport) void SnowScreenShow();

extern "C" __declspec(dllimport) void CubeVectorGraph();
extern "C" __declspec(dllimport) void initVectorGraph();
extern "C" __declspec(dllimport) void EllipseScreenColor();
extern "C" __declspec(dllimport) int initScreenProtect();
extern "C" __declspec(dllimport) int stopScreenProtect();

extern "C" __declspec(dllimport) int __kPrintScreen();

extern "C" __declspec(dllimport) void __kScreenProtect(int p1, int p2, int p3, int p4);

extern "C" __declspec(dllimport) void initTrajectory();
#endif


