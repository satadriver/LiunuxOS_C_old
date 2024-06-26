
#include "video.h"
#include "Utils.h"
#include "exception.h"
#include "task.h"
#include "process.h"
#include "Pe.h"
#include "memory.h"
#include "Thread.h"

#define EXCEPTION_TIPS_COLOR 0X9F3F00

int gExceptionCounter = 0;

void __kException(DWORD param) {

	char showinfo[1024];

	DWORD no = *(DWORD*)param;

	DWORD rcr0 = 0;
	DWORD rcr2 = 0;
	DWORD rcr3 = 0;
	DWORD rcr4 = 0;
	
	__asm {
		mov eax, cr0
		mov rcr0, eax
		mov eax, cr2
		mov rcr2, eax
		mov eax, cr3
		mov rcr3, eax
		//mov eax, cr4
		__emit 0x0f
		__emit 0x20
		__emit 0xe0
		mov rcr4, eax	
	}


	LPPROCESS_INFO process = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;

	int tid = process->tid;
	int pid = process->pid;
	int level = process->level;

	DWORD module = process->moduleLinearAddr;

	if (no == 8 || no == 10 || no == 11 || no == 12 || no == 13 || no == 14 || no == 17)
	{
		LPEXCEPTIONCODESTACK tss = (LPEXCEPTIONCODESTACK)param;
		if (tss->eflags & 0x20000)
		{
			DWORD rva = tss->eip;

			__printf(showinfo,
				"v86 Exception type:%d,pid:%d,error code:%x,EIP RVA:%x,esp0:%x,ss0:%x,eip:%x,cs:%x,eflags:%x,esp3:%x,ss3:%x,"
				"ds:%x,es:%x,fs:%x,gs:%x,eax:%x,ecx:%x,edx:%x,ebx:%x,ebp:%x,esi:%x,edi:%x,cr0:%x,cr2:%x,cr3:%x,cr4:%x\n",
				tss->no, tid, tss->errcode, rva, tss->esp0, tss->ss0, tss->eip, tss->cs, tss->eflags, tss->esp3, tss->ss3,
				tss->ds, tss->es, tss->fs, tss->gs, tss->eax, tss->ecx, tss->edx, tss->ebx, tss->ebp, tss->esi, tss->edi,
				rcr0, rcr2, rcr3, rcr4);
		}
		else if (level & 3)
		{
			DWORD rva = rvaInFile(module, tss->eip );

			__printf(showinfo,
				"Exception type:%d,pid:%d,error code:%x,EIP RVA:%x,esp0:%x,ss0:%x,eip:%x,cs:%x,eflags:%x,esp3:%x,ss3:%x,"
				"ds:%x,es:%x,fs:%x,gs:%x,eax:%x,ecx:%x,edx:%x,ebx:%x,ebp:%x,esi:%x,edi:%x,cr0:%x,cr2:%x,cr3:%x,cr4:%x\n",
				tss->no,tid, tss->errcode,rva, tss->esp0, tss->ss0, tss->eip, tss->cs, tss->eflags, tss->esp3, tss->ss3,
				tss->ds, tss->es, tss->fs, tss->gs, tss->eax, tss->ecx, tss->edx, tss->ebx, tss->ebp, tss->esi, tss->edi,
				rcr0,rcr2,rcr3,rcr4);
		}
		else {
			DWORD rva = rvaInFile(module, tss->eip );

			__printf(showinfo,
				"Exception type:%d,pid:%d,error code:%x,EIP RVA:%x,esp0:%x,ss0:%x,eip:%x,cs:%x,eflags:%x,"
				"ds:%x,es:%x,fs:%x,gs:%x,eax:%x,ecx:%x,edx:%x,ebx:%x,ebp:%x,esi:%x,edi:%x,cr0:%x,cr2:%x,cr3:%x,cr4:%x\n",
				tss->no,tid, tss->errcode,rva, tss->esp0, tss->ss0, tss->eip, tss->cs, tss->eflags,
				tss->ds, tss->es, tss->fs, tss->gs, tss->eax, tss->ecx, tss->edx, tss->ebx, tss->ebp, tss->esi, tss->edi,
				rcr0, rcr2, rcr3, rcr4);
		}
	}
	else {
		LPEXCEPTIONSTACK tss = (LPEXCEPTIONSTACK)param;

		if (tss->eflags & 0x20000)
		{
			DWORD rva = tss->eip;

			__printf(showinfo,
				"v86 Exception type:%d,pid:%d,EIP RVA:%x,esp0:%x,ss0:%x,eip:%x,cs:%x,eflags:%x,esp3:%x,ss3:%x,"
				"ds:%x,es:%x,fs:%x,gs:%x,eax:%x,ecx:%x,edx:%x,ebx:%x,ebp:%x,esi:%x,edi:%x,cr0:%x,cr2:%x,cr3:%x,cr4:%x\n",
				tss->no, tid, rva, tss->esp0, tss->ss0, tss->eip, tss->cs, tss->eflags, tss->esp3, tss->ss3,
				tss->ds, tss->es, tss->fs, tss->gs, tss->eax, tss->ecx, tss->edx, tss->ebx, tss->ebp, tss->esi, tss->edi,
				rcr0, rcr2, rcr3, rcr4);
		}
		else if (level & 3)
		{

			DWORD rva = rvaInFile(module, tss->eip );

			__printf(showinfo,
				"Exception type:%d,pid:%d,EIP RVA:%x,esp0:%x,ss0:%x,eip:%x,cs:%x,eflags:%x,esp3:%x,ss3:%x,"
				"ds:%x,es:%x,fs:%x,gs:%x,eax:%x,ecx:%x,edx:%x,ebx:%x,ebp:%x,esi:%x,edi:%x,cr0:%x,cr2:%x,cr3:%x,cr4:%x\n",
				tss->no,tid, rva,tss->esp0, tss->ss0, tss->eip, tss->cs, tss->eflags, tss->esp3, tss->ss3,
				tss->ds, tss->es, tss->fs, tss->gs, tss->eax, tss->ecx, tss->edx, tss->ebx, tss->ebp, tss->esi, tss->edi,
				rcr0, rcr2, rcr3, rcr4);
		}
		else {
			DWORD rva = rvaInFile(module, tss->eip );

			__printf(showinfo,
				"Exception type:%d,pid:%d,EIP RVA:%x,esp0:%x,ss0:%x,eip:%x,cs:%x,eflags:%x,"
				"ds:%x,es:%x,fs:%x,gs:%x,eax:%x,ecx:%x,edx:%x,ebx:%x,ebp:%x,esi:%x,edi:%x,cr0:%x,cr2:%x,cr3:%x,cr4:%x\n",
				tss->no,tid,rva, tss->esp0, tss->ss0, tss->eip, tss->cs, tss->eflags,
				tss->ds, tss->es, tss->fs, tss->gs, tss->eax, tss->ecx, tss->edx, tss->ebx, tss->ebp, tss->esi, tss->edi,
				rcr0, rcr2, rcr3, rcr4);
		}
	}

	__logShow((unsigned char*)showinfo, EXCEPTION_TIPS_COLOR);

	gExceptionCounter++;
	//if (gExceptionCounter < 3)
	{
		TASK_LIST_ENTRY * list = __findProcessByTid(tid);
		LPPROCESS_INFO taskinfo = list->process;
		if (taskinfo)
		{
			__sprintf(showinfo, "task pid:%d,level:%d,status:%x,counter:%d,entry:%x,delay:%d,function:%s,file:%s\n",
				taskinfo->tid, taskinfo->level, taskinfo->status,taskinfo->counter, taskinfo->moduleaddr,0, taskinfo->funcname, taskinfo->filename);
			__logShow((unsigned char*)showinfo, EXCEPTION_TIPS_COLOR);
		}
	}

	if (tid == pid)
	{
		__terminateProcess(pid | 0x80000000, process->filename, process->funcname, 0);
	}
	else {
		__kTerminateThread(tid | 0x80000000, process->filename,process->funcname, 0);
	}

	if (level)
	{
		__sleep(0);
	}
	else {
		__asm {
			hlt
		}
	}
}