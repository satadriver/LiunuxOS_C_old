#pragma once
#include "def.h"
#include "video.h"



extern "C" __declspec(dllexport) void initRightMenu(RIGHTMENU * menu, int tid);

extern "C" __declspec(dllexport) int __kDrawWindowsMenu();

extern "C" __declspec(dllexport) int __drawRightMenu(RIGHTMENU* menu);

extern "C" __declspec(dllexport) int __restoreRightMenu(RIGHTMENU* menu);




