#pragma once
#include "def.h"

//#define LIUNUX_DEBUG_FLAG

#define HD_READ_COMMAND			0X20
#define HD_WRITE_COMMAND		0X30

#define HD_LBA48READ_COMMAND	0X24
#define HD_LBA48WRITE_COMMAND	0X34

//multiple read command can read more than one sector with instruction rep insw/insb/insd
#define HD_MUTIPLEREAD_COMMAND		0X29
#define HD_MUTIPLEWRITE_COMMAND		0X39

#define DEVICE_MASTER			0XE0
#define DEVICE_SLAVE			0XF0

#define ONCE_READ_LIMIT			128

#pragma pack(1)

typedef struct
{
	unsigned char len;
	unsigned char reserved;
	unsigned short seccnt;
	unsigned int segoff;
	unsigned int secnolow;
	unsigned int secnohigh;
}INT13PAT, *LPINT13PAT;


typedef struct
{
	unsigned char bwork;	
	unsigned char intno;
	unsigned int reax;		//2
	unsigned int recx;		//6
	unsigned int redx;		//a
	unsigned int rebx;		//e
	unsigned int resi;		//12
	unsigned int redi;		//16
	unsigned short res;		//1a
	unsigned short rds;		//1c
	unsigned int result;	//1e
}V86VMIPARAMS, *LPV86VMIPARAMS;

#pragma pack()

int getHarddiskInfo(char * buf);

int readSectorLBA48(unsigned int secnoLow, unsigned int secnoHigh, unsigned char seccnt, char * buf,int device);

int writeSectorLBA48(unsigned int secnoLow, unsigned int secnoHigh, unsigned char seccnt, char * buf, int device);


int readSectorLBA24(unsigned int secno, unsigned char seccnt, char * buf, int device);

int writeSectorLBA24(unsigned int secno, unsigned char seccnt, char * buf, int device);


int readSectorLBA24Mimo(unsigned int secno, unsigned char seccnt, char * buf, int device);

int readSectorLBA48Mimo(unsigned int secnoLow, unsigned int secnoHigh, unsigned char seccnt, char * buf, int device);

int waitFree(WORD port);
int waitComplete(WORD port);

int testHdPort(unsigned short port);

int testHdPortMimo(unsigned short port);

void __initStatusPort(unsigned char master_slave);

int getHdPort();

int readPortSector(unsigned int secno, DWORD secnohigh, unsigned int seccnt, char * buf);
int writePortSector(unsigned int secno, DWORD secnohigh, unsigned int seccnt, char * buf);

int vm86ReadSector(unsigned int secno, DWORD secnohigh, unsigned int seccnt, char * buf);
int vm86WriteSector(unsigned int secno, DWORD secnohigh, unsigned int seccnt, char * buf);

#ifdef DLL_EXPORT
extern "C"  __declspec(dllexport)  int vm86ReadBlock(unsigned int secno, DWORD secnohigh, unsigned short seccnt, char * buf, int disk,int sectorsize);
extern "C"  __declspec(dllexport)  int vm86WriteBlock(unsigned int secno, DWORD secnohigh, unsigned short seccnt, char * buf, int disk, int sectorsize);
extern "C"  __declspec(dllexport)  int(__cdecl * readSector)(unsigned int secnolow,DWORD secnohigh, unsigned int seccnt, char * buf);
extern "C"  __declspec(dllexport)  int(__cdecl * writeSector)(unsigned int secnolow,DWORD secnohigh, unsigned int seccnt, char * buf);
#else
extern "C"  __declspec(dllimport)  int vm86ReadBlock(unsigned int secno, DWORD secnohigh, unsigned short seccnt, char * buf, int disk, int sectorsize);
extern "C"  __declspec(dllimport)  int vm86WriteBlock(unsigned int secno, DWORD secnohigh, unsigned short seccnt, char * buf, int disk, int sectorsize);
extern "C"  __declspec(dllimport)  int(__cdecl * readSector)(unsigned int secnolow, DWORD secnohigh, unsigned int seccnt, char * buf);
extern "C"  __declspec(dllimport)  int(__cdecl * writeSector)(unsigned int secnolow, DWORD secnohigh, unsigned int seccnt, char * buf);
#endif




 


