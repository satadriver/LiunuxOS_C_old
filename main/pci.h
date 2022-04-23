#pragma once


// DW | Byte3 | Byte2 | Byte1 | Byte0 | Addr
// -- - +-------------------------------------------------------- - +---- -
// 0 | Device ID | Vendor ID | 00
// -- - +-------------------------------------------------------- - +---- -
// 1 | Status　　　　　 | Command　　　　　　 | 04
// -- - +-------------------------------------------------------- - +---- -
// 2 | Class Code　　　　　　　　 | Revision ID　 | 08
// -- - +-------------------------------------------------------- - +---- -
// 3 | BIST　　 | Header Type | Latency Timer | Cache Line | 0C
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
// 10 | CardBus CIS pointer　　　　　　　　　 | 28
// -- - +-------------------------------------------------------- - +---- -
// 11 | Subsystem Device ID　　 | Subsystem Vendor ID　　 | 2C
// -- - +-------------------------------------------------------- - +---- -
// 12 | Expansion ROM Base Address　　　　　　　　 | 30
// -- - +-------------------------------------------------------- - +---- -
// 13 | Reserved(Capability List) | 34
// -- - +-------------------------------------------------------- - +---- -
// 14 | Reserved　　　　　　　　　　　　　 | 38
// -- - +-------------------------------------------------------- - +---- -
// 15 | Max_Lat　 | Min_Gnt　 | IRQ Pin　 | IRQ Line　　 | 3C
// ------------------------------------------------------------------ -


//BAR最后一位为0表示这是映射的IO内存，为1是表示这是IO 端口，当是IO内存的时候1-2位表示内存的类型，
//bit 2为1表示采用64位地址，为0表示采用32位地址。bit1为1表示区间大小超过1M，为0表示不超过1M.bit3表示是否支持可预取

//1111 1111 1111 1111 1111 1111 1111 1111
#include "def.h"



int getBasePort(DWORD * baseregs, WORD devClsVender,DWORD * dev, DWORD * irqpin);

int getNetcard(DWORD * regs,DWORD * dev, DWORD * irq);

int getSvga(DWORD * regs,DWORD * dev, DWORD * irq);

int getSdcard(DWORD * regs,DWORD * dev, DWORD * irq);
int getSmbus(DWORD * regs,DWORD * dev, DWORD * irq);

int getUsb(DWORD * regs,DWORD * dev, DWORD * irq);

#ifdef DLL_EXPORT

extern "C"  __declspec(dllexport) int showPciDevs();
#else
extern "C"  __declspec(dllimport) int showPciDevs();
#endif




int listpci(DWORD *dst);
void showAllPciDevs();