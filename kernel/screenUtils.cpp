#include "screenutils.h"
#include "video.h"
#include "Utils.h"
#include "file.h"
#include "window.h"
#include "keyboard.h"
#include "mouse.h"
#include "gdi.h"
#include "slab.h"
#include "page.h"

int gCircleX = 0;
int gCircleY = 0;
int gRadius = 128;
int gDeltaX = 6;
int gDeltaY = 6;
//int gCircleColor = 0xffffff;
int gCircleColor = 0;

//屏幕保护开关
int gScreenProtectFlag = 0;
//屏幕保护窗口句柄
int gScreenProtectWindowID = 0;

//光标背景恢复或者刷屏的标志
int gCursorFlag = 0;
//延时次数标记
int gCursorDelay = 0;

int gCursorEnable = FALSE;
DWORD *gCursorX = 0;
DWORD *gCursorY = 0;
int gCursorColor = 0;

unsigned char gCursorBackup[1024];

#define CURSOR_HEARTBEAR_INTERVAL 300

#define SREENPROTECT_COLOR 0
//0XBBFFFF 0X87CEEB


void pauseBreak() {
	return;
}


extern "C" __declspec(dllexport) int __kPrintScreen() {

	int screensize = gVideoHeight*gVideoWidth*gBytesPerPixel;

	char * data = (char*)__kMalloc(gWindowSize);
	BITMAPFILEHEADER * hdr = (BITMAPFILEHEADER*)data;
	hdr->bfType = 0x4d42;
	hdr->bfSize = screensize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	hdr->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	hdr->bfReserved1 = 0;
	hdr->bfReserved2 = 0;

	BITMAPINFOHEADER * info = (BITMAPINFOHEADER*)(data + sizeof(BITMAPFILEHEADER));
	info->biBitCount = gBytesPerPixel * 8;
	info->biHeight = -gVideoHeight;
	info->biWidth = -gVideoWidth;
	info->biSize = 40;
	info->biSizeImage = gBytesPerPixel * gVideoWidth * gVideoHeight;
	info->biClrImportant = 0;
	info->biClrUsed = 0;
	info->biCompression = 0;
	info->biXPelsPerMeter = 0;
	info->biYPelsPerMeter = 0;

	__memcpy((char*)(data + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)),
		(char*)gGraphBase, screensize);

	char filename[256];
	__printf(filename, "c:\\%x.bmp",*(unsigned int*)TIMER0_TICK_COUNT);
	int ret = writeFile(filename, (char*)data, screensize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER), FILE_WRITE_APPEND);

	__kFree((DWORD)data);
	return 0;
}


int initScreenProtect() {
	int ret = 0;

	unsigned int r = random();

	gCircleX = r % gVideoWidth;
	if (gCircleX + gRadius >= gVideoWidth)
	{
		gCircleX = gVideoWidth - gRadius;
	}
	else if (gCircleX <= gRadius)
	{
		gCircleX = gRadius;
	}

	gCircleY = r % gVideoHeight;
	if (gCircleY + gRadius >= gVideoHeight)
	{
		gCircleY = gVideoHeight - gRadius;
	}
	else if (gCircleY <= gRadius)
	{
		gCircleY = gRadius;
	}

	int screensize = gVideoHeight*gVideoWidth*gBytesPerPixel;

	unsigned char * dst = (unsigned char*)gGraphBase + screensize;

	unsigned char * src = (unsigned char*)gGraphBase;

	__memcpy((char*)dst, (char*)src, screensize);

	DWORD background = SREENPROTECT_COLOR;
	//__memset4((char*)src, background, screensize);
	//__memset((char*)src, SREENPROTECT_COLOR, screensize);
	POINT p;
	p.x = 0;
	p.y = 0;
	__drawRectangle(&p, gVideoWidth, gVideoHeight, SREENPROTECT_COLOR, 0);

	sphere7(gCircleX, gCircleY, gRadius, SREENPROTECT_COLOR, (unsigned char*)gGraphBase + screensize * 2);
	//ret = __drawColorCircle(gCircleX, gCircleY, gRadius, gCircleColor, (unsigned char*)gGraphBase + screensize*2);

	gScreenProtectWindowID = addWindow(0, 0, 0, 0,"__screenProtect");
	
	gScreenProtectFlag = TRUE;
	return TRUE;
}



int stopScreenProtect() {
	int ret = 0;

	gScreenProtectFlag = FALSE;

	removeWindow(gScreenProtectWindowID);
	gScreenProtectWindowID = 0;
	
	int screensize = gVideoHeight*gVideoWidth*gBytesPerPixel;

	ret = __restoreCircle(gCircleX, gCircleY, gRadius, (unsigned char*)gGraphBase + screensize * 2);

	unsigned char * src = (unsigned char*)gGraphBase + screensize;

	unsigned char * dst = (unsigned char*)gGraphBase;

	__memcpy((char*)dst, (char*)src, screensize);

	return TRUE;
}



extern "C" __declspec(dllexport) void __kScreenProtect() {

	if (gCursorEnable)
	{
		gCursorDelay++;
		if (gCursorDelay >= CURSOR_HEARTBEAR_INTERVAL)
		{
			gCursorDelay = 0;
			drawCursor();
		}
	}

	if (gScreenProtectFlag == FALSE)
	{
		return;
	}
	else {
		unsigned int ck = __kGetKbd(gScreenProtectWindowID);
		//unsigned int ck = __getchar(gScreenProtectWindowID);
		unsigned int asc = ck & 0xff;
		if(asc)
		//if (asc == 0x1b || asc == 0x0a)
		{
			stopScreenProtect();
			return;
		}

		MOUSEINFO mouseinfo;
		mouseinfo.status = 0;
		//__memset((char*)&mouseinfo, 0, sizeof(MOUSEINFO));
		getmouse(&mouseinfo, gScreenProtectWindowID);
		if (mouseinfo.status /*& 4*/)	//middle click
		{
			stopScreenProtect();
			return;
		}
	}

	int ret = 0;

	int screensize = gVideoHeight*gVideoWidth*gBytesPerPixel;

	

	DWORD oldx = gCircleX;
	DWORD oldy = gCircleY;

	gCircleX += gDeltaX;
	if (gCircleX + gRadius >= gVideoWidth)
	{
		gCircleX = gVideoWidth - gRadius;
		gDeltaX = -gDeltaX;
	}
	else if (gCircleX <= gRadius)
	{
		gCircleX = gRadius;
		gDeltaX = -gDeltaX;
	}

	gCircleY += gDeltaY;
	if (gCircleY + gRadius >= gVideoHeight)
	{
		gCircleY = gVideoHeight - gRadius;
		gDeltaY = -gDeltaY;
	}
	else if (gCircleY <= gRadius)
	{
		gCircleY = gRadius;
		gDeltaY = -gDeltaY;
	}

	gCircleColor ++;
	
	ret = __restoreCircle(oldx, oldy, gRadius, (unsigned char*)gGraphBase + screensize * 2);

	sphere7(gCircleX, gCircleY, gRadius, SREENPROTECT_COLOR, (unsigned char*)gGraphBase + screensize * 2);
	//ret = __drawColorCircle(gCircleX, gCircleY, gRadius, gCircleColor, (unsigned char*)gGraphBase + screensize * 2);
	return ;
}


void setCursor(int enable,DWORD *x, DWORD *y,unsigned int color) {
	gCursorEnable = enable;
	gCursorX = x;
	gCursorY = y;
	gCursorColor = color;
}


int restoreCursor() {
	gCursorFlag = TRUE;
	return drawCursor();
}

int drawCursor() {
	if (gCursorY == 0 || gCursorX == 0)
	{
		return 0;
	}

	int ch = GRAPHCHAR_HEIGHT / 4;
	int cw = GRAPHCHAR_WIDTH ;

	POINT p;
	p.x = *gCursorX;
	p.y = *gCursorY + GRAPHCHAR_HEIGHT - ch;

	int ret = 0;


	if (gCursorFlag)
	{
		ret = __restoreRectangle(&p, cw, ch, (unsigned char*)gCursorBackup);
		gCursorFlag = FALSE;
	}
	else {
		ret = __drawRectangle(&p, cw, ch, gCursorColor, (unsigned char*)gCursorBackup);
		gCursorFlag = TRUE;
	}
	return gCursorFlag;
}