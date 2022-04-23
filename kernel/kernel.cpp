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
#include "slab.h"
#include "page.h"
#include "dosProcess.h"
#include "gdi.h"
#include "coprocessor.h"
#include "Thread.h"
#include "debugger.h"
#include "descriptor.h"
#include "elf.h"


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
DWORD gAsmTsses;

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
int __kernelEntry(LPVESAINFORMATION vesa, DWORD fontbase,DWORD v86Proc,DWORD v86Leave ,DWORD kerneldata,DWORD kernel16,DWORD kernel32,DWORD lpasmTsses) {

	int ret = 0;
	
	gV86VMIEntry = v86Proc;
	gV86VMLeave = v86Leave;
	gKernelData = kerneldata;
	gKernel16 = kernel16;
	gKernel32 = kernel32;
	gAsmTsses = lpasmTsses;

	//must be first to prepare for showing
	__getVideoParams(vesa, fontbase);

	char szout[1024];

	getGdtIdt();

	initMemory();

	initPage();

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

	__createDosInFileTask(gV86VMIEntry, "V86VMIEntry");

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

	__sprintf(szout, "Hello world of Liunux!\r\n");

	initDebugger();

// 	floppyInit();
// 	FloppyReadSector(0, 1, (unsigned char*)FLOPPY_DMA_BUFFER);

// 	ret = loadLibRunFun("c:\\liunux\\main.dll", "__kMainProcess");
// 	__printf(szout, "__kMainProcess result:%x\n", ret);
// 	__drawGraphChars((unsigned char*)szout, 0);

	__kCreateProcessFromAddrFunc(VSMAINDLL_LOAD_ADDRESS, 0x100000,  "__kExplorer", 3, 0);

	while (1)
	{
		if (__findProcessFuncName("__kExplorer") == FALSE)
		{
			__kCreateProcess(VSMAINDLL_LOAD_ADDRESS, 0x100000, "main.dll", "__kExplorer", 3, 0);
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
	__drawGraphChars((unsigned char*)szout, 0);

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

#include <stdio.h>

//注意二位数组在内存中的排列和结构
void mytest() {

	//runElfFunction("c:\\liunux\\test.so", "__testfunction");
	WORD tmpesp = 0x1234;

	__asm
	{
		push 0x87654321;

		_emit 0xff
		_emit 0x75
		_emit 0xfc

		push  tmpesp

		push 0x12345678

// 		//_emit 0x66
// 		_emit 0x0f
// 		_emit 0x1f
// 		_emit 0x44
// 		_emit 0x00
// 		_emit 0x00
	}
	FILE * fp = fopen("res/elftest/test.so", "rb");

	fseek(fp, 0, 2);

	int filesize = ftell(fp);
	fseek(fp, 0, 0);

	char * data = new char[filesize + 0x10000];

	int size = fread(data, 1, filesize, fp);

	fclose(fp);


	char * elfhdr = new char[0x100000];

	char * elf = elfhdr + (0x10000 - ((DWORD)elfhdr & 0xffff));

	int result = mapFile((DWORD)data,filesize, (DWORD)elf);

	initElf((DWORD)data, filesize, (DWORD)elf);

	realoc((DWORD)data,(DWORD)elf);

	typedef int(*ptrfunction)(int x, int y);
	DWORD addr = getSymAddrByName((DWORD)data, "__testfunction", (DWORD)elf);
	if (addr)
	{
		ptrfunction lpfunction = (ptrfunction)addr;
		result = lpfunction(6, 7);
		char szout[1024];
		//__sprintf(szout, "linux function result:%x\r\n", result);
	}

	//DWORD mainfunc= getSymAddrByName((unsigned int)data, "main", (DWORD)elf);

 	DWORD finalfunc = getFiniSection((unsigned int)data,(DWORD)elf);
	DWORD initarray = getInitSection((unsigned int)data, (DWORD)elf);

	DWORD initsec = getInitArraySection((unsigned long)data, (DWORD)elf);
	DWORD finalsec = getFiniArraySection((unsigned long)data, (DWORD)elf);

	DWORD init = getInit((unsigned long)data, (DWORD)elf);
	DWORD fini = getFini((unsigned long)data, (DWORD)elf);

	for (int i = 0;i<100;i ++)
	{
		size -= 1;
	}
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