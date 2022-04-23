#include "process.h"
#include "def.h"
#include "video.h"
#include "Utils.h"
#include "task.h"
#include "Pe.h"
#include "file.h"
#include "dosProcess.h"
#include "pevirtual.h"
#include "Kernel.h"
#include "memory.h"
#include "slab.h"
#include "page.h"
#include "ListEntry.h"
#include "window.h"
#include "elf.h"



void __kFreeProcess(int pid) {

	freeProcessMemory();

	//do not need to free stack esp 0,because it must be existed in head of 100M
	freeProcessPages();

	//clearcr3();

	//destroyWindows();
}


//1 先停止代码，然后释放内存，顺序不能反
//2 先停止其他线程，然后停止本线程，顺序不能反了
void __terminateProcess(int vpid, char * filename, char * funcname, DWORD lpparams) {

	int pid = vpid & 0x7fffffff;

	char szout[1024];

	int retvalue = 0;
	__asm {
		mov retvalue, eax
	}

	LPPROCESS_INFO tss = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	if (tss->pid != pid)
	{
		__printf(szout, "__terminateProcess pid:%x,filename:%s,funcname:%s,current pid:%x not equal\n", pid, filename, funcname, tss->pid);
		__drawGraphChars((unsigned char*)szout, 0);
	}
	else {
		__printf(szout, "__terminateProcess pid:%x,filename:%s,funcname:%s\n", pid, filename, funcname);
		__drawGraphChars((unsigned char*)szout, 0);
	}

	LPPROCESS_INFO process = 0;
	TASK_LIST_ENTRY * p = 0;
	do 
	{
		p = __findProcessByPid(pid);
		if (p == 0)
		{
			break;
		}
		else if ( p->process->pid != p->process->tid)
		{
			TASK_LIST_ENTRY * list = removeTaskList(p->process->tid);

		}
		else {
			process = p->process;
		}
	} while (p);

	__kFreeProcess(tss->pid);

	removeTaskList(process->pid);

	if (vpid & 0x80000000)
	{
		__sleep(0);
	}
	else {
		__sleep(-1);
	}
}



int __initProcess(LPPROCESS_INFO tss, int pid, DWORD filedata, char * filename, char * funcname,DWORD level, DWORD runparam) 
{
	int result = 0;

	char szout[1024];

	DWORD imagesize = getSizeOfImage((char*)filedata);
	DWORD alignsize = 0;
	tss->vaddr = USER_SPACE_START;
	tss->vasize = 0;
	DWORD vaddr = tss->vaddr + tss->vasize;
	DWORD pemap = (DWORD)__kProcessMalloc(imagesize,&alignsize,pid, vaddr);
	if (pemap <= 0) {
		tss->status = TASK_OVER;
		__printf(szout, "__initProcess %s %s __kProcessMalloc ERROR\n", funcname, filename);
		__drawGraphChars((unsigned char*)szout, 0);
		return FALSE;
	}
	tss->vasize += alignsize;

	tss->moduleaddr = pemap;
	tss->moduleLinearAddr = USER_SPACE_START;

	tss->tss.trap = 1;

	tss->tss.ldt = ((DWORD)glpLdt - (DWORD)glpGdt);

	// 	__printf(szout, "membase:%x,va size:%x,va:%x\n",pemap,tss->vasize,tss->va);
	// 	__drawGraphChars((unsigned char*)szout, 0);

	mapFile((char*)filedata, (char*)pemap);

	DWORD entry = 0;
	DWORD type = getType((DWORD)pemap);
	if (type & 0x2000)
	{
		//getAddrFromName 已经加了一个pemap，所以返回值必须减去它
		entry = getAddrFromName((DWORD)pemap, funcname);
		if (entry == FALSE) {
			__printf(szout, "__kCreateTask not found export function:%s in:%s\n", funcname, filename);
			__drawGraphChars((unsigned char*)szout, 0);

			__kFree(pemap);
			tss->status = TASK_OVER;
			return FALSE;
		}
		else {
			entry = entry - pemap + USER_SPACE_START;
		}
	}
	else {
		entry = getEntry((char*)pemap) + USER_SPACE_START;
	}
	tss->tss.eip = entry;

	relocTableV((char*)pemap, USER_SPACE_START);

	importTable((DWORD)pemap);

	setImageBaseV((char*)pemap, USER_SPACE_START);

	tss->tss.cr3 = initCr3((DWORD)pemap);
	if (tss->tss.cr3 == 0)
	{
		clearCr3((DWORD*)tss->tss.cr3);
		__kFreeProcess(pid);
		tss->status = TASK_OVER;
		return FALSE;
	}

	if (level & 3)
	{
		copyBackupTables(0, USER_SPACE_START, (DWORD*)tss->tss.cr3);
		//copyBackupTables(0, MEMMORY_HEAP_BASE, (DWORD*)tss->tss.cr3);
	}
	else {
		//copyBackupTables(0, MEMMORY_HEAP_BASE, (DWORD*)tss->tss.cr3);
		copyBackupTables(0, USER_SPACE_START, (DWORD*)tss->tss.cr3);
	}
	copyBackupTables(USER_SPACE_END, 0 - USER_SPACE_END, (DWORD*)tss->tss.cr3);

	DWORD syslevel = level & 3;
	tss->level = syslevel;

	DWORD eflags = 0x210;	//if = 1,et = 1
	if (syslevel)
	{
		eflags |= (syslevel<<12);	//iopl = 3
	}
	//eflags |= 0x4000;		//nt == 1
	tss->tss.eflags = eflags;

	tss->tss.eax = 0;
	tss->tss.ecx = 0;
	tss->tss.edx = 0;
	tss->tss.ebx = 0;
	tss->tss.esi = 0;
	tss->tss.edi = 0;

	//不用映射到cr3
	tss->tss.esp0 = TASKS_STACK0_BASE + (pid + 1) * TASK_STACK0_SIZE - STACK_TOP_DUMMY;
	tss->tss.ss0 = KERNEL_MODE_STACK;

	DWORD espsize = 0;
	vaddr = tss->vaddr + tss->vasize;
	LPTASKPARAMS params = 0;
	if (syslevel == 0)
	{
		tss->tss.ds = KERNEL_MODE_DATA;
		tss->tss.es = KERNEL_MODE_DATA;
		tss->tss.fs = KERNEL_MODE_DATA;
		tss->tss.gs = KERNEL_MODE_DATA;
		tss->tss.cs = KERNEL_MODE_CODE;
		tss->tss.ss = KERNEL_MODE_STACK;

		tss->espbase = __kProcessMalloc(KTASK_STACK_SIZE, &espsize,pid, vaddr);
		if (tss->espbase == FALSE)
		{
			__kFreeProcess(tss->pid);
			tss->status = TASK_OVER;
			return FALSE;
		}

		result = phy2linear(vaddr, tss->espbase, KTASK_STACK_SIZE, (DWORD*)tss->tss.cr3);
		if (result == FALSE)
		{
			__kFreeProcess(tss->pid);
			tss->status = TASK_OVER;
			return FALSE;
		}

		params = (LPTASKPARAMS)(tss->espbase + KTASK_STACK_SIZE  - STACK_TOP_DUMMY - sizeof(TASKPARAMS));
		tss->tss.esp = (DWORD)vaddr + KTASK_STACK_SIZE - STACK_TOP_DUMMY - sizeof(TASKPARAMS);
		tss->tss.ebp = (DWORD)vaddr + KTASK_STACK_SIZE - STACK_TOP_DUMMY - sizeof(TASKPARAMS);
	}
	else {
		tss->tss.ds = USER_MODE_DATA | syslevel | 4;
		tss->tss.es = USER_MODE_DATA | syslevel | 4;
		tss->tss.fs = USER_MODE_DATA | syslevel | 4;
		tss->tss.gs = USER_MODE_DATA | syslevel | 4;
		tss->tss.cs = USER_MODE_CODE | syslevel | 4;
		tss->tss.ss = USER_MODE_STACK | syslevel | 4;

		tss->espbase = __kProcessMalloc(UTASK_STACK_SIZE,&espsize,pid, vaddr);
		if (tss->espbase == FALSE)
		{
			__kFreeProcess(tss->pid);
			tss->status = TASK_OVER;
			return FALSE;
		}

		result = phy2linear(vaddr, tss->espbase, UTASK_STACK_SIZE, (DWORD*)tss->tss.cr3);
		if (result == FALSE)
		{
			__kFreeProcess(tss->pid);
			tss->status = TASK_OVER;
			return FALSE;
		}

		params = (LPTASKPARAMS)(tss->espbase + UTASK_STACK_SIZE - STACK_TOP_DUMMY - sizeof(TASKPARAMS));
		tss->tss.esp = (DWORD)vaddr + UTASK_STACK_SIZE - STACK_TOP_DUMMY - sizeof(TASKPARAMS);
		tss->tss.ebp = (DWORD)vaddr + UTASK_STACK_SIZE - STACK_TOP_DUMMY - sizeof(TASKPARAMS);
	}
	tss->vasize += espsize;

	params->terminate = (DWORD)__terminateProcess;
	params->terminate2 = (DWORD)__terminateProcess;
	params->tid = pid;
	__strcpy(params->szFileName, filename);
	params->filename = params->szFileName;
	__strcpy(params->szFuncName, funcname);
	params->funcname = params->szFuncName;
	params->lpcmdparams = &params->cmdparams;
	if (runparam)
	{
		__memcpy((char*)params->lpcmdparams, (char*)runparam, sizeof(TASKCMDPARAMS));
	}

	tss->counter = 0;
	tss->errorno = 0;

	tss->pid = pid;
	tss->tid = pid;
	__strcpy(tss->filename, filename);
	__strcpy(tss->funcname, funcname);

	tss->window = 0;

	LPPROCESS_INFO thistss = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	tss->ppid = thistss->pid;
	tss->sleep = 0;

// 	__printf(szout, "imagebase:%x,imagesize:%x,map base:%x,entry:%x,cr3:%x,esp:%x\n",
// 		getImageBase((char*)pemap), imagesize, pemap, entry, tss->tss.cr3,tss->espbase);
// 	__drawGraphChars((unsigned char*)szout, 0);

	addTaskList(tss->tid);

	return TRUE;
}


int __kCreateProcessFromName(char * filename, char * funcname, int syslevel, DWORD params) {
	int result = 0;
	char szout[1024];
	if (filename == 0 || funcname == 0)
	{
		__printf(szout, "__kCreateProcess filename or functionname is null\n");
		__drawGraphChars((unsigned char*)szout, 0);
		return FALSE;
	}

	int filesize = 0;
	char * filedata = 0;
	filesize = readFile(filename, (char**)&filedata);
	if (filesize <= 0)
	{
		__printf(szout, "__kCreateProcess readFileTo:%s error\n", filename);
		__drawGraphChars((unsigned char*)szout, 0);
		return FALSE;
	}

	result = __kCreateProcess((DWORD)filedata, filesize,filename,funcname, syslevel, params);

	__kFree((DWORD)filedata);

	return TRUE;
}


int __kCreateProcessFromAddrFunc(DWORD filedata, int filesize,char * funcname,int syslevel, DWORD params) {
 	char filename[1024];
// 	__getDateTimeStr(filename);

	__printf(filename, "process_%x", *(unsigned int*)TIMER0_TICK_COUNT);

	return __kCreateProcess(filedata, filesize, filename, funcname, syslevel, params);
}


int __kCreateProcess(DWORD filedata, int filesize,char * filename,char * funcname, int syslevel, DWORD params) {

	int ret = 0;
	char szout[1024];

	//同一个进程不论线性还是物理地址，都可以访问。
	//当一个进程把当前进程的线性地址转换为物理地址，传递给另一个进程，
	//进入另外一个的这个进程后，其访问这个物理地址的时候是要经过映射的
	//因此简单的想通过访问共同的物理地址实现共享是不行的

// 	DWORD filedata = linear2phy(lpfiledata);
// 	char * filename = (char *)linear2phy((DWORD)fn);
// 	char * funcname = (char *)linear2phy((DWORD)functionname);
// 	DWORD params = linear2phy(paramlist);

	int mode = syslevel & 0xfffffffc;
	DWORD level = syslevel & 3;

	TASKRESULT result;
	ret = __getFreeTask(&result);
	if (ret == FALSE)
	{
		__printf(szout, "__kCreateProcess filename:%s function:%s __getFreeTask error\n", filename, funcname);
		__drawGraphChars((unsigned char*)szout, 0);
		return FALSE;
	}

	if (mode & INFILE_DOS_PROCESS_FLAG)
	{
		ret = __initDosTss(result.lptss, result.number, filedata, filename, funcname, mode + (level | 3), params);
		return ret;
	}

	int petype = getPeType(filedata);
	if (petype == 1 || petype == 0)
	{
		if (filesize == 0)
		{
			return FALSE;
		}
		else {
			DWORD dosaddr = __initDosExe(filedata, filesize, result.number);
			if (dosaddr)
			{
				ret = __initDosTss(result.lptss, result.number, dosaddr, filename, funcname, 3, params);
				return ret;
			}
			else {
				__printf(szout, "__kCreateProcess __initDosTss:%s error\n", filename);
				__drawGraphChars((unsigned char*)szout, 0);
				return FALSE;
			}
		}
	}
	else if (petype == 2)
	{
		DWORD type = getType((DWORD)filedata);
		if (type & 0x2000)
		{
			if (funcname == 0)
			{
				__printf(szout, "__kCreateProcess run dll without function name\n");
				__drawGraphChars((unsigned char*)szout, 0);
				return FALSE;
			}
		}
		ret = __initProcess(result.lptss, result.number, filedata, filename, funcname, level, params);
		return ret;
	}
	else if (petype == 3)
	{
		return runElfFunction(filename, funcname);
	}
	else {
		ret = FALSE;
	}

	return ret;
}


