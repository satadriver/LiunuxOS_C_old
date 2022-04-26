
#include "video.h"
#include "Utils.h"
#include "Kernel.h"
#include "window.h"
#include "mouse.h"
#include "slab.h"
#include "task.h"
#include "math.h"
#include "gdi.h"
#include "PEresource.h"
#include "file.h"


VESAINFORMATION *gVesaInfo;
unsigned char* gCCFontBase = 0;
DWORD gFontBase = 0;
unsigned int gGraphBase = 0;
int gVideoWidth = 0;
int gVideoHeight = 0;
int gBytesPerPixel = 0;
int gBytesPerLine = 0;
//int gBytesPerFont;
//int gFontLineSize = 0;
int gWindowHeight = 0;
unsigned int gWindowSize = 0;

int gBigFolderWidth = 0;
int gBigFolderHeight = 0;
int gSmallFolderWidth = 0;
int gSmallFolderHeight = 0;

int gShowX = 0;
int gShowY = 0;


#include "pci.h"


int __getVideoParams(LPVESAINFORMATION vesaInfo,DWORD fontbase){
	int result = 0;

	gVesaInfo = (VESAINFORMATION*)VESA_INFO_BASE;

	__memcpy((char*)gVesaInfo, (char*)vesaInfo, sizeof(VESAINFORMATION));

	__memcpy((char*)gVesaInfo + sizeof(VESAINFORMATION), (char*)vesaInfo + sizeof(VESAINFORMATION), sizeof(VESAINFOBLOCK));

	DWORD svgaregs[16];
	DWORD svgadev = 0;
	DWORD svgairq = 0;
	result = getBasePort(svgaregs, 0x0300, &svgadev, &svgairq);
	if ( (svgaregs) && (svgaregs[0] & 1) == 0 )		//memory address
	{
		//gVesaInfo.PhyBasePtr = (svgaregs[0] & 0xfffffff0);
	}
	


	gFontBase = fontbase;
	gBytesPerPixel = vesaInfo->BitsPerPixel >> 3;
	gBytesPerLine = vesaInfo->BytesPerScanLine;
	gVideoHeight = vesaInfo->YRes;
	gVideoWidth = vesaInfo->XRes;
	gGraphBase = vesaInfo->PhyBasePtr + vesaInfo->OffScreenMemOffset;

	//gBytesPerFont = gBytesPerPixel * GRAPHCHAR_WIDTH;
	//gFontLineSize = gBytesPerLine * GRAPHCHAR_HEIGHT;

	gWindowHeight = gVideoHeight - TASKBAR_HEIGHT;
	gWindowSize = gWindowHeight * gBytesPerLine;

	gSmallFolderHeight = gVideoHeight / 20;
	gSmallFolderWidth = gVideoWidth / 20;

	gBigFolderHeight = gVideoHeight / 10;
	gBigFolderWidth = gVideoWidth / 10;

	gShowY = 0;
	gShowX = 0;

	POINT p;
	p.x = 0;
	p.y = 0;
	__drawRectangle(&p, gVideoWidth,gVideoHeight , BACKGROUND_COLOR,0);

	return 0;
}

int __drawShutdown(LPWINDOWCLASS window) {
	if (window->capHeight == 0)
	{
		return TRUE;
	}

	int singlefs = window->frameSize >> 1;

	int x = window->pos.x + singlefs + window->width - window->capHeight;

	int y = window->pos.y + singlefs;

	window->shutdownx = x;
	window->shutdowny = y;

	unsigned char * pos = (unsigned char *)__getpos(x, y) + gGraphBase;

	unsigned char * keepy = pos;
	for (int i = 0; i < window->capHeight;i ++)
	{
		for (int j = 0; j < window->capHeight; j ++)
		{
			if (i == j || i == 0 || j == 0)
			{
				int color = DEFAULT_FONT_COLOR;
				for (int k = 0; k < gBytesPerPixel; k++)
				{
					*pos = color;
					pos++;
					color = color >> 8;
				}
			}
			else if (i + j == window->capHeight)		//y�����ᷴת,���������ţ�����y=x - blocks,����y=blocks - x
			{
				int color = DEFAULT_FONT_COLOR;
				for (int k = 0; k < gBytesPerPixel; k++)
				{
					*pos = color;
					pos++;
					color = color >> 8;
				}
			}
			else {
				pos += gBytesPerPixel;
			}
		}
		keepy = keepy + gBytesPerLine;
		pos = keepy;
	}

	return (int)pos - gGraphBase;
}









int __drawRectangleFrameCaption(LPPOINT p, int width, int height, int color, int framesize, int framecolor,int capsize,
	int capcolor,char * capname,char * backdata) {
	int ret = 0;

	int startpos = __getpos(p->x ,p->y) + gGraphBase;

	unsigned char * ptr = (unsigned char*)startpos;
	unsigned char * keep = ptr;

	int singlefs = framesize >> 1;

	for (int i = 0; i < height + framesize + capsize; i++)
	{
		for (int j = 0; j < width + framesize; j++)
		{
			int c = 0;

			if ( (i < singlefs) || (i >= height + singlefs + capsize) )
			{
				c = framecolor;
			}
			else if ((j < singlefs) || (j >= width + singlefs))
			{
				c = framecolor;
			}
			else if ((i >= singlefs) && (i < capsize + singlefs) )
			{
				c = capcolor;
			}
			else {
				c = color;
			}

			for (int k = 0; k < gBytesPerPixel; k++)
			{
				*backdata = *ptr;
				backdata++;

				*ptr = (c & 0xff);
				ptr++;
				c = (c >> 8);
			}
		}

		keep += gBytesPerLine;
		ptr = (unsigned char*)keep;
	}

	int showend = (int)ptr;

	if (*capname && capsize)
	{
		DWORD cappos = __getpos(p->x + singlefs, p->y + singlefs);

		showend = __drawGraphCharPos((unsigned char*)capname, DEFAULT_FONT_COLOR, (unsigned int)cappos);
	}

	return (int)showend - gGraphBase;
}


int __backupWindow(LPWINDOWCLASS window) {
	int startpos = window->pos.y * gBytesPerLine + window->pos.x * gBytesPerPixel + gGraphBase;
	unsigned char * ptr = (unsigned char*)startpos;
	unsigned char * keep = ptr;
	unsigned char * srcdata = (unsigned char * ) window->backGround;
	for (int i = 0; i < window->height; i++)
	{
		for (int j = 0; j < window->width; j++)
		{
			for (int k = 0; k < gBytesPerPixel; k++)
			{
				*srcdata = *ptr;
				ptr++;
				srcdata++;
			}
		}

		keep += gBytesPerLine;
		ptr = (unsigned char*)keep;
	}

	return (int)ptr - gGraphBase;
}

int __drawWindow(LPWINDOWCLASS window,int active) {

	int ret = 0;

	__kRestoreMouse();

	window->backsize = gBytesPerPixel*(window->height+window->frameSize + window->capHeight)*(window->width + window->frameSize);

	window->backGround = __kMalloc(window->backsize);

	window->id = addWindow(active,(DWORD*)&window->showX, (DWORD*)&window->showY,~window->color,window->winname);

	ret = __drawRectangleFrameCaption(&window->pos, window->width, window->height, window->color, window->frameSize, window->frameColor,
		window->capHeight,window->capColor,window->caption,(char*)window->backGround);

	ret = __drawShutdown(window);

	__kRefreshMouseBackup();

	__kDrawMouse();

	return 0;
}



int __refreshWindow(LPWINDOWCLASS window, int active) {

	__restoreWindow(window);

	return 0;
}


int __drawBackGroundWindow(LPWINDOWCLASS window, int active) {

	int ret = 0;

	if (active)
	{
		__kRestoreMouse();

		window->backsize = gBytesPerPixel*(window->height + window->frameSize + window->capHeight)*(window->width + window->frameSize);

		window->backGround = __kMalloc(window->backsize);
	}

	ret = __drawRectangle(&window->pos, window->width, window->height, window->color, (unsigned char*)window->backGround);

	if (active)
	{
		__kRefreshMouseBackup();

		__kDrawMouse();
	}

	return 0;
}





int __restoreWindow(LPWINDOWCLASS window) {

	__kRestoreMouse();

	removeWindow(window->id);

	int startpos = window->pos.y * gBytesPerLine + window->pos.x * gBytesPerPixel + gGraphBase;
	unsigned char * ptr = (unsigned char*)startpos;
	unsigned char * keep = ptr;
	unsigned char * srcdata = (unsigned char * ) window->backGround;

	for (int i = 0; i < window->height + window->frameSize + window->capHeight; i++)
	{
		for (int j = 0; j < window->width + window->frameSize; j++)
		{
			for (int k = 0; k < gBytesPerPixel; k++)
			{
				*ptr = *srcdata;
				ptr++;
				srcdata++;
			}
		}

		keep += gBytesPerLine;
		ptr = (unsigned char*)keep;
	}
	__kRefreshMouseBackup();
	__kDrawMouse();

	if (window->backGround)
	{
		__kFree(window->backGround);
	}

	//removeWindow(window->id);

	//__terminatePid(window->pid);

	return (int)ptr - gGraphBase;
}



int __getpos(int x, int y) {
	return y*gBytesPerLine + gBytesPerPixel * x;
}


int __showGraphString(FILEMAP * filemap) {

	int resultpos = __showGraphChar(filemap);

	filemap->showY = resultpos / gBytesPerLine;
	filemap->showX = (resultpos % gBytesPerLine) / gBytesPerPixel;

	if (filemap->showY >= gWindowHeight)
	{
		filemap->showY = 0;
		filemap->showX = 0;
	}
	return 0;
}



int __showGraphChar(FILEMAP * filemap) {
	int len = __strlen((char*)filemap->name);

	unsigned int pos = __getpos(filemap->showX, filemap->showY);

	unsigned char * showpos = pos + (unsigned char *)gGraphBase;

	unsigned char * keepy = showpos;
	unsigned char * keepx = keepy;

	for (int i = 0; i < len; i++)
	{
		unsigned int ch = filemap->name[i];
		if (ch == '\n')
		{
			int posy = (unsigned int)(showpos - gGraphBase) / gBytesPerLine;
			unsigned int nowpos = __getpos(0, posy + GRAPHCHAR_HEIGHT*filemap->zoomin);
			if (nowpos >= gWindowSize)
			{
				nowpos = 0;
			}
			showpos = (unsigned char*)nowpos + gGraphBase;
			keepx = showpos;
			keepy = showpos;
			continue;
		}
		else if (ch == '\r')
		{
			int posy = (unsigned int)(showpos - gGraphBase) / gBytesPerLine;
			showpos = (unsigned char*)__getpos(0, posy) + gGraphBase;
			keepx = showpos;
			keepy = showpos;
			continue;
		}

		int idx = ch << 3;
		unsigned char * p = (unsigned char*)gFontBase + idx;

		for (int j = 0; j < GRAPHCHAR_HEIGHT; j++)
		{
			unsigned char f = p[j];
			int m = 128;
			for (int k = 0; k < GRAPHCHAR_WIDTH; k++)
			{
				unsigned int c = 0;
				if (f & m)
				{
					c = filemap->namecolor;
					for (int n = 0; n < gBytesPerPixel*filemap->zoomin; n++)
					{
						*showpos = c;
						c = c >> 8;

						showpos++;
					}
				}
				else if (filemap->namebgcolor) {
					c = filemap->namebgcolor;
					for (int n = 0; n < gBytesPerPixel*filemap->zoomin; n++)
					{
						*showpos = c;
						c = c >> 8;

						showpos++;
					}
				}
				else {
					showpos += gBytesPerPixel*filemap->zoomin;
				}

				m = m >> 1;
			}

			keepx += gBytesPerLine*filemap->zoomin;
			showpos = keepx;
		}

		keepy = keepy + GRAPHCHAR_WIDTH*gBytesPerPixel*filemap->zoomin;

		int mod = (unsigned int)(keepy - gGraphBase) % gBytesPerLine;
		if (mod == 0)
		{
			keepy = keepy + GRAPHCHAR_HEIGHT*gBytesPerLine*filemap->zoomin;
			if ((unsigned int)(keepy - gGraphBase) >= gWindowSize)
			{
				keepy = (unsigned char*)gGraphBase;
			}
		}

		keepx = keepy;
		showpos = keepy;
	}
	return (int)(showpos - gGraphBase);
}





int __backspaceChar() {

	gShowX -= GRAPHCHAR_WIDTH;
	if (gShowX < 0 && gShowY > 0)
	{
		gShowX = gVideoWidth - GRAPHCHAR_WIDTH;
		gShowY -= GRAPHCHAR_HEIGHT;
		if (gShowY <= 0)
		{
			gShowY = 0;
		}
	}
	else if (gShowX < 0 && gShowY <= 0)
	{
		gShowY = 0;
		gShowX = 0;
	}

	unsigned int pos = __getpos(gShowX, gShowY);
	int showpos = __drawGraphChar((unsigned char*)" ", BACKGROUND_COLOR,pos, BACKGROUND_COLOR);

	return showpos;
}


int __drawGraphChars(unsigned char * font, int color) {
#ifdef LIUNUX_DEBUG_LOG_ON
	unsigned int pos = __getpos(gShowX, gShowY);
	int resultpos = __drawGraphChar(font, color, pos, 0);		//BACKGROUND_COLOR

	gShowY = resultpos / gBytesPerLine;
	
	gShowX = (resultpos % gBytesPerLine) / gBytesPerPixel;

	if (gShowY >= gWindowHeight)
	{
		gShowY = 0;
		gShowX = 0;
	}
#else
	logInMem((char*)font, __strlen((char*)font));
#endif
	return 0;
}


int __logShow(unsigned char * font, int color) {
	unsigned int pos = __getpos(gShowX, gShowY);
	int resultpos = __drawGraphChar(font, color, pos, 0);		//BACKGROUND_COLOR

	gShowY = resultpos / gBytesPerLine;

	gShowX = (resultpos % gBytesPerLine) / gBytesPerPixel;

	if (gShowY >= gWindowHeight)
	{
		gShowY = 0;
		gShowX = 0;
	}
	return 0;
}

int __drawGraphCharPos(unsigned char * font, int color, unsigned int pos) {
	return __drawGraphChar(font, color, pos, 0);		//BACKGROUND_COLOR
}


int __drawGraphChar(unsigned char * font, int color, unsigned int pos,int bgcolor) {
	int len = __strlen((char*)font);

	unsigned char * showpos = pos + (unsigned char *)gGraphBase;
	
	unsigned char * keepy = showpos;
	unsigned char * keepx = keepy;

	for (int i = 0; i < len; i++)
	{
		unsigned int ch = font[i];
		if (ch == '\n')
		{
			int posy = (unsigned int)(showpos - gGraphBase) / gBytesPerLine;
			unsigned int nowpos = __getpos(0, posy + GRAPHCHAR_HEIGHT);
			if (nowpos >= gWindowSize)
			{
				nowpos = 0;
			}
			showpos = (unsigned char*)nowpos + gGraphBase ;

			keepx = showpos;
			keepy = showpos;
			continue;
		}else if (ch == '\r')
		{
			int posy = (unsigned int)(showpos - gGraphBase) / gBytesPerLine;
			showpos = (unsigned char*)__getpos(0, posy) + gGraphBase;
			keepx = showpos;
			keepy = showpos;
			continue;	
		}else if (ch == 0)
		{
			break;
		}

		int idx = ch << 3;
		unsigned char * p = (unsigned char*)gFontBase + idx;

		for (int j = 0; j < GRAPHCHAR_HEIGHT; j++)
		{
			unsigned char f = p[j];
			int m = 128;
			for (int k = 0; k < GRAPHCHAR_WIDTH; k++)
			{
				unsigned int c = 0;
				if (f & m)
				{
					c = color;
					for (int n = 0; n < gBytesPerPixel; n++)
					{
						*showpos = c;
						c = c >> 8;

						showpos++;
					}
				}
				else if(bgcolor){
					c = bgcolor;
					for (int n = 0; n < gBytesPerPixel; n++)
					{
						*showpos = c;
						c = c >> 8;

						showpos++;
					}
				}else{
					showpos += gBytesPerPixel;
				}

				m = m >> 1;
			}

			keepx += gBytesPerLine;
			showpos = keepx;
		}

		keepy = keepy + GRAPHCHAR_WIDTH*gBytesPerPixel;

		int mod = (unsigned int)(keepy - gGraphBase) % gBytesPerLine;
		if (mod == 0)
		{
			keepy = keepy + (GRAPHCHAR_HEIGHT-1)*gBytesPerLine;
			if ((unsigned int)(keepy + GRAPHCHAR_HEIGHT *gBytesPerLine  - gGraphBase) > gWindowSize)
			{
				keepy = (unsigned char*)gGraphBase;
			}
		}

		keepx = keepy;
		showpos = keepy;
	}
	return (int)(showpos - gGraphBase);
}


unsigned short * getCCIdxInGBK(unsigned short gbk) {
	unsigned char low = (gbk & 0xff);
	unsigned char high = ( gbk >> 8);

	unsigned int idx = ((low - 0xa1) * 94 + (high - 0xa1)) * 32;
	return (unsigned short*)(gCCFontBase + idx);
}


int __drawCC(unsigned char * font, int color,DWORD pos,DWORD bgcolor) {

	int len = __strlen((char*)font);

	unsigned char * showpos = pos + (unsigned char *)gGraphBase;

	unsigned char * keepy = showpos;
	unsigned char * keepx = showpos;

	for (int i = 0; i < len; )
	{
		unsigned char chlow = font[i];
		if (chlow == '\n')
		{
			int posy = (unsigned int)(showpos - gGraphBase) / gBytesPerLine;
			unsigned int nowpos = __getpos(0, posy + GRAPH_CHINESECHAR_HEIGHT);
			if (nowpos >= gWindowSize)
			{
				nowpos = 0;
			}
			showpos = (unsigned char*)nowpos + gGraphBase;

			keepx = showpos;
			keepy = showpos;
			i++;
			continue;
		}
		else if (chlow == '\r')
		{
			int posy = (unsigned int)(showpos - gGraphBase) / gBytesPerLine;
			showpos = (unsigned char*)__getpos(0, posy) + gGraphBase;
			keepx = showpos;
			keepy = showpos;
			i++;
			continue;
		}
		else if (chlow == 0) {
			break;
		}

		unsigned char chhigh = font[i+1];

		if (chlow >= 0xa0 && chhigh >= 0xa0)
		{

		}
		else if (chlow > 0 && chlow < 0x80)
		{
			unsigned char copychar[2];
			copychar[0] = chlow;
			copychar[1] = 0;
			i ++;
			__drawGraphChars(copychar, color);
			continue;
		}
		else {
			break;
		}

		unsigned short chw = *(WORD*)(font+i);
		i += 2;

		unsigned short* lpcc = getCCIdxInGBK(chw);

		for (int j = 0; j < 16; j++)
		{
			unsigned short f = lpcc[j];
			f = __ntohs(f);
			unsigned int m = 0x8000;
			for (int k = 0; k < 16; k++)
			{
				unsigned int c = 0;
				if (f & m)
				{
					c = color;
					for (int n = 0; n < gBytesPerPixel; n++)
					{
						*showpos = (unsigned char)c;
						c = c >> 8;
						showpos++;
					}
				}
				else if (bgcolor) {
					c = bgcolor;
					for (int n = 0; n < gBytesPerPixel; n++)
					{
						*showpos = (unsigned char)c;
						c = c >> 8;
						showpos++;
					}
				}
				else {
					showpos += gBytesPerPixel;
				}

				m = m >> 1;
			}

			keepx += gBytesPerLine;
			showpos = keepx;
		}

		keepy = keepy + GRAPH_CHINESECHAR_WIDTH*gBytesPerPixel;

		//����Ҫ����������ֱ��ʲ���
		int mod = (unsigned int)(keepy - gGraphBase) % gBytesPerLine;
		if (mod == 0)
		{
			keepy = keepy + (GRAPH_CHINESECHAR_HEIGHT-1)*gBytesPerLine;
			if ((unsigned int)(keepy + GRAPH_CHINESECHAR_HEIGHT*gBytesPerLine - gGraphBase) > gWindowSize)
			{
				keepy = (unsigned char*)gGraphBase;
			}
		}

		keepx = keepy;
		showpos = keepy;
	}
	return (int)(showpos - gGraphBase);
}


int __drawCCS(unsigned char * font, int color) {
	unsigned int pos = __getpos(gShowX, gShowY);
	int resultpos = __drawCC(font, color, pos, BACKGROUND_COLOR);

	gShowY = resultpos / gBytesPerLine;

	gShowX = (resultpos % gBytesPerLine) / gBytesPerPixel;

	if (gShowY >= gWindowHeight)
	{
		gShowY = 0;
		gShowX = 0;
	}
	return 0;
}


int __drawHorizon(int x, int y, int len,int color) {
	unsigned char* pos = (unsigned char*)__getpos(x, y);
	for (int i = 0 ;i < len; i++)
	{
		int c = color;
		for (int j = 0;j < gBytesPerPixel;j ++)
		{
			*pos = (c & 0xff);
			c = c >> 8;
			pos++;
		}
	}
	return len;
}


int __drawVertical(int x, int y, int len, int color) {
	unsigned char* pos = (unsigned char*)__getpos(x, y);
	unsigned char * keep = pos;
	for (int i = 0; i < len; i++)
	{
		int c = color;
		for (int j = 0; j < gBytesPerPixel; j++)
		{
			*pos = (c & 0xff);
			c = c >> 8;
			pos++;
		}
		keep += gBytesPerLine;
		pos = keep;
	}
	return len;
}

int __drawGraphCharIntr(char * font,int color,int pos,int bgcolor){
	int params[4] ;
	__memset((char*)params, 0, 4*sizeof(int));
	params[0] = (int)font;
	params[1] = color;
	params[2] = pos;
	params[3] = bgcolor;
	__asm{
		mov eax,4
		lea edi,params
		int 80h
	}
}



//�������Ļ�ϵ�λ�ñ������Կ��ڲ����������Ĵ����У�
//ÿ���Ĵ�����8 λ���������γ�һ�� 16 λ����ֵ�����磬0 ��ʾ�������Ļ�ϵ� 0 �е� 0 �У�
//80 ��ʾ���ڵ� 1 �е� 0 �У���Ϊ��׼ VGA �ı�ģʽ�� 25 �У�ÿ�� 80 ���ַ������ԣ����������Ļ���½�ʱ����ֵΪ 25 * 80 - 1 = 1999����

//�����Ĵ����Ķ˿ں���0x03d4��ͨ���������Ĵ���д������ֵ��ȡ��Ӧ���Կ��ڲ��Ĵ�����ֵ��
//���� 8 λ���Ĵ���������ֵ�ֱ��� 14��0x0e���� 15��0x0f�����ֱ�洢���λ�õĸ� 8 λ�͵� 8 λ��
//ָ���������Ĵ�����ֵ֮��ͨ�����ݶ˿�0x03d5��ȡ���ݡ�


int __restoreCircle(int x, int y, int radius,unsigned char * backup) {

	__kRestoreMouse();

	int squreRadius = radius*radius;

	int pixelcnt = 0;

	int startx = x - radius;
	int endx = x + radius;
	int starty = y - radius;
	int endy = y + radius;

	unsigned int pos = __getpos(startx, starty);
	unsigned char * showpos = pos + (unsigned char *)gGraphBase;

	unsigned char * keepy = showpos;
	for (int i = starty; i <= endy; i++)
	{
		for (int j = startx; j <= endx; j++)
		{
			int deltaX2 = (j - x)*(j - x);
			int deltaY2 = (i - y)*(i - y);
			if (deltaY2 + deltaX2 <= squreRadius)
			{
				for (int i = 0; i < gBytesPerPixel; i++)
				{
					*showpos = *backup;
					showpos++;
					backup++;
				}

				pixelcnt++;
			}
			else {
				showpos += gBytesPerPixel;
			}
		}
		keepy += gBytesPerLine;
		showpos = keepy;
	}
	__kRefreshMouseBackup();
	__kDrawMouse();

	return (unsigned int)showpos - gGraphBase;
}

extern "C"  __declspec(dllexport) int __drawColorCircle(int x, int y, int radius, int color, unsigned char * backup) {

	__kRestoreMouse();

	int squreRadius = radius*radius;

	int pixelcnt = 0;

	int startx = x - radius;
	int endx = x + radius;
	int starty = y - radius;
	int endy = y + radius;

	unsigned int pos = __getpos(startx, starty);
	unsigned char * showpos = pos + (unsigned char *)gGraphBase;
	//unsigned char * keepx = showpos;
	unsigned char * keepy = showpos;
	for (int i = starty; i <= endy; i++)
	{
		for (int j = startx; j <= endx; j++)
		{
			int deltaX2 = (j - x)*(j - x);
			int deltaY2 = (i - y)*(i - y);
			if (deltaY2 + deltaX2 <= squreRadius)
			{

				unsigned int c = color;
				(color)++;

				for (int i = 0; i < gBytesPerPixel; i++)
				{

					if (backup) {
						*backup = *showpos;
						backup++;
					}

					*showpos = c;
					showpos++;
					c = (c >> 8);
				}

				pixelcnt++;
			}
			else {
				showpos += gBytesPerPixel;
			}
		}
		keepy += gBytesPerLine;
		showpos = keepy;
	}
	__kRefreshMouseBackup();
	__kDrawMouse();

	return (unsigned int)showpos - gGraphBase;
}



int __drawCircle(int x,int y,int radius,int color,unsigned char * backup) {
	__kRestoreMouse();

	int squreRadius = radius*radius;

	int pixelcnt = 0;

	int startx = x - radius;
	int endx = x + radius;
	int starty = y - radius;
	int endy = y + radius;

	unsigned int pos = __getpos(startx, starty);
	unsigned char * showpos = pos + (unsigned char *)gGraphBase;
	//unsigned char * keepx = showpos;
	unsigned char * keepy = showpos;
	for (int i = starty; i <= endy; i++)
	{
		for (int j = startx; j <= endx; j++)
		{
			int deltaX2 = (j - x)*(j - x);
			int deltaY2 = (i - y)*(i - y);
			if (deltaY2 + deltaX2 <= squreRadius)
			{
				unsigned int c = color;
				for (int i = 0;i < gBytesPerPixel;i++)
				{
					if (backup) {
						*backup = *showpos;
						backup++;
					}

					*showpos = c;
					showpos++;
					c = (c >> 8);
				}

				pixelcnt++;
			}
			else {
				showpos += gBytesPerPixel;
			}
		}
		keepy += gBytesPerLine;
		showpos = keepy;
	}
	__kRefreshMouseBackup();
	__kDrawMouse();

	return (unsigned int)showpos - gGraphBase;
}


int __drawRectangle(LPPOINT p, int width, int height, int color,unsigned char * backup) {
	__kRestoreMouse();

	int startpos = p->y * gBytesPerLine + p->x * gBytesPerPixel + gGraphBase;
	unsigned char * ptr = (unsigned char*)startpos;
	unsigned char * keep = ptr;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			int c = color;
			for (int k = 0; k < gBytesPerPixel; k++)
			{
				if (backup)
				{
					*backup = *ptr;
					backup++;
				}

				*ptr = (c & 0xff);
				ptr++;
				c = (c >> 8);
			}
		}

		keep += gBytesPerLine;
		ptr = (unsigned char*)keep;
	}
	__kRefreshMouseBackup();
	__kDrawMouse();
	return (int)ptr - gGraphBase;
}


int __restoreRectangle(LPPOINT p, int width, int height,unsigned char * backup) {
	__kRestoreMouse();

	int startpos = p->y * gBytesPerLine + p->x * gBytesPerPixel + gGraphBase;
	unsigned char * ptr = (unsigned char*)startpos;
	unsigned char * keep = ptr;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			for (int k = 0; k < gBytesPerPixel; k++)
			{
				*ptr = *backup;
				ptr++;
				backup++;
			}
		}

		keep += gBytesPerLine;
		ptr = (unsigned char*)keep;
	}
	__kRefreshMouseBackup();
	__kDrawMouse();
	return (int)ptr - gGraphBase;
}


int __drawRectangleFrame(LPPOINT p, int width, int height, int color, int framesize, int framecolor, char * back) {
	int startpos = __getpos(p->x, p->y) + gGraphBase;
	unsigned char * ptr = (unsigned char*)startpos;
	unsigned char * keep = ptr;

	int singlefs = framesize >> 1;

	for (int i = 0; i < height + framesize; i++)
	{
		for (int j = 0; j < width + framesize; j++)
		{
			int c = 0;

			if ((i <= singlefs) || (i >= height + singlefs))
			{
				c = framecolor;
			}
			else if ((j <= singlefs) || (j >= width + singlefs))
			{
				c = framecolor;
			}
			else {
				c = color;
			}

			for (int k = 0; k < gBytesPerPixel; k++)
			{
				if (back)
				{
					*back = *ptr;
					back++;
				}
				*ptr = (c & 0xff);
				ptr++;
				c = (c >> 8);
			}
		}

		keep += gBytesPerLine;
		ptr = (unsigned char*)keep;
	}

	return (int)ptr - gGraphBase;
}



int __drawFileMap(LPFILEMAP computer) {

	__kRestoreMouse();

	computer->backsize = gBytesPerPixel*(computer->width + computer->frameSize)*(computer->height + computer->frameSize);

	unsigned int ptr = __drawRectangleFrame(&computer->pos, computer->width, computer->height,
		computer->color, computer->frameSize, computer->frameColor, (char*)computer->backGround);

	DWORD offset = 0;
	DWORD size = 0;
	getResFromID(MAIN_DLL_BASE, computer->id, 3, &offset, &size);
	if (offset && size)
	{
		LPBITMAPINFOHEADER bmpinfo = (LPBITMAPINFOHEADER)offset;
		bmpinfo->biHeight = bmpinfo->biHeight / 2;
		unsigned char * data = (unsigned char *)offset + sizeof(BITMAPINFOHEADER);
		showBmpBits(computer->pos.x, computer->pos.y, (LPBITMAPINFOHEADER)offset,data);
	}
	

	ptr = ptr + GRAPHCHAR_HEIGHT*gBytesPerLine*computer->zoomin + GRAPHCHAR_HEIGHT*4*gBytesPerPixel*computer->zoomin;

	computer->showX = (ptr % gBytesPerLine) / gBytesPerPixel;
	computer->showY = ptr / gBytesPerLine;

	int showend = __showGraphString(computer);

	__kRefreshMouseBackup();
	__kDrawMouse();

	return showend;
}

int __drawDot(int x, int y, DWORD color) {
	char * ptr = (char*)__getpos(x, y) + gGraphBase;
	char * c = (char*)&color;
	for (int k = 0; k < gBytesPerPixel; k++)
	{
		*ptr = *c;
		ptr++;
		c++;
	}

	return 1;
}

int __drawLine(int x1, int y1, int x2, int y2,DWORD color) {

	if (x1 == x2 && y1==y2)
	{
		return __drawDot(x1, y1, color);
	}

	int xinc = 1;
	if (x1> x2)
	{
		xinc = -1;
	}

	int yinc = 1;
	if (y1 > y2)
	{
		yinc = -1;
	}

	char * ptr = (char*)__getpos(x1, y1) + gGraphBase;
	char * oldpos = ptr;

	if (x1 == x2)
	{
		for (int i = y1; i != y2; i += yinc)
		{
			char * c = (char*)&color;
			for (int k = 0; k < gBytesPerPixel; k++)
			{
				*ptr = *c;
				ptr++;
				c++;
			}
		}

		return y2 - y1;
	}
	else if (y1 == y2)
	{
		for (int i = x1; i != x2; i += xinc)
		{
			char * c = (char*)&color;
			for (int k = 0; k < gBytesPerPixel; k++)
			{
				*ptr = *c;
				ptr++;
				c++;
			}
		}

		return x2 - x1;
	}
	else {
		char * c = (char*)&color;

		float rate = (x2 - x1) / (y2 - y1);

		for (int j = y1; j != y2; j += yinc)
		{
			for (int i = x1; i != x2; i += xinc)
			{
				int oldx = (j - y1)*rate + x1;
				if (i == oldx)
				{
					c = (char*)&color;
					for (int k = 0; k < gBytesPerPixel; k++)
					{
						*ptr = *c;
						ptr++;
						c++;
					}
					break;
				}
				else {
					ptr += gBytesPerPixel;
				}
			}
			oldpos += gBytesPerLine;
			ptr = oldpos;
		}

		return (x2 - x1)*(y2 - y1);
	}
}


int __diamond(int startx, int starty,int raduis,int cnt,DWORD color) {

	int n = cnt, i, j;
	double t = 3.14159 * 2 / n, r = raduis;
	double x0 = startx, y0 = starty, x[64], y[64];
	for (i = 0; i < n; i++)
	{
		x[i] = r*cos(i*t) + x0;
		y[i] = r*sin(i*t) + y0;
	}

	for (i = 0; i <= n - 2; i++) {
		for (j = i + 1; j <= n - 1; j++) {
			__drawLine(x[i], y[i], x[j], y[j],color);
		}
	}

	return 0;
}


int __diamond2(int startx,int starty,int raduis,int cnt,DWORD color) {
	//���ܣ�һ�ʻ��ƽ��ʯͼ����n>=5,n��������

	int n = cnt, i, j;
	double t = 3.14159 * 2 / n, r = raduis;
	double x0 = startx, y0 = starty, x[64], y[64];
	for (i = 0; i < n; i++)
	{
		x[i] = r*cos(i*t) + x0;
		y[i] = r*sin(i*t) + y0;
	}

	for (i = 1; i <= n / 2; i++) {

		for (j = 0; j < n; j++)
		{
			if ((j + i) >= n)
			{
				int r = (j + i) % n;
				__drawLine(x[j], y[j], x[r], y[r], color);
			}
			else {
				__drawLine(x[j], y[j], x[j + i], y[j + i], color);
			}
		}
	}

	return 0;
}