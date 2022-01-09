#include "Thread.h"
#include "Utils.h"
#include "video.h"
#include "Pe.h"
#include "process.h"
#include "peVirtual.h"
#include "memory.h"


DWORD __kTerminateThread(int vtid, char * filename, char * funcname, DWORD lpparams) {

	int tid = vtid & 0x7fffffff;

	removeTaskList(tid);

	LPPROCESS_INFO tss = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	tss->status = TASK_OVER;

	char szout[1024];
	__printf(szout, "__kTerminateThread tid:%d,filename:%s,funcname:%s\n", tid, filename, funcname);
	__drawGraphChars((unsigned char*)szout, 0);

	if (vtid & 0x80000000)
	{
		__sleep(0);
	}
	else {
		__sleep(-1);
	}
	
	return 0;
}

DWORD __kCreateThread(DWORD addr, DWORD module, DWORD runparam,char * funcname) {

	int ret = 0;

	char szout[1024];

	TASKRESULT freetask;
	ret = __getFreeTask(&freetask);
	if (ret == FALSE)
	{
		return FALSE;
	}
	
	//创建线程中，如果想要修改父进程的信息，必须在CURRENT_TASK_TSS_BASE中修改，如果CURRENT_TASK_TSS_BASE没被修改，线程切换时信息还是会被替换
	LPPROCESS_INFO process = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;

	LPPROCESS_INFO tss = freetask.lptss;

	__memcpy((char*)tss, (char*)process, sizeof(PROCESS_INFO));

	tss->status = TASK_SUSPEND;

	//tss->tss.link = 0;
	
	tss->tss.eflags = process->tss.eflags;
	int level = process->level;
	if (level)
	{
		tss->tss.eflags = tss->tss.eflags | ((level &3)<<12);
	}

	tss->tss.eip = addr;
	tss->moduleaddr = linear2phy(module);
	tss->moduleLinearAddr = module;
	
	tss->tss.eax = 0;
	tss->tss.ecx = 0;
	tss->tss.edx = 0;
	tss->tss.ebx = 0;
	tss->tss.esi = 0;
	tss->tss.edi = 0;

	tss->tss.esp0 = TASKS_STACK0_BASE + (freetask.number + 1) * TASK_STACK0_SIZE - STACK_TOP_DUMMY;
	tss->tss.ss0 = KERNEL_MODE_STACK;

	DWORD espsize = 0;
	DWORD vaddr = tss->vaddr + tss->vasize;
	LPTASKPARAMS params = 0;
	if (tss->level == 0)
	{
		tss->tss.ds = KERNEL_MODE_DATA;
		tss->tss.es = KERNEL_MODE_DATA;
		tss->tss.fs = KERNEL_MODE_DATA;
		tss->tss.gs = KERNEL_MODE_DATA;
		tss->tss.cs = KERNEL_MODE_CODE;
		tss->tss.ss = KERNEL_MODE_STACK;

		tss->espbase = __kProcessMalloc(KTASK_STACK_SIZE, &espsize, process->pid,vaddr);
		if (tss->espbase == FALSE)
		{
			tss->status = TASK_OVER;
			return FALSE;
		}

		ret = phy2linear(vaddr, tss->espbase, KTASK_STACK_SIZE, (DWORD*)tss->tss.cr3);
		if (ret == FALSE)
		{
			__kFree(tss->espbase);
			tss->status = TASK_OVER;
			return FALSE;
		}

		params = (LPTASKPARAMS)(tss->espbase + KTASK_STACK_SIZE - STACK_TOP_DUMMY - sizeof(TASKPARAMS));
		tss->tss.esp = (DWORD)vaddr + KTASK_STACK_SIZE - STACK_TOP_DUMMY - sizeof(TASKPARAMS);
		tss->tss.ebp = (DWORD)vaddr + KTASK_STACK_SIZE - STACK_TOP_DUMMY - sizeof(TASKPARAMS);
	}
	else {
		tss->tss.ds = USER_MODE_DATA | tss->level;
		tss->tss.es = USER_MODE_DATA | tss->level;
		tss->tss.fs = USER_MODE_DATA | tss->level;
		tss->tss.gs = USER_MODE_DATA | tss->level;
		tss->tss.cs = USER_MODE_CODE | tss->level;
		tss->tss.ss = USER_MODE_STACK | tss->level;

		tss->espbase = __kProcessMalloc(UTASK_STACK_SIZE, &espsize,process->pid,vaddr);
		if (tss->espbase == FALSE)
		{
			tss->status = TASK_OVER;
			return FALSE;
		}

		ret = phy2linear(vaddr, tss->espbase, UTASK_STACK_SIZE, (DWORD*)tss->tss.cr3);
		if (ret == FALSE)
		{
			__kFree(tss->espbase);
			tss->status = TASK_OVER;
			return FALSE;
		}

		params = (LPTASKPARAMS)(tss->espbase + UTASK_STACK_SIZE - STACK_TOP_DUMMY - sizeof(TASKPARAMS));
		tss->tss.esp = (DWORD)vaddr + UTASK_STACK_SIZE - STACK_TOP_DUMMY - sizeof(TASKPARAMS);
		tss->tss.ebp = (DWORD)vaddr + UTASK_STACK_SIZE - STACK_TOP_DUMMY - sizeof(TASKPARAMS);
	}
	//还没有addTaskList,导致当前的tss中的vasize错误
	tss->vasize += espsize;

	params->terminate = (DWORD)__kTerminateThread;
	params->terminate2 = (DWORD)__kTerminateThread;
	params->tid = freetask.number;							//param1:pid
	__strcpy(params->szFileName, process->filename);
	params->filename = params->szFileName;		//param2:filename
	__strcpy(params->szFuncName, funcname);
	params->funcname = params->szFuncName;		//param2:filename
	params->lpcmdparams = &params->cmdparams;
	if (runparam)
	{
		__memcpy((char*)params->lpcmdparams, (char*)runparam, sizeof(TASKCMDPARAMS));
	}

	tss->pid = process->pid;

	tss->tid = freetask.number;

	tss->counter = 0;

	__strcpy(tss->filename, process->filename);
	__strcpy(tss->funcname, funcname);

	tss->window = 0;

	addTaskList(tss->tid);

	return TRUE;
}