#include "def.h"

#include "Explorer.h"
#include "console.h"
#include "video.h"
#include "mouse.h"
#include "keyboard.h"
#include "task.h"
#include "graph.h"
#include "soundBlaster/sbPlay.h"
#include "screenUtils.h"
#include "Utils.h"
#include "menu.h"
#include "windowclass.h"
#include "Pe.h"
#include "window.h"
#include "system.h"
#include "satadriver.h"
#include "UserUtils.h"
#include "Kernel.h"
#include "sysregs.h"
#include "WindowClass.h"
#include "Utils.h"
#include "paint.h"
#include "slab.h"
#include "Thread.h"
#include "mouse.h"
#include "pci.h"
#include "window.h"
#include "keyboard.h"
#include "FileManager.h"
#include "descriptor.h"
#include "debugger.h"
#include "gdi.h"
#include "pci.h"
#include "hept.h"
#include "cmosAlarm.h"
#include "elf.h"

int __kExplorer(unsigned int retaddr, int tid, char * filename, char * funcname, DWORD param) {
	int ret = 0;

	char szout[1024];
	__printf(szout, "__kExplorer task retaddr:%x,pid:%x,name:%s,funcname:%s,param:%x\n", retaddr, tid, filename, funcname, param);
	__drawGraphChars((unsigned char*)szout, 0);

	initWindowList();

	WINDOWCLASS window;
	initDesktopWindow(&window, "__deskTop", tid);

	WINDOWCLASS taskbar;
	initTaskbarWindow(&taskbar, filename, tid);

	FILEMAP computer;
	initBigClickItem(&computer, "My Computer", tid,1, gVideoWidth - 2 * gBigFolderWidth, gBigFolderHeight);

	FILEMAP atapi;
	initBigClickItem(&atapi, "CD-ROM", tid,2, gVideoWidth - 2 * gBigFolderWidth, gBigFolderHeight + gBigFolderHeight + gBigFolderHeight);

	FILEMAP floppy;
	initBigClickItem(&floppy, "Floppy", tid,3, gVideoWidth - 2 * gBigFolderWidth,
		gBigFolderHeight + gBigFolderHeight + gBigFolderHeight + gBigFolderHeight + gBigFolderHeight);

	__kDrawWindowsMenu();

	RIGHTMENU menu;
	initWindowsRightMenu(&menu, tid);

	char cputype[1024];
	getCpuType(cputype);
	char cpuinfo[1024];
	getCpuInfo(cpuinfo);
	__printf(szout, "CPU MODEL:%s,details:%s,video height:%d,width:%d,pixel:%d\n", cputype, cpuinfo, gVideoHeight, gVideoWidth, gBytesPerPixel);
	__drawGraphChars((unsigned char*)szout, 0);

	showPciDevs();

	__kdBreakPoint();

	
	__kAddCmosAlarm(30, (DWORD)__doAlarmTask, 0);

	//initEfer();

	getRCBA();

	sysenterEntry(0, 0);

	callgateEntry(0, 0);

	repeatDrawCCFontString();

	//runElfFunction("c:\\liunux\\test.so", "__testfunction");

	//__drawLine(100, 100, 200, 200, 0)

	//__diamond(400, 400, 200, 7, 0);

	TASKCMDPARAMS taskcmd;
	__memset((char*)&taskcmd, 0, sizeof(TASKCMDPARAMS));

	while (1)
	{
		unsigned int ck = __kGetKbd(window.id);
		//unsigned int ck = __getchar(window.id);
		unsigned int asc = ck & 0xff;
		if (asc == VK_F1)
		{
			if (__findProcessFileName("__kConsole") == FALSE)
			{
				__kCreateProcess(VSMAINDLL_LOAD_ADDRESS, 0x100000, "main.dll", "__kConsole", 3, 0);
			}
			continue;
		}
		else if (asc == VK_F2)
		{
			if (__findProcessFileName("__kLogWatch") == FALSE)
			{
				TASKCMDPARAMS cmd;
				__memset((char*)&cmd, 0, sizeof(TASKCMDPARAMS));
				cmd.cmd = SHOW_SYSTEM_LOG;
				cmd.addr = LOG_BUFFER_BASE;
				cmd.filesize = (DWORD)gLogDataPtr - LOG_BUFFER_BASE;

				DWORD thread = getAddrFromName(MAIN_DLL_BASE, "__kShowWindow");
				return __kCreateThread((DWORD)thread, MAIN_DLL_BASE, (DWORD)&cmd, "__kLogWatch");
				//__kCreateProcess(VSMAINDLL_LOAD_ADDRESS, 0x100000, "main.dll", "__kLogWatch", 3, 0);
			}
			continue;
		}
		else if (asc == VK_F3)
		{
			__createDosInFileTask(gV86VMIEntry, "V86VMIEntry");
			continue;
		}

		MOUSEINFO mouseinfo;
		__memset((char*)&mouseinfo, 0, sizeof(MOUSEINFO));
		ret = __kGetMouse(&mouseinfo, window.id);
		if (mouseinfo.status & 1)	//left click
		{
			if (menu.action)
			{
				__restoreMenu(&menu);

				if ((mouseinfo.x > menu.pos.x) && (mouseinfo.x < menu.pos.x + menu.width))
				{
					if (mouseinfo.y > menu.pos.y && mouseinfo.y < menu.pos.y + menu.height)
					{
						int funcno = (mouseinfo.y - menu.pos.y) / GRAPHCHAR_HEIGHT / 2;
						if (funcno > 0 && funcno < menu.validItem)
						{
							DWORD func = menu.funcaddr[funcno];

							int cnt = menu.paramcnt[funcno];

							int stacksize = cnt * 4;

							DWORD * thisparams = (DWORD*)&menu.funcparams[funcno][0];

							__asm {
								mov ecx, cnt
								cmp ecx, 0
								jz _callfunc
								mov esi, thisparams
								add esi, stacksize
								sub esi,4
								_params :
								mov eax, [esi]
								push eax
								sub esi, 4
								loop _params
								_callfunc :
								mov eax, func
								call eax
								add esp, stacksize
							}
						}
					}
				}

				menu.action = 0;
			}

			if (mouseinfo.x >= computer.pos.x && mouseinfo.x < computer.pos.x + computer.frameSize + computer.width)
			{
				if (mouseinfo.y >= computer.pos.y && mouseinfo.y <= computer.pos.y + computer.height + computer.frameSize)
				{
					taskcmd.cmd = UNKNOWN_FILE_SYSTEM;
					__strcpy(taskcmd.filename, "FileMgrHD");

					__kCreateProcess(VSMAINDLL_LOAD_ADDRESS, 0x100000, "main.dll", "__kFileManager", 3, (DWORD)&taskcmd);
				}
			}

			if (mouseinfo.x >= atapi.pos.x && mouseinfo.x < (atapi.pos.x + atapi.frameSize + atapi.width))
			{
				if (mouseinfo.y >= atapi.pos.y && mouseinfo.y <= (atapi.pos.y + atapi.height + atapi.frameSize))
				{
					taskcmd.cmd = CDROM_FILE_SYSTEM;
					__strcpy(taskcmd.filename, "FileMgrISO");

					__kCreateProcess(VSMAINDLL_LOAD_ADDRESS, 0x100000, "main.dll", "__kFileManager", 3, (DWORD)&taskcmd);
				}
			}

			if (mouseinfo.x >= floppy.pos.x && mouseinfo.x < (floppy.pos.x + floppy.frameSize + floppy.width))
			{
				if (mouseinfo.y >= floppy.pos.y && mouseinfo.y <= (floppy.pos.y + floppy.height + floppy.frameSize))
				{
					taskcmd.cmd = FLOPPY_FILE_SYSTEM;
					__strcpy(taskcmd.filename, "FileMgrFllopy");

					__kCreateProcess(VSMAINDLL_LOAD_ADDRESS, 0x100000, "main.dll", "__kFileManager", 3, (DWORD)&taskcmd);
				}
			}
		}
		else if (mouseinfo.status & 2)	//right click
		{
			if (mouseinfo.x > gVideoWidth - TASKBAR_HEIGHT && mouseinfo.x < gVideoWidth)
			{
				if ((mouseinfo.y > gVideoHeight - TASKBAR_HEIGHT) && mouseinfo.y < gVideoHeight)
				{
					menu.pos.x = mouseinfo.x;
					menu.pos.y = mouseinfo.y;
					menu.action = mouseinfo.status;
					__drawRightMenu(&menu);
				}
			}
		}
		else if (mouseinfo.status & 4)	//middle click
		{
// 			menu.pos.x = mouseinfo.x;
// 			menu.pos.y = mouseinfo.y;
// 			menu.action = mouseinfo.status;
		}

		__sleep(0);
	}
	return 0;
}


DWORD isDesktop() {
	LPPROCESS_INFO tss = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	if (__strcmp(tss->filename,"Explorer.exe") == 0)
	{
		return TRUE;
	}

	return FALSE;
}