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
	short OffScreenMemSize; //�����õ���ʾ������, ��KBΪ��λ
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

typedef struct {
	unsigned long signature;			//VESA
	unsigned short version;				//2
	unsigned short oem_dos_offset;
	unsigned short oem_dos_seg;
	unsigned char capacities[4];		//1
	unsigned short mode_dos_offset;
	unsigned short mode_dos_seg;
	unsigned short blockTotal;			//����0x10000�����Դ��С
	unsigned char reserved[236];

}VESAINFOBLOCK;


// vbe info��VGAͼ��ϵͳ��ز���
typedef struct  {
	char          VBESignature[4];            // �ַ���"VESA"          
	unsigned   short   VBEVersion;                 // VBE�汾�ţ�BCD��
	char /*far*/* OEMStringPtr;              // ָ��OEM���̵ı�־����ָ��        
	long          Capabilities;               // ��ʾ������  
	char /*far*/* VideoModePtr;              // ָ����֧����ʾģʽ�б��ָ��
	unsigned    short  VRAMMemory;                 // ��ʾ�ڴ��С����λΪ64KB      
	// ����ΪVESA VBE 2.0�汾���϶���
	unsigned  short    OemSoftwareRev;             // VBE�����OEM�޶��汾�ţ�BCD��
	char /*far*/* OemVendorNamePtr;           // ָ����ʾ�����쳧�̵��ַ���ָ��
	char /*far*/* OemProductNamePtr;          // ָ����ʾ�������̵��ַ�����ָ��
	char /*far*/* OemProductRevPtr;           // ָ����ʾ���޶��汾�Ż�Ƭ�ȼ����ַ�����ָ��
	char         reserved[222];               // ����
	char         OEMData[256];                // VESA2.0�����϶���
} VBEINFO;

#pragma pack()



#define TASKBARCOLOR				0X00CFCFCF
#define TIMERZONECOLOR				0X00E8E8E8

#define BACKGROUND_COLOR			0X00B0E0E6

#define FILECOLOR					0x00cd3333	//red
#define FILEFONTCOLOR				0x009c9c9c	//gray
#define FILEFONTBGCOLOR				0X00C0C0C0	//gray

#define FOLDERCOLOR					0x00ffa500	//brown
#define FOLDERFONTCOLOR				0x001c1c1c	//gray
#define FOLDERFONTBGCOLOR			0X00D3D3D3	//silver

#define DEFAULT_FONT_COLOR			0
#define CMOS_TIMESTAMP_SINGLE_COLOR	0X9f3F3F
#define CMOS_TIMESTAMP_DOUBLE_COLOR	0X3f9F3F
#define CMOS_TIMESTAMP_THIRD_COLOR	0X3f3F9F
#define CONSOLE_FONT_COLOR			0XFFFFFF

#define GRAPH_CHINESECHAR_HEIGHT	16
#define GRAPH_CHINESECHAR_WIDTH		16
#define GRAPHCHAR_HEIGHT			8
#define GRAPHCHAR_WIDTH				8
#define TASKBAR_HEIGHT				(GRAPHCHAR_HEIGHT*4)

#define FOLDERFIRSTX				60
#define FOLDERFIRSTY				60
#define FULLWINDOW_TOP				0
#define FULLWINDOW_LEFT				0

#define RIGHTCLICK_MENU_WIDTH		160
#define RIGHTCLICK_MENU_HEIGHT		240




extern "C" int g_ScreenMode;

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

typedef struct {

	int left;
	int top;
	int right;
	int bottom;

}RECT,*LPRECT;

typedef struct  _FILEICON
{
	_FILEICON* next;
	_FILEICON* prev;
	 POINT pos;		//window postion x and y

	 int frameSize;
	 int frameColor;

	 int width;
	 int height;

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
}FILEICON,*LPFILEICON;


typedef struct __WINDOWCLASS {
	__WINDOWCLASS* next;
	__WINDOWCLASS* prev;

	//window left and top
	POINT pos;

	int frameSize;	// the width of window frame. so need to divide 2
	int frameColor;

	int capHeight;
	int capColor;

	//client top,left,right,bottom,not the window's
	int top;
	int left;
	int right;
	int bottom;

	//client width,not the window width
	int width;
	//client height,not the window height
	int height;

	unsigned int color;	//client color

	int showY;	//font position x
	int showX;

	int fontcolor;

	int shutdownx;		//shutdown position x
	int shutdowny;

	int zoomin;

	unsigned int backBuf;

	unsigned int backsize;

	int focus;

	int id;		//window id
	int tid;
	int pid;

	char cursorBuf[GRAPHCHAR_HEIGHT * GRAPHCHAR_HEIGHT * 4];
	int cursorID;
	int cursorColor;
	int tag;
	int showBakX;
	int showBakY;	

	char caption[WINDOW_NAME_LIMIT];

	char winname[WINDOW_NAME_LIMIT];

}WINDOWCLASS,*LPWINDOWCLASS;


typedef struct {
	POINT pos;
	int action;

	int width;
	int height;
	unsigned int color;
	DWORD funcaddr[RIGHTCLICK_MENU_HEIGHT / GRAPHCHAR_WIDTH / 2];
	char menuname[RIGHTCLICK_MENU_HEIGHT / GRAPHCHAR_WIDTH / 2][RIGHTCLICK_MENU_WIDTH / GRAPHCHAR_WIDTH ];
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


typedef struct
{
	WINDOWCLASS window;

	int cpl;
	int fsheight;

}FMWINDOW, * LPFMWINDOW;

#pragma pack()


unsigned short* getGBKCCIdx(unsigned short gbk);


#ifdef DLL_EXPORT
extern "C"  __declspec(dllexport) int __drawCC(unsigned char* str, int color, DWORD pos, DWORD bgcolor, WINDOWCLASS*);

extern "C"  __declspec(dllexport) int __initVideo(LPVESAINFORMATION vesa,DWORD fontbase);

extern "C"  __declspec(dllexport) int __drawWindow(LPWINDOWCLASS window);

extern "C"  __declspec(dllexport) int __restoreWindow(LPWINDOWCLASS window);

extern "C"  __declspec(dllexport) int __DestroyWindow(LPWINDOWCLASS window);

extern "C"  __declspec(dllexport) int __drawGraphCharInt(char * font, int color, int pos, int bgcolor);

extern "C"  __declspec(dllexport) int __drawGraphChar( char * font, int color,unsigned int pos, int bgcolor);

extern "C"  __declspec(dllexport) int __drawGraphChars( char * font, int color);

extern "C"  __declspec(dllexport) int __backspaceChar();

extern "C"  __declspec(dllexport) void clsClientRect(WINDOWCLASS * window);

extern "C"  __declspec(dllexport) int __getpos(int x, int y);

extern "C"  __declspec(dllexport) int __drawVertical(int x, int y, int len, int colorBuf, int color, char* bak);

extern "C"  __declspec(dllexport) int __drawHorizon(int x, int y, int len, int colorBuf, int color, char* bak);

extern "C"  __declspec(dllexport) int __drawRectWindow(LPPOINT p, int width, int height, int color,unsigned char * backup);

extern "C"  __declspec(dllexport) int __restoreCircle(int x, int y, int radius,int radius2, unsigned char * backup);

extern "C"  __declspec(dllexport) int __drawCircle(int x, int y, int radius, int radius2,int color, unsigned char * backup);

extern "C"  __declspec(dllexport) int __drawRectangleFrameCaption(LPPOINT p, int width, int height, int color, int framesize,
	int framecolor, int capsize, int capcolor, char * capname,char * backdata);

extern "C"  __declspec(dllexport) int __drawFileIconChars(FILEICON*);

extern "C"  __declspec(dllexport) int __drawCCS(unsigned char * font, int color);

extern "C"  __declspec(dllexport) int __DestroyRectWindow(LPPOINT p, int width, int height, unsigned char * backup);

extern "C"  __declspec(dllexport) int __drawRectFrame(LPPOINT p, int width, int height, int color, int framesize, int framecolor, char * back);

extern "C"  __declspec(dllexport) int removeFileManager(LPFMWINDOW w);

extern "C"  __declspec(dllexport) int drawFileManager(LPFMWINDOW w);

extern "C"  __declspec(dllexport) int __restoreRectFrame(LPPOINT p, int width, int height, int framesize, unsigned char* backup);

extern "C"  __declspec(dllexport) int __drawFileIcon(FILEICON*);

extern "C"  __declspec(dllexport) int __drawShutdown(LPWINDOWCLASS window);

extern "C"  __declspec(dllexport) int __drawLine(int x1, int y1, int x2, int y2, int colorBuf, DWORD color, char* bak);

extern "C"  __declspec(dllexport) int __drawDot(int x, int y, int colorBuf, DWORD color, char* bak);

extern "C"  __declspec(dllexport) int __diamond2(int startx, int starty, int raduis, int cnt, DWORD color);

extern "C"  __declspec(dllexport) int __diamond(int startx, int starty, int raduis, int cnt, DWORD color);

extern "C"  __declspec(dllexport) int __clearWindowChar(WINDOWCLASS* window);

extern "C"  __declspec(dllexport) int __drawWindowChars( char* font, int color, WINDOWCLASS* window);
#else
extern "C"  __declspec(dllimport)int __drawCC(unsigned char* str, int color, DWORD pos, DWORD bgcolor, WINDOWCLASS*);
extern "C" __declspec(dllimport)  int __initVideo(LPVESAINFORMATION vesa, DWORD fontbase);

extern "C"  __declspec(dllimport) int __drawWindow(LPWINDOWCLASS window);

extern "C"  __declspec(dllimport) int __restoreWindow(LPWINDOWCLASS window);

extern "C"  __declspec(dllimport) int __DestroyWindow(LPWINDOWCLASS window);

extern "C"  __declspec(dllimport) int __drawGraphCharInt(char * font, int color, int pos, int bgcolor);

extern "C"  __declspec(dllimport) int __drawGraphChar( char * font, int color, unsigned int pos, int bgcolor);

extern "C"  __declspec(dllimport) int __drawGraphChars( char * font, int color);

extern "C"  __declspec(dllimport) int __backspaceChar();

extern "C"  __declspec(dllimport) void clsClientRect(WINDOWCLASS * window);

extern "C"  __declspec(dllimport) int __getpos(int x, int y);

extern "C"  __declspec(dllimport) int __drawVertical(int x, int y, int len, int colorBuf, int color, char* bak);

extern "C"  __declspec(dllimport) int __drawHorizon(int x, int y, int len, int colorBuf, int color, char* bak);

extern "C"  __declspec(dllimport) int __drawRectWindow(LPPOINT p, int width, int height, int color, unsigned char * backup);

extern "C"  __declspec(dllimport) int __restoreCircle(int x, int y, int radius, int radius2, unsigned char * backup);

extern "C"  __declspec(dllimport) int __drawCircle(int x, int y, int radius, int radius2, int color, unsigned char * backup);

extern "C"  __declspec(dllimport) int __drawRectangleFrameCaption(LPPOINT p, int width, int height, int color, int framesize,
	int framecolor, int capsize, int capcolor, char * capname, char * backdata);

extern "C"  __declspec(dllimport) int __drawFileIconChars(FILEICON*);

extern "C"  __declspec(dllimport) int __drawCCS(unsigned char * font, int color);

extern "C"  __declspec(dllimport) int __DestroyRectWindow(LPPOINT p, int width, int height, unsigned char * backup);

extern "C"  __declspec(dllimport) int __drawRectFrame(LPPOINT p, int width, int height, int color, int framesize, int framecolor, char * back);

extern "C"  __declspec(dllimport) int removeFileManager(LPFMWINDOW w);

extern "C"  __declspec(dllimport) int drawFileManager(LPFMWINDOW w);

extern "C"  __declspec(dllimport) int __restoreRectFrame(LPPOINT p, int width, int height, int framesize, unsigned char* backup);

extern "C"  __declspec(dllimport) int __drawFileIcon(FILEICON*);

extern "C"  __declspec(dllimport) int __drawShutdown(LPWINDOWCLASS window);

extern "C"  __declspec(dllimport) int __drawDot(int x, int y, int colorBuf, DWORD color, char* bak);

extern "C"  __declspec(dllimport) int __drawLine(int x1, int y1, int x2, int y2, int colorBuf, DWORD color, char* bak);

extern "C"  __declspec(dllimport) int __diamond2(int startx, int starty, int raduis, int cnt, DWORD color);

extern "C"  __declspec(dllimport) int __diamond(int startx, int starty, int raduis, int cnt, DWORD color);

extern "C"  __declspec(dllimport) int __clearWindowChar(WINDOWCLASS * window);

extern "C"  __declspec(dllimport) int __drawWindowChars( char* font, int color, WINDOWCLASS * window);
#endif

#endif