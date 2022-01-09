#pragma once

#include "def.h"
#include "video.h"
#include "Utils.h"

#define RT_CURSOR           MAKEINTRESOURCE(1)
#define RT_BITMAP           MAKEINTRESOURCE(2)
#define RT_ICON             MAKEINTRESOURCE(3)
#define RT_MENU             MAKEINTRESOURCE(4)
#define RT_DIALOG           MAKEINTRESOURCE(5)
#define RT_STRING           MAKEINTRESOURCE(6)
#define RT_FONTDIR          MAKEINTRESOURCE(7)
#define RT_FONT             MAKEINTRESOURCE(8)
#define RT_ACCELERATOR      MAKEINTRESOURCE(9)
#define RT_RCDATA           MAKEINTRESOURCE(10)
#define RT_MESSAGETABLE     MAKEINTRESOURCE(11)

#pragma pack(1)
typedef struct tagBITMAPFILEHEADER
{
	UINT16 bfType;
	DWORD bfSize;			//����BMP�ļ��Ĵ�С
	UINT16 bfReserved1;
	UINT16 bfReserved2;
	DWORD bfOffBits;		//ƫ�������� λͼ�ļ�ͷ+λͼ��Ϣͷ+��ɫ�� �Ĵ�С
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
	DWORD biSize;			//ָ���˽ṹ��ĳ��ȣ�Ϊ40
	LONG biWidth;			//λͼ��
	LONG biHeight;			//λͼ��
	WORD biPlanes;			//ƽ������Ϊ1
	WORD biBitCount;		//������ɫλ����������1��2��4��8��16��24���µĿ�����32
	DWORD biCompression;	//ѹ����ʽ��������0��1��2������0��ʾ��ѹ��
	DWORD biSizeImage;		//ʵ��λͼ����ռ�õ��ֽ���
	LONG biXPelsPerMeter;	//X����ֱ���
	LONG biYPelsPerMeter;	//Y����ֱ���
	DWORD biClrUsed;		//ʹ�õ���ɫ�������Ϊ0�����ʾĬ��ֵ(2^��ɫλ��)
	DWORD biClrImportant;	//��Ҫ��ɫ�������Ϊ0�����ʾ������ɫ������Ҫ��
} BITMAPINFOHEADER, *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct tagRGBQUAD {
	BYTE    rgbBlue;
	BYTE    rgbGreen;
	BYTE    rgbRed;
	BYTE    rgbReserved;
} RGBQUAD;


typedef struct {
	short fix;
	short type;
	short cnt;
}ICON_HDR;

typedef struct {
	unsigned char	bwidth;
	unsigned char	bheight;
	unsigned char	bcolorcnt;
	unsigned char	breserved;
	WORD			wplanes;
	WORD			wbitcnt;
	DWORD			dwbytesinres;
	DWORD			dwimgoffset;
}ICON_DIR_ENTRY;

typedef struct {
	BITMAPINFOHEADER	bmphdr;
	RGBQUAD				quad;
	unsigned char		icxor;
	unsigned char		icand;
}ICONIMAGE;

#pragma pack()

extern "C" __declspec(dllexport) DWORD showIcon(int x, int y, LPBITMAPINFOHEADER lpbmpinfo);

int showTxtFile(char * filename);
int setCursor(unsigned char high8bits, unsigned char low8bits);

void sphere(int x, int y, int raduis, DWORD color, unsigned char* backup);

void sphere7(int x, int y, int raduis, DWORD color, unsigned char* backup);

extern "C" __declspec(dllexport) int repeatDrawCCFontString();

int initCCFont();


#ifdef DLL_EXPORT



extern "C" __declspec(dllexport) int showBmpBits(int x, int y, BITMAPINFOHEADER* info, unsigned char * data);

extern "C" int showBmpFile(char * filename);

extern "C" int showBmp(char * filename, unsigned char * data, int filesize,int x,int y);

extern "C" int playWavFile(char *filename);

#else


extern "C" __declspec(dllimport) int showBmpBits(int x, int y, BITMAPINFOHEADER* info, unsigned char * data);

extern "C" int showBmpFile(char * filename);

extern "C" int showBmp(char * filename, unsigned char * data, int filesize, int x, int y);


extern "C" int playWavFile(char *filename);

#endif



