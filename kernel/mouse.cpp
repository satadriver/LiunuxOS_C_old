#include "mouse.h"
#include "window.h"
#include "def.h"
#include "hardware.h"
#include "video.h"
#include "Utils.h"
#include "servicesProc.h"

DWORD gMouseTest = FALSE;

DWORD gMousePackSize = 0;

DWORD gMouseColor = MOUSE_SHOW_COLOR;
#define MOUSE_FACTOR_SIZE 40


void mousetest() {

	char szout[1024];
	MOUSEINFO mouseinfo;
	__memset((char*)&mouseinfo, 0, sizeof(MOUSEINFO));
	unsigned int ret = __kGetMouse(&mouseinfo,gMouseTest);
	if (ret)
	{
		DWORD pos = (gVideoHeight - GRAPHCHAR_HEIGHT*2) * gVideoWidth * gBytesPerPixel + (gVideoWidth/2)*gBytesPerPixel;

		__printf(szout, "mouse x:%x,mouse y:%x,status:%x\n", mouseinfo.x,mouseinfo.y,mouseinfo.status);
		__drawGraphChar((unsigned char*)szout, 0, pos, TASKBARCOLOR);
	}
}

int getmouse(LPMOUSEINFO lpinfo,int wid) {

	if (isTopWindow(wid))
	{
		__asm {
			mov edi, lpinfo
			mov eax, 3
			int 80h
		}
	}
	else {
		return FALSE;
	}
}


void restoreMouse() {
	__asm {
		mov eax, RESTORE_MOUSE
		int 80h
	}
}


void drawMouse() {
	__asm {
		mov eax, DRAW_MOUSE
		int 80h
	}
}

void invalidMouse() {
	LPMOUSEDATA data = (LPMOUSEDATA)MOUSE_BUFFER;
	data->bInvalid = TRUE;
}


void __kMouseProc() {
	LPMOUSEDATA data = (LPMOUSEDATA)MOUSE_BUFFER;
	int * pos = (int*)&data->mintrData.status;
	int counter = 0;
	while (TRUE)
	{
		int status = inportb(0x64);
		if ((status & 1) == 0)
		{
			break;
		}

		int md = inportbs(0x60);
		*pos = md;
		pos++;

		counter++;
		if (gMousePackSize )
		{
			if (counter >= 4)
			{
				break;
			}
		}
		else {
			if (counter >= 3)
			{
				break;
			}
		}
	}

	if (gMousePackSize == 3 || gMousePackSize == 4)
	{
		if (counter != 4)
		{
			return;
		}
	}
	else {
		if (counter != 3)
		{
			return;
		}
	}

	data->mintrData.y = - data->mintrData.y;

	if (data->mintrData.x || data->mintrData.y)
	{
		__kRestoreMouse();

		data->mouseX += data->mintrData.x;
		if (data->mouseX > gVideoWidth)
		{
			data->mouseX = gVideoHeight;
		}
		else if (data->mouseX < 0)
		{
			data->mouseX = 0;
		}

		data->mouseY += data->mintrData.y;
		if (data->mouseY > gVideoHeight)
		{
			data->mouseY = gVideoHeight;
		}
		else if (data->mouseY < 0)
		{
			data->mouseY = 0;
		}

		__kDrawMouse();
	}

	if (data->mintrData.status & 7)
	{
		data->mouseBuf[data->mouseBufHdr].status = data->mintrData.status;
		data->mouseBuf[data->mouseBufHdr].x = data->mouseX;
		data->mouseBuf[data->mouseBufHdr].y = data->mouseY;

		data->mouseBufHdr++;
		if (data->mouseBufHdr >= MOUSE_POS_LIMIT)
		{
			data->mouseBufHdr = 0;
		}
	}

	if (gMouseTest)
	{
		mousetest();
	}
}



int __kGetMouse(LPMOUSEINFO lpmouse, int wid) {

	if (isTopWindow(wid) )
	{
		LPMOUSEDATA data = (LPMOUSEDATA)MOUSE_BUFFER;
		if (data->mouseBufHdr == data->mouseBufTail)
		{
			return FALSE;
		}
		
		lpmouse->status = data->mouseBuf[data->mouseBufTail].status;
		lpmouse->x = data->mouseBuf[data->mouseBufTail].x;
		lpmouse->y = data->mouseBuf[data->mouseBufTail].y;
		data->mouseBufTail++;
		if (data->mouseBufTail >= MOUSE_POS_LIMIT)
		{
			data->mouseBufTail = 0;
		}
		return TRUE;
	}
	else {
		return FALSE;
	}
}

void __kDrawMouse() {
	LPMOUSEDATA data = (LPMOUSEDATA)MOUSE_BUFFER;
	int pos = __getpos(data->mouseX, data->mouseY) + gGraphBase;

	unsigned char * storeptr = (unsigned char*)data->mouseCoverData;

	for (unsigned int y = 0; y < data->mouseHeight; y++)
	{
		for (unsigned int x = 0; x < data->mouseWidth; x++)
		{
			int color = 0;
			if (6 * y >= 4 * x && 6 * y <= 9 * x) {
				if (6 * y <= 4 * (x + MOUSE_BORDER_SIZE) || 6 * y >= 9 * (x - MOUSE_BORDER_SIZE))
				{
					color = MOUSE_BORDER_COLOR;

				}
				else {
					color = gMouseColor;
					gMouseColor += 0x000f;
				}
				unsigned char * showpos = (unsigned char*)__getpos(x, y) + pos;

				for (int i = 0; i < gBytesPerPixel; i++)
				{
					*storeptr = *showpos;
					*showpos = color;
					color >>= 8;
					showpos++;
					storeptr++;
				}
			}
		}
	}
}

void __kRestoreMouse() {

	LPMOUSEDATA data = (LPMOUSEDATA)MOUSE_BUFFER;
	int pos = __getpos(data->mouseX, data->mouseY) + gGraphBase;

	unsigned char * storeptr = (unsigned char*)data->mouseCoverData;

	for (unsigned int y = 0; y < data->mouseHeight; y++)
	{
		for (unsigned int x = 0; x < data->mouseWidth; x++)
		{
			if (6 * y >= 4 * x && 6 * y <= 9 * x)
			{
				unsigned char * showpos = (unsigned char*)__getpos(x, y) + pos;

				for (int i = 0; i < gBytesPerPixel; i++)
				{
					*showpos = *storeptr;
					showpos++;
					storeptr++;
				}
			}
		}
	}
}

//4x< 6y < 9x
void __kRefreshMouseBackup() {

	LPMOUSEDATA data = (LPMOUSEDATA)MOUSE_BUFFER;
	int pos = __getpos(data->mouseX, data->mouseY) + gGraphBase;

	unsigned char * storebuf = (unsigned char*)data->mouseCoverData;

	for (unsigned int y = 0; y < data->mouseHeight; y ++)
	{
		for (unsigned int x = 0; x < data->mouseWidth; x ++)
		{
			if (6*y >= 4*x && 6*y <= 9*x)
			{
				unsigned char * showpos = (unsigned char*)__getpos(x, y) + pos;

				for (int i = 0;i < gBytesPerPixel;i ++)
				{
					*storebuf = *showpos;
					storebuf++;
					showpos++;
				}
			}
		}
	}
}


void waitPs2Out() {
	__asm {
		__waitPs2Out:
		in al, 64h
		test al, 1
		jz __waitPs2Out
		ret
	}
}





void waitPs2In() {
	__asm {
		__waitPs2In:
		in al, 64h
		test al, 2
		jnz __waitPs2In
		ret
	}
}

void __initMouse(int x,int y) {

	__asm {
		//call waitPs2In
		mov al,0xd4
		out 64h,al

		//call waitPs2In
		mov al,0xf2
		out 60h,al

		//call waitPs2In
		//mov al, 0x20
		//out 64h, al

		//call waitPs2Out

		in al,60h
		movzx eax,al
		mov gMousePackSize,eax
	}

	char szout[1024];
	__printf(szout, "mouse packet size:%d\n", gMousePackSize);
	__drawGraphChars((unsigned char*)szout,  0);

	LPMOUSEDATA data = (LPMOUSEDATA)MOUSE_BUFFER;
	data->mouseX = x/2;
	data->mouseY = y/2;

	if (x > y)
	{
		data->mouseWidth = y / MOUSE_FACTOR_SIZE;
		data->mouseHeight = y / MOUSE_FACTOR_SIZE;
	}
	else {
		data->mouseWidth = x / MOUSE_FACTOR_SIZE;
		data->mouseHeight = x / MOUSE_FACTOR_SIZE;
	}

	//__kRefreshMouseBackup();
	__kDrawMouse();
}

void insertMouse(MOUSEINFO * info) {
	LPMOUSEDATA data = (LPMOUSEDATA)MOUSE_BUFFER;
	data->mouseBuf[data->mouseBufHdr].status = info->status;
	data->mouseBuf[data->mouseBufHdr].x = info->x;
	data->mouseBuf[data->mouseBufHdr].y = info->y;

	data->mouseBufHdr++;
	if (data->mouseBufHdr >= MOUSE_POS_LIMIT)
	{
		data->mouseBufHdr = 0;
	}
}