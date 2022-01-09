#pragma once

#include "def.h"
#include "process.h"


#pragma pack(1)

typedef struct  
{
	DWORD address;
	DWORD status;
	DWORD pid;
}DOS_PE_CONTROL,*LPDOS_PE_CONTROL;

#pragma pack()

int __initDosTss(LPPROCESS_INFO tss, int num, DWORD addr, char * filename, char * funcname, DWORD syslevel, DWORD runparam);

int getDosPeAddr(DWORD filedata,int pid);

int relocDos(DWORD loadseg);

DWORD __initDosExe(DWORD filedata, int filesize,int pid);