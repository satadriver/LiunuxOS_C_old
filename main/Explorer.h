#pragma once


#include "def.h"

extern "C" __declspec(dllexport) int __kExplorer(unsigned int retaddr, int tid, char * filename, char * funcname, DWORD param);