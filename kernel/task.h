#pragma once
#include "descriptor.h"
#include "process.h"

#pragma pack(1)

typedef struct {
	unsigned int addr;
	char name[256];
}DLLMODULEINFO,*LPDLLMODULEINFO;


typedef struct {
	LIST_ENTRY list;
	LPPROCESS_INFO process;
	DWORD valid;
}TASK_LIST_ENTRY;

typedef struct {
	DWORD ss;
	DWORD gs;
	DWORD fs;
	DWORD es;
	DWORD ds;
	DWORD edi;
	DWORD esi;
	DWORD ebp;
	DWORD esp;
	DWORD ebx;
	DWORD edx;
	DWORD ecx;
	DWORD eax;
	DWORD eip;
	DWORD cs;
	DWORD eflags;
	DWORD esp3;
	DWORD ss3;
	DWORD es_v86;
	DWORD ds_v86;
	DWORD fs_v86;
	DWORD gs_v86;
}LIGHT_ENVIRONMENT;

#pragma pack()

#define DOS_TASK_OVER			0X20000000
#define DOS_PROCESS_RUNCODE		0X80000000


#define TASK_STATUS_PTR			0X80000000
#define TASK_OVER				0
#define TASK_RUN				1
#define TASK_SUSPEND			2



void prepareTss(LPPROCESS_INFO tss);

void tasktest();

TASK_LIST_ENTRY* addTaskList(int tid);

TASK_LIST_ENTRY* removeTaskList(int tid);

void __terminateTask(int pid, char * pname, char * funcname, DWORD lpparams);

int __initTask();

int __getFreeTask(LPTASKRESULT);

#ifdef DLL_EXPORT

extern "C"  __declspec(dllexport) DWORD __kTaskSchedule(LIGHT_ENVIRONMENT*);

extern "C"  __declspec(dllexport) int __terminateTid(int tid);

extern "C"  __declspec(dllexport) int __terminatePid(int pid);

extern "C"  __declspec(dllexport) int __terminateByFileName(char * filename);

extern "C"  __declspec(dllexport) int __terminateByFuncName(char * funcname);


extern "C"  __declspec(dllexport) TASK_LIST_ENTRY*  __findProcessByTid(int tid);

extern "C"  __declspec(dllexport) TASK_LIST_ENTRY*  __findProcessByPid(int pid);

extern "C"  __declspec(dllexport) TASK_LIST_ENTRY*  __findProcessFileName(char * filename);

extern "C"  __declspec(dllexport) TASK_LIST_ENTRY*  __findProcessFuncName(char * funcname);

extern "C"  __declspec(dllexport) int __createDosInFileTask(DWORD addr, char * filename);

#else

extern "C"  __declspec(dllimport) DWORD __kTaskSchedule(LIGHT_ENVIRONMENT*);

extern "C"  __declspec(dllimport) int __terminateTid(int tid);

extern "C"  __declspec(dllimport) int __terminatePid(int pid);

extern "C"  __declspec(dllimport) int __terminateByFileName(char * filename);

extern "C"  __declspec(dllimport) int __terminateByFuncName(char * funcname);

extern "C"  __declspec(dllimport) TASK_LIST_ENTRY* __findProcessByTid(int tid);

extern "C"  __declspec(dllimport) TASK_LIST_ENTRY*  __findProcessByPid(int pid);

extern "C"  __declspec(dllimport) TASK_LIST_ENTRY*  __findProcessFileName(char * filename);

extern "C"  __declspec(dllimport) TASK_LIST_ENTRY*  __findProcessFuncName(char * funcname);

extern "C"  __declspec(dllimport) int __createDosInFileTask(DWORD addr, char * filename);
#endif



// data
//cpl <= dpl,rpl >= cpl
//stack
//cpl == dpl == rpl
//code 
//cpl <= dpl,rpl >= cpl
//int gate and trap gate
//cpl <= dpl, rpl in intgate >=  dpl that in code descriptor
//call gate
//cpl <= dpl, rpl in callgate >=  dpl that in code descriptor
//task gate
//cpl <= dpl, rpl in taskgate >=  dpl that in code descriptor

//CPL�ǵ�ǰ���̵�Ȩ�޼���(Current Privilege Level)���ǵ�ǰ����ִ�еĴ������ڵĶε���Ȩ����������cs�Ĵ����ĵ���λ
//RPL˵�����ǽ��̶Զη��ʵ�����Ȩ��(Request Privilege Level)���Ƕ��ڶ�ѡ���Ӷ��Եģ�ÿ����ѡ�������Լ���RPL��
//��˵�����ǽ��̶Զη��ʵ�����Ȩ�ޣ��е���������
//DPL�洢�ڶ��������У��涨���ʸöε�Ȩ�޼���(Descriptor Privilege Level)��ÿ���ε�DPL�̶�

//CPL <= DPL (��): ��ǰ���м����ܵ����ţ�������ⲿ�жϻ�CPU�쳣����ȥ��һ�ж�
//DPL0������: ����CPU�쳣��DPLΪ0���������û�ֱ̬��ʹ��intָ����ʣ�Ӳ���ж���ȥ��һ�жϣ���˿������û�����CPU�쳣
//�������INT nָ���INTOָ������ת�ƣ���Ҫ����ж��š������Ż��������������е�DPL�Ƿ�����CPL<=DPL(�����������쳣���жϣ����е�DPL�� ����)