#pragma once

#include "../def.h"



#define SOUNDBLASTER_BASE_PORT	0X220

#define FORMAT_MONO      00
#define FORMAT_STEREO    0X20
#define FORMAT_SIGNED    0X10
#define FORMAT_UNSIGNED  00


#pragma pack(1)

typedef struct {
	char id[4];
	DWORD size;
	char type[4];
}WAVFILEHEADER;

typedef struct
{
	char id[4];
	DWORD size;					//10h

	WORD wFormatag;				//14h �����ʽ������WAVE_FORMAT_PCM��WAVEFORMAT_ADPCM��
	WORD nChannls;				//16h ��������������Ϊ1��˫����Ϊ2
	DWORD nSamplesPerSec;		//18h ����Ƶ��
	DWORD nAvgBytesperSec;		//1ch ÿ���������
	WORD nBlockAlign;			//20h �����
	WORD wBitsPerSample;		//22h WAVE�ļ��Ĳ�����С
} WAVEFORMAT;

#pragma pack()

extern int gWavDataSize;
extern char * gWavDataPtr;
extern WAVEFORMAT * gWavFormat;

#ifdef DLL_EXPORT
extern "C" __declspec(dllexport) int sbplay(char * data, int size);

extern "C" __declspec(dllexport) void __kSoundInterruptionProc();
#else
extern "C" __declspec(dllimport) int sbplay(char * data, int size);

extern "C" __declspec(dllimport) void __kSoundInterruptionProc();
#endif