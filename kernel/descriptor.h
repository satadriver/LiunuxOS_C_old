#pragma once
#include "def.h"

#define SYSTEM_DESCRIPTOR		0
#define STORAGE_DESCRIPTOR		1

#define GD0A_32_4G				0xcf
#define GD0A_32_1M				0x4f
#define GD0A_16_64K				0
#define GD0A_16_1M				0x0f

#define SEG_T_CODEEO			0x98
#define SEG_T_CODEER			0x9a
#define SEG_T_DATARO			0x90
#define SEG_T_DATARW			0x92

#define TASKGATE286				0x81
#define LDTGATE					0x82
#define TASKGATE286_B			0x83
#define CALLGATE286				0x84
#define TASKGATE				0x85
#define INTRGATE286				0x86
#define TRAPGATE286				0x87
#define INVALIDTSSDESC			0x9
#define TSSDESC					0x89
#define TSSDESC_B				0x8b
#define CALLGATE				0x8c
#define INTRGATE				0x8e
#define TRAPGATE				0x8f

#pragma pack(1)

typedef struct {
	WORD size;
	DWORD addr;
}DESCRIPTOR_REG;

typedef struct {
	unsigned short	limitLow;
	unsigned short	baseLow;
	unsigned char	baseMid;
	unsigned char	attr;
	unsigned char	gd0a_lh;
	unsigned char	baseHigh;
}SEGDESCRIPTOR, *LPSEGDESCRIPTOR;

typedef struct  
{
	unsigned char jmpcode;		//0xea
	unsigned short seg;
	unsigned long offset;
}JUMP_LONG;

typedef struct
{
	unsigned char callcode;		//0x9a
	unsigned short seg;
	unsigned long offset;
}CALL_LONG;

typedef struct
{
	unsigned char jmpcode;
	unsigned short seg;
	unsigned short offset;
}JUMP_SHORT;

typedef struct {
	unsigned short	addrLow;
	unsigned short	selector;
	unsigned char	paramCnt;
	unsigned char	attr;
	unsigned short	addrHigh;
}SYSDESCRIPTOR, *LPSYSDESCRIPTOR;

#pragma pack()

extern "C" __declspec(dllexport) void sysenterEntry(DWORD * params, DWORD paramslen);

extern "C" __declspec(dllexport) void callgateEntry(DWORD * params, DWORD paramLen);

int sysenterInit(DWORD entryAddr);

void readmsr(DWORD no, DWORD * lowpart, DWORD *highpart);

void writemsr(DWORD no, DWORD lowpart, DWORD highpart);

void initLdt(LPSEGDESCRIPTOR lpldt);

void initCallGate(LPSYSDESCRIPTOR lpcg);

#ifdef DLL_EXPORT
extern "C" __declspec(dllexport) void initEfer();

#else
extern "C" __declspec(dllimport) void initEfer();

#endif



