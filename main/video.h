#pragma once

#include "def.h"

#ifndef _VIDEO_H_H_H
#define _VIDEO_H_H_H

#pragma pack (1)
typedef struct {
	short ModeAttr;
	char WinAAttr;
	char WinBAttr;//����A,B������
	short WinGran; //λ���С(��������), ��KBΪ��λ
	short WinSize; //���ڴ�С, ��KBΪ��λ
	short WinASeg; //����A����ʼ��ַ
	short WinBSeg; //����B����ʼ��ַ
	int BankFunc; //��ҳ�������ָ��
//16
	short BytesPerScanLine; //ÿ��ˮƽɨ������ռ���ֽ���
	short XRes;
	short YRes; //ˮƽ, ��ֱ����ķֱ���
	char XCharSize;
	char YCharSize; //�ַ��Ŀ�Ⱥ͸߶�
	char NumberOfplanes; //λƽ��ĸ���
// 25
	char BitsPerPixel; //ÿ���ص�λ��
	char NumberOfBanks; //CGA�߼�ɨ���߷�����
	char MemoryModel; //��ʾ�ڴ�ģʽ
	char BankSize; //CGAÿ��ɨ���ߵĴ�С
	char NumberOfImagePages; //��ͬʱ������������ͼ����
	char reserve_1; //Ϊҳ�湦�ܱ���
//31
//��ֱ��д��ɫģʽ�Ķ�������
	char RedMaskSize; //��ɫ��ռ��λ��
	char RedFieldPosition; //��ɫ�������Чλλ��
	char GreenMaskSize; //��ɫ��ռλ��
	char GreenFieldPosition; //��ɫ�������Чλλ��
	char BlueMaskSize; //��ɫ��ռλ��
	char BlueFieldPosition; //��ɫ�����Чλλ��
	char RsvMaskSize; //����ɫ��ռλ��
	char RsvFieldPosition; //����ɫ�������Чλλ��
	char DirectColorModeInfo; //ֱ����ɫģʽ����
// 40
// ����ΪVBE2.0�汾���϶���
	int PhyBasePtr; //��ʹ�õĴ��֡����ʱΪָ������ַ��32λ�����ַ
	int OffScreenMemOffset; //֡������ַ��32λƫ����
	short OffScreenMemSize; //���õ�, ��������ʾ������, ��KBΪ��λ
// 50
// ����ΪVBE3.0�����϶���
	short LinBytesPerScanLine; //���λ�������ÿ��ɨ���ߵĳ���, ���ֽ�Ϊ��λ
	char BnkNumberOfImagePages; //ʹ�ô��ڹ���ʱ����ʾҳ����
	char LinNumberOfImagePages; //ʹ�ô�����Ի�����ʱ����ʾҳ����
	char LinRedMaskSize; //ʹ�ô�����Ի�����ʱ��ɫ��ռλ��
	char LinRedFieldPosition; //ʹ�ô�����Ի�����ʱ��ɫ�����Чλλ��
	char LinGreenMaskSize; //ʹ�ô�����Ի�����ʱ��ɫ��ռ��λ��
	char LinGreenFieldPosition; //ʹ�ô�����Ի�����ʱ��ɫ�����Чλλ��
	char LinBlueMaskSize; //ʹ�ô�����Ի�����ʱ��ɫ��ռ��λ��
	char LinBlueFieldPosition; //ʹ�ô�����Ի�����ʱ��ɫ�����Чλλ��
	char LinRsvMaskSize; //ʹ�ô�����Ի�����ʱ����ɫ��ռλ��
	char LinRsvFieldPosition; //ʹ�ô�����Ի�����ʱ����ɫ�����Чλλ��
	char reserve_2[194]; //����
}VESAINFORMATION, *LPVESAINFORMATION;

#pragma pack()



#define TASKBARCOLOR			0X00CFCFCF
#define TIMERZONECOLOR			0X00E8E8E8

#define BACKGROUND_COLOR	0X00B0E0E6

#define FILECOLOR			0x00cd3333	//red
#define FILEFONTCOLOR		0x009c9c9c	//gray
#define FILEFONTBGCOLOR		0X00C0C0C0	//gray

#define FOLDERCOLOR			0x00ffa500	//brown
#define FOLDERFONTCOLOR		0x001c1c1c	//gray
#define FOLDERFONTBGCOLOR	0X00D3D3D3	//silver

#define DEFAULT_FONT_COLOR		0
#define CMOS_TIMESTAMP_SINGLE_COLOR	0X9f3F3F
#define CMOS_TIMESTAMP_DOUBLE_COLOR	0X3f9F3F
#define CMOS_TIMESTAMP_THIRD_COLOR	0X3f3F9F
#define CONSOLE_FONT_COLOR		0XFFFFFF

#define GRAPH_CHINESECHAR_HEIGHT		16
#define GRAPH_CHINESECHAR_WIDTH			16
#define GRAPHCHAR_HEIGHT		8
#define GRAPHCHAR_WIDTH			8
#define TASKBAR_HEIGHT			(GRAPHCHAR_HEIGHT*4)

#define FOLDERFIRSTX			60
#define FOLDERFIRSTY			60
#define FULLWINDOW_TOP			0
#define FULLWINDOW_LEFT			0

#define RIGHTCLICK_MENU_WIDTH	160
#define RIGHTCLICK_MENU_HEIGHT	240

//extern "C"  VESAINFORMATION gVesaInfo;


unsigned short * getCCIdxInGBK(unsigned short gbk);

#ifdef DLL_EXPORT

extern "C"  __declspec(dllexport) unsigned char* gCCFontBase;
extern "C"  __declspec(dllexport) DWORD gFontBase;
extern "C"  __declspec(dllexport) unsigned int gGraphBase;
extern "C"  __declspec(dllexport) int gBytesPerLine;
extern "C"  __declspec(dllexport) int gVideoWidth;
extern "C"  __declspec(dllexport) int gVideoHeight;
extern "C"  __declspec(dllexport) int gBytesPerPixel;
//extern int gBytesPerFont;
//extern int gFontLineSize;

extern "C"  __declspec(dllexport) int gBigFolderWidth;
extern "C"  __declspec(dllexport) int gBigFolderHeight;
extern "C"  __declspec(dllexport) int gSmallFolderWidth;
extern "C"  __declspec(dllexport) int gSmallFolderHeight;

extern "C"  __declspec(dllexport) int gWindowHeight;
extern "C"  __declspec(dllexport) unsigned int gWindowSize;
extern "C"  __declspec(dllexport) int gShowX;
extern "C"  __declspec(dllexport) int gShowY;

extern "C"  __declspec(dllexport) unsigned int gConsoleWidth;

extern "C"  __declspec(dllexport) unsigned int gConsoleHeight;
#else
extern "C"  __declspec(dllimport) unsigned char* gCCFontBase;
extern "C"  __declspec(dllimport) DWORD gFontBase;
extern "C"  __declspec(dllimport) unsigned int gGraphBase;
extern "C"  __declspec(dllimport) int gBytesPerLine;
extern "C"  __declspec(dllimport) int gVideoWidth;
extern "C"  __declspec(dllimport) int gVideoHeight;
extern "C"  __declspec(dllimport) int gBytesPerPixel;
//extern int gBytesPerFont;
//extern int gFontLineSize;

extern "C"  __declspec(dllimport) int gBigFolderWidth;
extern "C"  __declspec(dllimport) int gBigFolderHeight;
extern "C"  __declspec(dllimport) int gSmallFolderWidth;
extern "C"  __declspec(dllimport) int gSmallFolderHeight;

extern "C"  __declspec(dllimport) int gWindowHeight;
extern "C"  __declspec(dllimport) unsigned int gWindowSize;
extern "C"  __declspec(dllimport) int gShowX;
extern "C"  __declspec(dllimport) int gShowY;

extern "C"  __declspec(dllimport) unsigned int gConsoleWidth;

extern "C"  __declspec(dllimport) unsigned int gConsoleHeight;
#endif

#pragma pack(1)

typedef struct{
	int x;
	int y;
}POINT,*LPPOINT;

typedef struct  
{
	 POINT pos;
	 int width;
	 int height;
	 int frameSize;
	 int frameColor;

	 unsigned int color;
	 char name[WINDOW_NAME_LIMIT];
	 int nameHeight;
	 unsigned int namecolor;

	 int zoomin;
	 int showY;
	 int showX;

	 int id;
	 int tid;
	 int pid;

	 unsigned char * backGround;
	 unsigned int backsize;

	 int namebgcolor;
}FILEMAP,*LPFILEMAP;


typedef struct __WINDOWCLASS {
	//window left and top
	POINT pos;
	//client width,not window width
	int width;
	//client height,not window height
	int height;

	int frameSize;
	int frameColor;

	unsigned int color;
	int fontcolor;
	char caption[WINDOW_NAME_LIMIT];
	char winname[WINDOW_NAME_LIMIT];
	int capHeight;
	int capColor;

	int zoomin;
	int showY;
	int showX;

	int id;
	int tid;
	int pid;
	

	int top;
	int left;
	int right;
	int bottom;

	int shutdownx;
	int shutdowny;

	unsigned int backGround;
	unsigned int backsize;
	__WINDOWCLASS * next;
	__WINDOWCLASS * prev;
}WINDOWCLASS,*LPWINDOWCLASS;


typedef struct {
	POINT pos;
	int action;

	unsigned int width;
	unsigned int height;
	unsigned int color;
	DWORD funcaddr[RIGHTCLICK_MENU_HEIGHT / GRAPHCHAR_WIDTH / 2];
	char menuname[RIGHTCLICK_MENU_HEIGHT / GRAPHCHAR_WIDTH / 2][RIGHTCLICK_MENU_WIDTH / GRAPHCHAR_WIDTH];
	DWORD funcparams[RIGHTCLICK_MENU_HEIGHT / GRAPHCHAR_WIDTH / 2][16];
	DWORD paramcnt[RIGHTCLICK_MENU_HEIGHT / GRAPHCHAR_WIDTH / 2];
	int validItem;

	int zoomin;

	int id;
	int tid;
	int pid;
	
	char name[WINDOW_NAME_LIMIT];

	unsigned int backGround;
	unsigned int backsize;
}RIGHTMENU, *LPRIGHTMENU;




#pragma pack()


#ifdef DLL_EXPORT

extern "C"  __declspec(dllexport) int __logShow(unsigned char * font, int color);
extern "C"   int __getVideoParams(LPVESAINFORMATION vesa,DWORD fontbase);

extern "C"  __declspec(dllexport) int __drawWindow(LPWINDOWCLASS window,int active);

extern "C"  __declspec(dllexport) int __drawBackGroundWindow(LPWINDOWCLASS window, int active);

extern "C"  __declspec(dllexport) int __backupWindow(LPWINDOWCLASS window);
extern "C"  __declspec(dllexport) int __restoreWindow(LPWINDOWCLASS window);

extern "C"  __declspec(dllexport) int __drawGraphCharIntr(char * font, int color, int pos, int bgcolor);

extern "C"  __declspec(dllexport) int __drawGraphCharPos(unsigned char * font, int color, unsigned int pos);

extern "C"  __declspec(dllexport) int __drawGraphChar(unsigned char * font, int color,unsigned int pos, int bgcolor);

extern "C"  __declspec(dllexport) int __drawGraphChars(unsigned char * font, int color);

extern "C"  __declspec(dllexport) int __backspaceChar();

extern "C"  __declspec(dllexport) int __getpos(int x, int y);

extern "C"  __declspec(dllexport) int __drawVertical(int x, int y, int len, int color);

extern "C"  __declspec(dllexport) int __drawHorizon(int x, int y, int len, int color);

extern "C"  __declspec(dllexport) int __drawRectangle(LPPOINT p, int width, int height, int color,unsigned char * backup);

extern "C"  __declspec(dllexport) int __restoreCircle(int x, int y, int radius, unsigned char * backup);
extern "C"  __declspec(dllexport) int __drawCircle(int x, int y, int radius, int color, unsigned char * backup);

extern "C"  __declspec(dllexport) int __drawColorCircle(int x, int y, int radius, int color, unsigned char * backup);

extern "C"  __declspec(dllexport) int __drawRectangleFrameCaption(LPPOINT p, int width, int height, int color, int framesize,
	int framecolor, int capsize, int capcolor, char * capname,char * backdata);

extern "C"  __declspec(dllexport) int __showGraphString(FILEMAP *);

extern "C"  __declspec(dllexport) int __showGraphChar(FILEMAP *);

extern "C"  __declspec(dllexport) int __drawCCS(unsigned char * font, int color);

extern "C"  __declspec(dllexport) int __restoreRectangle(LPPOINT p, int width, int height, unsigned char * backup);

extern "C"  __declspec(dllexport) int __drawRectangleFrame(LPPOINT p, int width, int height, int color, int framesize, int framecolor, char * back);
extern "C"  __declspec(dllexport) int __drawFileMap(LPFILEMAP);

extern "C"  __declspec(dllexport) int __drawShutdown(LPWINDOWCLASS window);

extern "C"  __declspec(dllexport) int __drawLine(int x1, int y1, int x2, int y2, DWORD color);

extern "C"  __declspec(dllexport) int __drawDot(int x, int y, DWORD color);

extern "C"  __declspec(dllexport) int __diamond2(int startx, int starty, int raduis, int cnt, DWORD color);
extern "C"  __declspec(dllexport) int __diamond(int startx, int starty, int raduis, int cnt, DWORD color);
#else
extern "C"  __declspec(dllimport) int __logShow(unsigned char * font, int color);

extern "C" int __getVideoParams(LPVESAINFORMATION vesa, DWORD fontbase);

extern "C"  __declspec(dllimport) int __drawWindow(LPWINDOWCLASS window, int active);

extern "C"  __declspec(dllimport) int __drawBackGroundWindow(LPWINDOWCLASS window, int active);

extern "C"  __declspec(dllimport) int __backupWindow(LPWINDOWCLASS window);
extern "C"  __declspec(dllimport) int __restoreWindow(LPWINDOWCLASS window);

extern "C"  __declspec(dllimport) int __drawGraphCharIntr(char * font, int color, int pos, int bgcolor);

extern "C"  __declspec(dllimport) int __drawGraphCharPos(unsigned char * font, int color, unsigned int pos);

extern "C"  __declspec(dllimport) int __drawGraphChar(unsigned char * font, int color, unsigned int pos, int bgcolor);

extern "C"  __declspec(dllimport) int __drawGraphChars(unsigned char * font, int color);

extern "C"  __declspec(dllimport) int __backspaceChar();

extern "C"  __declspec(dllimport) int __getpos(int x, int y);

extern "C"  __declspec(dllimport) int __drawVertical(int x, int y, int len, int color);

extern "C"  __declspec(dllimport) int __drawHorizon(int x, int y, int len, int color);

extern "C"  __declspec(dllimport) int __drawRectangle(LPPOINT p, int width, int height, int color, unsigned char * backup);

extern "C"  __declspec(dllimport) int __restoreCircle(int x, int y, int radius, unsigned char * backup);
extern "C"  __declspec(dllimport) int __drawCircle(int x, int y, int radius, int color, unsigned char * backup);

extern "C"  __declspec(dllimport) int __drawColorCircle(int x, int y, int radius, int color, unsigned char * backup);

extern "C"  __declspec(dllimport) int __drawRectangleFrameCaption(LPPOINT p, int width, int height, int color, int framesize,
	int framecolor, int capsize, int capcolor, char * capname, char * backdata);

extern "C"  __declspec(dllimport) int __showGraphString(FILEMAP *);

extern "C"  __declspec(dllimport) int __showGraphChar(FILEMAP *);

extern "C"  __declspec(dllimport) int __drawCCS(unsigned char * font, int color);

extern "C"  __declspec(dllimport) int __restoreRectangle(LPPOINT p, int width, int height, unsigned char * backup);

extern "C"  __declspec(dllimport) int __drawRectangleFrame(LPPOINT p, int width, int height, int color, int framesize, int framecolor, char * back);
extern "C"  __declspec(dllimport) int __drawFileMap(LPFILEMAP);

extern "C"  __declspec(dllimport) int __drawShutdown(LPWINDOWCLASS window);

extern "C"  __declspec(dllimport) int __drawDot(int x, int y, DWORD color);
extern "C"  __declspec(dllimport) int __drawLine(int x1, int y1, int x2, int y2, DWORD color);

extern "C"  __declspec(dllimport) int __diamond2(int startx, int starty, int raduis, int cnt, DWORD color);
extern "C"  __declspec(dllimport) int __diamond(int startx, int starty, int raduis, int cnt, DWORD color);
#endif

#endif