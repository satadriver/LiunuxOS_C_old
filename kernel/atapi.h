#pragma once

#include "def.h"



int checkAtapiPort(WORD port);

int atapiCmd(unsigned char *cmd);



int readAtapiSector(char * buf, unsigned int secno, unsigned char seccnt);

extern unsigned char gAtapiCmdOpen[12];
extern unsigned char gAtapiCmdClose[12];

#ifdef DLL_EXPORT
extern "C" __declspec(dllexport) int rejectCDROM(int dev);
extern "C" __declspec(dllexport) int getAtapiDev(int disk, int maxno);
#else
extern "C" __declspec(dllimport) int rejectCDROM(int dev);
extern "C" __declspec(dllimport) int getAtapiDev(int disk, int maxno);
#endif
int reject(int dev);


