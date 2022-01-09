#include "window.h"
#include "task.h"
#include "video.h"
#include "Utils.h"
#include "screenUtils.h"
#include "slab.h"
#include "ListEntry.h"
#include "memory.h"



LPWINDOWSINFO gWindowsList = 0;
LPWINDOWSINFO gWindowLast = 0;

LPWINDOWSINFO checkWindowExist(char * wname) {
	LPWINDOWSINFO info = (LPWINDOWSINFO)gWindowsList;
	do
	{
		if (info->valid && info->windowname[0] && __strcmp(info->windowname,wname)== 0 )
		{
			return info;
		}
		else {
			if (info->list.next == 0)
			{
				break;
			}
			info = (LPWINDOWSINFO)(info->list.next);
		}
	} while (info != gWindowsList);

	return 0;
}

LPWINDOWSINFO checkWindowExist(DWORD wid) {
	LPWINDOWSINFO info = (LPWINDOWSINFO)gWindowsList;
	do
	{
		if (info->valid && info->id == wid )
		{
			return info;
		}
		else {
			if (info->list.next == 0)
			{
				break;
			}
			info = (LPWINDOWSINFO)(info->list.next);
		}
	} while (info != gWindowsList);

	return 0;
}

LPWINDOWSINFO getFreeWindow() {
	LPWINDOWSINFO info = (LPWINDOWSINFO)gWindowsList;

	int c = WINDOW_LIST_BUF_SIZE / sizeof(WINDOWSINFO);
	for (int i = 0; i < c; i++)
	{
		if (info[i].valid == 0)
		{
			//info[i].valid = TRUE;
			return &info[i];
		}
	}
	return 0;
}


void initWindowList() {
	gWindowsList = (LPWINDOWSINFO)__kMalloc(WINDOW_LIST_BUF_SIZE);
	__memset((char*)gWindowsList, 0, WINDOW_LIST_BUF_SIZE);

	initListEntry((LPLIST_ENTRY)&gWindowsList->list);

	gWindowLast = gWindowsList;
}

DWORD isTopWindow(int wid) {
	LPWINDOWSINFO window = wid + gWindowsList;
	if (window == gWindowLast)
	{
		return TRUE;
	}
	return FALSE;
	//return (window == gWindowLast ? TRUE : FALSE);
}


DWORD getTopWindow() {
	return gWindowLast - gWindowsList;
}

int addWindow(int active, DWORD *x, DWORD *y, int color,char * wname) {
	char szout[1024];

	LPWINDOWSINFO window = checkWindowExist(wname);
	if (window)
	{
		return FALSE;
	}

	window = getFreeWindow();
	if (window == FALSE)
	{
		__printf(szout, "getFreeWindow error,first:%x,last:%x\n", gWindowsList, gWindowLast);
		__drawGraphChars((unsigned char*)szout, 0);
		return -1;
	}
	window->valid = TRUE;
	int i = window - gWindowsList;

	window->id = i;

	window->cursorColor = color;

	//exchange to physical address from linear address
	DWORD *addrx = (DWORD *)linear2phy((DWORD)x);
	DWORD *addry = (DWORD *)linear2phy((DWORD)y);

	window->cursorX = addrx;
	window->cursorY = addry;

	__strncpy(window->windowname, wname, WINDOW_NAME_LIMIT);
	window->windowname[WINDOW_NAME_LIMIT - 1] = 0;

	if (active)
	{
		window->valid |= 0x80000000;
		setCursor(TRUE, window->cursorX, window->cursorY, window->cursorColor);
	}
	else {
		setCursor(FALSE, window->cursorX, window->cursorY, window->cursorColor);
	}

	addlistTail(&gWindowsList->list, &window->list);

	gWindowLast = window;

// 	__printf(szout, "add windowid:%x,first:%x,top:%x\n", i, gWindowsList ,gWindowLast);
// 	__drawGraphChars((unsigned char*)szout, 0);

	return i;
}


int removeWindow(int id) {

	LPWINDOWSINFO window = gWindowsList + id;

	window->valid = FALSE;

	removelist( &window->list);
	
	if (gWindowLast == window)
	{
		gWindowLast = (LPWINDOWSINFO)window->list.prev;
		if (gWindowLast->valid & 0x80000000)
		{
			setCursor(TRUE, gWindowLast->cursorX, gWindowLast->cursorY, gWindowLast->cursorColor);
		}
		else {
			setCursor(FALSE, gWindowLast->cursorX, gWindowLast->cursorY, gWindowLast->cursorColor);
		}
	}

	return TRUE;
}


LPWINDOWCLASS addWindowList(LPWINDOWCLASS window) {
	LPPROCESS_INFO p = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	if (p->window)
	{
		LPWINDOWCLASS w = p->window;
		while (w->next)
		{
			w = w->next;
		}

		w->next = window;
		window->prev = w;
	}
	else {
		window->prev = 0;
		window->next = 0;
		p->window = window;
	}

	return window;
}


LPWINDOWCLASS removeWindowList() {
	LPPROCESS_INFO p = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	LPWINDOWCLASS lpw = p->window;
	while (lpw)
	{
		if (lpw->prev && lpw->next)
		{
			lpw->prev->next = lpw->next;
			lpw->next->prev = lpw->prev;
			break;
		}
		else if (lpw->prev)
		{
			lpw->prev->next = 0;
			break;
		}
		else if (lpw->next)
		{
			p->window = lpw->next;
			lpw->next->prev = 0;
			break;
		}
		else {
			p->window = 0;
			break;
		}
		
		lpw = lpw->next;
		if (lpw == 0)
		{
			break;
		}
	}

	return lpw;
}


int destroyWindows() {
	int cnt = 0;

	LPPROCESS_INFO p = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;

	LPWINDOWCLASS windows = p->window;
	
	if (windows)
	{
		LPWINDOWCLASS next = windows;

		while (next->next)
		{
			next = next->next;
		}	

		while (next)
		{
			__restoreWindow(next);
			next = next->prev;
			cnt++;
		}
	}

	return cnt;
}





LPWINDOWCLASS getWindowFromName(char * winname) {

	TASK_LIST_ENTRY * list = (TASK_LIST_ENTRY*)TASKS_LIST_BASE;
	do
	{
		if (list->valid && list->process)
		{
			LPWINDOWCLASS window = list->process->window;
			while (window)
			{
				if (__strcmp(window->caption, winname) == 0)
				{
					return window;
				}
				window = window->next;
			}
		}
		list = (TASK_LIST_ENTRY *)list->list.next;
		if (list == 0)
		{
			break;
		}
	} while (list != (TASK_LIST_ENTRY*)TASKS_LIST_BASE);

	return 0;
}
