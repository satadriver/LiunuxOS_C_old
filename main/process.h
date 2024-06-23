#pragma once
#include "def.h"
#include "descriptor.h"
#include "page.h"
#include "malloc.h"
#include "video.h"

#pragma pack(push,1)


typedef struct
{
	unsigned char	opcode;
	DWORD			offset;
	unsigned short	selector;
}JUMP32, *LPJUMP32;


typedef struct __TSS{
	DWORD link; // ����ǰһ�� TSS ��ѡ���ӣ�ʹ�� call ָ���л��Ĵ�����ʱ����CPU��д��

	DWORD esp0; //4
	DWORD ss0;  //8
	DWORD esp1; //12
	DWORD ss1;  //16
	DWORD esp2; //20
	DWORD ss2;  //24
				
	DWORD cr3;	//28

				// ������Щ�����������л��Ĵ���ֵ�õģ��л��Ĵ�����ʱ����CPU�Զ���д��
	DWORD eip;	//32
	DWORD eflags;
	DWORD eax;	//40
	DWORD ecx;
	DWORD edx;	//48
	DWORD ebx;
	DWORD esp;	//56
	DWORD ebp;
	DWORD esi;	//64
	DWORD edi;
	DWORD es;	//72
	DWORD cs;
	DWORD ss;	//80
	DWORD ds;
	DWORD fs;	//88
	DWORD gs;

	//static
	DWORD ldt;	//96
	unsigned short	trap;				//100
	unsigned short	iomapOffset;		//102
	unsigned char	intMap[32];
	unsigned char	iomap[8192];
	unsigned char	iomapEnd;			//104 + 32 + 8192

	unsigned char	fpu;
	unsigned char	prior;
	unsigned char	unused;
} TSS,*LPTSS;

#pragma pack(pop)

#pragma pack(1)
typedef struct  
{
	TSS tss;

	DWORD pid;

	DWORD tid;

	DWORD ppid;

	DWORD level;

	DWORD sleep;

	//�����ַ���������Ե�ַ
	DWORD moduleaddr;
	DWORD moduleLinearAddr;

	DWORD espbase;

	DWORD vaddr;

	//�ڴ����������ַƫ��
	DWORD vasize;

	DWORD errorno;

	DWORD counter;

	DWORD status;

	LPWINDOWCLASS window;

	char filename[256];

	char funcname[64];

}PROCESS_INFO,*LPPROCESS_INFO;

#pragma pack()

#pragma pack(push,1)
typedef struct
{
	DWORD cmd;
	DWORD addr;
	DWORD filesize;
	char filename[256];
}TASKCMDPARAMS, *LPTASKCMDPARAMS;

typedef struct
{
	DWORD terminate;		//ret address
	DWORD terminate2;		//param 1
	DWORD tid;
	char * filename;
	char * funcname;
	LPTASKCMDPARAMS lpcmdparams;
	char szFileName[256];
	char szFuncName[64];

	TASKCMDPARAMS cmdparams;
}TASKPARAMS, *LPTASKPARAMS;

typedef struct
{
	DWORD terminate;
	DWORD pid;
	char * filename;
	char * funcname;
	DWORD addr;
	DWORD param;
	char szFileName[256];
	char szFuncName[64];
}TASKDOSPARAMS, *LPTASKDOSPARAMS;

typedef struct {
	int number;
	LPPROCESS_INFO lptss;
}TASKRESULT,*LPTASKRESULT;

#pragma pack(pop)

int __initProcess(LPPROCESS_INFO tss, int num, DWORD filedata, char * filename, char * funcname, DWORD level, DWORD runparam);



void __kFreeProcess(int pid);

#ifdef DLL_EXPORT
extern "C" __declspec(dllexport) void __terminateProcess(int pid, char * filename, char * funcname, DWORD lpparams);
extern "C" __declspec(dllexport)int __kCreateProcessFromAddrFunc(DWORD filedata, int filesize, char * funcname,int syslevel, DWORD params);
extern "C" __declspec(dllexport)int __kCreateProcessFromName(char * filename, char * funcname, int syslevel, DWORD params);
extern "C" __declspec(dllexport)int __kCreateProcess(DWORD addr, int datasize, char * filename, char * funcname, int syslevel, DWORD param);
#else
extern "C" __declspec(dllimport) void __terminateProcess(int pid, char * filename, char * funcname, DWORD lpparams);
extern "C" __declspec(dllimport)int __kCreateProcessFromAddrFunc(DWORD filedata, int filesize, char * funcname, int syslevel, DWORD params);
extern "C" __declspec(dllimport)int __kCreateProcessFromName(char * filename, char * funcname, int syslevel, DWORD params);
extern "C" __declspec(dllimport)int __kCreateProcess(DWORD addr, int datasize, char * filename, char * funcname, int syslevel, DWORD param);
#endif



