#pragma once


// DW | Byte3 | Byte2 | Byte1 | Byte0 | Addr
// -- - +-------------------------------------------------------- - +---- -
// 0 | Device ID | Vendor ID | 00
// -- - +-------------------------------------------------------- - +---- -
// 1 | Status���������� | Command������������ | 04
// -- - +-------------------------------------------------------- - +---- -
// 2 | Class Code���������������� | Revision ID�� | 08
// -- - +-------------------------------------------------------- - +---- -
// 3 | BIST���� | Header Type | Latency Timer | Cache Line | 0C
// -- - +-------------------------------------------------------- - +---- -
// 4 | Base Address 0 | 10
// -- - +-------------------------------------------------------- - +---- -
// 5 | Base Address 1 | 14
// -- - +-------------------------------------------------------- - +---- -
// 6 | Base Address 2 | 18
// -- - +-------------------------------------------------------- - +---- -
// 7 | Base Address 3 | 1C
// -- - +-------------------------------------------------------- - +---- -
// 8 | Base Address 4 | 20
// -- - +-------------------------------------------------------- - +---- -
// 9 | Base Address 5 | 24
// -- - +-------------------------------------------------------- - +---- -
// 10 | CardBus CIS pointer������������������ | 28
// -- - +-------------------------------------------------------- - +---- -
// 11 | Subsystem Device ID���� | Subsystem Vendor ID���� | 2C
// -- - +-------------------------------------------------------- - +---- -
// 12 | Expansion ROM Base Address���������������� | 30
// -- - +-------------------------------------------------------- - +---- -
// 13 | Reserved(Capability List) | 34
// -- - +-------------------------------------------------------- - +---- -
// 14 | Reserved�������������������������� | 38
// -- - +-------------------------------------------------------- - +---- -
// 15 | Max_Lat�� | Min_Gnt�� | IRQ Pin�� | IRQ Line���� | 3C
// ------------------------------------------------------------------ -


// bit0λΪ0��ʾ����ӳ���IO�ڴ棬Ϊ1�Ǳ�ʾ����IO �˿ڣ�
// ����IO�ڴ��ʱ��1-2λ��ʾ�ڴ�����ͣ�
// bit1Ϊ1��ʾ�����С����1M��Ϊ0��ʾ������1M
// bit2Ϊ1��ʾ����64λ��ַ��Ϊ0��ʾ����32λ��ַ
// bit3��ʾ�Ƿ�֧�ֿ�Ԥȡ


#include "def.h"



#pragma pack(push,1)

typedef struct
{
	char low:2;
	char reg : 5;
	char reserved:1;

	char func : 3;
	char dev : 5;
	char bus;
	char unused : 7;
	char enable : 1;

}PCI_CONFIG_VALUE;


#pragma pack(pop)



extern "C" __declspec(dllexport) int listpci(DWORD * dst);

DWORD makePciAddr(int bus, int dev, int func, int reg);

int getPciDevBasePort(DWORD* baseregs, WORD devClsVender, DWORD* dev, DWORD* vd);

int getNetcard(DWORD* regs, DWORD* dev, DWORD* irq);

int getSvga(DWORD* regs, DWORD* dev, DWORD* irq);

int getSoundcard(DWORD* regs, DWORD* dev, DWORD* irq);

int getSmbus(DWORD* regs, DWORD* dev, DWORD* irq);

int getUsb(DWORD* regs, DWORD* dev, DWORD* irq);

int getLPC(DWORD* regs, DWORD* dev, DWORD* irq);

int getNorthBridge(DWORD* regs, DWORD* dev, DWORD* irq);

#ifdef DLL_EXPORT

extern "C"  __declspec(dllexport) int showPciDevs();
extern "C"  __declspec(dllexport) void showAllPciDevs();
#else
extern "C"  __declspec(dllimport) int showPciDevs();
extern "C"  __declspec(dllimport) void showAllPciDevs();
#endif




