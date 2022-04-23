#pragma once

#include "def.h"

#ifndef _VIDEO_H_H_H
#define _VIDEO_H_H_H

#pragma pack (1)
typedef struct {
	short ModeAttr;
	char WinAAttr;
	char WinBAttr;//窗口A,B的属性
	short WinGran; //位面大小(窗口粒度), 以KB为单位
	short WinSize; //窗口大小, 以KB为单位
	short WinASeg; //窗口A的起始段址
	short WinBSeg; //窗口B的起始段址
	int BankFunc; //换页调用入口指针
//16
	short BytesPerScanLine; //每条水平扫描线所占的字节数
	short XRes;
	short YRes; //水平, 垂直方向的分辨率
	char XCharSize;
	char YCharSize; //字符的宽度和高度
	char NumberOfplanes; //位平面的个数
// 25
	char BitsPerPixel; //每像素的位数
	char NumberOfBanks; //CGA逻辑扫描线分组数
	char MemoryModel; //显示内存模式
	char BankSize; //CGA每组扫描线的大小
	char NumberOfImagePages; //可同时载入的最大满屏图像数
	char reserve_1; //为页面功能保留
//31
//对直接写颜色模式的定义区域
	char RedMaskSize; //红色所占的位数
	char RedFieldPosition; //红色的最低有效位位置
	char GreenMaskSize; //绿色所占位数
	char GreenFieldPosition; //绿色的最低有效位位置
	char BlueMaskSize; //蓝色所占位数
	char BlueFieldPosition; //蓝色最低有效位位置
	char RsvMaskSize; //保留色所占位数
	char RsvFieldPosition; //保留色的最低有效位位置
	char DirectColorModeInfo; //直接颜色模式属性
// 40
// 以下为VBE2.0版本以上定义
	int PhyBasePtr; //可使用的大的帧缓存时为指向其首址的32位物理地址
	int OffScreenMemOffset; //帧缓存首址的32位偏移量
	short OffScreenMemSize; //可用的, 连续的显示缓冲区, 以KB为单位
// 50
// 以下为VBE3.0版以上定义
	short LinBytesPerScanLine; //线形缓冲区中每条扫描线的长度, 以字节为单位
	char BnkNumberOfImagePages; //使用窗口功能时的显示页面数
	char LinNumberOfImagePages; //使用大的线性缓冲区时的显示页面数
	char LinRedMaskSize; //使用大的线性缓冲区时红色所占位数
	char LinRedFieldPosition; //使用大的线性缓冲区时红色最低有效位位置
	char LinGreenMaskSize; //使用大的线性缓冲区时绿色所占的位数
	char LinGreenFieldPosition; //使用大的线性缓冲区时绿色最低有效位位置
	char LinBlueMaskSize; //使用大的线性缓冲区时蓝色所占的位数
	char LinBlueFieldPosition; //使用大的线性缓冲区时蓝色最低有效位位置
	char LinRsvMaskSize; //使用大的线性缓冲区时保留色所占位数
	char LinRsvFieldPosition; //使用大的线性缓冲区时保留色最低有效位位置
	char reserve_2[194]; //保留
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