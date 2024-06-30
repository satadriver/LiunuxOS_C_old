#include "Utils.h"
#include "def.h"
#include "Kernel.h"
#include "video.h"
#include "keyboard.h"
#include "mouse.h"
#include "process.h"
#include "task.h"
#include "Pe.h"
#include "satadriver.h"
#include "sectorReader.h"
#include "fat32/FAT32.h"
#include "fat32/fat32file.h" 
#include "file.h"
#include "NTFS/ntfs.h"
#include "NTFS/ntfsFile.h"
#include "pci.h"
#include "speaker.h"
#include "system.h"
#include "screenUtils.h"
#include "cmosAlarm.h"
#include "rs232.h"
#include "floppy.h"
#include "malloc.h"
#include "page.h"
#include "processDOS.h"
#include "gdi.h"
#include "coprocessor.h"
#include "Thread.h"
#include "debugger.h"
#include "descriptor.h"
#include "elf.h"
#include "page.h"

//#pragma comment(linker, "/ENTRY:DllMain")
//#pragma comment(linker, "/align:512")
//#pragma comment(linker, "/merge:.data=.text")
//#pragma comment(linker, "/merge:.rdata=.text")

LPSYSDESCRIPTOR glpCallGate = 0;
LPSEGDESCRIPTOR glpLdt = 0;
LPSEGDESCRIPTOR glpGdt = 0;
LPSYSDESCRIPTOR glpIdt = 0;
DWORD gV86VMIEntry = 0;
DWORD gV86VMLeave = 0;
DWORD gKernel16;
DWORD gKernel32;
DWORD gKernelData;



void getGdtIdt() {
	DESCRIPTOR_REG gdt;
	DESCRIPTOR_REG idt;
	__asm {
		lea eax,gdt
		sgdt [eax]

		lea eax,idt
		sidt [eax]
	}

	glpGdt = (LPSEGDESCRIPTOR)gdt.addr;

	glpIdt = (LPSYSDESCRIPTOR)idt.addr;

	int gdtcnt = (gdt.size + 1 ) >> 3;
	for (int i = 1;i < gdtcnt;i ++)
	{
		if (glpGdt[i].attr == 0xe2 || glpGdt[i].attr == 0x82)
		{
			glpLdt = &glpGdt[i];
			initLdt(glpLdt);
			break;
		}else if (glpGdt[i].attr == 0xec || glpGdt[i].attr == 0x8c)
		{
			glpCallGate = (LPSYSDESCRIPTOR)&glpGdt[i];
			initCallGate((LPSYSDESCRIPTOR)&glpGdt[i]);
			break;
		}
	}
}


//c++函数的导出函数对应函数声明的顺序，而不是函数体，函数体的参数一一对应于声明中的顺序
int __kernelEntry(LPVESAINFORMATION vesa, DWORD fontbase,DWORD v86Proc,DWORD v86Leave ,DWORD kerneldata,DWORD kernel16,DWORD kernel32) {

	int ret = 0;
	
	gV86VMIEntry = v86Proc;
	gV86VMLeave = v86Leave;
	gKernelData = kerneldata;
	gKernel16 = kernel16;
	gKernel32 = kernel32;
	//gAsmTsses = lpasmTsses;

	//must be first to prepare for showing
	__getVideoParams(vesa, fontbase);

	char szout[1024];

	getGdtIdt();

	initMemory();

	initPage();

	enablePage();

	__initTask();

	initDll();

	sysenterEntry(0,0);

	initRS232Com1();
	initRS232Com2();

	initEfer();

	initCoprocessor();

	__asm {
		sti
	}

	__printf(szout, "Hello world of Liunux!\r\n");

#ifndef TASK_SINGLE_TSS
	//__createDosInFileTask(gV86VMIEntry, "V86VMIEntry");
#endif

// 	TASKCMDPARAMS cmd;
// 	__memset((char*)&cmd, 0, sizeof(TASKCMDPARAMS));
// 	__kCreateThread((DWORD)__kSpeakerProc, (DWORD)&cmd, "__kSpeakerProc");

	//logFile("__kernelEntry\n");

	//__rmSectorReader(0, 1, szout, 1024);
	
// 	DWORD kernelMain = getAddrFromName(KERNEL_DLL_BASE, "__kKernelMain");
// 	if (kernelMain)
// 	{
// 		TASKCMDPARAMS cmd;
// 		__memset((char*)&cmd, 0, sizeof(TASKCMDPARAMS));
// 		__kCreateThread((unsigned int)kernelMain,(DWORD)&cmd, "__kKernelMain");
// 	}

	//must be after running V86VMIEntry and sti
	initFileSystem();

	initDebugger();

// 	floppyInit();
// 	FloppyReadSector(0, 1, (unsigned char*)FLOPPY_DMA_BUFFER);

// 	ret = loadLibRunFun("c:\\liunux\\main.dll", "__kMainProcess");
 	

	int imagesize = getSizeOfImage((char*)MAIN_DLL_SOURCE_BASE);

	__printf(szout, "__kMainProcess size:%x\n", imagesize);
	__kCreateProcessFromAddrFunc(MAIN_DLL_SOURCE_BASE, imagesize,  "__kExplorer", 3, 0);

	while (1)
	{
		if (__findProcessFuncName("__kExplorer") == FALSE)
		{
			__kCreateProcess(MAIN_DLL_SOURCE_BASE, imagesize, "main.dll", "__kExplorer", 3, 0);
		}

		__asm {
			hlt
		}
	}

	return 0;
}



void __kKernelMain(DWORD retaddr,int pid,char * filename,char * funcname,DWORD param) {

	int ret = 0;

 	char szout[1024];
	__printf(szout, "__kKernelMain task pid:%x,filename:%s,function name:%s\n", pid, filename,funcname);

// 	unsigned char sendbuf[1024];
// 	//最大不能超过14字节
// 	__strcpy((char*)sendbuf, "how are you?");
// 	ret = sendCom2Data(sendbuf, __strlen("how are you?"));
// 
// 	unsigned char recvbuf[1024];
// 	int recvlen = getCom2Data(recvbuf);
// 	if (recvlen > 0)
// 	{
// 		*(recvbuf + recvlen) = 0;
// 		
// 		__printf(szout, "recvbuf data:%s\n", recvbuf);
// 		__drawGraphChars((unsigned char*)szout, 0);
// 	}

	//setVideoMode(0x4112);

	while (1)
	{
		__sleep(1000);
	}

	while (1)
	{
		__asm {
			hlt
		}
	}
}



//注意二位数组在内存中的排列和结构
void mytest() {

	return;
}

#ifdef _USRDLL
int __stdcall DllMain( HINSTANCE hInstance,  DWORD fdwReason,  LPVOID lpvReserved) {
	return TRUE;
}
#else
int __stdcall WinMain(  HINSTANCE hInstance,  HINSTANCE hPrevInstance,  LPSTR lpCmdLine,  int nShowCmd )
{
	mytest();
	return TRUE;
}
#endif