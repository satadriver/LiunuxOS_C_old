#pragma once

#include "def.h"

#ifndef _UTILS_H_H_H
#define _UTILS_H_H_H

#pragma pack(1)
typedef struct  
{
	DWORD low;
	DWORD high;
}MYINT64,*LPMYINT64;


typedef struct  
{
	WORD year;
	WORD month;
	WORD dayInMonth;
	WORD dayInWeek;
	WORD hour;
	WORD minute;
	WORD second;
	WORD milisecond;
}DATETIME,*LPDATETIME;

#pragma pack()


#ifdef DLL_EXPORT
extern "C"  __declspec(dllexport) int __getDateTime(LPDATETIME datetime);
extern "C"  __declspec(dllexport) int __getDateTimeStr(void * str);

extern "C"  __declspec(dllexport) int __strlen(char * str);

extern "C"  __declspec(dllexport) int __memcpy(char * dst, char * src, int len);

extern "C"  __declspec(dllexport) int __strcpy(char * dst, char * src);

extern "C"  __declspec(dllexport) int __strncpy(char * dst, char * src, int limit);

extern "C"  __declspec(dllexport) int __memset(char * dst, int value, int len);
extern "C"  __declspec(dllexport) int __memset4(char * dst, int value, int len);

extern "C"  __declspec(dllexport) int __strcmp(char * src, char * dst);

extern "C"  __declspec(dllexport) int __memcmp(char * src, char * dst, int len);

extern "C"  __declspec(dllexport) char* __strstr(char * src, char * dst);

extern "C"  __declspec(dllexport) int __strcat(char * src, char * dst);

extern "C"  __declspec(dllexport) int __h2strh(unsigned int n, int lowercase, unsigned char * dst);

extern "C"  __declspec(dllexport) int __hstr2i(unsigned char * str);

extern "C"  __declspec(dllexport) int __h2strd(unsigned int h, char * strd);

extern "C"  __declspec(dllexport) int __strd2i(char * istr);

extern "C"  __declspec(dllexport) int __printf(char * buf, char * format, ...);

extern "C"  __declspec(dllexport) int __sprintf(char * buf, char * format, ...); 

extern "C"  __declspec(dllexport) int __dump(char * src, int len, int lowercase, unsigned char * dstbuf);

extern "C" __declspec(dllexport) int __substr(char * src, int pos, char * dst);
extern "C" __declspec(dllexport) int __substrLen(char * src, int pos, int len, char * dst);

extern "C" __declspec(dllexport) int __strupr(char * str);

extern "C" __declspec(dllexport) int __strlwr(char * str);

extern "C" __declspec(dllexport) int timer0Wait(unsigned short);


extern "C" __declspec(dllexport) int __shutdownSystem();
extern "C" __declspec(dllexport) int __reset();

extern "C" __declspec(dllexport) int __sleep(int millisecs);

extern "C" __declspec(dllexport) int getCpuType(char * name);

extern "C" __declspec(dllexport) int getCpuInfo(char * name);

extern "C" __declspec(dllexport) unsigned int random();

extern "C" __declspec(dllexport) int lower2upper(char *data, int len);

extern "C" __declspec(dllexport) int upper2lower(char *data, int len);
extern "C" __declspec(dllexport) DWORD __sqrtInteger(DWORD i);

extern "C" __declspec(dllexport) WORD __ntohs(WORD v);

extern "C" __declspec(dllexport)DWORD __ntohl(DWORD v);
extern "C" __declspec(dllexport)DWORD __inc(DWORD *v);
extern "C" __declspec(dllexport)void __initSpinLock(DWORD * v);
extern "C" __declspec(dllexport)WORD __enterSpinLock(DWORD * v);
extern "C" __declspec(dllexport)WORD __leaveSpinLock(DWORD * v);

extern "C" __declspec(dllexport) int asc2unicode(char * asc, int asclen, short * unicode);

extern "C" __declspec(dllexport) int unicode2asc(short * unicode, int unicodelen, char * asc);
#else
extern "C"  __declspec(dllimport) int __getDateTime(LPDATETIME datetime);
extern "C"  __declspec(dllimport) int __getDateTimeStr(void * str);
extern "C"  __declspec(dllimport) int __strlen(char * str);

extern "C"  __declspec(dllimport) int __memcpy(char * dst, char * src, int len);

extern "C"  __declspec(dllimport) int __strcpy(char * dst, char * src);

extern "C"  __declspec(dllimport) int __strncpy(char * dst, char * src, int limit);

extern "C"  __declspec(dllimport) int __memset(char * dst, int value, int len);
extern "C"  __declspec(dllimport) int __memset4(char * dst, int value, int len);

extern "C"  __declspec(dllimport) int __strcmp(char * src, char * dst);

extern "C"  __declspec(dllimport) int __memcmp(char * src, char * dst, int len);

extern "C"  __declspec(dllimport) char* __strstr(char * src, char * dst);

extern "C"  __declspec(dllimport) int __strcat(char * src, char * dst);

extern "C"  __declspec(dllimport) int __h2strh(unsigned int n, int lowercase, unsigned char * dst);

extern "C"  __declspec(dllimport) int __hstr2i(unsigned char * str);

extern "C"  __declspec(dllimport) int __h2strd(unsigned int h, char * strd);

extern "C"  __declspec(dllimport) int __strd2i(char * istr);

extern "C"  __declspec(dllimport) int __printf(char * buf, char * format, ...);

extern "C"  __declspec(dllimport) int __sprintf(char * buf, char * format, ...);

extern "C"  __declspec(dllimport) int __dump(char * src, int len, int lowercase, unsigned char * dstbuf);

extern "C" __declspec(dllimport) int __substr(char * src, int pos, char * dst);
extern "C" __declspec(dllimport) int __substrLen(char * src, int pos, int len, char * dst);

extern "C" __declspec(dllimport) int __strupr(char * str);

extern "C" __declspec(dllimport) int __strlwr(char * str);

extern "C" __declspec(dllimport) int timer0Wait(unsigned short);


extern "C" __declspec(dllimport) int __shutdownSystem();
extern "C" __declspec(dllimport) int __reset();

extern "C" __declspec(dllimport) int __sleep(int millisecs);

extern "C" __declspec(dllimport) int getCpuType(char * name);

extern "C" __declspec(dllimport) int getCpuInfo(char * name);

extern "C" __declspec(dllimport) unsigned int random();

extern "C" __declspec(dllimport) int lower2upper(char *data, int len);

extern "C" __declspec(dllimport) int upper2lower(char *data, int len);

extern "C" __declspec(dllimport) DWORD __sqrtInteger(DWORD i);


extern "C" __declspec(dllimport)DWORD __ntohs(WORD v);
extern "C" __declspec(dllimport)DWORD __ntohl(DWORD v);
extern "C" __declspec(dllimport)DWORD __inc(DWORD *v);
extern "C" __declspec(dllimport)void __initSpinLock(DWORD * v);
extern "C" __declspec(dllimport)WORD __enterSpinLock(DWORD * v);
extern "C" __declspec(dllimport)WORD __leaveSpinLock(DWORD * v);

extern "C" __declspec(dllimport) int asc2unicode(char * asc, int asclen, short * unicode);

extern "C" __declspec(dllimport) int unicode2asc(short * unicode, int unicodelen, char * asc);
#endif


#define __memmove __memcpy


#endif